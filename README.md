# Major-2-Shell
Team Members: Eric Tsuchiya (ert0068), Brandon Tsuchiya (bat1062).

This is for assignment Major 2. We have completed one built in function and one shell support function per person. We have a total of two built in functions and two support functions as our group only consists of two people. 

Eric Tsuchiya created MyCD and pipeline functions.
Brandon Tsuchiya created the exit and redirection functions.
Eric and Brandon Tsuchiya both contributed to all other functions collectively and equally responsibly. 

A quick explanation on how these functions work:

MyCD - simply call "cd" to change directory while the shell is running. If user does not include an optional argument, cd will default to the users home directory. User may also include which directory they wish to travel to.

Exit function - simply calling "exit" will exit the program. If called at any point, will shut down the program as soon as it runs. Currently working on making it that if exit is called at any point, it would be the last function to execute.

Pipeline - user can include pipeline in a command call like "cmd1 | cmd2". Currently working on making the function accept 3 commands and two pipes, but only works for one pipe at the moment.

Redirection - allows user to call for redirection to change output to a file. Currently only works for output files at the moment. Working on implementing input files.

Program is ready for user testing.
