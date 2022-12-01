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

/*CURRENT IDEAS:
 * Need to implement a queue. Add each line (command + args) to queue. Add exit function last if called.
 * ex: ls; cd; exit; echo test; = ls; cd; echo test; exit;
 * */

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
int ParseArgs(char lines[MAX][MAX]); //e.g., "ls -a -l" is converted to "ls" "-a" "-l"
void ExecuteCommands();

// Built-In-Functions
void MyCD(char *dir_input, int arg_count);
void MyExit();

void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line);
void PipeCommands(char *args[], char *first_command, int arg_count);

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
int ParseArgs(char lines[MAX][MAX])
{	
	for (int i = 0; i < lineIndex; i++) // for each line
        {
                char* currentLine = lines[i]; // get current line
                char* word = strtok(currentLine, " "); // get one word/item at a time.
                int currentLineWordCount = 0; // keep track of how many words/items in eaech line

                // get each word/item
                while (word != NULL) //while there are words in the current line, get
                {
                        lineWords[i][currentLineWordCount] = word; // store each word into lineWords
                        currentLineWordCount++; // increment currnet line word count
                        word = strtok(NULL, " ");
                }
                lineWordCount[i] = currentLineWordCount; //store currrent line word count into line word count array
        }

        return 0;
}


//Execute commands
void ExecuteCommands(){
                for (int i = 0; i < lineIndex; i++) //for each line
                { //start of for loop
                         // using parent child exec wait model to run commands like echo, ls, cat, etc
                        pid_t pid = fork();
                        // child process - if user runs a built-in-function, child process terminates and parent handles.
                        if(pid == 0)
                        {
                                // child terminates if built-in-function called
				if (strcmp (lineWords[i][0], "cd") == 0)
                                        kill(getpid(), SIGINT);
                                else if (strcmp (lineWords[i][0], "exit") == 0)
					MyExit();
                                // if not cd or exit, child process handles
                                else if ( execvp(lineWords[i][0], lineWords[i]) == -1 ) // if exec does not work correctly, issue error
                                        printf("%s command not found\n", lineWords[i][0]);
                        }
                        // parent process
                        else if (pid > 0)
                        {
                                wait(NULL);
                                // cd command
                                if (strcmp(lineWords[i][0], "cd") == 0) //if first word of line (command)
                                        MyCD(lineWords[i][1], lineWordCount[i]); //sending argument 1 (directory destination option)
                                //exit command
				else if (strcmp(lineWords[i][0], "exit") == 0) //if first word of line (command)
				       	MyExit();
                        }
                        // error case
                        else
                                perror("fork error\n");
                } //end of for loop
}


//Interactive Mode
void InteractiveMode()
{ // start of InteractiveMode function
	
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
	
	while (1)
	{ // start of while loop - iterate until exit function called
		// clear arrays and indexes with every iteration
		memset(lines, 0, MAX);
		lineIndex = 0;
		memset(lineWords, 0, sizeof(lineWords));
		memset(lineWordCount, 0, MAX);
		
		printf("%s ", prompt);
	
		// user input
		char input[MAX]; // Interactive Mode user input
		fgets(input, 512, stdin); // get input
		input[strlen(input) - 1] = '\0'; // remove last \n
		
		ParseCommands(input); // call ParseCommands
		ParseArgs(lines); // call ParseArgs
		ExecuteCommands();
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
	printf("Exiting program");
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

	ParseArgs(lines);
	ExecuteCommands();
}
