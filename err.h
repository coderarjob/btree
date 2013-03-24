/*
*	B-TREE
*	GLOBAL ERROR ARRAY
*
*	Contains Global error array and array codes.
*	Author: Arjob Mukherjee (arjob@aiicsl.com)
*	Date:	27 FEB 2013
*	
*/
#ifndef ERR_H
#define ERR_H

/*Errors of basic disk improved*/
#define MALLOCFLD	0x1
#define	OPENFLD	0x2
#define	BUFFFLD	0x3
#define READFLD 0x4
#define LSKFLD	0x5
#define WRITEFLD 0x6
#define FLEUDCLD 0x7
#define BT_MLCFLD 0x8
#define TP_MLCFLD 0x9
#define BT_INVALID 0xA
#define ND_MLCFLD 0xB

/*Declarations*/
extern char *serror[];		/*error descriptions*/					
extern int aerror[1000];	/*storage of error codes*/
extern int eindex;			/*number of error*/

/*macros*/
#define error_add(code) aerror[eindex++]=code
#define getlasterror() aerror[eindex-1]
#define geterrorcount() eindex

#endif
