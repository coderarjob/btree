/*
* DISKLL PROJECT
* PART OF PRESENCE SUBPROJECT
* BASIC DISK IMPROVED
* 
* Version: 0.3a
* Author: Arjob Mukherjee (arjob@aiicsl.com)
* Created: 31 Jan 2013
*/

#include "bdi.h"

#define _ERR 0x2
#define _EOF 0x4
#define _UNBUFF 0x8
#define MAXBUFF 1000	/*bytes*/

#define NDEBUG

#define CALL_EVNT(file,event,arg) do{if (file->_events != NULL) if (event != NULL) event(arg);}while(0)
#define ADD_ERROR(file,er) do{error_add(er); if (file->_events != NULL) if (file->_events->onerror != NULL) file->_events->onerror();}while(0)
#define ISEND(file) (file->len==0 && file->mode & _EOF)

static int fillfile(DFILE *file);
void *malloc(int);

DFILE *openfile(char *filename, unsigned char isbuffered, BDIEVENTS *events)
{
	/*1. Create DFILE structure*/

	DFILE *newfile;
	if ((newfile = malloc(sizeof(DFILE))) == NULL)
	{
		#ifdef DEBUG
		  printf("Error: Cannot create file. Malloc failed.\n");
		#endif
		
		error_add(MALLOCFLD);		
 		return NULL;
	}

	/*2. Open file*/

	if ((newfile->fd = open(filename,O_RDWR,0)) == -1)
	{
		#ifdef DEBUG
			printf("Error: Cannot create file. Open() failed.\n");
	 	#endif
	 
	 	error_add(OPENFLD);
		return NULL;
	}

	/*3. Create Buffer*/

	newfile->mode=0;

	if (!isbuffered)
		/*Create unbuffered FILE*/
		newfile->mode = _UNBUFF;
	else
		/*Create buffered FILE*/
		if ((newfile->buffer = malloc(MAXBUFF)) == NULL)
		{
			#ifdef DEBUG
				printf("Error: Cannot create buffer. Malloc failed.\n");
		 	#endif
		 	
		 	error_add(BUFFFLD);
		 	return NULL;
		}

	/*4. Set default values*/
	newfile->start=0;
	newfile->end=-1;
	newfile->len=0;
	newfile->pt = newfile->buffer;
	newfile->_events=events;

	if (events)
		if (events->oncreate)
			events->oncreate(filename,isbuffered);

	return newfile;
}

int readfile(DFILE *file, unsigned char *buffer, int len)
{
	#ifdef DEBUG
		printf("READFILE\n");
	#endif

	/*CHECK OF NULL*/
	if (file == NULL)
	{
		error_add(FLEUDCLD);
		return -1;
	}

	/*Read directly from file*/

	int l = 0;

	if (file->mode & _UNBUFF)
	{
		
		/*UNBUFFERD READ*/
		if ((l = read(file->fd, buffer, len)) == -1)
		{
			#ifdef DEBUG
				printf("Error: Unbffered file read failed.\n");
		 	#endif

			ADD_ERROR(file,READFLD);

			/*set error flag*/
			file->mode |= _ERR;
			return -1;
		}

		/*CALL EVENT*/
		if (file->_events != NULL) 
		if (file->_events->onread != NULL) 
			file->_events->onread(file,l);

		return l;
	}

	/*Read from BUFFER*/

	int olen = len;
	while (len > 0 && !ISEND(file) && (file->mode & _ERR) == 0)
	{
		int needtofill = !ISEND(file) && file->len==0;

		if (needtofill) 
			if (fillfile(file) == -1){
				#ifdef DEBUG
					printf("Error: Cound not fill buffer.\n");
			 	#endif

				return -1;
			}

		*buffer++ = *file->pt++;
		file->len--;
		len--;
	}

	if (file->mode & _ERR)
		return -1;

	/*CALL EVENT*/
	if (file->_events != NULL) 
	if (file->_events->onread != NULL) 
		file->_events->onread(file,olen-len);

	return olen-len;
}


int writefile(DFILE *file, unsigned char *buffer, int len)
{
	#ifdef DEBUG
		printf("WRITEFILE\n");
	#endif

	/*CHECK OF NULL*/
	if (file == NULL)
	{
		error_add(FLEUDCLD);
		return -1;
	}

	/*WRITE directly from file*/

	int l = 0;

	if (file->mode & _UNBUFF)
	{

		if ((l = write(file->fd, buffer, len)) == -1)
		{
			#ifdef DEBUG
				printf("Error: Unbffered file write failed.\n");
		 	#endif

			ADD_ERROR(file,WRITEFLD);

			/*set error flag*/
			file->mode |= _ERR;
			return -1;
		}

		/*CALL EVENT*/
		if (file->_events != NULL) 
		if (file->_events->onwrite != NULL) 
			file->_events->onwrite(file,l);

		return l;
	}

	
	/*WRITE INTO BUFFER*/

	/*FILL FILE WITH DATA*/
	int needtofill = !(ISEND(file)) & (file->len==0);

	if (needtofill) 
		if (fillfile(file) == -1){
			#ifdef DEBUG
				printf("Error: Cound not fill buffer.\n");
		 	#endif

			return -1;
		}

	/*DO ACCTUAL WRITE*/
	//printf("needtofill %d:,\nFILE END IS: %ld\n",file->mode & _EOF, file->end );

	int olen = len;
	while (len > 0 && (file->mode & _ERR) == 0)
	{
		int needtoflush = file->end-file->start+1 >= MAXBUFF;

		if (needtoflush)
			if (flushfile(file) == -1)
			{
				#ifdef DEBUG
					printf("Error: Flush failed.\n");
		 		#endif

				return -1;
			}

		*file->pt++ = *buffer++;
		
		if (file->len == 0){
			file->end++;
			file->len=0;
		}
		else
			file->len--;

		len--;
	}

	if (file->mode & _ERR)
		return -1;

	/*CALL EVENT*/
	if (file->_events != NULL) 
	if (file->_events->onwrite != NULL) 
		file->_events->onwrite(file,olen-len);

	return olen-len;
}

