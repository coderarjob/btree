/*
* DISKLL PROJECT
* PART OF PRESENCE SUBPROJECT
*
* BASIC DISK IMPROVED HEADER FILE
* Version: 0.3a
* Author: Arjob Mukherjee (arjob@aiicsl.com)
* Dated: 27 FEB 2013
*/

#ifndef BDI_H
#define BDI_H

#include <stdio.h>
#include <stddef.h> /*For NULL*/
#include <fcntl.h> //for open
#include <unistd.h> //for read, write, lseek
#include "err.h"

#define BUFFERED 1
#define UNBUFFERED 0

typedef struct {
	int fd; 				/*File identifier*/
	unsigned char *buffer;	/*internal buffer where data is stored temporarily*/
	unsigned char *pt;		/*next read and write position in the buffer*/
	long start;				/*the address of the first byte in buffer*/
	long end;				/*the address of the last byte in the buffer*/
	int len;				/*number of bytes that can be read from buffer at a point of time*/
	int mode;				/*contains status flag for internal operation*/
	struct bdi_events* _events;	/*Events that are called to indicate occurence of one event*/
} DFILE;

struct bdi_events{
	void (*oncreate)(char *filename,int isbuffered);
	void (*onread)(DFILE*,int len);
	void (*onwrite)(DFILE*,int len);
	void (*onredirect)(DFILE*,int fromcache,long newloc);
	void (*onfill)(DFILE*);
	void (*onflush)(DFILE*);
	void (*onclose)(void);
	void (*onerror)(void);
};

typedef struct bdi_events BDIEVENTS;

void setbdieventhandlers(DFILE*,BDIEVENTS* events);

DFILE *openfile(char *filename, unsigned char isbuffered, BDIEVENTS *events);
int readfile(DFILE *fd, unsigned char *buffer, int length);
int writefile(DFILE *fd, unsigned char *buffer, int length);
int flushfile(DFILE *fd);
int redirectto(DFILE *fd, long location);
int closefile(DFILE *file);

#endif
