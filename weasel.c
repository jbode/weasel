#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <getopt.h>

#define STRSIZE                 258
#define DEFAULT_POPULATION_SIZE 100
#define DEFAULT_START_LEN        10
#define DEFAULT_POINT_PROB        1
#define DEFAULT_DUPLICATE_PROB    1
#define DEFAULT_DELETE_PROB       1
#define DEFAULT_TRANSPOSE_PROB    1
#define DEFAULT_TARGET_STRING   "methinks it is like a weasel"
#define DEFAULT_ALPHABET        " abcdefghijklmnopqrstuvwxyz" \
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
			        "0123456789.,?!\\'"

/**
 * Possible mutation events.  
 */
enum MutationEvent {
    NO_CHANGE,          // a  -> a 
    FIRST_EVENT = NO_CHANGE,
    POINT_CHANGE,       // a  -> b
    DUPLICATE,          // a  -> aa
    DELETE,             // ab -> b
    TRANSPOSE,          // ab -> ba
    LAST_EVENT = TRANSPOSE
};

/**
 * Return a random value in the range [min,max]
 */
int randInRange( int min, int max )
{
  double scale = 1.0 / RAND_MAX;
  double range = max - min + 1;
  return min + (int) (scale * range * rand() );
}

/**
 * Write a random sequence of len characters to target
 */
void randomString( char *target, size_t len, const char *alphabet )
{
  size_t asize = strlen( alphabet );
  for ( size_t i = 0; i < len-1; i++ )
    *target++ = alphabet[randInRange(0,asize-1)];
  *target = 0;
}

/**
 * Compute the score of the candidate string
 */
int score( const char *candidate, const char *target )
{
  int finalScore = 0;
  while ( *candidate && *target )
  {
    int tmp = *candidate++ - *target++;
    if ( tmp < 0 )
      tmp = -tmp;
    finalScore += tmp;
  }
  while ( *candidate )
    finalScore += *candidate++;
  while( *target )
    finalScore += *target++;

  return finalScore;
}

/**
 * Set up the mutation event probability table
 */
void setupProbTable( int *probTable, int point, int duplicate, int delete, 
                     int transpose )
{
  int probSum = point + duplicate + delete + transpose;
  probTable[NO_CHANGE]    = 100 - probSum;
  probTable[POINT_CHANGE] = probTable[NO_CHANGE] + point;
  probTable[DUPLICATE]    = probTable[POINT_CHANGE] + duplicate;
  probTable[DELETE]       = probTable[DUPLICATE] + delete;
  probTable[TRANSPOSE]    = probTable[DELETE] + transpose;
}

/**
 * Get the next mutation event, based on that event's probability
 */
enum MutationEvent getMutationEvent( int *probTable )
{
  int roll = randInRange(1,100);
  for ( enum MutationEvent e = FIRST_EVENT; e <= LAST_EVENT; e++ )
  {
    if ( roll < probTable[e] )
      return e;
  }
  return NO_CHANGE;
}
  
/**
 * Make a mutated copy of the input string; returns the mutation rate for 
 * that string
 */
double mutate( const char *src, char *dst, int *probTable, 
                const char *alphabet )
{
  size_t alen = strlen( alphabet );
  int chars = 0;
  int mutCount = 0;

  while ( *src )
  {
    chars++;
    switch ( getMutationEvent( probTable ) )
    {
      case NO_CHANGE: 
        *dst++ = *src++; 
        break;

      case POINT_CHANGE: 
        *dst++ = alphabet[randInRange(0, alen-1)]; 
        src++;
        mutCount++; 
        break;

      case DUPLICATE: 
        *dst++ = *src; 
        *dst++ = *src++;
        mutCount++; 
        break;

      case DELETE: 
        src++; 
        mutCount++;
        break;

      case TRANSPOSE:
        if ( *(src + 1) )
        {
          *dst++ = *(src + 1);
          *dst++ = *src;
          src += 2;
          mutCount++;
        }
        break;
    }
  }
  *dst = 0;
  return (double) mutCount / (double) chars;
}

