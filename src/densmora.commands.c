/***************************************************************************************
 * File: densmora.commands.c
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Implementation file containing definitions of functions for creating, 
 * 		managing, and deleting various structs used to read in, parse, 
 * 		and store user commands entered on the command line. 
 * 		See densmora.commands.h for definitions of each struct.
 **************************************************************************************/

/* Inclusion of header file corresponding to this implementation file. */
#include "densmora.commands.h"


/***************************************************************************************
 * Function Name: newCommand
 * Description:	Function that allocates memory for and initializes a new commandInfo
 * 		struct, calling a subroutine called getCommandLine to get a new
 * 		command from the user and another subroutine called parseCommand to
 * 		convert the command into a string vector for passing onto execvp()
 * 		calls. 
 *
 * 		Recieves a pointer to a BackgroundCommands struct containing
 * 		all child processes of the current process that are either currently
 * 		running in the background or have finished but have not yet been
 * 		cleaned up; this BackgroundCommands pointer is passed into
 * 		getCommandLine subroutine so that it can check for any
 * 		backgroud processes that have ended before each time it prompts
 * 		the user for a command. 
 *
 * 		Returns the pointer to the CommandInfo struct.
 **************************************************************************************/

struct CommandInfo* newCommand(struct BackgroundCommands* bgCommandsList)
{
	/* Declare and allocate new CommandInfo pointer. */
	struct CommandInfo* myCommand;
	myCommand = (struct CommandInfo*)malloc(sizeof(struct CommandInfo));

	/* Initialize pointers to hold pieces of command line to NULL. */
	myCommand->commandLine = NULL;
	myCommand->commandArgs = NULL;
	myCommand->outputRedirDest = NULL;
	myCommand->inputRedirDest = NULL;
	myCommand->expansionList = NULL;


	/* Initialize all flags to FALSE. */
	myCommand->backgroundFlag = FALSE;
	myCommand->inputFlag = FALSE;
	myCommand->outputFlag = FALSE;

	/* Allocate memory for parentPid string, 
	 * get pid of current process (which will be parent of the process to which it passes parsed command),
	 * and read pid into string. */
	myCommand->parentPid = (char*)malloc(50 * sizeof(char));
	memset(myCommand->parentPid, '\0', 50);
	pid_t rawParentPid = getpid();
	sprintf(myCommand->parentPid, "%d", (int)rawParentPid);

	/* Loop until a command within the maximum number of chars and maximum number of args has been read in. */
	int tooManyArgs;	/* flag to track if too many arguments are in commandLine. */
	do
	{
		/* Get input line, storing return value of line length in temporary variable. */
		int lineLength = getCommandLine(myCommand, bgCommandsList);

		/* If & is the last word of commandLine, set backgroundFlag to true
		 * and replace & and preceding space with null terminators. */
		if (myCommand->commandLine[lineLength-1] == '&' && myCommand->commandLine[lineLength-2] == ' ')
		{
			myCommand->backgroundFlag = TRUE;
			myCommand->commandLine[lineLength-1] = '\0';
			myCommand->commandLine[lineLength-2] = '\0';
		}

		/* Parse command to set values of args and redir variables,
		 * setting tooManyArgs flag to value returned by parseCommand function. */
		tooManyArgs = parseCommand(myCommand);

		/* If too many args were entered, warn the user before looping again. */
		if (tooManyArgs == TRUE)
		{
			fprintf(stderr, "Num Args Error: Only a maximum of %d arguments\n", MAX_COMMAND_ARGS); 
			fflush(stderr);
			fprintf(stderr, "(excluding the command path and any io redirection) are allowed per command.\n");
			fflush(stderr);
		}
	} while(tooManyArgs == TRUE);

	/* Return the pointer to the command to the calling function. */
	return myCommand;
}


/***************************************************************************************
 * Function Name: getCommandLine
 * Description: Prompts for and reads in command line from user.
 * 		Receives pointer to CommandInfo struct whose commandLine variable will
 * 		be filled with the command line as well as pointer to BackgroundCommands
 * 		struct so that background commands can be checked to see if they have
 * 		finished before each new command prompt.
 * 		Returns number of chars read into commandLine.
 **************************************************************************************/

