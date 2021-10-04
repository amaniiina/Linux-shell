
Program description:

    This program gets from the user a command and sends it to the system in order to execute it, it is basically a command prompt or terminal.
    It supports pipe and redirection of four types: > , >> , < , 2>. It also supports a combination of a pipe followed by one of three redirection
    types: > , >> , 2>.


Program files: 
shell.c - source code
shell - run file
makefile - to compile the program


Compile and running: 
In Linux terminal:
    To compile: gcc shell.c -o shell
    To run: ./shell
In Visual Studio Code:
    To compile: Ctrl + shift + b
    To run: Ctrl + f5


Input: 
shell - desired commands with arguments.

Output:
shell - depends on the inputted command, type done to quit.

