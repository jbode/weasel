#!/usr/bin/python3

import random
import math
import optparse
from datetime import datetime

default_alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-.,!?'\" "
default_target = "methinks it is like a weasel"
default_size = 100
default_length = 10

default_delete = 1
default_transpose = 1
default_duplicate = 1
default_point = 1

alphabet = default_alphabet
target = default_target

delete = 100 - default_delete
transpose  = delete - default_transpose
duplicate = transpose - default_duplicate
point = duplicate - default_point
no_change = 95

#
# Create a random sequence of slen characters
# taken from the given alphabet
#
def randomString( slen, alpha ):
  r = []
  for i in range( slen ):
    r.append( random.choice( alpha ))
  s = ''.join( r )
  return s

#
# Make a mutated copy of the input string; returns the mutated
# string and the mutation rate.
#
def mutate( src, alpha ):
  tmp = []
  mutCount = 0
  for i in range( len( src ) ):
    action = random.randint(0,99)
    if action >= point and action < duplicate:
      tmp.append( random.choice( alpha ))
      mutCount += 1
    elif action >= duplicate and action < transpose:
      tmp.append( src[i] )
      tmp.append( src[i] )
      mutCount += 1
    elif action >= transpose and action < delete:
      if i < len( src ) - 1:
        tmp.append( src[i+1] )
      tmp.append( src[i] )
      mutCount += 1
    elif action >= delete:
      mutCount += 1
    else:
      tmp.append( src[i] )

  return ''.join( tmp ), float(mutCount) / float(len( src ))

#
# Comparison function for sorting the population
#
def cmpWeasel( w0, w1 ):
  return w0[2] - w1[2]

#
# Compute the score of the candidate string
#
def score( cand, targ ):
  clen = len( cand )
  tlen = len( targ )
  s = 0
  for i in range( min( clen, tlen ) ):
    s += abs( ord( cand[i] ) - ord( targ[i] ) )
  if clen < tlen:
    for i in range( clen, tlen ):
      s += ord( targ[i] )
  if tlen < clen:
    for i in range( tlen, clen ):
      s += ord( cand[i] );
  return s

#
# Main driver
#
def main():
  p = optparse.OptionParser()
  p.add_option( '--target', '-T', default=default_target, 
                help='string to match (default "' + default_target + '")' );
  p.add_option( '--alphabet', '-A', default=default_alphabet, 
                help='source alphabet (default "' + default_alphabet + '")' )
  p.add_option( '--size', '-S', default=default_size, help='population size (default ' + str(default_size) + ')' )
  p.add_option( '--point', '-p', default=default_point, help='chance of point change event (0 to 100, default ' + str(default_point) +')' )
  p.add_option( '--duplicate', '-d', default=default_duplicate, help='chance of duplicate event (0 to 100, default ' + str(default_duplicate) + ')' )
  p.add_option( '--delete', '-x', default=default_delete, help='chance of delete event (0 to 100, default ' + str(default_delete) + ')' )
  p.add_option( '--transpose', '-t', default=default_transpose, help='chance of transpose event (0 to 100, default ' + str(default_transpose) + ')' )
  p.add_option( '--length', '-l', default=default_length, help='initial string size (default ' + str(default_length) + ')' )

  options, arguments = p.parse_args()

  alphabet = options.alphabet
  target = options.target

  if all( elem in options.alphabet for elem in options.target ) == False:
    print( "Target contains characters not in the alphabet!")
    return -1;
    
  delete = 100 - int(options.delete)
  transpose = delete - int(options.transpose)
  duplicate = transpose - int(options.duplicate)
  point = duplicate - int(options.point)

  population = []
  generation = 0

  for i in range(int(options.size)):
    candidate = randomString( int(options.length), options.alphabet )
    population.append( (generation, candidate, score(candidate, options.target), 0 ))

  population.sort( key=lambda weasel: weasel[2] )

  print( "%15s%15s%15s%8s%s" % ("Generation", "Score", "Mut Rate", " ", "Value"))
  print( "%15s%15s%15s%8s%s" % ("----------", "-----", "--------", " ", "-----"))

  print( "%15d%15d%15.2f%8s%s" % (population[0][0], population[0][2], population[0][3], " ", population[0][1] ))

  bestFit = population[0][1]

  startTime = datetime.now()
  while population[0][2] != 0:
    generation += 1
    breeder_max = int(int(options.size) / 10)
    
    for x in range(breeder_max,int(options.size)):
      candidate, mutRate = mutate( population[x%breeder_max][1], options.alphabet )
      population[x] = (generation, candidate, score( candidate, options.target ), mutRate)
    population.sort( key=lambda weasel: weasel[2] )
    if population[0][1] != bestFit:
      bestFit = population[0][1]
      print( "%15d%15d%15.2f%8s%s" %( population[0][0], population[0][2], population[0][3], " ", population[0][1]))
  endTime = datetime.now()
  
  print( "Execution time: %s" %(endTime - startTime) )

if __name__ == '__main__':
  main()