int getCommandLine(struct CommandInfo* myCommand, struct BackgroundCommands* bgCommandsList)
{
	/* Declare variable to store chars read in by getline() calls to use in do-while loop conditional below. */
	int charsRead;

	/* Read in command from user, looping again if any of the following are true:
	 * 1. An error occurs during reading or a blank line is entered (charsRead < 1 after '\n' removed)
	 * 2. Too long of a line is entered (charsRead > MAX_COMMAND_CHARS)
	 * 3. The line is a comment line beginning with '#' */
	do
	{
		/* Declare local variable to store buffer size and initialize to 0 for getline call. */
		size_t bufferSize = 0;
		
		/* If myCommand->commandLine is not NULL, a previous call to getline allocated memory for it.
		 * Free the memory and reset myCommand->commandLine to NULL to prepare for next getline call. */
		if (myCommand->commandLine != NULL)
		{
			free(myCommand->commandLine);
			myCommand->commandLine = NULL;
		}

		/* Before each prompt, if bgCommandsList contains any commands, 
		 * call checkCommandStatuses function on bgCommandsList. */
		
		if (bgCommandsList->numNodes > 0)
		{
			checkCommandStatuses(bgCommandsList);
		}
		
		/* Prompt user for command with ':' symbol and read in command with getline */
		printf(": "); fflush(stdout);
		charsRead = getline(&(myCommand->commandLine), &bufferSize, stdin);
			
		/* If getline function call was interrupted by a signal and returned error status -1,
		 * clear the error from stdin in preparation for next getline call. */
		if (charsRead == -1)
		{
			clearerr(stdin);
		}

		/* Otherwise, if at least 1 char was read and the line does not begin with a '#',
		 * remove the trailing newline character and decrement charsRead to reflect
		 * total length of actual command. Note that blank lines and comment lines
		 * will simply be ignored and cause the loop to iterate again. */
		else if (charsRead >= 1 && myCommand->commandLine[0] != '#')
		{
			myCommand->commandLine[charsRead-1] = '\0';
			charsRead--;

			/* If updated charsRead exceeds max command line length,
			 * print an error message to inform the user. */
			if (charsRead > MAX_COMMAND_CHARS)
			{
				fprintf(stderr, "Command Length Error: Commands can only be a maximum length\n");
				fflush(stderr);
				fprintf(stderr, "of %d chars. Please enter a new command.\n", MAX_COMMAND_CHARS);
				fflush(stderr);
			}
		}	
	} while(charsRead < 1 || charsRead > MAX_COMMAND_CHARS || myCommand->commandLine[0] == '#');

	/* Now that the loop above has exited,
	 * return the number of chars read in to calling function. */
	return charsRead;
}


/***************************************************************************************
 * Function Name: parseCommand
 * Description: Receives a CommandInfo pointer. Parses the commandLine in that pointer,
 * 		allocating memory for the commandArgs array and storing the command path
 * 		and arguments in that array, setting the inputFlag and/or outputFlag
 * 		if io redirection is requested, and setting inputRedirDest
 * 		or outputRedirDest to the desired destinations if applicable.
 * 		Returns TRUE if the user has tried to pass in more than 
 * 		MAX_COMMAND_ARGS arguments and frees memory of commandArgs array.
 * 		Otherwise, returns FALSE, leaving commandArgs array allocated
 * 		and filled as described above.
 **************************************************************************************/

