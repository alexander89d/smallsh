# smallsh

## Project Overview

I completed this project as part of CS 344 (Operating Systems I) during Fall 2019 while I was a student at Oregon State University. **Students in that class were granted express permission to post the source code they submitted for this project publicly for use in professional portfolios.**

This project implements a simple Linux command shell. It was developed and tested on a Linux class server running the bash shell (before this shell begins running). The program is written in C (C99 with GNU extensions, compiled with the GCC compiler). 

The smallsh shell implements and supports the following 3 built-in commands:
- cd (allows changing the working directory)
- status (returns the exit status of or signal raised by the most recently executed foreground command, excluding built-in commands)
- exit (exits smallsh)

Other commands are handled using C's `excecvp()` function. 

Other specifications of smallsh are as follows:
- Command lines are written in the format `command [arg1 arg2 ...] [< input_file] [> output_file] [&]`
- Command lines can be up to 2048 characters long and contain up to 512 arguments.
- Blank command lines are ignored.
- Command lines beginning with the # symbol are treated as comments and ignored.
- The special variable `$$` is expanded into the process id of the currently running process. No other special variables are supported in command lines.
- Commands are run in the foreground by default but can be run in the background by including the & symbol at the end of the command line.
- Input can be redirected using [< input_file], and output can be redirected using [> output_file]. These redirections can be placed in any order relative to each other, but they must occur after all arguments in the command line, and they must occur before the & symbol for running the command in the background (if that symbol is used in this command line).
- SIGINT can be used to interrupt the current foreground process and return control to its parent. SIGINT does _not_ cause the smallsh process or any children running in the background to terminate.
- SIGSTP can be used to deactivate and reactivate the ability to run commands in the background.

## Repository Structure

The source code for smallsh is located in the src/ folder. **All files in the src/ directory have not been modified since being submitted as part of this course assignment.** For details about how to compile this program, please see [the original README file in the src/ directory](src/readme.txt) that was submitted as part of this course assignment.