/**
 * Display usage statement
 */
void usage( const char *progName, const char *error )
{
  const char *msg =
    "USAGE: %s -h or \n"
    "       %s [mutation parameters] [search parameters] [logging parameters]\n"
    "\n"
    "mutation parameters:\n"
    "  -p n - point change probability\n"
    "  -d n - duplication probability\n"
    "  -x n - deletion probability\n"
    "  -t n - transposition probability\n"
    "\"n\" is an integer between 0 and 100 (default is 1)\n"
    "mutation probabilities must sum to less than or equal to 100\n"
    "\n"
    "search parameters:\n"
    "  -S n   - population size; n must be an integer greater than 10 (default is %d)\n"
    "  -T str - target string (default is \"%s\")\n"
    "  -A str - alphabet for new strings (default is \"%s\")\n"
    "  -l n   - initial string size (default is %d)\n"
    "\n"
    "logging parameters\n"
    "  -L logfile - writes detailed logging information to logfile (default is no detailed logging)\n";

  if ( error != NULL && strcmp( error, "" ) != 0 )
    fprintf( stderr, "ERROR: %s\n", error );
    
  fprintf( stderr, msg, progName, progName, DEFAULT_POPULATION_SIZE, DEFAULT_TARGET_STRING, DEFAULT_ALPHABET, DEFAULT_START_LEN );
  exit( 0 );
}

/**
 * Process command line parameters
 */
void getParameters( int argc, char **argv, int *point, int *duplicate, 
                     int *delete, int *transpose, size_t *popSize, 
                     char *target, char *alphabet, int *len, 
                     int *log, char *logFileName )
{
  int opt;

  while ( (opt = getopt(argc, argv, "hp:d:x:t:S:T:A:l:L:")) != -1 )
  {
    switch ( opt )
    {
      char *chk;
      case 'h':
        usage( argv[0], NULL );
        exit(0);
        break;

      case 'p':
        *point = (int) strtol( optarg, &chk, 10 );
        if ( ( !isspace( *chk ) && *chk != 0 ) || ( *point < 0 || *point > 100 ))
          usage( argv[0], "invalid point change argument" );
        break;

      case 'd':
        *duplicate = (int) strtol( optarg, &chk, 10 );
        if ( ( !isspace( *chk ) && *chk != 0 ) || ( *duplicate < 0 || *duplicate > 100 ))
          usage( argv[0], "invalid duplicate argument" );
        break;

      case 'x':
        *delete = (int) strtol( optarg, &chk, 10 );
        if ( ( !isspace( *chk ) && *chk != 0 ) || ( *delete < 0 || *delete > 100 ))
          usage( argv[0], "invalid delete argument" );
        break;

      case 't':
        *transpose = (int) strtol( optarg, &chk, 10 );
        if ( ( !isspace( *chk ) && *chk != 0 ) || ( *transpose < 0 || *transpose > 100 ))
          usage( argv[0], "invalid transpose argument" );
        break;

      case 'S':
        *popSize = (size_t) strtoul( optarg, &chk, 10 );
        if ( ( !isspace( *chk ) && *chk != 0 ) || *popSize < 10 )
          usage( argv[0], "invalid population size" );
        break;

      case 'T':
        strcpy( target, optarg );
        break;

      case 'A':
        strcpy( alphabet, optarg );
        break;

      case 'l':
        *len = (int) strtol( optarg, &chk, 10 );
        if ( !isspace( *chk ) && *chk != 0 )
          usage( argv[0], "invalid string length" );
        break;

      case 'L':
        *log = 1;
        strcpy( logFileName, optarg );
        break;
    }
  }

  if ( *point + *duplicate +*delete + *transpose > 100 )
  {
    usage( argv[0], "The sum of all mutation probabilities must be <= 100" );
  }
}