int parseCommand(struct CommandInfo* myCommand)
{
	int vectSize = 0;	/* Stores number of elements in commandArgs vector. */
	int numArgs = 0;	/* Stores number of arguments passed in through commandLine (excluding command path). */
	char* strToken;		/* Contains tokenized string returned by strtok_r. */
	char* savePtr;		/* Contains savePtr for use by strtok_r. */
		
	/* Allocate space for args vector with size max number of arguments + 2 extra indices
	 * (one for path variable at the first index, one for NULL pointer at final index). */
	myCommand->commandArgs = (char**)malloc((MAX_COMMAND_ARGS + 2) * sizeof(char*));

	/* Initialize all indices of commandArgs vector to NULL. */
	for (int i = 0; i < MAX_COMMAND_ARGS + 2; i++)
	{
		myCommand->commandArgs[i] = NULL;
	}

	/* Get path variable and store in first index of array. */
	strToken = strtok_r(myCommand->commandLine, " ", &savePtr);
	
	/* If strToken contains the pattern "$$", send it to epxandPid function,
	 * setting the path variable to the returned, expanded string. */
	if (strstr(strToken, "$$") != NULL)
	{
		myCommand->commandArgs[0] = expandPid(myCommand, strToken);
	}
	
	/* Otherwise, simply set path variable to strToken. */
	else
	{
		myCommand->commandArgs[0] = strToken;
	}

	/* Increment vectSize but not numArgs since first element has been stored in vector,
	 * but it is the path (rather than an argument) of the command and, therefore, does not count
	 * toward MAX_COMMAND_ARGS. */
	vectSize++;

	/* Get next token of commandLine in preparation for while loop below
	 * (which will only run if there is at least 1 argument or io redirection after path variable). */
	strToken = strtok_r(NULL, " ", &savePtr);
	
	/* Fill argsVector and get io redirection info.
	 * Loop until end of commandLine is reached
	 * (strtok_r returns NULL any time it is called after it has tokenized
	 * the last token of commandLine). */
	while (strToken != NULL)
	{
		/* If input redirection operator is encountered, 
		 * set inputFlag and set inputRedirDest to next token of commandLine. */
		if (strlen(strToken) == 1 && strToken[0] == '<')
		{
			/* Set inputFlag to TRUE and get next token of commandLine for location of input redirection. */
			myCommand->inputFlag = TRUE;
			strToken = strtok_r(NULL, " ", &savePtr);

			/* If strToken contains the pattern "$$", send to expandPid function
			 * so that all instances of "$$" in the token are expanded, assigning
			 * its return value to inputRedirDest. */
			if (strstr(strToken, "$$") != NULL)
			{
				myCommand->inputRedirDest = expandPid(myCommand, strToken);
			}

			/* Otherwise, simply assign strToken to inputRedirDest. */
			else
			{
				myCommand->inputRedirDest = strToken;
			}
		}

		/* Else if output redirection operator is encountered,	
		 * set outputFlag and set outputRedirDest to next token of commandLine. */
		else if (strlen(strToken) == 1 && strToken[0] == '>')
		{
			/* Set outputFlag to TRUE and get next token of commandLine for location of output redirection. */
			myCommand->outputFlag = TRUE;
			strToken = strtok_r(NULL, " ", &savePtr);

			/* If strToken contains the pattern "$$", send to expandPid function
			 * so that all instances of "$$" in the token are expanded, assigning
			 * its return value to outputRedirDest. */
			if (strstr(strToken, "$$") != NULL)
			{
				myCommand->outputRedirDest = expandPid(myCommand, strToken);
			}

			/* Otherwise, simply assign strToken to outputRedirDest. */
			else
			{
				myCommand->outputRedirDest = strToken;
			}
		}

		/* Otherwise, if less than the max number of arguments have been read in,
		 * read in the next argument to the next open index of the args array
		 * and increment vectSize and numArgs. */
		else if (numArgs < MAX_COMMAND_ARGS)
		{
			/* If strToken contains the pattern "$$", send to expandPid function
			 * so that all instances of "$$" in the token are expanded, assigning
			 * its return value to the next open index of commandArgs. */
			if (strstr(strToken, "$$") != NULL)
			{
				myCommand->commandArgs[vectSize] = expandPid(myCommand, strToken);
			}

			/* Otherwise, simply assign strToken to next open index of commandArgs. */
			else
			{
				myCommand->commandArgs[vectSize] = strToken;
			}
			
			/* Increment vectSize and numArgs. */
			vectSize++;
			numArgs++;
		}

		/* Otherwise, since the maximum number of arguments have been read in, 
		 * immediately return TRUE to calling function to indicate that too many args have been read in
		 * and so that no more args are stored in vector.  Since new command will be entered
		 * before newly-allocated commandInfo struct is returned to user, and that command will be parsed
		 * into a newly-allocated commandArgs vector, free memory allocated to commandArgs vector 
		 * and set it to NULL.
		 * Note that nothing is done to change values of io flag or dest variables
		 * since io redirection appears at end of command line after all arguments.
		 * If this condition evaluates to true, this function returns before
		 * io redirection variables are ever changed from their initial values. */
		else
		{
			/* Free commandArgs vector and reset pointer to NULL. */
			free(myCommand->commandArgs);
			myCommand->commandArgs = NULL;

			/* If expansionList has been allocated, free its data and reset pointer to NULL
			 * as well since new command is about to be read in after returning
			 * to newCommand function. */
			if (myCommand->expansionList != NULL)
			{
				deletePidExpansions(myCommand->expansionList);
				myCommand->expansionList = NULL;
			}

			/* Return TRUE to calling function to indicate that too many args were entered. */
			return TRUE;
		}

		/* Get next token of commandLine in preparation for next iteration. */
		strToken = strtok_r(NULL, " ", &savePtr);
	}

	/* Since the loop above terminated without returning TRUE,
	 * return FALSE to indicate that the number of args read in does not exceed the maximum. */
	return FALSE;
}


