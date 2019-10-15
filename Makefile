# Compile an executable named sshell from sshell.c
all: sshell.c
	gcc -Wall -Werror -o sshell sshell.c
clean:
	rm sshell