int redirectto(DFILE *file, long newlocation)
{
	#ifdef DEBUG
		printf("REDIRECT\n");
	#endif

	/*CHECK OF NULL*/
	if (file == NULL)
	{
		error_add(FLEUDCLD);
		return -1;
	}

	if (newlocation >= file->start && newlocation <= file->end) {
		/*CALL EVENT*/
		if (file->_events != NULL) 
			if (file->_events->onredirect != NULL) 
				file->_events->onredirect(file,1,newlocation);

		//reposition file pointer.
		file->pt = &file->buffer[newlocation-file->start];
		file->len = file->end - newlocation +1;
		
		#ifdef DEBUG
		  printf("Redirected by manipulation. location: %ld\n", newlocation);
		#endif
	}
	else
	{
		#ifdef DEBUG
			printf("Start: %ld, End: %ld, Location: %ld\n", file->start, file->end, newlocation);
			printf("Redirected by lseek,location: %ld\n", newlocation);
		#endif

		/*CALL EVENT*/
		if (file->_events != NULL) 
			if (file->_events->onredirect != NULL) 
				file->_events->onredirect(file,0,newlocation);

		if (flushfile(file) == -1)
		{
			#ifdef DEBUG
				printf("Error: Flush failed.\n");
	 		#endif

			return -1;
		}

		if (lseek(file->fd, newlocation, 0) == -1)
		{
			#ifdef DEBUG
				printf("Error: lseek failed.\n");
	 		#endif

			ADD_ERROR(file,LSKFLD);

			/*set error flag*/
			file->mode |= _ERR;
			return -1;
		}

		file->start = newlocation;
		file->end = newlocation-1;
		file->len=0;
		file->pt = file->buffer;
		file->mode &= ~_EOF; /*Reset EOF flag. If not ISEND() will give wrong output.*/
	}

	return 1;
}

static int fillfile(DFILE *file)
{
	#ifdef DEBUG
		printf("FILLFILE\n");
	#endif

	if (file->mode & _UNBUFF)
	{
		#ifdef DEBUG
			printf("Warning: Trying to fill when mode is unbuffered.\n");
		#endif

		return 1;
	}

	/*CALL EVENT*/
	CALL_EVNT(file,file->_events->onfill,file);

	/*Fill buffer with new data*/
	int len;
	if ((len = read(file->fd, file->buffer, MAXBUFF)) == -1)
	{
		#ifdef DEBUG
			printf("Error: Read failed.\n");
	 	#endif

		ADD_ERROR(file,READFLD);

		/*set error flag*/
		file->mode |= _ERR;
		return -1;
	}

	if (len < MAXBUFF)
		file->mode |= _EOF;
	else
		file->mode &= ~_EOF;

	file->len = len;
	file->start = file->end+1;
	file->end = file->start + len -1;
	file->pt = file->buffer;

	return 1;

}

int flushfile(DFILE *file)
{
	#ifdef DEBUG
		printf("FLUSH FILE\n");
	#endif

	if (file == NULL)
	{
		error_add(FLEUDCLD);
		return -1;
	}

	if (file->mode & _UNBUFF)
	{
		#ifdef DEBUG
			printf("Warning: Trying to fill when mode is unbuffered.\n");
		#endif

		return 1;
	}

	/*CALL EVENT*/
	CALL_EVNT(file,file->_events->onflush,file);


	/*Redirect to start location before beginning writing*/
	if (lseek(file->fd, file->start, 0) == -1)
	{

		#ifdef DEBUG
			printf("Error: lseek failed.\n");
 		#endif

		ADD_ERROR(file,LSKFLD);

		/*set error flag*/
		file->mode |= _ERR;
		return -1;
	}

	/*Write buffer into the file*/
	int len;
	if ((len = write(file->fd, file->buffer, file->end - file->start +1)) == -1)
	{
		#ifdef DEBUG
			printf("Error: Write failed.\n");
	 	#endif

		ADD_ERROR(file,WRITEFLD);

		/*set error flag*/
		file->mode |= _ERR;
		return -1;
	}

	file->start = file->end+1;
	file->end = file->start-1;
	file->len=0;
	file->pt = file->buffer;

	return 1;
}

int closefile(DFILE *file)
{
	if (file == NULL)
	{
		error_add(FLEUDCLD);
		return -1;
	}

	/*CALL EVENT*/
	CALL_EVNT(file,file->_events->onclose,);

	if (flushfile(file) == -1)
		return -1;

	if (close(file->fd) == -1)
		return -1;
	else
		return 1;
}

void setbdieventhandlers(DFILE *file, BDIEVENTS* events)
{
	if (file==NULL)
		error_add(FLEUDCLD);
	else
		file->_events=events;
}