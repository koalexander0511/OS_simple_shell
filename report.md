# Data structures
```c
struct Command {
	// argv[0] is program name
	// argv[1+] are parameters
	char** argv;

	// File that will redirec to stdin
	char* filenameIn;

	// File where stdout result will be writen to
	char* filenameOut;
};
```

```c
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
```


# Highest level design

```c
char* cwd;
int iteration_i = 0;

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

		// Needed for unique temporary file name (used for `stdout->stdin` transfer in pipelines)
		iteration_i++;
	}

	return EXIT_SUCCESS;
}
```

# Parsing pipeline at high level


```c
Pipeline* parsePipeline(char* str) {

	// Trimming whitetspaces 
    // ....

	// Parse background mode (trailing &)
	// ...

	// If command contains another &, then show error and restart
	// ...

	// Read pipeline parts as strings into array
	// ...

	// Handle empty input
    // ...
    
    // Iterate through array of strings and parse commands into Command* array
	// If any command parse returned NULL, return NULL
    // ...
	
	// Create and return pipeline object
	// ...
}
```

```c
Command* parseCommand(char* str) {
	// Handle < operator
    // ...

	// Handle > operator
    // ...

	// Parse char* str into argv (command name and argumens)
    // ...

	// On empty input return NULL
    // ...

	// Handle parse-level built-in commands (cd, exit) - this will be discussed later
    // ...

	// Create and return Command object
    // ...
}
```

# Handling built-in commands

Built-in commands are split into parse-level and runtime-level commands.

Runtime-level commands can be used in pipelines and are executed in child process. Currently there is only one runtime-level command: pwd

Parse-level commands cannot be used in pipelines and are executed during parsing

```c
int handleBuiltInCommandsRuntimeLevel(char** argv) {
	if(strcmp(argv[0], "pwd") == 0) {
		puts(cwd);
		return 1;
	}
	return 0;
}
```

```c
int handleBuiltInCommandsParseLevel(char** argv, char* str_original) {
	if (strcmp(argv[0], "exit") == 0) {
		if(getpgid(background_process_pid) >= 0) {
			puts("Error: active jobs still running");
			return 1;
		} else {
			puts("Bye...");
			exit(EXIT_SUCCESS);
			// Code does not reach here due to exit
			return 1;
		}
	} else if(strcmp(argv[0], "cd") == 0) {
		int retval = chdir(argv[1]);
		if(retval != 0) {
			perror("Error");
		}
		printf("+ completed '%s' [%d]\n", str_original, retval);
		getcwd(cwd, BUF_SIZE);
		return 1;
	}
	return 0;
}
```

# Executing pipeline at high level

```c
void executePipeline(Pipeline* pipeline) {
	if(pipeline->isBackground) {
		// Execute commands in new process
        // Save latest child process id to check status later on exit command
        // In current implementation only one latest id is saved
        // So checking does not work correctly in some cases
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
```

```c
int executeCommandsFromPipeline(Pipeline* pipeline) {
	if(pipeline->command_count == 1) {
		// Handle simple case with only one command
	} else {
        
        // Temporary file will be used for `stdout->stdin` transfer between commands in pipeline
        // There is a direct method to do that in C
        // But I faced problem that my implementation of that did not work
        // And I could not find the reason
        // So I implemented that using temporary files
        
        // Create temporary file, whose name is based on number of iteration, so that
		// temporary files from different pipelines won't conflict with each other
		char* tmpFilename = (char*)calloc(BUF_SIZE, sizeof(char));
		strcat(tmpFilename, "tmp");
		sprintf(tmpFilename + 3, "%d", iteration_i);

		
		// Connecting commands from pipeline with temporary files
		// cmd1 -> tmpFile -> cmd2 -> tmpFile -> cmd3 ...
		// ...

		// Delete tmp file
        // ...
	}

	// Show result status message
    // ...
}
```

```c
int executeCommand(Command* command) {
	int retval;

	if (fork() == 0){
	    // CHILD PROCESS
	    
        // Redirect input file to stdin if any
       	// ...
        
        // Redirect stdout to output file if any
       	// ...

		// Try to recognize and execute runtime-level built-in command
		// ...

		// If no runtime-level built-in command was recognized
        // Execute command and close CHILD PROCESS
        // ...
        
        // Handle the case of error
        // ...
	} else {
	    // PARENT PROCESS
	    // Wait for child to finish
        // ...
	}
}
```