void getParametersString( const char *str, int *point, int *duplicate, int *delete, int *transpose,
                          size_t *popSize, char *target, char *alphabet, int *len )
{
  fprintf( stderr, "checking for query string\n" );

  if ( str )
  {
    char *tmp = malloc( strlen( str ) + 1 );
    strcpy( tmp, str );

    char *keyvals[8] = {NULL};
    size_t i = 0;

    keyvals[0] = strtok( tmp, "&" );
    while ( i < 8 && keyvals[i] )
      keyvals[++i] = strtok( NULL, "&" );

    for ( i = 0; i < 8 && keyvals[i]; i++ )
    {
      char *key = strtok( keyvals[i], "=" );
      char *value = strtok( NULL, "=" );

      fprintf( stderr, "key = %s, value = %s\n", key, value );

      if ( !strcmp( key, "target" ) )
      {
        char *nxt = value;
        while ( (nxt = strchr( nxt, '+' ) ) )
          *nxt = ' ';
        strcpy( target, value );
      }
      else if ( !strcmp( key, "alphabet" ) )
      {
        char *nxt = value;
        while ( ( nxt = strchr( nxt, '+' ) ) )
          *nxt = ' ';
        strcpy( alphabet, value );
      }
      else if ( !strcmp( key, "point" ) )
      {
        *point = (int) strtol( value, NULL, 0 );
      }
      else if ( !strcmp( key, "duplicate" ) )
      {
        *duplicate = (int) strtol( value, NULL, 0 );
      }
      else if ( !strcmp( key, "delete" ) )
      {
        *delete = (int) strtol( value, NULL, 0 );
      }      
      else if ( !strcmp( key, "transpose" ) )
      {
        *transpose = (int) strtol( value, NULL, 0 );
      }
      else if ( !strcmp( key, "popSize" ) )
      {
        *popSize = (size_t) strtoul( value, NULL, 0 );
      }
      else if ( !strcmp( key, "startLen" ) )
      {
        *len = (int) strtol( value, NULL, 0 );
      }
    }
    free( tmp );
  }
  else
  {
    fprintf( stderr, "No inputs\n" );
  }
}

/**
 * Data type for each weasel
 */
struct weasel {
  char str[STRSIZE];
  int score;
  unsigned long generation;
  double mutRate;
};

/**
 * Compare two weasels; called by qsort
 */
int cmpWeasel( const void *l, const void *r )
{
  const struct weasel *lw = l;
  const struct weasel *rw = r;

  return lw->score - rw->score;
}

/**
 * Write progress to a log file on disk.
 */
void progress( struct weasel *population, size_t popSize, FILE *log, 
                int width )
{
  size_t breedPopSize = popSize / 10;
  size_t column = 0;
 
  fprintf( log, "\nGeneration: %lu\n\n", population[breedPopSize].generation);
  fprintf( log, "Breeding population:\n");
  column = 0;
  for ( size_t i = 0; i < breedPopSize; i++ )
  {
    fprintf( log, "%4zu: %7.4f %-*.*s%s", i, population[i].mutRate, width, 
      width, population[i].str, ++column % 3 == 0 ? "\n" : "    " );
  }
  fprintf( log, "\n\nProgeny:\n" );
  column = 0;
  for ( size_t i = breedPopSize; i < popSize; i++  )
  {
    fprintf( log, "%4zu: %7.4f %-*.*s%s", i, population[i].mutRate, width, 
      width, population[i].str, ++column % 3 == 0 ? "\n" : "    " );
  }
  fputc( '\n', log );
}
  
