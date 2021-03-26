/***************************************************************************************
 * File: densmora.smallshMain.c
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description:	File containing implementation of main() function for smallsh program.
 * 		Main function coordinates actions of other functions (including
 * 		requesting creation of new ForegroundExitMethod, BackgroundCommands,
 * 		and CommandInfo structs; determining which functions should process
 * 		each command; and freeing dynamically-allocated memory. Receives
 * 		no arguments when program is started from  command line. Returns 0
 * 		to indicate successful smallsh exit.
 **************************************************************************************/

/* My own header file inclusions. */
#include "densmora.commands.h"
#include "densmora.smallshBuiltins.h"
#include "densmora.signalHandlers.h"

/* Implementation of main function (see description at top of file). */

int main()
{
	int timeToExit = FALSE;			/* Flag set to TRUE once user enters "exit" command. */
	
	/* Set global foregroundActive flag to FALSE at beginning of execution of program. */
	foregroundActive = FALSE;

	/* Declare and intitialize ForegroundExitMethod struct pointer to keep track of exit method of last foreground
	 * child process. Initialized to exit status 0. */
	struct ForegroundExitMethod* lastFgStatus = initForegroundExitMethod();
	
	/* Declare and initialze BackgroundCommandsList linked list to keep
	 * track of running background processes by their pids. */
	struct BackgroundCommands* bgCommandsList;
	bgCommandsList = newBackgroundCommands();

	/* Call initializeSignalHandlers() function to define how the parent shell should handle
	 * SIGINT and SIGTSTP signals. */
	initializeSignalHandlers();
	
	/* Iterate repeatedly to get new user commands until user chooses to exit. */
	while (timeToExit == FALSE)
	{
		/* Declare CommandInfo struct pointer,
		 * initializing it with pointer returned by newCommand function. */
		struct CommandInfo* myCommand = newCommand(bgCommandsList);
		
		/* If user has chosen to exit, set timeToExit to TRUE and free dynamically-allocated memory. */
		if (strcmp(myCommand->commandArgs[0], "exit") == 0)
		{
			/* Set timeToExit flag to TRUE and delete myCOmmand. */
			timeToExit = TRUE;
			
			/* Free memory associated with lastFgStatus and bgCommandsList. */
			free(lastFgStatus);
			lastFgStatus = NULL;
			deleteBackgroundCommands(bgCommandsList);
			bgCommandsList = NULL;
		}
		
		/* Otherwise, if user has entered "cd" as first word of command line,
		 * call built-in cd function. */
		else if (strcmp(myCommand->commandArgs[0], "cd") == 0)
		{
			/* Call built-in cd function, passing argument stored at commandArgs[1].
			 * Since commandArgs[0] contains the "cd" command itself, commandArgs[1] will
			 * either contain a pathname or a NULL pointer since an index containing a NULL
			 * pointer is always at the end of commandArgs for purposes of calling execvp(). */
			smallshCd(myCommand->commandArgs[1]);
		}

		/* Otherwise, if user has entered "status" as first word on the command line,
		 * call built-in status function, passing it lastFgStatus.  */
		else if (strcmp(myCommand->commandArgs[0], "status") == 0)
		{
			smallshStatus(lastFgStatus);
		}
		
		/* Otherwise, if the user has requested that this command be run in the background
		 * and background commands are currently allowed, run it in the background. */
		else if (myCommand->backgroundFlag == TRUE && allowBackgroundCommands == TRUE)
		{
			runBackground(myCommand, bgCommandsList);
		}

		/* Otherwise, since this is either a foreground command or one requested
		 * to be run in the backgroud but unable to be run there due to status of allowBackgroundCOmmands flag,
		 * run the requested command in the foreground. */
		else
		{
			/* Set foregroundActive to TRUE, run foreground command,
			 * and then reset foregroundActive to FALSE. */
			foregroundActive = TRUE;
			runForeground(myCommand, lastFgStatus);
			foregroundActive = FALSE;

			/* If sigtstpDuringForegroundProcess is TRUE,
			 * set global flag to FALSE and raise the SIGTSTP signal
			 * so that background command permissions are toggled
			 * and message is printed by signal handler. */
			if (sigtstpDuringForegroundProcess == TRUE)
			{
				raise(SIGTSTP);
				sigtstpDuringForegroundProcess = FALSE;
			}
		}
		
		/* Delete the command just processed in preparation for next iteration. */
		deleteCommand(myCommand);
		myCommand = NULL;
	}

	/* Now that loop has exited since user typed "exit" as first word of command line,
	 * return 0 to terminate shell and indicate successful execution to operating system. */
	return 0;
}
