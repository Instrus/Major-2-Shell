#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>

#define MAX 512 // user's input is less than 512 bytes

//Created by Eric Tsuchiya (ert0068) and Brandon Tsuchiya (bat0162)
//MyCD and PipeCommands created by Eric Tsuchiya
//Exit and CommandRedirect created by Brandon Tsuchiya
//Only needed 1 built in function and 1 shell support function per person.

pid_t ppid; // gloabal parent id
pid_t cpid; // global child id

void InteractiveMode();
void BatchMode(char *file);

// Global variables
char lines[MAX][MAX]; // separated into lines
int lineIndex = 0; // for keeping track of number of lines
char* lineWords[MAX][MAX]; // separating lines into arrays of words/items
int lineWordCount[MAX]; // for keeping track of number of words/items
char IBuffer[1024]; // used for getcwd (in MyCD)
const char* homeDir; // used for current users home directory

int ParseCommands(char *userInput); //e.g., "ls -a -l; who; date;" is converted to "ls -al" "who" "date"
int ParseArgs(char* line); //e.g., "ls -a -l" is converted to "ls" "-a" "-l" -  WAS char lines[MAX][MAX]
void ExecuteCommands();

// Built-In-Functions
void MyCD(char *dir_input, int arg_count);
void MyExit();

void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line);
int PipeCommands(char* input); //was int //was char *args[], char *first_command, int arg_count

char* prompt;

int EXIT_CALLED = 0;//Functions seem to treat this as a global variable -DM


int main(int argc, char *argv[])
{    
	// Error checking on user's input
	 if (!(argc < 3))
	 {
		 fprintf(stderr, "Error: Too many parameters\n");
		 fprintf(stderr, "Usage: './output [filepath]'\n");
		 exit(0);//No memory needs to be cleared
	 }

	 // Get current users home directory
	if ((homeDir = getenv("HOME")) == NULL)
                       homeDir = getpwuid(getuid())->pw_dir; // get current users home directory

	// Determining to run Interactive or Batch mode
	if(argc == 1) InteractiveMode();
	else if(argc == 2) BatchMode(argv[1]);

	//gets the parent id and sets it to ppid
	ppid = getpid();

	return 0;
}


// ParseCommands used to parse user input during Interactive Mode using ";" as delimiter
int ParseCommands(char *userInput)
{
	char* line = strtok(userInput, ";"); //get line
	while (line != NULL)
	{
	strcpy( lines[lineIndex], line); // store each line into an array (STORE IN LINES)
	line = strtok(NULL, ";"); // get new line
	lineIndex++;
	}

	return 0;
}


// ParseArgs - acepts lines from either Interactive/Batch
int ParseArgs(char* line) // need a single line
{	
	char* word = strtok(line, " "); // get one word/item at a time.
	char* tempArray[MAX]; // hold each item. don't need more than 25 atm.
	int index = 0;

        // get each word/item
        while (word != NULL) //while there are words/items in line
        {
        	tempArray[index] = word;
                word = strtok(NULL, " "); //get another word.
		index += 1;
        }

		ExecuteCommands(tempArray, index); //passing index for size value (mostly for MyCD)
        return 0;
}


//Execute commands
void ExecuteCommands(char* elements[MAX], int size)
{
	// using parent child exec wait model to run commands like echo, ls, cat, etc
        pid_t pid = fork();
        // child process - if user runs a built-in-function, child process terminates and parent handles.  
	if (strcmp(elements[0], "exit") == 0)
		MyExit();
	
	if(pid == 0) 
        {
        	// child terminates if built-in-function called
		if (strcmp (elements[0], "cd") == 0) //if first element = cd
                	kill(getpid(), SIGINT);
                else if ( execvp(elements[0], elements) == -1 ){ //if does not work
			printf("%s command not found\n", elements[0]); //change to stderr
		}
	}
	//parent process
        else if (pid > 0)
        {
        	wait(NULL);
                
		// cd command
                if (strcmp(elements[0], "cd") == 0) //if first word of line (command) (THINK I NEED TO FIX MYCD)
                	MyCD(elements[1], size); //sending argument 1 (directory destination option)
	}
        //error case
        else
		perror("fork error\n");
	memset(elements, 0, MAX);
}



