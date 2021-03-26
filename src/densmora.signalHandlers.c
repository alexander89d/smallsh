/***************************************************************************************
 * File: densmora.signalHandlers.c
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Implementation file containing function definitions for functions that
 * 		define signal actions for SIGIGINT and SIGTSTP signals received
 * 		by the parent process and/or its children. 
 **************************************************************************************/

#include "densmora.signalHandlers.h"

/* Define and initailze global variables allowBackgroundCommands and foregroundActive,
 * which were defined in densmora.signalHandlers.h 
 * CLASS MATERIAL CITATION: idea of using volatile sig_atmoic_t variables so that
 * program always uses their most up-to-date values given by TA Michael Andrews's response
 * to class Piazza Post @426. */

volatile sig_atomic_t allowBackgroundCommands = TRUE;
volatile sig_atomic_t foregroundActive = FALSE;
volatile sig_atomic_t sigtstpDuringForegroundProcess = FALSE;

/***************************************************************************************
 * Function Name: initializeSignalHandlers
 * Description:	Defines non-default actions for SIGINT and SIGTSTP signals for process
 * 		which calls this function. Receives and returns nothing.
 **************************************************************************************/

void initializeSignalHandlers()
{
	/* Declare sigaction structs for SIGINT and SIGTSTP signals, initializing as empty.
	 * memset() is used for initiailzition because syntax like SIGINT_action = {0} threw gcc compiler warning.
	 * memset() ensures entire struct is initialized to 0.*/
	struct sigaction SIGINT_action, SIGTSTP_action;
	memset(&SIGINT_action, 0, sizeof(struct sigaction));
	memset(&SIGTSTP_action, 0, sizeof(struct sigaction));

	/* Set SIGINT_action struct's sa_handler to SIG_IGN so that a SIGINT
	 * does not cause the current process (or any children for whom the SIGINT_action is not overwritten)
	 * to terminate. */
	SIGINT_action.sa_handler = SIG_IGN; 
	
	/* Set data members of SIGTSTP_action struct so that signal handler is catchSIGTSTP,
	 * all other incoming signals are blocked until the signal handler returns, and no flags are set. 
	 * CLASS MATERIAL CITATION: Idea for using SA_RESTART flag for signal handler
	 * so that any system call in process when signal handler was trigerred
	 * restarts after the signal handler returns provided by instructor Tyler Lawson's
	 * response to class Piazza Post @336. */
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;

	/* Call sigaction function to set signal actions for when SIGINT or SIGTSTP signals are received. */
	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	/* Initialize allowBackgroundCommands global flag to TRUE so that background commands are allowed
	 * until SIGTSTP signal is received by the current process for the first time,
	 * initialize foregroundActive global flag to false so that it only becomes true when a new foreground
	 * child process begins, and intialize sigtstpDuringForegroundProcess to FALSE so that it is not set
	 * to true until the first time during the current process
	 * that SIGTSTP is called while a foreground child process is executing.. */
	allowBackgroundCommands = TRUE;
	sigtstpDuringForegroundProcess = FALSE;
}


/***************************************************************************************
 * Function Name: cathSIGTSTP
 * Description:	Sigaction handler function for processes which register it as their
 * 		SIGTSTP handler. Toggles between allowing and not allowing background
 * 		commands within the shell. Receives signal number (which is not needed
 * 		since function is only registered to one signal type). Returns nothing.
 **************************************************************************************/

void catchSIGTSTP(int signo)
{
	/* String messages and their lengths used by write function calls within this signal handler. */
	char* backgroundDisabled= "\nEntering foreground-only mode (& is now ignored)\n";
	const int lenBackgroundDisabled = 50;
	char* backgroundEnabled = "\nExiting foreground-only mode\n";
	const int lenBackgroundEnabled = 30;
	char* reprompt = ": ";
	const int lenReprompt = 2;
	
	/* If there is an active foreground process, set the flag indicating
	 * that this signal handler was called so it can be called again. */
	if (foregroundActive == TRUE)
	{
		sigtstpDuringForegroundProcess = TRUE;
	}

	/* Otherwise, if allowBackgroundCommands is currently set to TRUE, set it to FALSE
	 * and print out message informing user that background commands are now disabled. */
	else if (allowBackgroundCommands == TRUE)
	{
		allowBackgroundCommands = FALSE;
		write(STDOUT_FILENO, backgroundDisabled, lenBackgroundDisabled);

		/* Print out new prompt colon if this signal was not raised by main after
		 * a child process exited. */
		if (sigtstpDuringForegroundProcess == FALSE)
		{
			write(STDOUT_FILENO, reprompt, lenReprompt);
		}
	}

	/* Otherwise, if allowBackgroundCommands is currently set to FALSE,
	 * set it to TRUE and print out message informing user that background commands are now enabled. */
	else
	{
		allowBackgroundCommands = TRUE;
		write(STDOUT_FILENO, backgroundEnabled, lenBackgroundEnabled);

		/* Print out new prompt colon if this signal was not raised by main after
		 * a child process exited. */
		if (sigtstpDuringForegroundProcess == FALSE)
		{
			write(STDOUT_FILENO, reprompt, lenReprompt);
		}
	}
}

/***************************************************************************************
 * Function Name: reenableSIGINT
 * Description: Called by processes for which the default process of SIGINT should be
 * 		resotred and restores the default functionality. Receives
 * 		and returns nothing.
 **************************************************************************************/

void reenableSIGINT()
{
	/* Reset SIGINT's sa_handler to SIG_DFL for any process during which this function is called. */
	struct sigaction SIGINT_action;
	memset(&SIGINT_action, 0, sizeof(struct sigaction));
	SIGINT_action.sa_handler = SIG_DFL;
	sigaction(SIGINT, &SIGINT_action, NULL);
}


/***************************************************************************************
 * Function Name: ignoreSIGTSTP
 * Description: Function called by any processes by which SIGTSTP should simply be
 * 		ignored. Receives and returns nothing.
 **************************************************************************************/

void ignoreSIGTSTP()
{
	/* Set SIGTSTP's sa_handler to SIG_IGN for any process during which this function is called. */
	struct sigaction SIGTSTP_action;
	memset(&SIGTSTP_action, 0, sizeof(struct sigaction));
	SIGTSTP_action.sa_handler = SIG_IGN;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}
