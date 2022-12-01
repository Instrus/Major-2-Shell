CC := gcc

# source files list
SRC = main.c

# object file list
OBJ := main.o

# default: compile program
default: $(OBJ)
	$(CC) -o  newshell $(OBJ)

# object file compilation
%.o: %.c
	$(CC) -c $^

# clean directory
clean:
	rm $(OBJ)
	rm newshell
