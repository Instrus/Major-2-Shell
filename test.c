#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){

	//user input
	char userInput[512] = "test one; test two; test three"; //get user input
	printf("Before: %s\n", userInput);

	//separating lines
	char* line = strtok(userInput, ";"); //get each line
	char* lines[512]; // array of lines
	int lineIndex = 0; // keep track of each line

        while (line != NULL) //while there are more lines
        {
        lines[lineIndex] = line; //store each line into array
        line = strtok(NULL, ";"); //get a new line
        lineIndex++; //incremenet lineIndex
        }
	lines[lineIndex] = NULL; //store NULL in last.
	
	printf("results\n"); //print results (lines)
	for (int i = 0; i < lineIndex; i++){
		printf("%s\n", lines[i]);
	}


	// separate lines into words. 
	printf("\nSeparating into words\n");

	char* lineWords[512][512]; // words array of each line THIS IS WHERE ARRAYS OF WORDS ARE STORED (first [] is location of each array, second holds the words)
	int lineWordCount[512]; //how many words in each line

	//start at line 0, iterate through each one.
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

        //lineWords[someindex] = NULL; //FIX THIs

	//-------------------------------------------------
	
	//print every word of every line
	printf("Printing each word in each line\n");

	for (int i = 0; i < lineIndex; i++){ //for each line
		printf("Line %d: ", i);
	
	//probably need to have an array of how many words are in each line...
	for (int x = 0; x < lineWordCount[i]; x++){
		printf("%s ", lineWords[i][x] );
	}
		printf("\n");	
	}


}
