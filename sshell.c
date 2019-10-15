#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

// Constants
const char* WHITESPACE = " \t\n\v\f\r";
size_t BUF_SIZE = 513;

// Global variables
char* cwd;
int iteration_i = 0;
int background_process_pid = -1;

// Structure that represents Command
struct Command {
	// argv[0] is program name
	// argv[1+] are parameters
	char** argv;

	// File that will redirec to stdin
	char* filenameIn;

	// File where stdout result will be writen to
	char* filenameOut;
};
typedef struct Command Command;

// Function declaration to parse command from string
Command* parseCommand(char* str);

// Function declaration for handling built-in commands
int handleBuiltInCommandsParseLevel(char** argv, char* original_cmd);
int handleBuiltInCommandsRuntimeLevel(char** argv);

// Function declaration to execute command
int executeCommand(Command* cmd);

// Structure that represents Pipeline
struct Pipeline {
	// Original input string (needed when showing result message)
	char* str;

	// Array of commands
	Command** commands;

	// Size of array
	int command_count;

	// Whether pipeline is background
	char isBackground;
};
typedef struct Pipeline Pipeline;

// Function declaration to parse pipeline from string
Pipeline* parsePipeline(char* str);

// Function declarations to execute pipeline
void executePipeline(Pipeline* pipeline);
int executeCommandsFromPipeline(Pipeline* pipeline);

// Show prompt and read line from stdin
void readInput(char* cmd);

int main()
{
	// Initialize buffers
	char* cmd = (char *)malloc(BUF_SIZE * sizeof(char));
	cwd = (char *)malloc(BUF_SIZE * sizeof(char));

	getcwd(cwd, BUF_SIZE);

	while(1) {
		readInput(cmd);

		// Parse pipeline from string
		Pipeline* pipeline = parsePipeline(cmd);

		if(!pipeline) {
			// Ignore empty input and parse errors
			continue;
		}

		executePipeline(pipeline);

		// Needed for unique temporary file name
		iteration_i++;
	}

	return EXIT_SUCCESS;
}

void readInput(char* cmd) {
	// Print prompt
	printf("sshell$ ");
	fflush(stdout);

	// Read line
	getline(&cmd, &BUF_SIZE, stdin);

	/* Print command line if we're not getting stdin from the
		 * terminal */
	if (!isatty(STDIN_FILENO)) {
		printf("%s", cmd);
		fflush(stdout);
	}

	// Remove newline character (last character) from input
	cmd[strlen(cmd) - 1] = '\0';
}

Pipeline* parsePipeline(char* str) {
	// Next string operations are mutable,
	// so we need to store the copy of original command
	char* str_original = (char *)malloc(BUF_SIZE * sizeof(char));
	strcpy(str_original, str);

	// Trim whitespaces
	int length = strlen(str);
	while(isspace(str[length - 1])) {
		length--;
	}

	// Parse background mode
	char isBackground = 0;
	if(str[length - 1] == '&') {
		length--;
		str[length] = '\0';
		isBackground = 1;
	}

	// If command still contains &, then show error and restart
	if(strstr(str, "&")) {
		puts("Error: mislocated background sign");
		return 0;
	}

	// Initialize variables
	char** cmd_strs = malloc(BUF_SIZE*sizeof(char*));
	Command** commands = malloc(BUF_SIZE*sizeof(Command*));
	int command_count = 0;

	// Read pipeline parts as strings into cmd_strs
	char* cmd_str = strtok(str, "|");
	while (cmd_str != NULL)
	{
		cmd_strs[command_count] = cmd_str;
		cmd_str = strtok(NULL, "|");
		command_count++;
	}

	if(command_count == 0) {
		// On empty input
		if(isBackground) {
			// Goes here if "&" was inputed
			puts("Error: invalid command line");
		}
		return 0;
	}

	int i = 0;
	for(i = 0; i < command_count; i++) {
		// Parse commands
		commands[i] = parseCommand(cmd_strs[i]);
		if(!commands[i]) {
			// If error happened during command parse
			free(cmd_strs);
			free(commands);
			return 0;
		}
	}

	// Create and return pipeline object
	Pipeline* pipeline = (Pipeline*)malloc(sizeof(Pipeline));
	pipeline->str = str_original;
	pipeline->commands = commands;
	pipeline->isBackground = isBackground;
	pipeline->command_count = command_count;

	return pipeline;
}

Command* parseCommand(char* str) {
	// Next string operations are mutable,
	// so we need to store the copy of original command
	char* str_original = (char*)malloc(BUF_SIZE * sizeof(str));
	strcpy(str_original, str);

	// Parse input file
	char* fileIn = NULL;
	if(strstr(str, "<")) {
		char* str_ = strtok(str, "<");
		fileIn = strtok(NULL, "<");

		if(fileIn == NULL) {
			fprintf(stderr, "Error: no input file\n");
			return 0;
		}

		str = str_;
	}

	// Parse output filename if any
	char* fileOut = NULL;
	if(strstr(str, ">")) {
		char* str_ = strtok(str, ">");
		fileOut = strtok(NULL, ">");

		if(fileOut == NULL) {
			fprintf(stderr, "Error: no output file\n");
			return 0;
		}

		str = str_;
	}

	// Parse command name and arguments
	// Command name is argv[0]
	// argv[1+] are arguments

	char** argv = malloc(BUF_SIZE * sizeof(char*));
	int argv_i = 0;

	char* token = strtok(str, WHITESPACE);
	while (token != NULL)
	{
		argv[argv_i] = token;
		token = strtok(NULL, WHITESPACE);
		argv_i++;
	}

	if(argv_i == 0) {
		free(argv);
		return 0;
	}

	// Handle exit and cd commands

	if(handleBuiltInCommandsParseLevel(argv, str_original)) {
		free(argv);
		return 0;
	}

	// Create and return Command object

	Command *cmd = calloc(sizeof(Command), 1);
	cmd->argv = argv;
	cmd->filenameIn = fileIn;
	cmd->filenameOut = fileOut;

	return cmd;
}

