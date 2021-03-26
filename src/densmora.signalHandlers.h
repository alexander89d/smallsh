/***************************************************************************************
 * File: densmora.signalHandlers.h
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Header file containing function prototypes for functions that define
 * 		signal actions for SIGIGINT and SIGTSTP signals received by the parent
 * 		process and/or its children. See densmora.signalHandlers.c
 * 		for function implementations.
 **************************************************************************************/

#ifndef DENSMORA_SIGNAL_HANDLERS
#define DENSMORA_SIGNAL_HANDLERS

/* Built-in header file inclusions. */
#include <signal.h>
#include <unistd.h>
#include <string.h>

/* My own header file inclusion. */
#include "densmora.smallshConstants.h"

/* Global flag variables for use by signal handlers.
 * CLASS MATERIAL CITATION: idea of using volatile sig_atmoic_t variables so that
 * program always uses their most up-to-date values given by TA Michael Andrews's response
 * to class Piazza Post @426. */

/* Global flag variable to indicate whether or not backgroud commands are currently allowed.
 * Needs to be accessible to and modifiable by catchSIGTSTP function any time SIGTSTP is received by parent process. */
extern volatile sig_atomic_t allowBackgroundCommands;

/* Global flag variable to indicate whether or not a foreground command is currently running.
 * Needs to be accessible to catchSIGTSTP function any time SIGTSTP is received by parent process. */
extern volatile sig_atomic_t foregroundActive;

/* Global flag variable to indicate whether or not a SIGTSTP signal was sent while
 * a foreground child process was executing but has not yet been processed. */
extern volatile sig_atomic_t sigtstpDuringForegroundProcess;

/* Function prototypes. */
void initializeSignalHandlers();
void catchSIGTSTP(int signo);
void reenableSIGINT();
void ignoreSIGTSTP();

#endif
