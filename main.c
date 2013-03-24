#include "btree.h"

void printnodes(BTREE *tree, bnode *node);

int main(int argc, char **argv)
{
	BTREE *tree = openbtree(argv[1],NULL);
	if (tree == NULL){
		printf("Error: %s\n", serror[getlasterror()] );
		return 0;
	}

	switch (argv[2][0])
	{
		case 'f':
			if (format_btree_file(tree) == -1){
				printf("Error: %s\n", serror[getlasterror()] );
				return 0;
			}

			printf("%s\n", "Format was done.");
			return 0;
		case '1':
			if (initialize(tree) == -1)
			{
				printf("Error: %s\n", serror[getlasterror()] );
				perror("BTREE ERROR");
				return 0;
			}
			printf("File Initialized\n");

			insert_into_btree(tree,17);
			printf("----------------\n");
			insert_into_btree(tree,5);
			printf("----------------\n");
			insert_into_btree(tree,18);
			printf("----------------\n");
			insert_into_btree(tree,3);
			printf("----------------\n");

			break;
		case '2':
			if (initialize(tree) == -1)
			{
				printf("Error: %s\n", serror[getlasterror()] );
				perror("BTREE ERROR");
				return 0;
			}
			printf("File Initialized\n");

			if (insert_into_btree(tree,20) == -1)
				printf("Insertion failed----\n");

			break;
		case 'r':
			if (initialize(tree) == -1)
			{
				printf("Error: %s\n", serror[getlasterror()] );
				perror("BTREE ERROR");
				return 0;
			}
			printf("File Initialized\n");

			printf("root:");
			printnodes(tree, tree->root);

			break;
	};
	
	close_btree(tree);

	/*Errors*/
	printf("\n");
	int i;
	for (i=0;i<=eindex;i++)
		printf("%s\n", serror[i]);
	perror("System error:");

	return 0;
}

void printnodes(BTREE *tree, bnode *node)
{
	int x,y=0;

	printf("(%ld):", node->start );
	for (x = 0; x< node->n; x++)
		printf("%ld ", node->key[x]);

	if (node->isleaf)
		return;

	for (y=0;y < node->n+1; y++){
		printf("\n");
		printnodes(tree, getnode(tree, node->children[y]));
	}
}