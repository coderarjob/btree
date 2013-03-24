/* 
* 	B-TREE
*	GLOBAL ERROR ARRAY
*
*	Contains error strings and variable definations.
*	Author: Arjob Mukherjee (arjob@aiicsl.com)
*	Date:	27 FEB 2013
*/

#include "err.h"

//definations
char *serror[] = {"No error",
				"Malloc failed to create FILE", 
				"open systen call failed.",
				"malloc failed to create internal buffer.",
				"read system call failed.",
				"leak system call failed.",
				"write system call failed.",
				"Invaid operation. No file is open."
				"Malloc failed to create BTREE."
				"Malloc failed to create Node Pool."
				"BTREE is invalid. File may be closed."
				"Malloc failed to create new node."};

								
int aerror[1000];
int eindex;
