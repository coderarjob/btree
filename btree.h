#ifndef H_BTREE_h
#define H_BTREE_h 

#include "bdi.h"
#include "err.h"
#include "comtype.h"

#include <malloc.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NODEPOOL_LENGTH 10
#define NODE_ORDER 4

/*Location of node in file. Thus node_ptr. 
This pointer type guarantees that correct node is 
retrived and properly disposed.*/
typedef unsigned long node_ptr;

typedef unsigned char boolean;
				
#define NONE (node_ptr)0
									
typedef struct
{
	long start;
	int n;
	node_ptr children[NODE_ORDER+1];
	boolean isleaf;
	long key[NODE_ORDER];
	unsigned int isdirty;
}bnode;

typedef struct 
{
	node_ptr location;
	bnode *node;
	unsigned int ref_count;
	int invk_count;					/*Number of times this node was searched. Used in implementing the replacement policy*/
}nodepooltype;

typedef struct  
{
	bnode *root;					/*the root itself*/
	unsigned long filelength;				/*Dynamic (real time) size of the btree file*/
	DFILE *file;
	nodepooltype nodepool[NODEPOOL_LENGTH];			/*Node pool, contains the nodes that are in memory.*/
	int pool_count;					/*Number of items in the node pool. Used to trigger replacement*/
}BTREE;

BTREE *openbtree(char *filename, BDIEVENTS *events);
int initialize(BTREE *);
int format_btree_file(BTREE *tree);
void close_btree(BTREE *tree);
bnode *getnode(BTREE *tree, node_ptr nodeloc);

int insert_into_btree(BTREE *tree, int value);
#endif