/***************************************************************************************
 * File: densmora.smallshBuiltins.h
 * Author: Alexander Densmore
 * Date: 11/20/19
 * Description: Header file containing prototypes of command functions built into
 * 		smallsh. See densmora.smallshBuiltins.c for implementations.
 **************************************************************************************/

#ifndef DENSMORA_SMALLSH_BUILTINS
#define DENSMORA_SMALLSH_BUILTINS

/* My own header file inclusion. */
#include "densmora.childProcesses.h"

/* Function prototypes */
void smallshCd(char* cdPath);
void smallshStatus(struct ForegroundExitMethod* lastFgStatus);

#endif
