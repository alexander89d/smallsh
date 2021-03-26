/***************************************************************************************
 * File: densmora.childProcesses.h
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Header file containing prototypes for functions that create child
 * 		processes of the parent process and execute the commands received
 * 		in either the foreground or background. See densmora.childProcesses.c
 * 		for function implementations.
 **************************************************************************************/
#ifndef DENSMORA_CHILD_PROCESSES
#define DENSMORA_CHILD_PROCESSES

/* Built-in header file inclusions. */
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

/* My own header file inclusions. */
#include "densmora.commands.h"
#include "densmora.signalHandlers.h"

/* Struct to store either exit status or signal number of last executed foreground child process
 * as well as a flag indicating whether it exited normally or was terminated by a signal. */

struct ForegroundExitMethod
{
	int code;		/* Numeric exit code or signal number. */
	int exitedNormally;	/* Flag indicating whether or not process exited normally. */
};


/* Function prototypes. */
struct ForegroundExitMethod* initForegroundExitMethod();
void runForeground(struct CommandInfo* myCommand, struct ForegroundExitMethod* lastFgStatus);
void runBackground(struct CommandInfo* myCommand, struct BackgroundCommands* bgCommandsList);
void executeChild(struct CommandInfo* myCommand, int isBgCommand);

#endif
