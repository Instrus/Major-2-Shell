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

pid_t ppid; // gloabal parent id
pid_t cpid; // global child id

void InteractiveMode();
void BatchMode(char *file);

int ParseCommands(char *userInput); //e.g., "ls -a -l; who; date;" is converted to "ls -al" "who" "date"
int ParseArgs(); //e.g., "ls -a -l" is converted to "ls" "-a" "-l"
void ExecuteCommands(char *command, char *full_line);
// Parsing variables
// ParseCommands
char* lines[MAX]; //store lines of user input
int lineIndex = 0; // keep track of how many lines there are
//ParseArgs
char* lineWords[MAX][MAX]; // words array of each line THIS IS WHERE ARRAYS OF WORDS ARE STORED (first [] is location of each array, second holds the words)
int lineWordCount[MAX]; //how many words in each line

// Bukt-in functions
void MyCD(char *dir_input, int arg_count);
void MyExit();
// Built-in-function variables
char IBuffer[1024]; //used for testing - remove later.
const char* homeDir; //current users home directory

void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line);
void PipeCommands(char *args[], char *first_command, int arg_count);
void signalHandle(int sig);

char CURRENT_DIRECTORY[MAX]; //current directory
char *COMMANDS[MAX]; //commands to be executed
char *MYPATH; //my PATH variable
const char *ORIG_PATH_VAR; //The original PATH contents
char *prompt;

int EXIT_CALLED = 0;//Functions seem to treat this as a global variable -DM


int main(int argc, char *argv[])
{    
	//error checking on user's input
	 if (!(argc < 3))
	 {
		 fprintf(stderr, "Error: Too many parameters\n");
		 fprintf(stderr, "Usage: './output [filepath]'\n");
		 exit(0);//No memory needs to be cleared
	 }

	if ((homeDir = getenv("HOME")) == NULL)
                       homeDir = getpwuid(getuid())->pw_dir; // get current users home directory

	//initialize your shell's enviroment
	MYPATH = (char*) malloc(1024);
	memset(MYPATH, '\0', sizeof(MYPATH));
	ORIG_PATH_VAR = getenv("PATH"); // needs to include <stdlib.h>

	//save the original PATH, which is recovered on exit
	//strcpy(MYPATH, ORIG_PATH_VAR);

	//make my own PATH, namely MYPATH
	//setenv("MYPATH", MYPATH, 1);

	if(argc == 1) InteractiveMode();
	else if(argc == 2) BatchMode(argv[1]);

	//gets the parent id and sets it to ppid
	ppid = getpid();

	//handles the signal (Ctrl + C)
	//signal(SIGINT, signalHandle);

	//handles the signal (Ctrl + Z)
	//signal(SIGTSTP, signalHandle);

	//free all variables initialized by malloc()
	//free(MYPATH);

	return 0;
}

//Separate lines.
//e.g., "ls -a -l; who; date;" is converted to "ls -al" "who" "date"
int ParseCommands(char *userInput){

	char* line = strtok(userInput, ";"); //get line
	while (line != NULL)
	{
	lines[lineIndex] = line; //store lines into line array
	line = strtok(NULL, ";"); //get new lines
	lineIndex++; //increment line index
	}
	lines[lineIndex] = NULL;
	return 1;
}


//e.g., "ls -a -l" is converted to "ls" "-a" "-l"
int ParseArgs(){ //receive line and arguments.

	for (int i = 0; i < lineIndex; i++) //increments through each line
	{
		char* currentLine = lines[i]; //get current line (temp var)
		char* word = strtok(currentLine, " ");; // get first word of current line
		int currentLineWordCount = 0; // keep track of how many words in each line

		//get each word
		while (word != NULL) //while there are words in the current line, get
        	{
        		lineWords[i][currentLineWordCount] = word; //store each word into lineWords (Store each word at per line at word count)
			currentLineWordCount++; //inc currnet line word count
        		word = strtok(NULL, " "); //get another word
        	}
		lineWordCount[i] = currentLineWordCount; //store currrent line word count into line word count array
	}

        lineWords[lineIndex][0] = NULL; // Might not be optimized
	return 1;

}


void ExecuteCommands(char *command, char *full_line){
}


void InteractiveMode()
{ //start of InteractiveMode
	while (1)
	{ // start of while loop

		//every iteration clear arrays and indexes
		memset(lines, 0, MAX);
		lineIndex = 0;
		memset(lineWords, 0, sizeof(lineWords));
		memset(lineWordCount, 0, MAX);
		
		printf("Interactive> ");
		
		// User input
		char input[MAX]; // general user input
		fgets(input, 512, stdin); // get input
		input[strlen(input) - 1] = '\0';
		
		ParseCommands(input);
		ParseArgs();
		
		//print each line
        	for (int i = 0; i < lineIndex; i++) // for each line
		{
                	printf("Line %d: ", i);

        		for (int x = 0; x < lineWordCount[i]; x++)
                		printf("%s ", lineWords[i][x] );
                	printf("\n");
        	}
		
		//for loop to get command and argument
		for (int i = 0; i < lineIndex; i++) //for each line
		{ //start of for loop

			//fork - lesser functions handeled by child and exec, built-in-functions handeled by parent
			pid_t pid = fork();
			
			//using child process for optional commands like ls and echo
			if(pid == 0)
			{
				// if running a built in command, kill child process and run in parent.
				if (strcmp (lineWords[i][0], "cd") == 0) //check current lines element one (lineWords[][])
					kill(getpid(), SIGINT);
				if (strcmp (lineWords[i][0], "exit") == 0)
					kill(getpid(), SIGINT);
				// If simple command, use exec to run
				else if ( execvp(lineWords[i][0], lineWords[i]) == -1 ) //will probably give me an issue for last lineWord
					printf("%s command not found\n", lineWords[i][0]);
			}
			else if (pid > 0)
			{
				wait(NULL);	
				// cd command
				if (strcmp(lineWords[i][0], "cd") == 0)
                                	MyCD(lineWords[i][1], lineWordCount[i]); //sending argument 1
				//exit command
			}
			else
				perror("fork error\n");

		} //end of for loop
	} // end of while loop
} // end of InteractiveMode


void MyCD(char *dir_input, int arg_count){
	//using (command + arguments) for arg_count. Actual arg count = (arg_coun - 1)

	// If more than one option, return
	if (arg_count > 2){
		printf("Error: too many arguments\n");
		return;
	}
	// If no arguments/options, go current users home.
	if (arg_count == 1){
		chdir(homeDir);
		return;
	}
	if (chdir(dir_input) == 0){ //if chdir is successful
		getcwd(IBuffer, 1024);
		printf("In directory: %s\n", IBuffer);
		return;
	} 
	else {
		printf("Error: directory not found\n");
		return;
	}
}


void MyExit(){

}


void BatchMode(char *file){

	FILE *fptr = fopen(file, "r");
    //error checking for fopen function
    if(fptr == NULL) {
		fprintf(stderr, "Error: Batch file not found or cannot be opened\n");
		//MyExit();
    }

    char *batch_command_line = (char *)malloc(MAX);
    memset(batch_command_line, '\0', sizeof(batch_command_line));

    //reads a line from fptr, stores it into batch_command_line
    while(fgets(batch_command_line, MAX, fptr)){
	//remove trailing newline
	batch_command_line[strcspn(batch_command_line, "\n")] = 0;
//... (256 lines left)
    }
}