/***************************************************************************************
 * Function Name: expandPid
 * Description: Receives pointers to a CommandInfo struct and a string that contains
 * 		one or more instances of "$$" for expansion. Returns a pointer to a
 * 		new string to the calling function with all instances of "$$" expanded
 * 		to myCommand->parentPid. Stores address of new string in
 * 		myCommand->expansionList so that it can be kept track of
 * 		for deletion when freeing all memory dynamically allocated to 
 * 		myCommand later. 
 **************************************************************************************/

char* expandPid(struct CommandInfo* myCommand, char* originalStr)
{
	int idxO = 0;				/* originalStr index for use in loop below. */
	int lenO = strlen(originalStr);		/* Length of originalStr for use in loop condition below. */
	char* pidStr = myCommand->parentPid;	/* Pid string to which all occurrences of target are expanded. */
	int lenPid = strlen(pidStr);		/* Length of pidStr. */
	char* expandedStr = NULL;		/* Copy of originalStr with each instance of "$$" replaced by pidStr. */
	int idxE = 0;				/* Index of expandedStr for use in loop below. */
	int maxBytesE = 0;			/* Max number of bytes to allocate for expandedStr. */
	
	/* Calculate the maximum number of bytes expandedStr could be.
	 * The target will occur in originalStr a maximum of lenO / 2 times
	 * (if lenO is even, then every character could be part of a pair;
	 * if lenO is odd, then one of the characters in it would not be able to be part of a pair and could
	 * be an extra trailing character after all pairs are expanded).
	 * Thus, the expanded string will contain a maximum of maxTargetHits * lenPid bytes
	 * for the case where originalStr consists only of consecutive pairs of "$$" + 2 extra bytes 
	 * (1 for extra character if lenO is odd and all other characters are part of "$$" pairs, 
	 * and 1 for the null terminator). */ 
	int maxTargetHits = lenO / 2;
	maxBytesE = maxTargetHits * lenPid + 2;

	/* In the extremely unlikely event that maxBytesE is less than lenO+1,
	 * reset it to lenO+1 so that all characters of original string can fit. */
	if (maxBytesE < lenO + 1)
	{
		maxBytesE = lenO + 1;
	}

	/* Allocate memory for expandedStr and set all indices to null character. */
	expandedStr = (char*)malloc(maxBytesE * sizeof(char));
	memset(expandedStr, '\0', maxBytesE);
	
	/* Loop through originalStr, copying characters into expandedStr and expanding any instance of "$$" to pidStr. */
	while (idxO < lenO)
	{
		/* If this is not the last character of originalStr,
		 * and if the current char and next char of originalStr are both '$',
		 * place pid in expandedStr starting at idxE. */
		if (idxO < lenO - 1 && originalStr[idxO] == '$' && originalStr[idxO + 1] == '$')
		{
			/* Copy each character of pidStr to expandedStr. */
			for (int idxPid = 0; idxPid < lenPid; idxPid++)
			{
				expandedStr[idxE] = pidStr[idxPid];
				idxE++;
			}
			/* Increase idxO by 2 since current char and next char of originalStr
			 * have been replaced by pid in expandedStr. */
			idxO += 2;
		}

		/* Otherwise, simply copy the character at idxO to idxE, incrementing both of those indices by 1. */
		else
		{
			expandedStr[idxE] = originalStr[idxO];
			idxO++;
			idxE++;
		}
	}

	/* If expansionList has not yet been allocated for this command due to this being
	 * the first word needing expanded, allocate it now (that way, commands that do not contain "$$"
	 * will never needlessly have their expansionLists allocated). */
	if (myCommand->expansionList == NULL)
	{
		myCommand->expansionList = newPidExpansions();
	}

	/* Add new node with expandedStr to expansionList so that this newly-allocated string
	 * is kept track of for deletion later, and return expandedStr to calling function
	 * so that it can be assigned to proper place in commandArgs array or an io redirection desitnation variable. */
	addPidNode(myCommand->expansionList, expandedStr);
	return expandedStr;
}


