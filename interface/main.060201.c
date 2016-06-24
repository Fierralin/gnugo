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

static void show_copyright(void);
static void show_version(void);
static void show_help(void);
static void show_debug_help(void);
static void show_debug_flags(void);

static void socket_connect_to(const char *host_name, unsigned int port,
				  FILE **input_file, FILE **output_file);
static void socket_listen_at(const char *host_name, unsigned int port,
				 FILE **input_file, FILE **output_file);
static void socket_close_connection(FILE *input_file, FILE *output_file);
static void socket_stop_listening(FILE *input_file, FILE *output_file);


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
  /*
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
  else
  /* Initialize and empty sgf tree if there was no infile. */
	//sgftreeCreateHeaderNode(&sgftree, board_size, komi, handicap); // --------------
  sgftree_readfile(&sgftree, infilename);

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


static void
show_version(void)
{
  printf("GNU Go %s\n", VERSION);
}

#define USAGE "__"

#define USAGE1 "__"

#define USAGE2 "__"

#define COPYRIGHT "__"

/* USAGE_DEBUG Split in half because of VC limit on constant string
 * length of 2048 characters!*/
#define USAGE_DEBUG "__"

#define USAGE_DEBUG2 "__"


/*
 * Since the maximum string length is 2048 bytes in VC++ we
 * split the help string.
 */
static void
show_help(void) {
  printf(USAGE, DEFAULT_LEVEL);
  printf(USAGE1, MIN_BOARD, MAX_BOARD, MAX_HANDICAP);
  printf(USAGE2, DEFAULT_MEMORY <= 0 ? reading_cache_default_size() :
	 (float) DEFAULT_MEMORY);
}


static void show_debug_help(void) {
  set_depth_values(DEFAULT_LEVEL,0);
  printf(USAGE_DEBUG USAGE_DEBUG2,
	 DEFAULT_LEVEL, depth, backfill_depth, fourlib_depth, ko_depth, branch_depth,
	 backfill2_depth, break_chain_depth, superstring_depth, aa_depth,
	 owl_distrust_depth, owl_branch_depth,
	 owl_reading_depth, owl_node_limit, semeai_node_limit);
}

static void show_debug_flags(void) {
  printf(DEBUG_FLAGS);
}

static void show_copyright(void) {
  printf(COPYRIGHT);
}


#ifdef ENABLE_SOCKET_SUPPORT


#if !defined(_WIN32) && !defined(_WIN32_WCE)

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define closesocket	close
#define init_sockets()

#else	/* on Windows */


#include <winsocket.h>


static void
init_sockets(void)
{
  WSADATA data;
  WORD version = MAKEWORD(1, 1);

  if (WSAStartup(version, &data) != NO_ERROR) {
	fprintf(stderr, "WSAStartup() failed with error %d\n", WSAGetLastError());
	exit(EXIT_FAILURE);
  }
}


#endif	/* on Windows */


