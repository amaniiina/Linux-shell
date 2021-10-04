all: ex2.c
	gcc shell.c -o shell
all-GDB: ex2.c
	gcc -g shell.c -o shell