int PipeCommands(char* input)
{
	char* line = strtok(input, "|"); // separate each line
        char* lineArray[2]; // hold each line - only works for 2 lines at the moment
        int lineIndex = 0;

        // parse lines
        while (line != NULL) //while there are words/items in line
        {
        	lineArray[lineIndex] = line;
                line = strtok(NULL, "|"); //get another word.
                lineIndex += 1;
        }

	// parse items/words
	char* itemArray[2][MAX]; // item array to separate lines into commands and arguments
	int wordCount[2]; //keep track of how many words in each line
	for (int i = 0; i < 2; i++) //for each line (2 in this case)
	{
		char* item = strtok(lineArray[i], " "); // get one word/item at a time.
        	int wordIndex = 0; //keep track of how many words/items per line

        	// get each word/item
        	while (item != NULL) //while there are words/items in line
        	{
        		itemArray[i][wordIndex] = item; //store item
                	item = strtok(NULL, " "); //get another word.
                	wordIndex += 1;
        	}
		wordCount[i] = wordIndex; // store count for each line
	}

	//run each command
	
	int fd[2]; // file descriptors for pipe 1

	//create pipe
	if (pipe(fd) == -1)
		return 1;

	//fork 1
	int pid1 = fork();
	if (pid1 < 0)
		return 2;

	// child process 1
	if (pid1 == 0)
	{
		//reroute: 
		dup2(fd[1], STDOUT_FILENO); //dupes fd[1] (write end of pipe) into fd[2]
		close(fd[0]);
		execvp(itemArray[0][0], itemArray[0]); // run command 1
		close(fd[1]);
	}

	//fork 2
	int pid2 = fork();
	if (pid2 < 0)
		return 3;

	//child process 2
	if (pid2 == 0){
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);
		execvp(itemArray[1][0], itemArray[1]); // run command 2
	}

	close(fd[0]);
	close(fd[1]);
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
	return 0;
}


void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line){

}


//Interactive Mode
void InteractiveMode()
{ // start of InteractiveMode function
	
	//customize prompt
	char answer;
	char newPrompt[50];
	printf("Would you like to customize the prompt? [y/n]\n");
	answer = getchar();
	if (answer == 'y' || answer == 'Y')
	{
		do 
		{
			printf("Enter prompt (hint: no spaces): ");
			scanf("%s", newPrompt); //problem: scanf only gets first  word and ignores spaces...
			if (strchr(newPrompt, ' '))
				printf("Please do not use spaces\n");
			//getchar();
		} while (strchr (newPrompt, ' '));
		getchar();
		prompt = newPrompt;
	} else 
	{
		prompt = "Interactive Mode>";
		getchar(); // clear buffer
	}
	
	// Interactive Move starts here
	while (1)
	{ // start of while loop - iterate until exit function called

		memset(lines, 0, MAX); // clear lines
		lineIndex = 0; //clear lineIndex

		printf("%s ", prompt);
	
		// user input
		char input[MAX]; // Interactive Mode user input
		fgets(input, 512, stdin); // get input
		input[strlen(input) - 1] = '\0'; // remove last \n
		
		ParseCommands(input); // call ParseCommands

		//check to see if we have lines that contain | or not.
		for (int i = 0; i < lineIndex; i++){ // for every line:
			if(strchr(lines[i], '|'))
				PipeCommands(lines[i]);
			else 
				ParseArgs(lines[i]);
		}

		//clear input
		//memset(input, 0, MAX);

	}// end of while loop
} // end of InteractiveMode function


// MyCD function - handles changing directory - IMPLEMENTED BY ERIC TSUCHIYA
void MyCD(char *dir_input, int arg_count)
{
	// return if more than one option. (cd call counts as argument one, extra option is two)
	if (arg_count > 2){
		printf("Error: too many arguments\n");
		return;
	}
	// If no arguments/options, go current users home.
	if (arg_count == 1){
		chdir(homeDir);
		return;
	}
	// If user included one option (destination), if directory change was successful, display current location
	if (chdir(dir_input) == 0){
		getcwd(IBuffer, 1024);
		printf("In directory: %s\n", IBuffer);
		return;
	} 
	else {
		fprintf(stderr, "Error: directory not found!\n");
		return;
	}
}


//simple exit function - IMPLEMENTED BY BRANDON TSUCHIYA
void MyExit()
{
	printf("Exiting program\n");
	kill(ppid, SIGINT); // Forced kill parent process because during testing, the more commands called, the less likely for exit function to properly close.
	exit(0);
}



// Batch Mode
void BatchMode(char *file)
{
	// open file
	FILE *fptr = fopen(file, "r");
	// batchInput for gathering lines of text from file
	char batchInput[MAX];
	
	//error checking for fopen function
    	if(fptr == NULL)
	{
		fprintf(stderr, "Error: Batch file not found or cannot be opened\n");
		MyExit();
	}

	// run while loop until end of file reached
        while(!feof(fptr))
	{
		// if fgets receives a NULL value, break from loop
                if (fgets(batchInput, MAX, fptr) == NULL){ //atempt to store line in batchInput
                        //printf("NULL at index %d\n", lineIndex);
                        break;
                }
		batchInput[strlen(batchInput) - 1] = '\0'; // remove last \n
		strcpy(lines[lineIndex], batchInput);
                lineIndex++;
        }

	// close file after all input gathered
	fclose(fptr);

	// Print each line
        for (int i = 0; i < lineIndex; i++)
                printf("Line %d: %s\n", i, lines[i]);

	// run commands
	for (int i = 0; i < lineIndex; i++)
	{ // for every line:
        	if(strchr(lines[i], '|'))
                	PipeCommands(lines[i]);
                else
                	ParseArgs(lines[i]); //send current line to ParseArgs and then ran.
       }

} 