int handleBuiltInCommandsParseLevel(char** argv, char* str_original) {
	if (strcmp(argv[0], "exit") == 0) {
		if(getpgid(background_process_pid) >= 0) {
			puts("Error: active jobs still running");
			return 1;
		} else
		{
			fprintf(stderr, "Bye...\n");
			exit(EXIT_SUCCESS);
			// Code does not reach here due to exit
			return 1;
		}
	} else if(strcmp(argv[0], "cd") == 0) {
		int retval = chdir(argv[1]);
		if(retval != 0) {
			perror("Error");
		}
		//printf("%s/%s\n",cwd, argv[1]);
		fprintf(stderr,"+ completed '%s' [%d]\n", str_original, retval);
		getcwd(cwd, BUF_SIZE);
		return 1;
	}
	return 0;
}

// Run-time built-in commands (pwd) are handled here
// returns 1 if built-in command was recognized and executed
// 0 otherwise
int handleBuiltInCommandsRuntimeLevel(char** argv) {
	if(strcmp(argv[0], "pwd") == 0) {
		puts(cwd);
		return 1;
	}
	return 0;
}

// Execute pipeline
void executePipeline(Pipeline* pipeline) {
	if(pipeline->isBackground) {
		// Execute commands in new process
		background_process_pid = fork();
		if(background_process_pid == 0) {
			executeCommandsFromPipeline(pipeline);
			_exit(EXIT_SUCCESS);
		}
	} else {
		// Execute commands in current process
		executeCommandsFromPipeline(pipeline);
	}
}

// Execute commands from pipeline
int executeCommandsFromPipeline(Pipeline* pipeline) {
	int retval;
	if(pipeline->command_count == 1) {
		// If pipeline has only one command
		// Then there is not need to link anything
		retval = executeCommand(pipeline->commands[0]);
	} else {
		// Connecting commands from pipeline with temporary files
		// cmd1 -> tmpFile1 -> cmd2 -> tmpFile2 -> cmd3 ...
		int i;
		for(i = 1; i < pipeline->command_count; i++) {
			if(pipeline->commands[i]->filenameIn != 0) {
				fprintf(stderr, "Error: mislocated input redirection\n");
				return 1;
			}
		}
		for(i = 0; i < pipeline->command_count - 1; i++) {
			if(pipeline->commands[i]->filenameOut != 0) {
				fprintf(stderr, "Error: mislocated output redirection\n");
				return 1;
			}
			// Create temporary file, whose name is based on number of iteration, so that
			// temporary files from different pipelines won't conflict with each other
			char* tmpFilename = (char*)calloc(BUF_SIZE, sizeof(char));
			strcat(tmpFilename, "tmp");
			sprintf(tmpFilename + 3, "%d", iteration_i + 100 * i);
			pipeline->commands[i]->filenameOut = tmpFilename;
			pipeline->commands[i + 1]->filenameIn = tmpFilename;
		}


		for(i = 0; i < pipeline->command_count; i++) {
			retval = executeCommand(pipeline->commands[i]);
			if(retval != EXIT_SUCCESS) {
				break;
			}
		}

		for(i = 1; i < pipeline->command_count; i++) {
			// Delete tmp file
			remove(pipeline->commands[i]->filenameIn);
			// Free tmp filename memory
			free(pipeline->commands[i]->filenameIn);
		}

	}

	// Show result status message
	fprintf(stderr, "+ completed '%s' [%d]\n", pipeline->str, retval);
	return retval;
}

void str_strip(char** str_pointer);

int executeCommand(Command* command) {
	int retval;

	// Try to recognize and execute built-in command
	if(handleBuiltInCommandsRuntimeLevel(command->argv)) {
		// If built-in command was recognized and executed, exit
		return 0;
	}

	if (fork() == 0){
	    // CHILD PROCESS
	    // Executing command
	    int fdIn, fdOut;
		if(command->filenameIn) {
			str_strip(&(command->filenameIn));
			fdIn = open(command->filenameIn, O_RDONLY);
			if(fdIn <= 0) {
				perror("Error");
	    		_exit(EXIT_FAILURE);
			}
			dup2(fdIn, 0);
		}
		if(command->filenameOut) {
			str_strip(&(command->filenameOut));
			if(command->filenameOut == 0 || strlen(command->filenameOut) == 0) {
				fprintf(stderr, "Error: no output file\n");
				_exit(EXIT_FAILURE);
			}
			fdOut = open(command->filenameOut, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			if(fdOut <= 0) {
				perror("Error");
	    		_exit(EXIT_FAILURE);
			}
			dup2(fdOut, 1);
		}

		// Execute command and close CHILD PROCESS
		execvp(command->argv[0], command->argv);

	    // Will get here only if error happened
	    perror("Error");
	    _exit(EXIT_FAILURE);
	} else {
	    // PARENT PROCESS
	    wait(&retval);
	    return retval;
	}
}

// Clear starting and trailing spaces
void str_strip(char** str_pointer) {
	while(isspace((*str_pointer)[0]) && (*str_pointer)[0] != '\0') {
		*str_pointer = (*str_pointer) + 1;
	}
	int length = strlen(*str_pointer);
	while(length > 0 && isspace((*str_pointer)[length - 1])) {
		(*str_pointer)[length - 1] = '\0';
		length--;
	}
 }
