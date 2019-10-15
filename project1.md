<header id="title-block-header">

# ECS 150: Project #1 - Simple shell

Prof. Joël Porquet

UC Davis, Winter Quarter 2019

</header>

<nav id="TOC">

*   [<span class="toc-section-number">1</span> Changelog](#changelog)
*   [<span class="toc-section-number">2</span> General information](#general-information)
*   [<span class="toc-section-number">3</span> Objectives of the project](#objectives-of-the-project)
*   [<span class="toc-section-number">4</span> Program description](#program-description)
*   [<span class="toc-section-number">5</span> Suggested work phases](#suggested-work-phases)
*   [<span class="toc-section-number">6</span> Submission](#submission)
*   [<span class="toc-section-number">7</span> Academic integrity](#academic-integrity)

</nav>

# <span class="header-section-number">1</span> Changelog

_Note that the specifications for this project are subject to change at anytime for additional clarification. Make sure to always refer to the **latest** version._

*   v1: First publication

# <span class="header-section-number">2</span> General information

*   Due before **11:59 PM, Wednesday, January 23rd, 2019**.
*   You will be working with a partner for this project.
*   The reference work environment is the CSIF.

# <span class="header-section-number">3</span> Objectives of the project

The objectives of this programming project are:

*   Reviewing most of the concepts learned in previous programming courses: data structures, file manipulation, command line arguments, Makefile, etc.
*   Discovering and making use of many of system calls that UNIX-like operating systems typically offer, especially syscalls belonging to the following categories: processes, files, pipes, and signals.
*   Understanding how a shell works behind the hood, and how processes are launched and configured.
*   Writing high-quality C code by following established industry standards.

# <span class="header-section-number">4</span> Program description

## <span class="header-section-number">4.1</span> Introduction

The goal of this project is to understand important UNIX system calls by implementing a simple shell called **sshell**. A shell is a command-line interpreter: it accepts input from the user under the form of command lines and executes them.

In the following example, it is the shell that is in charge of printing the _shell prompt_, understanding the supplied command line (redirect the output of executable program `ls` with the argument `-l` to the input of executable program `cat`), execute it and wait for it to finish before prompting the user for a new command line.

<div class="highlight">

<pre><span></span><span class="gp">jporquet@pc10:~/ecs150 $</span> ls -l <span class="p">|</span> cat
<span class="go">total 12K</span>
<span class="go">-rw------- 1 jporquet users 11K 2018-01-06 11:08 final_exam.md</span>
<span class="gp">jporquet@pc10:~/ecs150 $</span>
</pre>

</div>

Similar to well-known shells such as _bash_ or _zsh_, your shell will be able to:

1.  execute user-supplied commands with optional arguments
2.  offer a selection of builtin commands
3.  redirect the standard input or standard output of commands to files
4.  pipe the output of commands to other commands
5.  put commands in the background

A working example of the simple shell can be found on the CSIF, at `/home/cs150/public/p1/sshell_ref`.

## <span class="header-section-number">4.2</span> Constraints

The shell must be written in C, be compiled with GCC and only use the standard functions provided by the [GNU C Library](https://www.gnu.org/software/libc/manual/) (aka `libc`). _All_ the functions provided by the `libc` can be used, but your program cannot be linked to any other external libraries.

Your source code should adopt a sane and consistent coding style and be properly commented when necessary. One good option is to follow the relevant parts of the [Linux kernel coding style](https://www.kernel.org/doc/html/latest/process/coding-style.html).

## <span class="header-section-number">4.3</span> Assessment

Your grade for this assignment will be broken down in two scores:

*   Auto-grading: ~60% of grade

    Running an auto-grading script that tests your program and checks the output against various inputs

*   Manual review: ~40% of grade

    The manual review is itself broken down into different rubrics:

    *   Report file: ~50%
    *   Submission : ~5%
    *   Makefile: ~5%
    *   Quality of implementation: ~30%
    *   Code style: ~10%

# <span class="header-section-number">5</span> Suggested work phases

## <span class="header-section-number">5.1</span> Phase 0: preliminary work

In this preliminary phase, copy the skeleton C file `/home/cs150/public/p1/sshell.c` to your directory. Compile it into an executable named `sshell` and run it.

<div class="highlight">

<pre><span></span><span class="gp">jporquet@pc10:~/ $</span> ./sshell
<span class="go">...</span>
</pre>

</div>

What does it do?

### <span class="header-section-number">5.1.1</span> 0.1 Understand the code

Open the C file `sshell.c` and read the code. As you can notice, we use the function `system()` to run the command `/bin/date -u` (which displays the current time in UTC format) and print the _raw_ status value that the command returned to the standard output (`stdout`).

The problem is that `system()` is too high-level to use for implementing a realistic shell. For example, it doesn’t let you redirect the input or output, or run commands in the background.

Useful resources for this phase:

*   [http://man7.org/linux/man-pages/man3/system.3.html](http://man7.org/linux/man-pages/man3/system.3.html)
*   [https://www.gnu.org/software/libc/manual/html_mono/libc.html#Running-a-Command](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Running-a-Command)

### <span class="header-section-number">5.1.2</span> 0.2 Makefile

Write a simple Makefile that generates an executable `sshell` from the file `sshell.c`, using GCC.

The compiler should be run with the `-Wall` (enable all warnings) and `-Werror` (treat all warnings as errors) options.

There should also be a `clean` rule that removes any generated files and puts the directory back in its original state.

Useful resources for this phase:

*   [https://www.gnu.org/software/make/manual/make.html](https://www.gnu.org/software/make/manual/make.html)
*   [https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)

## <span class="header-section-number">5.2</span> Phase 1: running commands the hard way

Instead of using the function `system()`, modify the program in order to use the _fork+exec+wait_ method, as seen in the first week of class.

In a nutshell, your shell should fork and create a child process; the child process should run the specified command with exec while the parent process waits until the child process has completed and the parent can collect its exit status.

The output of the program execution should be as follows.

<div class="highlight">

<pre><span></span><span class="gp">jporquet@pc10:~/ $</span> ./sshell
<span class="go">Tue Apr  4 21:12:18 UTC 2017</span>
<span class="go">+ completed '/bin/date -u' [0]</span>
<span class="gp">jporquet@pc10:~/ $</span>
</pre>

</div>

There are a couple of non-apparent differences between this output and the output of the provided skeleton code:

*   The information message following the execution of the command is printed to `stderr` and not `stdout`.

    This can be verified by redirecting the error output to `/dev/null` and checking that the information message is not printed anymore:

    <div class="highlight">

    <pre><span></span><span class="gp">jporquet@pc10:~/ $</span> ./sshell <span class="m">2</span>>/dev/null
    <span class="go">Tue Apr  4 21:12:18 UTC 2017</span>
    <span class="gp">jporquet@pc10:~/ $</span>
    </pre>

    </div>

*   The printed status (i.e. `0` in the example above) is not the full _raw_ status value anymore, it is the _exit_ status only. Refer to the _Process Completion Status_ section of the `libc` documentation to understand how to extract this value.

Useful resources for this phase: [https://www.gnu.org/software/libc/manual/html_mono/libc.html#Processes](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Processes)

## <span class="header-section-number">5.3</span> Phase 2: read commands from the input

Until now, your program is only running a hard-coded command. In order to be interactive, the shell should instead read commands from the user and execute them.

In this phase, modify your shell in order to print the _shell prompt_ ‘`sshell$` ’ (without the quotes but with the trailing white space) and read a complete command line from the user. We assume that the maximum length of a command line never exceeds 512 characters.

Since it would be annoying for the user to always type the complete paths of the commands to execute (e.g. `/bin/ls`), programs should be searched according to the [`$PATH` environment variable](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Standard-Environment). For that, you need to carefully choose which of the `exec` functions should be used.

For this phase, you can assume that the user can only enter the name of a program (e.g. `ls`, `ps`, `date`, etc.) without any argument.

Example of output:

<div class="highlight">

<pre><span></span><span class="go">sshell$ date</span>
<span class="go">Tue Apr  4 14:09:08 PDT 2017</span>
<span class="go">+ completed 'date' [0]</span>
<span class="go">sshell$</span>
</pre>

</div>

Useful resources for this phase:

*   [https://www.gnu.org/software/libc/manual/html_mono/libc.html#Line-Input](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Line-Input)
*   [https://www.gnu.org/software/libc/manual/html_mono/libc.html#Executing-a-File](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Executing-a-File)

### <span class="header-section-number">5.3.1</span> Error management

In case of user errors (e.g. invalid input, command not found, etc.), the shell should display an error message on `stderr` and wait for the next input, but it should **not** die.

Look through the document to see all the possible error messages that your shell should print (they all start with `Error:`). Here are a couple examples:

<div class="highlight">

<pre><span></span><span class="go">sshell$ &</span>
<span class="go">Error: invalid command line</span>
<span class="go">sshell$ toto</span>
<span class="go">Error: command not found</span>
<span class="go">+ completed 'toto' [1]</span>
<span class="go">sshell$</span>
</pre>

</div>

The only reason for which the shell is allowed to die (with an exit value of `1`) is if a system call actually fails. For example, `malloc()` fails to allocate memory or `fork()` fails to spawn a child.

## <span class="header-section-number">5.4</span> Phase 3: arguments

In this phase, you must add to your shell the ability to handle command lines containing programs and their arguments.

A command is defined as the name of a program, followed by optional arguments, each separated by white spaces (at least one, but can also be more than one). In order to simplify your implementation, you can assume that a command will never have more than 16 arguments (name of the program included).

For this phase, you will need to start _parsing_ the command line in order to interpret what needs to be run. Refer to the `libc` documentation to learn more about strings in C (and particularly sections 5.1, 5.3, 5.4, 5.7 and 5.10): [https://www.gnu.org/software/libc/manual/html_mono/libc.html#String-and-Array-Utilities](https://www.gnu.org/software/libc/manual/html_mono/libc.html#String-and-Array-Utilities)

Example of commands which include arguments (with more or less white spaces separating arguments):

<div class="highlight">

<pre><span></span><span class="go">sshell$ date -u</span>
<span class="go">Tue Apr  4 22:07:03 UTC 2017</span>
<span class="go">+ completed 'date -u' [0]</span>
<span class="go">sshell$ date                      -u</span>
<span class="go">Tue Apr  4 22:46:41 UTC 2017</span>
<span class="go">+ completed 'date                      -u' [0]</span>
<span class="go">sshell$</span>
</pre>

</div>

At this point, and if you have not already, it probably is the right time to think of how you could represent commands using a data structure. After all, a `struct` object in C is nothing different than a C++/Java class without methods. But such an object can still contain fields that contain the object’s properties, and C++-like methods can be implemented as simple functions that receive objects as parameters.

Example:

<div class="highlight">

<pre><span></span><span class="cm">/* C++ class */</span>
<span class="k">class</span> <span class="nc">myclass</span> <span class="p">{</span>
    <span class="kt">int</span> <span class="n">a</span><span class="p">;</span>

    <span class="n">mymethod</span><span class="p">(</span><span class="kt">int</span> <span class="n">b</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">a</span> <span class="o">=</span> <span class="n">b</span><span class="p">;</span>
    <span class="p">}</span>
<span class="p">};</span>

<span class="cm">/* Equivalent in C */</span>
<span class="k">struct</span> <span class="n">myobj</span> <span class="p">{</span>
    <span class="kt">int</span> <span class="n">a</span><span class="p">;</span>
<span class="p">};</span>

<span class="n">myfunc</span><span class="p">(</span><span class="k">struct</span> <span class="n">myobj</span> <span class="o">*</span><span class="n">obj</span><span class="p">,</span> <span class="kt">int</span> <span class="n">b</span><span class="p">)</span> <span class="p">{</span>
    <span class="n">obj</span><span class="o">-></span><span class="n">a</span> <span class="o">=</span> <span class="n">b</span><span class="p">;</span>
<span class="p">}</span>
</pre>

</div>

Hint: the result of parsing the command line should be the instance of a data structure which contains all the information necessary to run the specified command (and the command line should not have to be parsed again).

## <span class="header-section-number">5.5</span> Phase 4: builtin commands

Usually, when a user enters a command, the related program is an _external_ executable file. For example, `ls` refers to the executable file `/bin/ls` while `fdisk` refers to `/sbin/fdisk`.

For some commands, it is preferable, or even necessary, that the shell itself implements the command instead of running an external program. In this phase, your shell must implement the commands `exit`, `cd` and `pwd`.

For simplicity, you can assume that these builtin commands will never be called with incorrect arguments (i.e. no arguments for `exit` and `pwd` and exactly one argument for `cd`).

### <span class="header-section-number">5.5.1</span> `exit`

Receiving the builtin command `exit` should cause the shell to exit properly (i.e. with exit status `0`). Before exiting, the shell must print the message ‘`Bye...`’ on `stderr`.

Example:

<div class="highlight">

<pre><span></span><span class="gp">jporquet@pc10:~/ $</span> ./sshell
<span class="go">sshell$ exit</span>
<span class="go">Bye...</span>
<span class="gp">jporquet@pc10:~/ $</span> <span class="nb">echo</span> <span class="nv">$?</span>
<span class="go">0</span>
</pre>

</div>

### <span class="header-section-number">5.5.2</span> `cd` and `pwd`

The user can change the _current working directory_ (i.e. the directory the shell is currently “in”) with `cd` or display it with `pwd`.

Example:

<div class="highlight">

<pre><span></span><span class="go">sshell$ pwd</span>
<span class="go">/home/jporquet/ecs150</span>
<span class="go">+ completed 'pwd' [0]</span>
<span class="go">sshell$ cd ..</span>
<span class="go">+ completed 'cd ..' [0]</span>
<span class="go">sshell$ pwd</span>
<span class="go">/home/jporquet</span>
<span class="go">+ completed 'pwd' [0]</span>
<span class="go">sshell$ cd toto</span>
<span class="go">Error: no such directory</span>
<span class="go">+ completed 'cd toto' [1]</span>
<span class="go">sshell$</span>
</pre>

</div>

Useful resources for this phase: [https://www.gnu.org/software/libc/manual/html_mono/libc.html#Working-Directory](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Working-Directory)

## <span class="header-section-number">5.6</span> Phase 5: Input redirection

The standard input redirection is indicated by using the meta-character `<` followed by a file name. Such redirection implies that the command located right before `<` is to read its input from the specified file instead of the shell’s standard input (that is from the keyboard if the shell is run in a terminal).

Example:

<div class="highlight">

<pre><span></span><span class="go">sshell$ cat file</span>
<span class="go">titi</span>
<span class="go">toto</span>
<span class="go">+ completed 'cat file' [0]</span>
<span class="go">sshell$ grep toto<file</span>
<span class="go">toto</span>
<span class="go">+ completed 'grep toto<file' [0]</span>
<span class="go">sshell$ grep toto < tata</span>
<span class="go">Error: cannot open input file</span>
<span class="go">sshell$ cat <</span>
<span class="go">Error: no input file</span>
<span class="go">sshell$</span>
</pre>

</div>

Note that the input redirection symbol can or not be surrounded by white spaces.

## <span class="header-section-number">5.7</span> Phase 6: Output redirection

The standard output redirection is indicated by using the meta-character `>` followed by a file name. Such redirection implies that the command located right before `>` is to write its output to the specified file instead of the shell’s standard output (that is on the screen if the shell is run in a terminal).

Example:

<div class="highlight">

<pre><span></span><span class="go">sshell$ echo Hello world!>file</span>
<span class="go">+ completed 'echo Hello world!>file' [0]</span>
<span class="go">sshell$ cat file</span>
<span class="go">Hello world!</span>
<span class="go">+ completed 'cat file' [0]</span>
<span class="go">sshell$ echo hack > /etc/passwd</span>
<span class="go">Error: cannot open output file</span>
<span class="go">sshell$ echo ></span>
<span class="go">Error: no output file</span>
<span class="go">sshell$</span>
</pre>

</div>

Note that the output redirection symbol can or not be surrounded by white spaces.

## <span class="header-section-number">5.8</span> Phase 7: Pipeline commands

So far, a command line could only be composed of one command (name of program and optional arguments). In this phase, we introduce the notion of pipeline of commands.

The pipe sign is indicated by the meta-character `|` and allows multiple commands to be connected to each other. When the shell encounters a pipe sign, it indicates that the output of the command located before the pipe sign must be connected to the input of the command located after the pipe sign. There can be multiple pipe signs on the same command line to connect multiple commands to each other.

Example:

<div class="highlight">

<pre><span></span><span class="go">sshell$ echo Hello world! | grep Hello|wc -l</span>
<span class="go">1</span>
<span class="go">+ completed 'echo Hello world! | grep Hello|wc -l' [0][0][0]</span>
<span class="go">sshell$</span>
</pre>

</div>

Note that there is no limit on the number of commands part of a pipeline as long as it fits into the command line (i.e. 512 characters).

The information message must display the exit value of each command composing the pipeline separately. This means that commands can have different exit values as shown in the example below (the first command succeeds while the second command fails with exit value 2).

<div class="highlight">

<pre><span></span><span class="go">sshell$ echo hello | ls file_that_doesnt_exists</span>
<span class="go">ls: cannot access 'file_that_doesnt_exists': No such file or directory</span>
<span class="go">+ completed 'echo hello | ls file_that_doesnt_exists' [0][2]</span>
<span class="go">sshell$</span>
</pre>

</div>

In a pipeline of commands, only the first command can have its input redirected and only the last command can have its output redirected.

<div class="highlight">

<pre><span></span><span class="go">sshell$ cat file | grep toto < file</span>
<span class="go">Error: mislocated input redirection</span>
<span class="go">sshell$ echo Hello world! > file | cat file</span>
<span class="go">Error: mislocated output redirection</span>
<span class="go">sshell$</span>
</pre>

</div>

Hint: for this phase, you will probably need to think of a data structure that can be used to represent a job (i.e. a pipeline of one or more commands).

Useful resources for this phase (sections 15.1 and 15.2): [https://www.gnu.org/software/libc/manual/html_mono/libc.html#Pipes-and-FIFOs](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Pipes-and-FIFOs)

## <span class="header-section-number">5.9</span> Phase 8: Background commands

Up until now, when the user enters a command line and the shell executes it, the shell waits until the specified job is completed before it displays the prompt again and is able to accept a new command line to be supplied.

The ampersand character `&` indicates that the specified job should be executed in the background. In that case, the shell should not wait for the job’s completion but immediately display the prompt and allow for a new command line to be entered. The background sign may only appear as the last token of a command line.

When a background job finally completes, the shell should display the exit status of all the job’s commands right before a new prompt is being displayed.

Example:

<div class="highlight">

<pre><span></span><span class="go">sshell$ sleep 1&</span>
<span class="go">sshell$ sleep 5</span>
<span class="go">+ completed 'sleep 1&' [0]</span>
<span class="go">+ completed 'sleep 5' [0]</span>
<span class="go">sshell$ echo > file & | grep toto</span>
<span class="go">Error: mislocated background sign</span>
<span class="go">sshell$</span>
</pre>

</div>

Trying to exit while there are still running jobs in the background should be considered a user error.

Example:

<div class="highlight">

<pre><span></span><span class="go">sshell$ sleep 5&</span>
<span class="go">sshell$ exit</span>
<span class="go">Error: active jobs still running</span>
<span class="go">+ completed 'exit' [1]</span>
<span class="go">sshell$ <Return></span>
<span class="go">... more than 5 seconds later ...</span>
<span class="go">sshell$ <Return></span>
<span class="go">+ completed 'sleep 5&' [0]</span>
<span class="go">sshell$ exit</span>
<span class="go">Bye...</span>
</pre>

</div>

Useful resources for this phase: [https://www.gnu.org/software/libc/manual/html_mono/libc.html#Process-Completion](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Process-Completion)

# <span class="header-section-number">6</span> Submission

Since we will use auto-grading scripts in order to test your program, make sure that it strictly follows the specified output format.

## <span class="header-section-number">6.1</span> Content

Your submission should contain, besides your source code, the following files:

*   `AUTHORS`: student ID of each partner, one entry per line. For example:

    <div class="highlight">

    <pre><span></span><span class="gp">$</span> cat AUTHORS
    <span class="go">00010001</span>
    <span class="go">00010002</span>
    <span class="gp">$</span>
    </pre>

    </div>

*   `IGRADING`: **only if your group has been selected for interactive grading for this project**, the interactive grading time slot you registered for.

    *   If your group has been selected for interactive grading for this project, this file must contain exactly one line describing your time slot with the format: `%m/%d/%y - %I:%M %p` (see `man date` for details). For example, an appointment on Monday January 15th at 2:10pm would be transcribed as `01/15/19 - 02:10 PM`.

    <div class="highlight">

    <pre><span></span><span class="gp">$</span> cat IGRADING
    <span class="go">01/15/19 - 02:10 PM</span>
    <span class="gp">$</span>
    </pre>

    </div>

*   `REPORT.md`: a description of your submission. Your report must respect the following rules:

    *   It must be formatted in markdown language as described in this [Markdown-Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet).

    *   It should contain no more than 300 lines and the maximum width for each line should be 80 characters (check your editor’s settings to configure it automatically –please spare yourself and do not do the formatting manually).

    *   It should explain your high-level design choices, details about the relevant parts of your implementation, how you tested your project, the sources that you may have used to complete this project, and any other information that can help understanding your code.

    *   Keep in mind that the goal of this report is not to paraphrase the assignment, but to explain **how** you implemented it.

*   `Makefile`: a Makefile that compiles your source code without any errors or warnings (on the CSIF computers), and builds an executable named `sshell`.

    The compiler should be run with the options `-Wall -Werror`.

    There should also be a `clean` rule that removes generated files and puts the directory back in its original state.

Your submission should be empty of any clutter files (such as executable files, core dumps, backup files, `.DS_Store` files, and so on).

## <span class="header-section-number">6.2</span> Git bundle

Your submission must be under the shape of a Git bundle. In your git repository, type in the following command (your work must be in the branch `master`):

<div class="highlight">

<pre><span></span><span class="gp">$</span> git bundle create p1.bundle master
</pre>

</div>

It should create the file `p1.bundle` that you will submit via `handin`.

Before submitting, **do make sure** that your bundle was properly been packaged by extracting it in another directory and verifying the log:

<div class="highlight">

<pre><span></span><span class="gp">$</span> <span class="nb">cd</span> /path/to/tmp/dir
<span class="gp">$</span> git clone /path/to/p1.bundle -b master sshell
<span class="gp">$</span> <span class="nb">cd</span> sshell
<span class="gp">$</span> ls
<span class="go">AUTHORS IGRADING REPORT.md Makefile sshell.c ...</span>
<span class="gp">$</span> git log
<span class="go">...</span>
</pre>

</div>

## <span class="header-section-number">6.3</span> Handin

Your Git bundle, as created above, is to be submitted with `handin` from one of the CSIF computers by **only one person of your group**:

<div class="highlight">

<pre><span></span><span class="gp">$</span> handin cs150 p1 p1.bundle
<span class="go">Submitting p1.bundle... ok</span>
<span class="gp">$</span>
</pre>

</div>

You can verify that the bundle has been properly submitted:

<div class="highlight">

<pre><span></span><span class="gp">$</span> handin cs150 p1
<span class="go">The following input files have been received:</span>
<span class="go">...</span>
<span class="gp">$</span>
</pre>

</div>

# <span class="header-section-number">7</span> Academic integrity

You are expected to write this project **from scratch**, thus avoiding to use any existing source code available on the Internet. Asking someone else to write your code (e.g., on website such as Chegg.com) is not acceptable and will result in severe sanctions.

You must specify in your report any sources that you have viewed to help you complete this assignment. All of the submissions will be compared with MOSS to determine if students have excessively collaborated, or have used the work of past students.

Any failure to respect the class rules, as explained above or in the syllabus, or the [UC Davis Code of Conduct](http://sja.ucdavis.edu/cac.html) will automatically result in the matter being transferred to Student Judicial Affairs.

* * *

<div class="center">

Copyright © 2019 Joël Porquet

</div>
