#include "gnugo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_UNISTD_H
/* For isatty(). */
#include <unistd.h>
#else
#include <io.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "liberty.h"

#include "gg-getopt.h"
#include "gg_utils.h"
#include "winsocket.h"

#include "interface.h"
#include "sgftree.h"
#include "random.h"

int printSGFPRO(SGFProperty *pro) {
	SGFProperty *tmpnode;
	tmpnode = pro;
	while (tmpnode != NULL) {
		fprintf(stderr, "[%d|%s]", tmpnode->name, tmpnode->value);
		tmpnode = tmpnode->next;
	}
}

int printNode(SGFNode *node) {
	 fprintf(stderr, "[%p]NodeProp: ", node);
	 printSGFPRO(node->props);
	 if (node->child != NULL) {
		 fprintf(stderr, "\nChild:");
		 printNode(node->child);
	 }
	 if (node->next != NULL) {
		 fprintf(stderr, "\nNext:");
		 printNode(node->next);
	 }
}

int printSGF(SGFTree *tree) {
	fprintf(stderr, "root[%p], lastnode[%p]\n"
					"SGFTREE:\n", tree->root, tree->lastnode);	
	printNode(tree->root);
}

int printGameinfo(Gameinfo *gameinfo) {
	fprintf(stderr, "\nGameinfo\nhandicap[%d]\n"
					"tomove[%d]\n"
					"computer[%d]; \n", gameinfo->handicap,
			gameinfo->to_move,
			gameinfo->computer_player);
}

int printBoard() {
  int tmpi, tmpj;
  for (tmpi = 0; tmpi < 19; tmpi++) {
    for (tmpj = 0; tmpj < 19; tmpj++) {
      fprintf(stderr, "%u|", board[tmpi * 20 + tmpj]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "-------------------------\n");
}

int
main(int argc, char *argv[]) {
  Gameinfo gameinfo;
  SGFTree sgftree;

  char *infilename = "/root/github/gnugo/interface/ttt.sgf";
  char *untilstring = NULL;
  int orientation = 0;

  float memory = (float) DEFAULT_MEMORY; /* Megabytes used for hash table. */

  int seed = 0;
  int seed_specified = 0;

  sgftree_clear(&sgftree);
  gameinfo_clear(&gameinfo);

  /* Start random number seed. */
  if (!seed_specified) seed = time(0);

  /* Initialize the GNU Go engine. */
  init_gnugo(memory, seed);

  printBoard();
  sgftree_readfile(&sgftree, infilename);
  printBoard();
  gameinfo_play_sgftree_rot(&gameinfo, &sgftree, untilstring, orientation);
  printBoard();

  gameinfo.game_record = sgftree;
  
  fprintf(stderr, "==========================\n");
  //printSGF(&sgftree);
//  printGameinfo(&gameinfo);

  printBoard();
  load_and_score_sgf_file_newnew();
  printBoard();

  sgfFreeNode(sgftree.root);

  return 0;
}  /* end main */

