#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>

#define STRSIZE 258
#define DEFAULT_POPULATION_SIZE 100
#define DEFAULT_START_LEN 10
#define DEFAULT_POINT_PROB 1
#define DEFAULT_DUPLICATE_PROB 1
#define DEFAULT_DELETE_PROB 1
#define DEFAULT_TRANSPOSE_PROB 1
#define DEFAULT_TARGET_STRING "methinks it is like a weasel"

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
  probTable[NO_CHANGE] = 100 - probSum;
  probTable[POINT_CHANGE] = probTable[NO_CHANGE] + point;
  probTable[DUPLICATE] = probTable[POINT_CHANGE] + duplicate;
  probTable[DELETE] = probTable[DUPLICATE] + delete;
  probTable[TRANSPOSE] = probTable[DELETE] + transpose;
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
    "  -A str - alphabet for new strings (default is a-ZA-Z0-9,.?!)\n"
    "  -l n   - initial string size (default is %d)\n"
    "\n"
    "logging parameters\n"
    "  -L logfile - writes detailed logging information to logfile (default is no detailed logging)\n";

  if ( error != NULL && strcmp( error, "" ) != 0 )
    fprintf( stderr, "ERROR: %s\n", error );
    
  fprintf( stderr, msg, progName, progName, DEFAULT_POPULATION_SIZE, DEFAULT_TARGET_STRING, DEFAULT_START_LEN );
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

  if ( lw->score < rw->score )
    return -1;
  else if (lw->score > rw->score )
    return 1;
  else
    return 0;
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
  char alphabet[STRSIZE] = " abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "0123456789,.';:?!";
  char target[STRSIZE] = "Methinks it is like a weasel";
  struct weasel fittest = {"", 0, 0};
  size_t popSize = DEFAULT_POPULATION_SIZE;
  unsigned long generation = 0UL;
  int startLen = 5;
  int writeLog = 0;
  FILE *log = NULL;
  char logName[512] = "weasel.log";

  getParameters( argc, argv, &point, &duplicate, &delete, 
                 &transpose, &popSize, target, alphabet, &startLen,
                 &writeLog, logName );
  
  setupProbTable( probTable, point, duplicate, delete, transpose );

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

    printf( "%15s%15s%8s%s\n", "Generation", "Score", " ", "Value" );
    printf( "%15s%15s%8s%s\n", "----------", "-----", " ", "-----" );

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
        printf( "%15lu%15d%8s%s\n", population[0].generation, 
                                    population[0].score, " ", 
                                    population[0].str );
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
    printf( "%15lu%15d%8s%s\n", population[0].generation, 
                                population[0].score, " ", 
                                population[0].str );
    free( population );    
  }
  if ( writeLog )
    fclose( log );

  return 0;
}

