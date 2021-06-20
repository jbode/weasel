This project is my spin on Richard Dawkins' "WEASEL" program
described in "The Blind Watchmaker", which illustrates part
of the process that drives evolution - random variation plus
non-random cumulative selection.  

See the Wikipedia article at https://en.wikipedia.org/wiki/Weasel_program
for a basic introduction.

Instead of only modeling point changes in a fixed string, this
program models 5 different types of mutations:

  - Point change (a -> b): the character from the source string
                           is discarded, and a replacement character
			   is chosen at random from the character
			   set.

  - Deletion (ab -> b):    the character from the source string is
                           discarded.  

  - Duplication (a -> aa): the character from the source string is
                           copied twice to the destination string.

  - Transposition (ab -> ba): adjacent characters in the source string
                              are swapped in the destination string

  - No change (a -> a):    the character from the source string is
                           copied to the destination string.

Each type of mutation has a separate probability of occurring.
By default, there is a 1% chance of a point change, deletion,
duplication, or transposition event.  There is a 96% chance
of no change.  

Population size is fixed, with a default of 100.  Each generation
only the top 10% of the population is retained and allowed to
reproduce.  "Fitness" is determined by the difference in character
encodings between the candidate string and the target; the lower
the difference, the closer the match.  
```
USAGE: weasel -h or 
       weasel [mutation parameters] [search parameters] [logging parameters]

mutation parameters:
  -p n - point change probability
  -d n - duplication probability
  -x n - deletion probability
  -t n - transposition probability
"n" is an integer between 0 and 100 (default is 1)
mutation probabilities must sum to less than or equal to 100

search parameters:
  -S n   - population size; n must be an integer greater than 10 (default is 100)
  -T str - target string (default is "methinks it is like a weasel")
  -A str - alphabet for new strings (default is a-ZA-Z0-9,.?!)
  -l n   - initial string size (default is 10)

logging parameters
  -L logfile - writes detailed logging information to logfile (default is no detailed logging)

```
Note that detailed logging information can get large in a hurry, especially
with large population sizes.  

Build the C version using the supplied makefile:

    make weasel

The project is written in C, using C99 features.  
