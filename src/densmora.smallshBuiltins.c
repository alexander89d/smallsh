/***************************************************************************************
 * File: densmora.smallshBuiltins.c
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Implementation file containing definitions of command functions built
 * 		into smallsh.
 **************************************************************************************/

#include "densmora.smallshBuiltins.h"


/***************************************************************************************
 * Function Name: smallshCd
 * Description:	Called by smallshMain when command path specified by a user-entered
 * 		command is "cd." Receives either the path name of a directory the user
 * 		specified or NULL if none was specified by the user (in which
 * 		case the default will be to switch to the HOME directory). If an
 * 		error occurs, error is reported to user. Returns nothing.
 **************************************************************************************/

void smallshCd(char* cdPath)
{
	/* If the path passed in is NULL, set it to HOME environment variable. */
	if (cdPath == NULL)
	{
		cdPath = getenv("HOME");
	}

	/* Change the current directory to that specified by cdPath, storing return value for inspection. */
	int chdirReturn = chdir(cdPath);

	/* If chdir returned -1, notify user of the error that occurred. */
	if (chdirReturn == -1)
	{
		perror(cdPath); fflush(stderr);
	}
}


/***************************************************************************************
 * Function Name: smallshStatus
 * Description:	Receives a ForegroundExitMethod struct pointer.
 * 		Prints the exit status or terminating signal number of the last run
 * 		foregroud child process. Retruns nothing.
 **************************************************************************************/

void smallshStatus(struct ForegroundExitMethod* lastFgStatus)
{
	/* If the child exited normally, print its exit status. */
	if (lastFgStatus->exitedNormally == TRUE)
	{
		printf("exit value %d\n", lastFgStatus->code); fflush(stdout);
	}
	
	/* Otherwise, if the child was terminated by a signal, print the terminating signal number. */
	else
	{
		printf("terminated by signal %d\n", lastFgStatus->code); fflush(stdout);
	}
}