/***************************************************************************************
 * Function Name: deleteCommand
 * Description: Receives a CommandInfo struct for deletion. Frees all dynamically
 * 		allocated memory associated with the struct's data members and then
 * 		frees memory of the struct itself. Returns nothing.
 **************************************************************************************/

void deleteCommand(struct CommandInfo* myCommand)
{
	/* If expansionList was allocated due to having 1 or more
	 * words in command line containing "$$", free memory allocated to it. */
	if (myCommand->expansionList != NULL)
	{
		deletePidExpansions(myCommand->expansionList);
	}
	
	/* Free dynamically allocated memory for commandLine, commandVector, parentPid, and command itself. */
	free(myCommand->commandArgs);
	free(myCommand->commandLine);
	free(myCommand->parentPid);
	free(myCommand);
}


/***************************************************************************************
 * Function Name: newBackgroundCommands
 * Description: Declares and allocates memory for a new, empty BackgroundCommands
 * 		linked list where each link contains the pid of a background child
 * 		process and a pointer to the next link. Receives nothing. Returns
 * 		pointer to the newly-allocated linked list.
 **************************************************************************************/

struct BackgroundCommands* newBackgroundCommands()
{
	/* Declare and initialize new BackgroundCommands struct to be returned to calling function. */
	struct BackgroundCommands* commandsList;
	commandsList = (struct BackgroundCommands*)malloc(sizeof(struct BackgroundCommands));

	/* Initialize numNodes to 0 and head to NULL. */
	commandsList->numNodes = 0;
	commandsList->head = NULL;

	/* Return commandsList to calling function. */
	return commandsList;
}


/***************************************************************************************
 * Function Name: addBackgroundNode
 * Description: Receives a pointer to a BackgroundCommands linked list and the pid
 * 		of a background process for which a node is to be added. Allocates
 * 		memory for the new load and adds it to the linked list. Returns nothing.
 **************************************************************************************/

void addBackgroundNode(struct BackgroundCommands* commandsList, pid_t pidIn)
{
	/* Declare and allocate memory for new BackgroundNode. */
	struct BackgroundNode* newNode;
	newNode = (struct BackgroundNode*)malloc(sizeof(struct BackgroundNode));

	/* Initialize pid based on pid passed in. */
	newNode->pid = pidIn;

	/* Add commandNode to the front of the list by simply placing it before old head and incrementing numNodes. */
	newNode->next = commandsList->head;
	commandsList->head = newNode;
	commandsList->numNodes++;
}


/***************************************************************************************
 * Function Name: checkCommandStatuses
 * Description: Iterates over BackgroundCommands linked list, checking to see whether
 * 		each process has completed. For any process that has completed, reports
 * 		its exit status or terminating signal, and removes that
 * 		process's node from the linked list. Receives the 
 * 		backgroundCommands linked list over which to iterate.
 * 		Returns nothing.
 **************************************************************************************/

