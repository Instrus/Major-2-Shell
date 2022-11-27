// Eric Tsuchiya - EUID: ert0068
// CSCE 3600.002
// November 11, 2022
// This is for the minor3 assignment - it takes user input and runs a child process depending on what the user enters.

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main()
{ // main START

	while (1) 
	{ //indefinite loop / start of while loop

		char input [500]; //for getting user input
		printf("minor3> ");
		fgets(input, 500, stdin); //get input
		input[strlen(input) - 1] = '\0'; //remove newline.

		//exit program if user enters "quit"
		if (strcmp (input, "quit") == 0) 
			{exit(0);}

                int i = 0; //index for word split
                char *word = strtok(input, " "); //split words using strktok (space as delimiter)
                char* split[500];

                while (word != NULL)
                {
                        split[i] = word;
                        word = strtok(NULL, " ");
                      	//printf("%s\n", split[i]);
                        i++;
                }
		split[i] = NULL; //add NULL at the end


		//if user enters any of these values, print command not found
		if (strcmp(split[0], "cd") == 0 || strcmp(split[0], "history") == 0 || strcmp(split[0], "exit") == 0)
		{
			printf("%s, command not found\n", input);
			continue;
		}

		//create child process
		pid_t pid = fork();

		if(pid == 0)
		{
			if ( execvp(split[0], split) == -1 ) //execvp - includes both path and an array for arguments
			{
			printf("%s, command not found\n", input); //if unsuccessful, print "command not found"
			}
			exit(0);
		}
		else if (pid > 0)
		{
		wait(NULL); //wait for child process
		}
		else
		{
			perror("fork error\n");
		}

	}//end of while loop

	return 0;

} // main END

