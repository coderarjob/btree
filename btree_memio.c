/*
* B-TREE PROJECT
* DATE: 20 MAR 2013
*
* This file contains the methods related to managing node pool. It provides
* API that connects the low level file IO to the BTree algorithms.
*
* This file does not deal with structures in the file.
*/

#include "btree.h"
#include "btreei.h"

BTREE *openbtree(char *filename, BDIEVENTS *events)
{
	#ifdef DEBUG
		printf("OPENBTREE\n");
	#endif

	BTREE *newtree=NULL;
	if ((newtree = malloc(sizeof(BTREE))) == NULL){
		error_add(BT_MLCFLD);
		return NULL;
	}

	if ((newtree->file = openfile(filename,1,events)) == NULL)
		return NULL;

	newtree->file->_events=events;
	newtree->pool_count=0;

	newtree->root=NULL;
	if ((newtree->filelength=getfilesize(filename)) < 0 )
		return NULL;

	return newtree;
} 

void close_btree(BTREE *tree)
{
	/*Close file and flush contents*/
	flushfile(tree->file);
	closefile(tree->file);

	free(tree->file);

	/*free node pool*/
	register int i;
	for (i=0;i<tree->pool_count;i++)
		free(tree->nodepool[i].node);

	tree->pool_count = 0;

}

static int getroot(BTREE *tree)
{
	#ifdef DEBUG
		printf("GETROOT\n" );
	#endif

	if ((tree == NULL)||(tree->file == NULL)){
		error_add(BT_INVALID);
		return -1;
	}

	/*Get root location*/
	if (redirectto(tree->file,ROOT_PTR_LOC) == -1)
		return -1;
	
	byte locb[INT32];
	if (readfile(tree->file,locb,INT32) == -1)
		return -1;

	node_ptr rootloc = bintolong(locb,INT32);

	#ifdef DEBUG
		printf("\troot was found in: %ld\n", rootloc );
	#endif

	/*get root itself from file*/
	if ((tree->root = readnode(tree,rootloc)) == NULL)
		return -1;

	return 1;
}

int initialize(BTREE *tree)
{
	#ifdef DEBUG
		printf("INITIALIZE\n");
	#endif

	if ((tree == NULL)||(tree->file == NULL)){
		error_add(BT_INVALID);
		return -1;
	}

	return getroot(tree);
}

node_ptr createnode(BTREE *tree)
{
	#ifdef DEBUG
		printf("CREATENODE\n");
	#endif

	if ((tree == NULL)||(tree->file == NULL)){
		error_add(BT_INVALID);
		return NONE;
	}

	bnode *newnode;
	if (( newnode = (bnode *)malloc(sizeof(bnode))) == NULL)
	{
		error_add(ND_MLCFLD);
		return NONE;
	}

	newnode->n=0;

	/*node location*/
	node_ptr nodeloc = tree->filelength;

	/*actual node object*/
	newnode->start= nodeloc;
	tree->filelength += NODE_INFILE_SIZE; //UPDATE FILE SIZE

	newnode->children[0]=NONE;

	/*Write the newly created node to file*/
	if (writenode(tree, newnode) == -1)
		return NONE;

	/*remove the object from memory and return the node_ptr*/
	free(newnode);
	newnode=NULL;

	/*return node_ptr*/
	return nodeloc;
}

bnode *getnode(BTREE *tree, node_ptr nodeloc)
{
	#ifdef DEBUG
		printf("GETNODE: %ld\n", nodeloc);
	#endif

	if ((tree == NULL)||(tree->file == NULL)){
		error_add(BT_INVALID);
		return NULL;
	}

	int l_count = 0; /*stores the least invoke count*/

	if (tree->pool_count > 0)
		l_count = tree->nodepool[0].invk_count;

	int replace_index=0;	/*default location to put new node if the 
							desired node is not already in the pool*/

	/*Search for the node in nodepool*/
	int i;
	for (i=0;i<tree->pool_count;i++)
	{	if (tree->nodepool[i].location == nodeloc)
		{
			/*Node was found in the pool, return the node 
			object after incrementing the reference count and invoke count*/
			tree->nodepool[i].ref_count++;
			tree->nodepool[i].invk_count++;

			#ifdef DEBUG
				printf("\tNode was found in pool. Index: %d\n", i);
			#endif

			return tree->nodepool[i].node;
		}

		/*Find the node to replace (if necessory) while in the for loop*/
		/*We replace the lest invoked node*/
		if (tree->nodepool[i].invk_count < l_count)
		{
			l_count = tree->nodepool[i].invk_count;
			replace_index = i;
		}
	}

	/*Get node from file and put it into the appropriate place in the pool*/

	#ifdef DEBUG
		printf("\tNode was not found in the pool. Trying to read from file.\n");
		printf("\treplace_index = %d, least invk_count: %d\n",replace_index, l_count );
	#endif

	bnode* rnode;
	if ((rnode = readnode(tree, nodeloc)) == NULL)
		return NULL;

	/*Put the read node into the pool*/
	nodepooltype poolobj;

	poolobj.node = rnode;
	poolobj.location=nodeloc;
	poolobj.ref_count = 1;
	poolobj.invk_count=1;

	tree->nodepool[replace_index] = poolobj;

	tree->pool_count++;

	#ifdef DEBUG
		printf("\tPool count is now: %d\n", tree->pool_count);
	#endif


	return rnode;
}

int dispose(BTREE *tree, bnode *node)
{
	#ifdef DEBUG
		printf("DISPOSE: %ld\n", node->start );
	#endif

	if (tree == NULL){
		error_add(BT_INVALID);
		return -1;
	}

	int i;
	node_ptr nodeloc = node->start;

	for (i=0;i<tree->pool_count;i++)
		if (tree->nodepool[i].location == nodeloc)
		{
			#ifdef DEBUG
				printf("\tNode was found\n");
			#endif

			if (--tree->nodepool[i].ref_count <= 0)
			{
				if (writenode(tree, node) == -1)
					return -1;

				#ifdef DEBUG
					printf("\tReference count has reached zero. Freeing node from pool.\n");
				#endif

				free (node);
				tree->pool_count--;
			}
			return 1;
		}
	

	#ifdef DEBUG
		printf("\tNode was not found\n");
	#endif

	return 0;
}