void checkCommandStatuses(struct BackgroundCommands* commandsList)
{
	/* Initialize currentNode to list head
	 * and previousNode (utilized by removeBackgroundNode function below) to NULL. */
	struct BackgroundNode* currentNode = commandsList->head;
	struct BackgroundNode* previousNode = NULL;

	/* Iterate through commandsList, removing any nodes whose processes have terminated. */
	while(currentNode != NULL)
	{
		/* See if process specified by currentNode->pid has completed. */
		int childExitMethod;		/* Holds exit status int into which waitpid writes. */
		pid_t pidReturned;		/* Holds pid returned by waitpid. */
		pidReturned = waitpid(currentNode->pid, &childExitMethod, WNOHANG);
		
		/* If pidReturned == currendNode->pid, process has terminated;
		 * get next node in preparation for next iteration and remove this node. Report to user exit status. */
		if (pidReturned == currentNode->pid)
		{
			/* If the child exited normally, print its exit status. */
			if (WIFEXITED(childExitMethod) != 0)
			{
				int exitStatus = WEXITSTATUS(childExitMethod);
				printf("background pid %d is done: exit value %d\n", (int)pidReturned, exitStatus);
				fflush(stdout);
			}
			/* If the child was terminated by a signal, print its termination signal. */
			if (WIFSIGNALED(childExitMethod) != 0)
			{
				int termSig = WTERMSIG(childExitMethod);
				printf("background pid %d is done: terminated by signal %d\n", (int)pidReturned, termSig);
				fflush(stdout);
			}

			/* Remove currentNode from commandsList so that its dynamically allocated data can be freed.
			 * Leave value of previousNode the same since gap created by removing currentNode will leave
			 * previousNode's value the same for next iteration. */
			struct BackgroundNode* garbageNode = currentNode;
			currentNode = currentNode->next;
			removeBackgroundNode(commandsList, garbageNode, previousNode);
		}

		/* Otherwise, the process has not exited; simply update previousNode and currentNode for next loop. */
		else
		{
			previousNode = currentNode;
			currentNode = currentNode->next;
		}
	}
}


/***************************************************************************************
 * Function Name: removeBackgroundNode
 * Description: Receives a BackgroundCommands linked list pointer, the node to be
 * 		removed (garbageNode), and the node before garbageNode. Removes
 * 		garbageNode from the linked list and deallocates its memory. 
 * 		Returns nothing.
 **************************************************************************************/

void removeBackgroundNode(struct BackgroundCommands* commandsList, 
			  struct BackgroundNode* garbageNode, struct BackgroundNode* previousNode)
{
	/* If garbageNode is list head, make head->next the new list head. */
	if (garbageNode == commandsList->head)
	{
		commandsList->head = commandsList->head->next;
	}
	/* Otherwise, connect previousNode to garbageNode->next. */
	else
	{
		previousNode->next = garbageNode->next;
	} 

	/* Now that gap that would be created by removing garbageNode has been eliminated, 
	 * free all dynamically allocated data of garbageNode and decrement numNodes. */
	free(garbageNode);
	garbageNode = NULL;
	commandsList->numNodes--;
}


/***************************************************************************************
 * Function Name: deleteBackgroundCommands
 * Description: Frees all dynamically-allocated memory associated with a
 * 		BackgroundCommands linked list. Reaps child processes that are zombies,
 * 		and kills all child background processes that are still running (reaping
 * 		them after killing them). Receives pointer to a BackgroundCommands list.
 * 		Returns nothing.
 **************************************************************************************/