static void
socket_connect_to(const char *host_name, unsigned int port,
		  FILE **input_file, FILE **output_file)
{
  struct sockaddr_in address;
  int connection_socket;
  struct hostent *host_data;
  char **address_pointer;

  init_sockets();

  if (!host_name)
	host_name = "127.0.0.1";

  host_data = gethostbyname(host_name);
  if (!host_data
	  || host_data->h_addrtype != AF_INET
	  || host_data->h_length != sizeof address.sin_addr) {
	fprintf(stderr, "Failed to resolve host name `%s'\n", host_name);
	exit(EXIT_FAILURE);
  }

  connection_socket = socket(PF_INET, SOCK_STREAM, 0);
  if (connection_socket == -1) {
	fprintf(stderr, "Unexpected error: failed to create a socket\n");
	exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_port = htons((unsigned short) port);

  for (address_pointer = host_data->h_addr_list; *address_pointer;
	   address_pointer++) {
	memcpy(&address.sin_addr, *address_pointer, sizeof address.sin_addr);
	if (connect(connection_socket, (struct sockaddr *) &address,
		sizeof address) != -1)
	  break;
  }

  if (! *address_pointer) {
	fprintf(stderr, "Failed to connect to %s:%u\n", host_data->h_name, port);
	closesocket(connection_socket);
	exit(EXIT_FAILURE);
  }

#if !USE_WINDOWS_SOCKET_CLUDGE

  *input_file  = fdopen(connection_socket, "r");
  *output_file = fdopen(dup(connection_socket), "w");

#else	/* USE_WINDOWS_SOCKET_CLUDGE */

  winsocket_activate(connection_socket);

  *input_file  = NULL;
  *output_file = NULL;

#endif	/* USE_WINDOWS_SOCKET_CLUDGE */
}


static void
socket_listen_at(const char *host_name, unsigned int port,
		 FILE **input_file, FILE **output_file)
{
  struct sockaddr_in address;
  int listening_socket;
  int connection_socket;

  init_sockets();

  if (host_name) {
	struct hostent *host_data;

	host_data = gethostbyname(host_name);
	if (!host_data
	|| host_data->h_addrtype != AF_INET
	|| host_data->h_length != sizeof address.sin_addr) {
	  fprintf(stderr, "Failed to resolve host name `%s'\n", host_name);
	  exit(EXIT_FAILURE);
	}

	host_name = host_data->h_name;
	memcpy(&address.sin_addr, host_data->h_addr_list[0],
	   sizeof address.sin_addr);
  }
  else
	address.sin_addr.s_addr = htonl(INADDR_ANY);

  listening_socket = socket(PF_INET, SOCK_STREAM, 0);
  if (listening_socket == -1) {
	fprintf(stderr, "Unexpected error: failed to create a socket\n");
	exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_port = htons((unsigned short) port);

  if (verbose) {
	if (host_name) {
	  fprintf(stderr, "Waiting for a connection on %s:%u...\n",
		  host_name, port);
	}
	else
	  fprintf(stderr, "Waiting for a connection on port %u...\n", port);
  }

  if (bind(listening_socket,
	   (struct sockaddr *) &address, sizeof address) == -1
	  || listen(listening_socket, 0) == -1
	  || (connection_socket = accept(listening_socket, NULL, NULL)) == -1) {
	if (host_name)
	  fprintf(stderr, "Failed to listen on %s:%u\n", host_name, port);
	else
	  fprintf(stderr, "Failed to listen on port %u\n", port);

	closesocket(listening_socket);
	exit(EXIT_FAILURE);
  }

  closesocket(listening_socket);

#if !USE_WINDOWS_SOCKET_CLUDGE

  *input_file  = fdopen(connection_socket, "r");
  *output_file = fdopen(dup(connection_socket), "w");

#else	/* USE_WINDOWS_SOCKET_CLUDGE */

  winsocket_activate(connection_socket);

  *input_file  = NULL;
  *output_file = NULL;

#endif	/* USE_WINDOWS_SOCKET_CLUDGE */
}


static void
socket_close_connection(FILE *input_file, FILE *output_file)
{
  /* When connecting, we close the socket first. */
  fclose(input_file);
  fclose(output_file);
}


static void
socket_stop_listening(FILE *input_file, FILE *output_file)
{
  int buffer[0x1000];

  if (verbose)
	fprintf(stderr, "Waiting for the client to disconnect...\n");

  /* When listening, we wait for the client to disconnect first.
   * Otherwise, socket doesn't get released properly.
   */
  do
	fread(buffer, sizeof buffer, 1, input_file);
  while (!feof(input_file));

  fclose(input_file);
  fclose(output_file);
}


#else  /* not ENABLE_SOCKET_SUPPORT */


static void socket_connect_to(const char *host_name, unsigned int port,
		  FILE **input_file, FILE **output_file)
{
  UNUSED(host_name);
  UNUSED(port);
  UNUSED(input_file);
  UNUSED(output_file);

  fprintf(stderr, "GNU Go was compiled without socket support, unable to connect\n");
  exit(EXIT_FAILURE);
}


static void
socket_listen_at(const char *host_name, unsigned int port,
		 FILE **input_file, FILE **output_file)
{
  UNUSED(host_name);
  UNUSED(port);
  UNUSED(input_file);
  UNUSED(output_file);

  fprintf(stderr, "GNU Go was compiled without socket support, unable to listen\n");
  exit(EXIT_FAILURE);
}


static void
socket_close_connection(FILE *input_file, FILE *output_file)
{
  UNUSED(input_file);
  UNUSED(output_file);
}


static void
socket_stop_listening(FILE *input_file, FILE *output_file)
{
  UNUSED(input_file);
  UNUSED(output_file);
}


#endif	/* not ENABLE_SOCKET_SUPPORT */

