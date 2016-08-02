#include "sgftree.h"
#include "sgf_properties.h"

int printSGFPRO(SGFProperty *pro) {
  SGFProperty *tmpnode;
  tmpnode = pro;
  while (tmpnode != NULL) {
	fprintf(stderr, "[%d|%s]", tmpnode->name, tmpnode->value);
	tmpnode = tmpnode->next;
  }
}

int printSGFNode(SGFNode *node) {
  fprintf(stderr, "[%p]NodeProp: ", node);

  if (node->props == NULL) fprintf(stdout, "-------------\n");
  else fprintf(stdout, "++++++++++++++++\n");

  printSGFPRO(node->props);
  if (node->child != NULL) {
	fprintf(stderr, "\nChild:");
	printSGFNode(node->child);
  }
  if (node->next != NULL) {
	fprintf(stderr, "\nNext:");
	printSGFNode(node->next);
  }
}

int printSGF(SGFTree *tree) {
  fprintf(stderr, "root[%p], lastnode[%p]\n"
	"SGFTREE:\n", tree->root, tree->lastnode);
  printSGFNode(tree->root);
}

int main(void) {
	SGFTree sgftree;
	// sgftree.root = readsgffile("form.SGF");
	fprintf(stdout, "before all$$$$$$$$$$\n");
	sgftree_clear(&sgftree);
	sgftree_readfile(&sgftree, "tmp.sgf");
	fprintf(stdout, "before output----------\n");
	printSGF(&sgftree);
}

