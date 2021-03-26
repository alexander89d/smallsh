/***************************************************************************************
 * File: densmora.smallshConstants.h
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Header file containing definitions of global constants shared among 
 * 		various files within smallsh program. Constants are defined here with
 * 		include guards so that they are only defined once.
 **************************************************************************************/

#ifndef DENSMORA_SMALLSH_CONSTANTS
#define DENSMORA_SMALLSH_CONSTANTS

/* Boolean value definitions. */
#define FALSE 0
#define TRUE 1

/* Command size constraint definitions. */
#define MAX_COMMAND_CHARS 2048
#define MAX_COMMAND_ARGS 512

/* File permission settings when creating files. */
#define FILE_PERMISSIONS 0660

#endif
