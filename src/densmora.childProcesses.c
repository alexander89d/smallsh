/***************************************************************************************
 * File: densmora.childProcesses.c
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Implementation file containing definitions of functions that create 
 * 		child processes of the parent process and execute the commands received
 * 		in either the foreground or background.
 **************************************************************************************/

#include "densmora.childProcesses.h"


/***************************************************************************************
 * Function Name: initForegroundExitMethod
 * Description:	Allocates memory for a ForegroundExitMethod struct and initializes
 * 		its data members so that exit status 0 is represented and can be
 * 		returned by status() function call before any foreground child
 * 		processes have been run. Receives nothing.
 * 		Returns a pointer to the new ForegroundExitMethod struct.
 **************************************************************************************/

struct ForegroundExitMethod* initForegroundExitMethod()
{
	/* Declare and allocate memory for ForegroundExitMethod struct. */
	struct ForegroundExitMethod* initialStatus;
	initialStatus = (struct ForegroundExitMethod*)malloc(sizeof(struct ForegroundExitMethod));

	/* Initialze code to 0 and exitedNormally flag to TRUE to represent
	 * initial status of exit status 0 before first foreground
	 * child process is executed. */
	initialStatus->code = 0;
	initialStatus->exitedNormally = TRUE;

	/* Return pointer to ForegroundExitStatus struct to calling function. */
	return initialStatus;
}


/***************************************************************************************
 * Function Name: runForeground
 * Description:	Receives a CommandInfo struct pointer with information about a command
 * 		the user would like to have executed in the foreground and a
 * 		ForegroundExitMethod struct pointer into which to write information
 * 		about the exit status of the foregroud process.
 * 		Creates a child process with fork() and has the child process
 * 		execute the requested command. Waits for child process to exit or be
 * 		terminated by a signal. Returns nothing since child exit status is
 * 		written into lastFgStatus.
 **************************************************************************************/

void runForeground(struct CommandInfo* myCommand, struct ForegroundExitMethod* lastFgStatus)
{
	pid_t childPid;		/* Pid returned by fork() */
	int childExitMethod;	/* Exit status variable sent to waitpid() function. */
	
	/* Fork off child process to run foreground command. */
	childPid = fork();

	/* If an error occurred when calling fork, report the error to the user. */
	if (childPid == -1)
	{
		perror("fork()"); fflush(stderr);
	}
	
	/* Otherwise, if this is the child process, execute the command in the foreground,
	 * setting up file redirection as needed. */
	else if (childPid == 0)
	{
		executeChild(myCommand, FALSE);
	}

	/* Otherwise, if this is the parent process, wait for the child to finish and process appropriately. */
	else
	{
		/* Have the parent wait for the child to complete. */
		waitpid(childPid, &childExitMethod, 0);

		/* If the child was killed by a signal,
		 * have the parent report it immediately and then update fgExitMethod. */
		if (WIFSIGNALED(childExitMethod) != 0)
		{
			/* Get terminating signal and report to user immediately. */
			int termSig = WTERMSIG(childExitMethod);
			printf("terminated by signal %d\n", termSig); fflush(stdout);

			/* Store terminating signal in lastFgStatus->code, and set exitedNormally flag to FALSE. */
			lastFgStatus->code = termSig;
			lastFgStatus->exitedNormally = FALSE;
		}

		/* Otherwise, if the child exited normally, simply store the exit status in fgExitMethod. */
		else if (WIFEXITED(childExitMethod) != 0)
		{
			int exitStatus = WEXITSTATUS(childExitMethod);
			lastFgStatus->code = exitStatus;
			lastFgStatus->exitedNormally = TRUE;
		}
	}
}


/***************************************************************************************
 * Function Name: runBackground
 * Description:	Receives pointers to a CommandInfo struct and a BackgroundCommands
 * 		linked list. Parent forks off child process and then adds new child's
 * 		pid to bgCommandsList. Returns nothing.
 **************************************************************************************/

