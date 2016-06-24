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

//static void show_copyright(void);
//static void show_version(void);
//static void show_help(void);
//static void show_debug_help(void);
//static void show_debug_flags(void);

//static void socket_connect_to(const char *host_name, unsigned int port,
//				  FILE **input_file, FILE **output_file);
//static void socket_listen_at(const char *host_name, unsigned int port,
//				 FILE **input_file, FILE **output_file);
//static void (FILE *input_file, FILE *output_file);
//static void socket_stop_listening(FILE *input_file, FILE *output_file);


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

/* names of playing modes */

enum mode {
  MODE_UNKNOWN = 0,
  MODE_ASCII,
  MODE_GTP,
  MODE_GMP,
  MODE_SGMP,
  MODE_SGF,
  MODE_LOAD_AND_ANALYZE,
  MODE_LOAD_AND_SCORE,
  MODE_LOAD_AND_PRINT,
  MODE_SOLO,
  MODE_REPLAY,
  MODE_DECIDE_STRING,
  MODE_DECIDE_CONNECTION,
  MODE_DECIDE_OWL,
  MODE_DECIDE_DRAGON_DATA,
  MODE_DECIDE_SEMEAI,
  MODE_DECIDE_TACTICAL_SEMEAI,
  MODE_DECIDE_POSITION,
  MODE_DECIDE_EYE,
  MODE_DECIDE_COMBINATION,
  MODE_DECIDE_SURROUNDED,
  MODE_DECIDE_ORACLE
};


/* Definitions of the --long options. Final column is
 * either an OPT_ as defined in the enum above, or it
 * is the equivalent single-letter option.
 * It is useful to keep them in the same order as the
 * help string, for maintenance purposes only.
 */


int
main(int argc, char *argv[])
{
  Gameinfo gameinfo;
  SGFTree sgftree;

  int i;
  int mandated_color = EMPTY;
  enum mode playmode = MODE_UNKNOWN;
  int replay_color = EMPTY;

  char *infilename = "ccc.sgf";
  char *untilstring = NULL;
  char *scoringmode = NULL;
  char *outfile = NULL;
  char *outflags = NULL;
  char *gtpfile = NULL;
  char *gtp_dump_commands_file = NULL;
  int gtp_tcp_ip_mode = 0;
  char *gtp_tcp_ip_address = NULL;

  char *printsgffile = NULL;

  char decide_this[8];
  char *decide_that = NULL;
  char debuginfluence_move[4] = "\0";

  int benchmark = 0;  /* benchmarking mode (-b) */
  FILE *output_check;
  int orientation = 0;

  char mc_pattern_name[40] = "";
  char mc_pattern_filename[320] = "";

  float memory = (float) DEFAULT_MEMORY; /* Megabytes used for hash table. */

  /* If seed is zero, GNU Go will play a different game each time. If
   * it is set using -r, GNU Go will play the same game each time.
   * (Change seed to get a different game).
   */
  int seed = 0;
  int seed_specified = 0;

  int requested_boardsize = -1;

  sgftree_clear(&sgftree);
  gameinfo_clear(&gameinfo);

  gg_optarg = "-l";
  scoringmode = gg_optarg; // no while switch here

  fprintf(stderr, "scoringmode: %s.\n", scoringmode); ///////////////


  /* Start random number seed. */
  if (!seed_specified)
	seed = time(0);

  /* Initialize the GNU Go engine. */
  init_gnugo(memory, seed);

  /* Load Monte Carlo patterns if one has been specified. Either
   * choose one of the compiled in ones or load directly from a
   * database file.
   */
  if (strlen(mc_pattern_filename) > 0) { //------------
	if (!mc_load_patterns_from_db(mc_pattern_filename, NULL))
	  return EXIT_FAILURE;
  }
  else if (strlen(mc_pattern_name) > 0) { // ----------------
	if (!choose_mc_patterns(mc_pattern_name)) {
	  fprintf(stderr, "Unknown Monte Carlo pattern database name %s.\n",
		  mc_pattern_name);
	  fprintf(stderr, "Use \"--mc-list-patterns\" to list the available databases.\n");
	  return EXIT_FAILURE;
	}
  }

  /* Read the infile if there is one. Also play up the position. */
  //*
  if (infilename) {
	if (!sgftree_readfile(&sgftree, infilename)) {
	  fprintf(stderr, "Cannot open or parse '%s'\n", infilename);
	  exit(EXIT_FAILURE);
	}

	if (gameinfo_play_sgftree_rot(&gameinfo, &sgftree, untilstring,
				  orientation) == EMPTY) {
	  fprintf(stderr, "Cannot load '%s'\n", infilename);
	  exit(EXIT_FAILURE);
	}
  }
  //else
  /* Initialize and empty sgf tree if there was no infile. */
	//sgftreeCreateHeaderNode(&sgftree, board_size, komi, handicap); // --------------
  //sgftree_readfile(&sgftree, infilename);

  /* Set the game_record to be identical to the loaded one or the
   * newly created empty sgf tree.
   */
  gameinfo.game_record = sgftree;

  if (outfile && playmode != MODE_LOAD_AND_PRINT) { // ---------------
	output_check = fopen(outfile, "w");
	if (!output_check) {
	  fprintf(stderr, "Error: could not open '%s' for writing\n", outfile);
	  exit(EXIT_FAILURE);
	}
	fclose(output_check);
  }

  if (mandated_color != EMPTY)
		gameinfo.to_move = mandated_color;

	  if (!infilename) {
		fprintf(stderr, "gnugo: --score must be used with -l\n");
		exit(EXIT_FAILURE);
	  }
	  load_and_score_sgf_file(&sgftree, &gameinfo, scoringmode);

  if (profile_patterns)
	report_pattern_profiling();

  sgfFreeNode(sgftree.root);

  return 0;
}  /* end main */

