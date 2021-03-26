/***************************************************************************************
 * File: densmora.commands.h
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Header file containing definitions of various structs and prototypes
 * 		of various functions used to read in, parse, and store user commands
 * 		entered on the command line. See below for descriptions of each struct.
 * 		See densmora.commands.c for descriptions of each function.
 **************************************************************************************/

#ifndef DENSMORA_COMMANDS
#define DENSMORA_COMMANDS

/* Built-in header file inclusions. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

/* My own header file inclusion. */
#include "densmora.smallshConstants.h"

/* Struct for storing node of PidExpansions linked list struct (defined below).
 * PidExpansions linked list is used to store strings of any words entered on the command line which contained "$$"
 * with every instance of "$$" replaced by the pid of the current process. That linked list is a member of the
 * CommandInfo struct, also defined below. */

struct PidNode
{
	char* strExpansion;	/* Expansion of any word from command line containing at least one instance of "$$". */
	struct PidNode* next;	/* Pointer to next node in list. */
};

/* Struct containing linked list of PidNode struct pointers. */

struct PidExpansions
{
	struct PidNode* head;	/* Pointer to head of linked list. */
};


/* Struct storing information about a command read in from the command line of smallsh. */

struct CommandInfo
{
	char* commandLine;			/* String storing raw command line (will be tokenized into vector). */
	char** commandArgs;			/* String vector storing command path and all arguments. */
	int outputFlag;				/* Set to true for output redirection. */
	char* outputRedirDest;			/* Stores location of output redirection. */
	int inputFlag;				/* Set to true for input redirection. */
	char* inputRedirDest;			/* Stores location of input redirection. */
	int backgroundFlag;			/* Set to true for background process command. */
	char* parentPid;			/* Pid of the process sending command to child. */
	struct PidExpansions* expansionList;	/* Ptr to linked list containing string expansions of words with "$$" */
};

/* Struct storing node of BackgroundCommands linked list. 
 * The linked list stores information about processes spawned by fork()
 * which have been asked to execute a command in the background but have not yet returned
 * and been cleaned up. */

struct BackgroundNode
{
	pid_t pid;				/* Pid of the child process. */
	struct BackgroundNode* next;		/* Address of next node in list. */
};

/* Struct containing linked list of BackgroundNode struct pointers described above. */

struct BackgroundCommands
{
	int numNodes;				/* Number of nodes in the list. */
	struct BackgroundNode* head;		/* Address of the list head. */
};

/* Function prototypes (see densmora.commands.c for function descriptions and implementations). */
struct CommandInfo* newCommand(struct BackgroundCommands* bgCommandsList);
int getCommandLine(struct CommandInfo* myCommand, struct BackgroundCommands* bgCommandsList);
int parseCommand(struct CommandInfo* myCommand);
char* expandPid(struct CommandInfo* myCommand, char* originalStr);
void deleteCommand(struct CommandInfo* myCommand);
struct BackgroundCommands* newBackgroundCommands();
void addBackgroundNode(struct BackgroundCommands* commandsList, pid_t pidIn);
void checkCommandStatuses(struct BackgroundCommands* commandsList);
void removeBackgroundNode(struct BackgroundCommands* commandsList, 
			  struct BackgroundNode* garbageNode, struct BackgroundNode* previousNode);
void deleteBackgroundCommands(struct BackgroundCommands* commandsList);
struct PidExpansions* newPidExpansions();
void addPidNode(struct PidExpansions* expansionList, char* strExpansionIn);
void deletePidExpansions(struct PidExpansions* expansionList);

#endif