void runBackground(struct CommandInfo* myCommand, struct BackgroundCommands* bgCommandsList)
{
	pid_t childPid;		/* Pid returned by fork() */

	/* Fork off child process to run background command. */
	childPid = fork();

	/* If an error occurred when calling fork, report the error to the user. */
	if (childPid == -1)
	{
		perror("fork()"); fflush(stderr);
	}
	
	/* Otherwise, if this is the child process, execute the command in the background,
	 * setting up file redirection as needed. */
	else if (childPid == 0)
	{
		executeChild(myCommand, TRUE);
	}

	/* Otherwise, this is the parent process. Have the parent add the new child to bgCommandsList,
	 * and notify the user of the pid. */
	else
	{
		addBackgroundNode(bgCommandsList, childPid);
		printf("background pid is %d\n", (int)childPid); fflush(stdout);
	}
}


/***************************************************************************************
 * Function Name: executedChild
 * Description:	Receives a command to be executed by child process and a flag
 * 		indicating whether or not it should be run in the background. Executes
 * 		the requested command after setting up any requested io redirection,
 * 		exiting child process with 1 and reporting error
 * 		if command cannot be executed or an io file cannot be opened.
 * 		Returns nothing.
 **************************************************************************************/

void executeChild(struct CommandInfo* myCommand, int isBgCommand)
{
	/* Set foreground child processes to use default action on SIGINT,
	 * and set all child processes to ignore SIGTSTP. */
	if (isBgCommand == FALSE)
	{
		reenableSIGINT();
	}
	ignoreSIGTSTP(); 

	/* If this is a background process and either of the io flags is not set,
	 * open /dev/null for reading and writing and redirect input and/or output to /dev/null. */
	if (isBgCommand == TRUE && (myCommand->inputFlag == FALSE || myCommand->outputFlag == FALSE))
	{
		/* Declare file descriptor for /dev/null and open it for reading and writing
		 * so that input and/or output can be redirected to it. */
		int devNull;
		devNull = open("/dev/null", O_RDWR | O_TRUNC);
		
		/* If /dev/null could not be opened, report error and exit child process. */
		if (devNull == -1)
		{
			fprintf(stderr, "cannot open /dev/null for background process default io redirection\n");
			fflush(stderr);
			exit(1);
		}

		/* If output is not redirected, redirect it to /dev/null for this background process. */
		if (myCommand->outputFlag == FALSE)
		{
			dup2(devNull, 1);
		}
		
		/* If input is not redirected, redirect it to dev/null/ for this background process. */
		if (myCommand->inputFlag == FALSE)
		{
			dup2(devNull, 0);
		}
	}

	/* If the ouput flag is set, open the requested file for writing and redirect output to it. */
	if (myCommand->outputFlag == TRUE)
	{
		/* Declare output file descriptor and open output file for writing, truncating it if it exists
		 * and creating it if it does not exist. */
		int outputFileno;
		outputFileno = open(myCommand->outputRedirDest, O_WRONLY | O_TRUNC | O_CREAT, FILE_PERMISSIONS);
		
		/* If an error occurred opening this file for output, notify the user and exit this process. */ 
		if (outputFileno == -1)
		{
			fprintf(stderr, "cannot open %s for output\n", myCommand->outputRedirDest); fflush(stderr);
			exit(1);
		}
		
		/* Redirect stdout to the requested file.*/
		dup2(outputFileno, 1);
	}

	/* If the input flag is set, open the requested file for writing and redirect output to it. */
	if (myCommand->inputFlag == TRUE)
	{
		/* Declare input file descriptor and open input file for reading. */
		int inputFileno;
		inputFileno = open(myCommand->inputRedirDest, O_RDONLY);

		/* If an error occurred opening this file for input, notify the user and exit this process. */
		if (inputFileno == -1)
		{
			fprintf(stderr, "cannot open %s for input\n", myCommand->inputRedirDest);  fflush(stderr);
			exit(1);
		}
		
		/* Redirect stdin to the requested file.*/
		dup2(inputFileno, 0);
	}
	
	/* Execute the command passed in through myCommand.
	 * If control returns to this function after execvp() call,
	 * report error condition to user since the exec call failed and return exit status 1. */
	execvp(myCommand->commandArgs[0], myCommand->commandArgs);
	perror(myCommand->commandArgs[0]); fflush(stderr);
	exit(1);
}
