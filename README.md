# CEN354 Operating Systems - Assignment #2:
# Linux Shell
---
**Assigned: March 17, 2020**

**Due date: April 08, 2020, 23:00**

*Submit through GitHub Classroom. Make sure you start submitting before the deadline. Late submissions will not be accepted.*

In this project, you'll build a simple Linux shell. The shell is the heart of the command-line interface, and thus is central to the Unix/C programming environment. Mastering use of the shell is necessary to become proficient in this world; knowing how the shell itself is built is the focus of this project.

There are three specific objectives to this assignment:

* To further familiarize yourself with the Linux programming environment.
* To learn how processes are created, destroyed, and managed.
* To gain exposure to the necessary functionality in shells.

## Overview

In this assignment, you will implement a *command line interpreter (CLI)* or, as it is more commonly known, a *shell*. The shell should operate in this basic way: when you type in a command (in response to its prompt), the shell creates a child process that executes the command you entered and then prompts for more user input when it has finished.

The shells you implement will be similar to, but simpler than, the one you run every day in Lİnux. If you don't know what shell you are running, it's probably `bash`. One thing you should do on your own time is learn more about your shell, by reading the man pages or other online materials.

## Program Specifications

### Basic Shell: `cen354sh`

Your basic shell, called `cen354sh` is
basically an interactive loop: it repeatedly prints a prompt `cen354sh> ` (note the space after the greater-than sign), parses the input, executes the command specified on that line of input, and waits for the command to finish. This is repeated until the user types `exit`.  The name of your final executable should be `cen354sh`.

The shell can be invoked with either no arguments or a single argument;
anything else is an error. Here is the no-argument way:

```
prompt> ./cen354sh
cen354sh>
```

At this point, `cen354sh` is running, and ready to accept commands. Type away!

The mode above is called *interactive* mode, and allows the user to type
commands directly. The shell does not need to support a *batch mode*, which instead reads input from a batch file and executes commands from therein.

