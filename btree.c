/*
* B-TREE PROJECT
* DATE: 17 MAR 2013
*
* This file contains the methods related to 
* basic operations on the btree data structure.
*
* Like inserting, deleting and splitting.
* This file does not deal with structures in the file and the management of
* node-pool.
*/

#include "btree.h"
#include "btreei.h"

static bnode *split(BTREE *tree, bnode *node, bnode *parent);
static bnode *createroot(BTREE *tree);

static bnode *createroot(BTREE *tree)
{
	#ifdef DEBUG
		printf("CREATEROOT\n");
	#endif

	if ((tree == NULL)||(tree->file == NULL)){
		error_add(BT_INVALID);
		return NULL;
	}

	bnode *newnode;
	if ((newnode = (bnode *)malloc(sizeof(bnode))) == NULL)
	{	
		error_add(ND_MLCFLD);
		return NULL;
	}

	newnode->n=0;
	newnode->start = tree->filelength;
	tree->filelength += NODE_INFILE_SIZE; //UPDATE FILE SIZE

	newnode->children[0]=NONE;

	/*write the newly created node into file in correct format*/
	if (writeroot(tree,newnode) == -1)
		return NULL;

	return newnode;	//return the node itself

}

static bnode *split(BTREE *tree, bnode *node, bnode *parent)
{
	#ifdef DEBUG
		printf("SPLIT: \n\tparent:%ld, node: %ld\n",parent==NULL?NONE:parent->start, node->start);
	#endif

	boolean isrootnew = FALSE;
	if (parent == NULL){
		if ((tree->root= parent = createroot(tree)) == NULL)
			return NULL;

		tree->root->isleaf=0;
		isrootnew=TRUE;
	}

	int median = NODE_ORDER/2;
	int value = node->key[median];

	#ifdef DEBUG
		printf("\tSplit: MEDIAN: %d, VALUE: %d\n", median, value );
	#endif

	/*INSERT MEDIAN INTO PARENT*/

	int i=parent->n-1;
	while((parent->key[i] > value) && (i > -1)){
		parent->key[i+1] = parent->key[i];
		parent->children[i+2] = parent->children[i+1];
		i--;
	}

	parent->key[i+1] = value;
	parent->n++;

	int p_index = i+1;

	/*Create new child*/

	node_ptr newnode= createnode(tree);
	bnode *newchild = getnode(tree, newnode);

	for (i=median+1; i<node->n; i++)
	{
		newchild->key[i-(median+1)] = node->key[i];
		newchild->children[i-(median+1)] = node->children[i];
	}

	newchild->children[i-(median+1)] = node->children[i];
	newchild->n = node->n - median - 1;
	newchild->isleaf = node->isleaf;
	node->n = median;

	parent->children[p_index] = node->start;
	parent->children[p_index+1] = newnode;

	/*clean up*/
	if (isrootnew){
		/*write the newly modified root*/
		#ifdef DEBUG
			printf("\tsplit: Writing new, modified root.\n");
		#endif

		if (writenode(tree, parent) == -1)
			return NULL; 
	}

	if (dispose(tree, newchild) == -1)
		return NULL;

	register int res;
	if ((res = dispose(tree, node) )== -1)
		return NULL;
	else if (res == 0)
		if (writenode(tree, node) == -1)
			return NULL;
		
	return parent;
}

int insert_into_btree(BTREE *tree, int value)
{
	#ifdef DEBUG
		printf("INSERT_INTO_BTREE: %d\n", value );
	#endif

	if ((tree == NULL)||(tree->file == NULL)){
		error_add(BT_INVALID);
		return -1;
	}

	boolean inserted= FALSE;

	bnode *pnode=NULL;
	bnode *cnode = tree->root;

	do
	{
		if (cnode->n==NODE_ORDER)
		{	
			/*Node is full, this split the node*/
			if ((cnode=split(tree,cnode,pnode)) == NULL)
				return -1;

			pnode=cnode;
		}
		else 
		{
			if (cnode->isleaf)
			{
				#ifdef DEBUG
					printf("Inserting--\n");
				#endif
					
				/*Appropriate leaf node was found, 
				so insert the value in correct place*/

				/*Note: leaf node contains no children. So only
				keys are inserted.*/

				int i=cnode->n-1;
				while((cnode->key[i] > value) && (i > -1)){
					cnode->key[i+1] = cnode->key[i];
					i--;
				}

				cnode->key[i+1] = value;
				cnode->n++;
				
				inserted=TRUE;

				#ifdef DEBUG
					printf("\tInserted.\n");
				#endif
			}
			else
			{
				/*Find the approriate child*/

				#ifdef DEBUG
					printf("\tSearching for correct node..inserting (%d)\n", value);
				#endif

				int i;
				for (i=cnode->n-1; i>-1 ;i--)
					if (cnode->key[i]<=value)
						break;

				node_ptr childloc = cnode->children[i+1];

				#ifdef DEBUG
					printf("\tChild selected: %ld\n", childloc);
				#endif

				/*Try disposing, useless nodes to free memory.*/

				/*If pnode is root, then dispose will fail, it that case
				* we will try to write the node manually. But otherwise
				* dispose should succeed.*/

				/*pnode is null by default*/
				if (pnode != NULL)
				{
					register int res;
					if ((res = dispose(tree, pnode) )== -1)
						return -1;
					else if (res == 0)
						if (writenode(tree, pnode) == -1)
							return -1;
				}

				pnode=cnode;
				if ((cnode= getnode(tree, childloc)) == NULL)
					return -1;
			}
		}
	}while(inserted!=1);

	if (writenode(tree, cnode) == -1)
		return -1;

	return 1;
}

