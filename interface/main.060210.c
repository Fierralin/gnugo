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

int
main(int argc, char *argv[]) {
  Gameinfo gameinfo;
  SGFTree sgftree;

  char *infilename = "ccc.sgf";
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

  sgftree_readfile(&sgftree, infilename);
  gameinfo_play_sgftree_rot(&gameinfo, &sgftree, untilstring, orientation);

  gameinfo.game_record = sgftree;

  load_and_score_sgf_file(&sgftree, &gameinfo, "-1");


  sgfFreeNode(sgftree.root);

  return 0;
}  /* end main */

