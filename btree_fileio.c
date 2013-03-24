
/*
* B-TREE PROJECT
* DATE: 20 MAR 2013
*
* This file contains the methods related to managing btree file. This is the
* Lowest level of where bytes are handled and saved or retrieved to and from
* file.
*/

#include "btree.h"
#include "btreei.h"

long getfilesize(char *name)
{
	#ifdef DEBUG
		printf("GETFILESIZE\n");
	#endif

	struct stat fstat;
	if (stat(name,&fstat) == -1)
		return -1;
	
	return fstat.st_size;
}

bnode *readnode(BTREE *tree, node_ptr node)
{
	#ifdef DEBUG
		printf("READNODE\n" );
	#endif

	/*Read raw bytes from the file*/
	byte barray[NODE_INFILE_SIZE];

	if (redirectto(tree->file, node) == -1)
		return NULL;

	#ifdef DEBUG
		printf("\tRedirected to %ld.\n", node);
	#endif

	int l;
	if ((l = readfile(tree->file,barray,NODE_INFILE_SIZE)) != NODE_INFILE_SIZE)
	{
		printf("Read failed. Read %d bytes\n",l);
		return NULL;
	}

	#ifdef DEBUG
		printf("\tread successfully.\n" );
	#endif

	/*Prepare a new node from the bytes that were read*/
	bnode *newnode = (bnode *)malloc(sizeof(bnode));
	newnode->start = node;
	
	int index=0;
	newnode->isleaf=(boolean)bintolong(&barray[index],INT8);
	index+=INT8;

	newnode->n=(int)bintolong(&barray[index],INT32);
	index+=INT32;

	/*read children*/
	int i;
	for (i=0; newnode->n >0 && i<newnode->n+1 ;i++)
	{
		newnode->children[i]=bintolong(&barray[index],INT32);
		index+=INT32;
	}

	/*read keys*/
	for (i=0;i<newnode->n;i++)
	{
		newnode->key[i]= bintolong(&barray[index],INT32);
		index+=INT32;
	}

	#ifdef DEBUG
		printf("\tBytes read is: %d. Assumed length: %d, Static Length was: %d\n",
			index, INT8+INT32+2*(newnode->n*INT32), NODE_INFILE_SIZE );
	#endif

	/*Construction was complete*/
	return newnode;
}

int writenode(BTREE *tree, bnode *node)
{
	#ifdef DEBUG
		printf("WRITENODE\n" );
	#endif

	/*Prepare the byte array from the node*/
	byte barray[NODE_INFILE_SIZE];

	int index = 0;
	tobin(node->isleaf,INT8,&barray[index]);
	index+=INT8;

	tobin(node->n,INT32,&barray[index]);
	index+=INT32;
	
	int i;
	for (i=0;i<node->n+1;i++)
	{
		tobin(node->children[i],INT32,&barray[index]);
		index+=INT32;
	}

	for (i=0;i<node->n;i++)
	{
		tobin(node->key[i],INT32,&barray[index]);
		index+=INT32;
	}

	#ifdef DEBUG
		printf("\tBytes written was: %d. Assumed length: %d, Static Length was: %d\n",
			index, INT8+INT32+2*(node->n*INT32), NODE_INFILE_SIZE );

		printf("\tNode is in %ld\n", node->start);
	#endif

	

	/*Write the bytes at the proper location*/
	if (redirectto(tree->file, node->start) == -1)
		return -1;

	#ifdef DEBUG
		printf("\tRedirected to %ld.\n", node->start);
	#endif

	int l;
	if ((l=writefile(tree->file,barray,NODE_INFILE_SIZE)) != NODE_INFILE_SIZE)
		return -1;

	#ifdef DEBUG
		printf("\tData written. %d bytes\n", l);
	#endif

	return 1;
}

int format_btree_file(BTREE *tree)
{
	#ifdef DEBUG
		printf("FORMAT_BTREE_FILE.\n");
	#endif

	/*Create the basic format for a valid BTREE*/

	if ((tree->root = (bnode *)malloc(sizeof(bnode))) == NULL)
	{	
		error_add(ND_MLCFLD);
		return -1;
	}

	tree->root->n=0;
	tree->root->start= DEF_ROOT_LOC;		
	tree->root->isleaf=1;

	if( writeroot(tree,tree->root) == -1)
		return -1;

	/*close btree file*/
	flushfile(tree->file);
	closefile(tree->file);
	
	return 1;
}

int writeroot(BTREE *tree,bnode *node)
{
	#ifdef DEBUG
		printf("WRITEROOT: root=%ld\n", node->start);
	#endif

	/*write root into file*/
	if (writenode(tree, node) == -1)
		return -1;

	/*If the node was written, then attempt to update 
	the old root location*/

	if (redirectto(tree->file, ROOT_PTR_LOC) == -1)
		return -1;

	byte rotloc;
	tobin(node->start,INT32,&rotloc);
	if (writefile(tree->file,&rotloc,INT32) != INT32)
		return -1;

	return 1;
}