You should structure your shell such that it creates a process for each new command (the exception are *built-in commands*, discussed below).  Your basic shell should be able to parse a command and run the program corresponding to the command.  For example, if the user types `ls -la /tmp`, your shell should run the program `/bin/ls` with the given arguments `-la` and `/tmp` (how does the shell know to run `/bin/ls`? It's something called the shell **path**; more on this below).

## Structure

### Basic Shell

The shell is very simple (conceptually): it runs in a while loop, repeatedly asking for input to tell it what command to execute. It then executes that command. The loop continues indefinitely, until the user types the built-in command `exit`, at which point it exits. That's it!

For reading lines of input, you should use `getline()`. This allows you to obtain arbitrarily long input lines with ease. The shell will be run in *interactive mode*, where the user types a command (one at a time) and the shell acts on it.

To parse the input line into constituent pieces, you might want to use
`strsep()`. Read the man page (carefully) for more details.

To execute commands, look into `fork()`, `exec()`, and `wait()/waitpid()`.
See the man pages for these functions, and also read the relevant [book
chapter](http://www.ostep.org/cpu-api.pdf) for a brief overview.

You will note that there are a variety of commands in the `exec` family; for this project, you must use `execv`. You should **not** use the `system()` library function call to run a command.  Remember that if `execv()` is successful, it will not return; if it does return, there was an error (e.g., the command does not exist). The most challenging part is getting the arguments correctly specified.

### Paths

In our example above, the user typed `ls` but the shell knew to execute the program `/bin/ls`. How does your shell know this?

It turns out that the user must specify a **path** variable to describe the set of directories to search for executables; the set of directories that comprise the path are sometimes called the *search path* of the shell. The path variable contains the list of all directories to search, in order, when the user types a command.

**Important:** Note that the shell itself does not *implement* `ls` or other commands (except built-ins). All it does is find those executables in one of the directories specified by `path` and create a new process to run them.

To check if a particular file exists in a directory and is executable,
consider the `access()` system call. For example, when the user types `ls`, and path is set to include both `/bin` and `/usr/bin`, try `access("/bin/ls", X_OK)`. If that fails, try `/usr/bin/ls`. If that fails too, it is an error.

Your initial shell path should contain one directory: `/bin`

Note: Most shells allow you to specify a binary specifically without using a search path, using either **absolute paths** or **relative paths**. For example, a user could type the **absolute path** `/bin/ls` and execute the `ls` binary without a search path being needed. A user could also specify a **relative path** which starts with the current working directory and specifies the executable directly, e.g., `./main`. In this project, you **do not** have to worry about these features.

### Built-in Commands

Whenever your shell accepts a command, it should check whether the command is a **built-in command** or not. If it is, it should not be executed like other programs. Instead, your shell will invoke your implementation of the built-in command. For example, to implement the `exit` built-in command, you simply call `exit(0);` in your `cen354sh` source code, which then will exit the shell.

In this project, you should implement `exit`, `cd`, and `path` as built-in commands.

* `exit`: When the user types `exit`, your shell should simply call the `exit` system call with 0 as a parameter. It is an error to pass any arguments to
  `exit`.

* `cd`: `cd` always take one argument (0 or >1 args should be signaled as an error). To change directories, use the `chdir()` system call with the argument supplied by the user; if `chdir` fails, that is also an error.

* `path`: The `path` command takes 0 or more arguments, with each argument separated by whitespace from the others. A typical usage would be like this: `cen354sh> path /bin /usr/bin`, which would add `/bin` and `/usr/bin` to the search path of the shell. If the user sets path to be empty, then the shell should not be able to run any programs (except built-in commands). The `path` command always overwrites the old path with the newly specified path.

### Redirection

Many times, a shell user prefers to send the output of a program to a file rather than to the screen. Usually, a shell provides this nice feature with the `>` character. Formally this is named as redirection of standard output. To make your shell users happy, your shell should also include this feature.

For example, if a user types `ls -la /tmp > output`, nothing should be printed on the screen. Instead, the standard output of the `ls` program should be rerouted to the file `output`. In addition, the standard error output of the program should be rerouted to the file `output`.

If the `output` file exists before you run your program, you should simple overwrite it (after truncating it).  

The exact format of redirection is a command (and possibly some arguments) followed by the redirection symbol followed by a filename. Multiple redirection operators or multiple files to the right of the redirection sign are errors.

Note: don't worry about redirection for built-in commands (e.g., we will
not test what happens when you type `path /bin > file`).

### Program Errors

**The one and only error message.** You should print this one and only error
message whenever you encounter an error of any type:

```
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
```

The error message should be printed to stderr (standard error), as shown above.

After any errors, your shell simply *continue processing* after printing the one and only error message.

There is a difference between errors that your shell catches and those that the program catches. Your shell should catch all the syntax errors specified in this project page. If the syntax of the command looks perfect, you simply run the specified program. If there are any program-related errors (e.g., invalid arguments to `ls` when you run it, for example), the shell does not have to worry about that (rather, the program will print its own error messages and exit).


### Miscellaneous Hints

Remember to get the **basic functionality** of your shell working before
worrying about all of the error conditions and end cases. For example, first get a single command running (probably first a command with no arguments, such as `ls`).

Next, add built-in commands. Then, try working on redirection.  Each of these requires a little more effort on parsing, but each should not be too hard to implement.

At some point, you should make sure your code is robust to white space of various kinds, including spaces (` `) and tabs (`\t`). In general, the user should be able to put variable amounts of white space before and after commands, arguments, and various operators; however, the redirection operator does not require whitespace.

Check the return codes of all system calls from the very beginning of your work. This will often catch errors in how you are invoking these new system calls. It's also just good programming sense.

Beat up your own code! You are the best (and in this case, the only) tester of this code. Throw lots of different inputs at it and make sure the shell behaves well. Good code comes through testing; you must run many different tests to make sure things work as desired. Don't be gentle -- other users certainly won't be.

Finally, keep versions of your code using Git. Also, don't forget to add a report about your project as a single `.pdf` file.

### Submission

Submit your `cen354sh` C source code, your `Makefile` and a `.pdf` file as your report. Make sure you add your name and student id at the end of your report.

When you accepted this assignment, you have created a repository as assignment2-your_GitHub_username. Cloning this repository gives you a local copy of your project. After adding your files you can commit the changes.
