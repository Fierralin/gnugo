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

/* long options which have no short form */
enum {OPT_BOARDSIZE = 127,
	  OPT_HANDICAPSTONES,
	  OPT_COLOR,
	  OPT_KOMI,
	  OPT_CLOCK_TIME,
	  OPT_CLOCK_BYO_TIME,
	  OPT_CLOCK_BYO_PERIOD,
	  OPT_AUTOLEVEL,
	  OPT_MODE,
	  OPT_INFILE,
	  OPT_OUTFILE,
	  OPT_QUIET,
	  OPT_GTP_INPUT,
	  OPT_GTP_CONNECT,
	  OPT_GTP_LISTEN,
	  OPT_GTP_DUMP_COMMANDS,
	  OPT_GTP_INITIAL_ORIENTATION,
	  OPT_GTP_VERSION,
	  OPT_SHOWCOPYRIGHT,
	  OPT_REPLAY_GAME,
	  OPT_DECIDE_STRING,
	  OPT_DECIDE_CONNECTION,
	  OPT_DECIDE_OWL,
	  OPT_DECIDE_DRAGON_DATA,
	  OPT_DECIDE_SEMEAI,
	  OPT_DECIDE_SURROUNDED,
	  OPT_DECIDE_TACTICAL_SEMEAI,
	  OPT_DECIDE_ORACLE,
	  OPT_EXPERIMENTAL_SEMEAI,
	  OPT_EXPERIMENTAL_OWL_EXT,
	  OPT_SEMEAI_NODE_LIMIT,
	  OPT_EXPERIMENTAL_CONNECTIONS,
	  OPT_ALTERNATE_CONNECTIONS,
	  OPT_WITH_BREAK_IN,
	  OPT_WITHOUT_BREAK_IN,
	  OPT_COSMIC_GNUGO,
	  OPT_NO_COSMIC_GNUGO,
	  OPT_LARGE_SCALE,
	  OPT_NO_LARGE_SCALE,
	  OPT_OPTIONS,
	  OPT_STANDARD_SEMEAI,
	  OPT_STANDARD_CONNECTIONS,
	  OPT_PRINT_LEVELS,
	  OPT_DECIDE_POSITION,
	  OPT_DECIDE_EYE,
	  OPT_DECIDE_COMBINATION,
	  OPT_BRANCH_DEPTH,
	  OPT_BACKFILL2_DEPTH,
	  OPT_BREAK_CHAIN_DEPTH,
	  OPT_SUPERSTRING_DEPTH,
	  OPT_AA_DEPTH,
	  OPT_DEBUG_FLAGS,
	  OPT_OWL_DISTRUST,
	  OPT_OWL_BRANCH,
	  OPT_OWL_READING,
	  OPT_OWL_NODE_LIMIT,
	  OPT_NOFUSEKIDB,
	  OPT_NOFUSEKI,
	  OPT_NOJOSEKIDB,
	  OPT_LEVEL,
	  OPT_MIN_LEVEL,
	  OPT_MAX_LEVEL,
	  OPT_LIMIT_SEARCH,
	  OPT_SHOWTIME,
	  OPT_SHOWSCORE,
	  OPT_DEBUG_INFLUENCE,
	  OPT_SCORE,
	  OPT_PRINTSGF,
	  OPT_PROFILE_PATTERNS,
	  OPT_CHINESE_RULES,
	  OPT_OWL_THREATS,
	  OPT_NO_OWL_THREATS,
	  OPT_JAPANESE_RULES,
	  OPT_FORBID_SUICIDE,
	  OPT_ALLOW_SUICIDE,
	  OPT_ALLOW_ALL_SUICIDE,
	  OPT_SIMPLE_KO,
	  OPT_NO_KO,
	  OPT_POSITIONAL_SUPERKO,
	  OPT_SITUATIONAL_SUPERKO,
	  OPT_CAPTURE_ALL_DEAD,
	  OPT_PLAY_OUT_AFTERMATH,
	  OPT_MIRROR,
	  OPT_MIRROR_LIMIT,
	  OPT_METAMACHINE,
	  OPT_RESIGN_ALLOWED,
	  OPT_NEVER_RESIGN,
	  OPT_MONTE_CARLO,
	  OPT_MC_GAMES_PER_LEVEL,
	  OPT_MC_PATTERNS,
	  OPT_MC_LIST_PATTERNS,
	  OPT_MC_LOAD_PATTERNS
};

int
main(int argc, char *argv[]) {
  Gameinfo gameinfo;
  SGFTree sgftree;

  int i;

  char *infilename = "ccc.sgf";
  char *untilstring = NULL;
  char *scoringmode = NULL;
  int orientation = 0;

  float memory = (float) DEFAULT_MEMORY; /* Megabytes used for hash table. */

  int seed = 0;
  int seed_specified = 0;


  sgftree_clear(&sgftree);
  gameinfo_clear(&gameinfo);

  gg_optarg = "-l";
  scoringmode = gg_optarg; // no while switch here

  /* Start random number seed. */
  if (!seed_specified) seed = time(0);

  /* Initialize the GNU Go engine. */
  init_gnugo(memory, seed);

  sgftree_readfile(&sgftree, infilename);
  gameinfo_play_sgftree_rot(&gameinfo, &sgftree, untilstring, orientation);

  gameinfo.game_record = sgftree;

  load_and_score_sgf_file(&sgftree, &gameinfo, scoringmode);

  if (profile_patterns)
	report_pattern_profiling();

  sgfFreeNode(sgftree.root);

  return 0;
}  /* end main */