int main( int argc, char **argv )
{
  srand( time( NULL ) );
  int point = DEFAULT_POINT_PROB;
  int duplicate = DEFAULT_DUPLICATE_PROB;
  int delete = DEFAULT_DELETE_PROB;
  int transpose = DEFAULT_TRANSPOSE_PROB;
  int probTable[] = { 96, 97, 98, 99, 100 };
  char alphabet[STRSIZE] = DEFAULT_ALPHABET;
  char target[STRSIZE] = DEFAULT_TARGET_STRING;
  struct weasel fittest = {"", 0, 0};
  size_t popSize = DEFAULT_POPULATION_SIZE;
  unsigned long generation = 0UL;
  int startLen = 5;
  int writeLog = 0;
  FILE *log = NULL;
  char logName[512] = "weasel.log";

  if ( getenv( "GATEWAY_INTERFACE" ) != NULL && strcmp( getenv( "GATEWAY_INTERFACE" ), "" ) != 0 )
  {
    /**
     * We were invoked by a Web client through the Common Gateway Interface, so we need to
     * create an HTML form for input.
     */
    printf( "Content-type: text/html\r\n\r\n" );
    printf( "<!DOCTYPE HTML><html><head><title>Pop the Weasel!!!</title>" );
    printf( "<link rel=\"stylesheet\" href=\"../weasel.css\"></head>" );
    printf( "<body><p>This is my particular spin on Dawkins' WEASEL program as descibed in <em>The Blind Watchmaker</em></p>" );
    printf( "<form action=\"./weasel\" method=\"post\">" );
    printf( "<label for=\"target\">Target string:</label><input type=\"text\" name=\"target\" value=\"%s\"><br>", DEFAULT_TARGET_STRING );
    printf( "<label for=\"alphabet\">Alphabet:</label><input type=\"text\" name=\"alphabet\" value=\"%s\"><br>", DEFAULT_ALPHABET );
    printf( "<label for=\"point\">Point change probability:</label><input type=\"text\" name=\"point\" value=\"%d\"><br>", DEFAULT_POINT_PROB );
    printf( "<label for=\"duplicate\">Duplication probability</label><input type=\"text\" name=\"duplicate\" value=\"%d\"><br>", DEFAULT_DUPLICATE_PROB );
    printf( "<label for=\"delete\">Deletion probability</label><input type=\"text\" name=\"delete\" value=\"%d\"><br>", DEFAULT_DELETE_PROB );
    printf( "<label for=\"transpose\">Transposition probability</label><input type=\"text\" name=\"transpose\" value=\"%d\"><br>", DEFAULT_TRANSPOSE_PROB );
    printf( "<label for=\"popSize\">Population size</label><input type=\"text\" name=\"popSize\" value=\"%d\"><br>", DEFAULT_POPULATION_SIZE );
    printf( "<label for=\"startLen\">Start string length</label><input type=\"text\" name=\"startLen\" value=\"%d\"><br>", DEFAULT_START_LEN );
    printf( "<input type=\"submit\" name=\"submitbutton\" id=\"submitbutton\" value=\"Pop That Weasel!\">" );
    printf( "</form><br><br>" );
  }

  char *method = getenv( "REQUEST_METHOD" );

  if ( method && strcmp( method, "GET" ) == 0 )
  {
    /**
     * We were invoked via an HTTP GET method, so we pull the input parameters
     * from the QUERY_STRING environment variable.
     */
    fprintf( stderr, "calling getParametersString with QUERY_STRING\n" );

    getParametersString( getenv( "QUERY_STRING" ), &point, &duplicate, &delete, &transpose, 
                        &popSize, target, alphabet, &startLen );
  }
  else if ( method && strcmp( method, "POST" ) == 0 )
  {
    /**
     * We were invoked via an HTTP POST method, so we pull the input parameters
     * from the standard input streaqm.
     */
    fprintf( stderr, "calling getParametersString with POST input\n" );
    char input[2048];
    if ( fgets( input, sizeof input, stdin ) )
    {
      getParametersString( input, &point, &duplicate, &delete, &transpose,
                           &popSize, target, alphabet, &startLen );
    }
  }
  else
  {    
    /**
     * We were invoked from the command line in a terminal session
     */
    getParameters( argc, argv, &point, &duplicate, &delete, 
                   &transpose, &popSize, target, alphabet, &startLen,
                   &writeLog, logName );
  }

  /**
   * Make sure the target doesn't contain any characters that are not
   * in the source character set.
   */
  if ( strspn( target, alphabet ) < strlen( target ) )
  {
    usage( argv[0], "Target string contains characters not in the alphabet!" );
    exit( -1 );
  }

  /**
   * Initialize our probability table.
   */
  setupProbTable( probTable, point, duplicate, delete, transpose );

  /**
   * Allocate memory for our Weasel population.
   */
  struct weasel *population = malloc( sizeof *population * popSize );
  if ( population )
  {
    /**
     * Set up the progress log file
     */
    if ( writeLog )
    {
      log = fopen( logName, "w" );
      if ( !log )
      {
        fprintf( stderr, "Could not open log file %s; will not log progress",
                 logName );
        writeLog = 0;
      }
    }

    /**
     * Generate the initial population of random strings and score them
     */
    for ( size_t i = 0; i < popSize; i++ )
    {
      randomString( population[i].str, startLen, alphabet );
      population[i].score = score( population[i].str, target );
      population[i].generation = generation;
      population[i].mutRate = 0.0;
    }

    /**
     * Sort the population by score
     */
    qsort( population, popSize, sizeof *population, cmpWeasel );

    if ( getenv( "GATEWAY_INTERFACE" ) && strcmp( getenv( "GATEWAY_INTERFACE" ), "" ) != 0 )
    {
      printf( "<table><tr><th>Generation</th><th>Score</th><th>Value</th></tr>" );
    }
    else
    {
      printf( "%15s%15s%8s%s\n", "Generation", "Score", " ", "Value" );
      printf( "%15s%15s%8s%s\n", "----------", "-----", " ", "-----" );
    }
    /**
     * As long as we don't have a match (minimum score is not 0), take the top
     * 10% of the population and make mutated copies of them, writing over the
     * bottom 90% of the population.  Display the individual with the lowest
     * score (will only be displayed once).
     */
    while ( population[0].score > 0 )
    {
      generation++;

      if ( strcmp( fittest.str, population[0].str ))
      {
        if ( getenv( "GATEWAY_INTERFACE" ) && strcmp( getenv( "GATEWAY_INTERFACE" ), "" ) != 0 )
        {
          printf( "<tr><td>%lu</td><td>%d</td><td>%s</td></tr>", population[0].generation,
            population[0].score, population[0].str );
        }
        else
        {
          printf( "%15lu%15d%8s%s\n", population[0].generation, 
                                      population[0].score, " ", 
                                      population[0].str );
        }
        fittest = population[0];
      }

      size_t breedPopSize = popSize / 10;
      for ( size_t i = breedPopSize; i < popSize; i++ )
      {
        population[i].mutRate = mutate( population[i%breedPopSize].str, 
                                        population[i].str, probTable, 
                                        alphabet );
        population[i].score = score( population[i].str, target );
        population[i].generation = generation;
      }
      if ( writeLog )
        progress( population, popSize, log, strlen( target ) );

      qsort( population, popSize, sizeof *population, cmpWeasel );
    }
    if ( getenv( "GATEWAY_INTERFACE" ) && strcmp( getenv( "GATEWAY_INTERFACE" ), "" ) != 0 )
    {
      printf( "<tr><td>%lu</td><td>%d</td><td>%s</td></tr></table>", population[0].generation,
        population[0].score, population[0].str );
    }
    else
    {
      printf( "%15lu%15d%8s%s\n", population[0].generation, 
                                  population[0].score, " ", 
                                  population[0].str );
    }
    free( population );    

    if ( getenv( "GATEWAY_INTERFACE" ) && strcmp( getenv( "GATEWAY_INTERFACE" ), "" ) != 0 )
    {
      printf( "</body></html>" );
    }
  }
  if ( writeLog )
    fclose( log );

  return 0;
}