void deleteBackgroundCommands(struct BackgroundCommands* commandsList)
{
	/* Initialize pointer to currentNode to list head. */
	struct BackgroundNode* currentNode = commandsList->head;

	/* Iterate through all nodes of list, terminating all processes and freeing all dynamically allocated memory. */
	while (currentNode != NULL)
	{
		/* Check to see if process with pid stored in current node has finished. */
		pid_t pidReturned;	/* Pid returned by waitpid (will equal 0 if process has not exited) */
		int childExitMethod;	/* Stores exit method of child process if it has returned. */
		pidReturned = waitpid(currentNode->pid, &childExitMethod, WNOHANG);

		/* Kill the process if it has not returned, and then call waitpid to clean it up. */
		if (pidReturned == 0)
		{
			kill(currentNode->pid, SIGKILL);
			pidReturned = waitpid(currentNode->pid, &childExitMethod, 0);
		}
		
		/* Declare temporary pointer variable to curentNode so that it can be deleted,
		 * and update currentNode for next iteration. */
		struct BackgroundNode* garbageNode = currentNode;
		currentNode = currentNode->next;

		/* Free all dynamically allocated data of garbageNode and decrement numNodes. */
		free(garbageNode);
		garbageNode = NULL;
		commandsList->numNodes--;
	}

	/* Free memory dynamically allocated to commandsList itself. */
	free(commandsList);
}


/***************************************************************************************
 * Function Name: newPidExpansions
 * Description: Recives no parameters. Allocates memory for a new PidExpansion list
 * 		pointer and returns pointer to calling function. PidExpansions
 * 		structs hold nodes containing strings dynamically allocated
 * 		by expandPid function. Since strtok_r is used to iterate over
 * 		commandLine and it automatically replaces the token " " with null
 * 		characters, any words in the command line that do not contain "$$"
 * 		are stored in commandArgs or io destination variables using pointers
 * 		returned from strtok_r. Therefore, when commandLine is deallocated,
 * 		this also deallocates memory for all words not containing "$$."
 * 		The PidExpansions struct and its related functions
 * 		are then just responsible for keeping track of and deleting
 * 		strings dynamically allocated by expandPid that are
 * 		not part of the original commandLine due to being a modified
 * 		copy of part of the commandLine.
 **************************************************************************************/

struct PidExpansions* newPidExpansions()
{
	/* Declare and allocate memory for new PidExpansions list. */
	struct PidExpansions* expansionList;
	expansionList = (struct PidExpansions*)malloc(sizeof(struct PidExpansions));

	/* Set head to NULL and return newly-allocated and initialized struct to calling function. */
	expansionList->head = NULL;
	return expansionList;
}


/***************************************************************************************
 * Function Name: addPidNode
 * Description: Receives a pointer to a previously-allocated PidExpansions struct
 * 		and a pointer to the strExpansion for which a new node should be added
 * 		to the expansion list. Allocates a new node containing strExpansionIn
 * 		and adds it to the front of the expansion list. Returns nothing.
 **************************************************************************************/

void addPidNode(struct PidExpansions* expansionList, char* strExpansionIn)
{
	/* Declare and allocate memory for new PidNode that will replace current head of list. */
	struct PidNode* newHead;
	newHead = (struct PidNode*)malloc(sizeof(struct PidNode));

	/* Set newHead’s strExpansion to the passed-in parameter and insert it before current expansionList head. */
	newHead->strExpansion = strExpansionIn;
	newHead->next = expansionList->head;
	expansionList->head = newHead;
}


/***************************************************************************************
 * Function Name: deletePidExpansions
 * Description: Receives a pointer to a PidExpansions struct. Frees all dynamically-
 * 		allocated memory associated with that struct. Returns nothing.
 **************************************************************************************/

void deletePidExpansions(struct PidExpansions* expansionList)
{
	/* Declare and initialize currentNode pointer for iterating over expansionList in loop below. */
	struct PidNode* currentNode = expansionList->head;

	/* Loop through each node of expansionList, freeing all associated dynamically-allocated memory. */
	while (currentNode != NULL)
	{
		/* Declare garbageNode so currentNode can be deleted,
		 * and point currentNode at currentNode->next to prepare for next iteration. */
		struct PidNode* garbageNode = currentNode;
		currentNode = currentNode->next;

		/* Free memory for string stored at garbageNode as well as garbageNode itself. */
		free(garbageNode->strExpansion);
		free(garbageNode);
		garbageNode = NULL;
	}
	
	/* Free memory of expansionList itself. */
	free(expansionList);
}
