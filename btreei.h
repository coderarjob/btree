#ifndef H_BTREEI_h
#define H_BTREEI_h

#define TRUE 1
#define FALSE 0

#define NODE_INFILE_SIZE 41
#define DEF_ROOT_LOC 7	/*Default location of the root itself*/
#define ROOT_PTR_LOC 3	/*Fixed location for the root pointer in the header*/

#define NDEBUG

/*Function for private use, within the library*/
int writeroot(BTREE *tree,bnode *node);
int writenode(BTREE *tree, bnode *node);
bnode *readnode(BTREE *tree, node_ptr node);
long getfilesize(char *name);

int dispose(BTREE *tree, bnode *node);
bnode *getnode(BTREE *tree, node_ptr nodeloc);
node_ptr createnode(BTREE *tree);

#endif
