#!/usr/bin/python

import random
import math
import optparse

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

def randomString( slen ):
  r = []
  for i in xrange( slen ):
    r.append( random.choice( alphabet ))
  s = ''.join( r )
  return s

def mutate( src ):
  tmp = []
  mutCount = 0
  for i in xrange( len( src ) ):
    action = random.randint(0,99)
    if action >= point and action < duplicate:
      tmp.append( random.choice( alphabet ))
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

def cmpWeasel( w0, w1 ):
  return w0[2] - w1[2]

def score( cand, targ ):
  clen = len( cand )
  tlen = len( targ )
  s = 0
  for i in xrange( min( clen, tlen ) ):
    s += abs( ord( cand[i] ) - ord( targ[i] ) )
  if clen < tlen:
    for i in xrange( clen, tlen ):
      s += ord( targ[i] )
  if tlen < clen:
    for i in xrange( tlen, clen ):
      s += ord( cand[i] );
  return s

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

  delete = 100 - int(options.delete)
  transpose = delete - int(options.transpose)
  duplicate = transpose - int(options.duplicate)
  point = duplicate - int(options.point)

  population = []
  generation = 0

  for i in xrange(int(options.size)):
    candidate = randomString( int(options.length) )
    population.append( (generation, candidate, score(candidate, target), 0 ))

  population.sort( cmpWeasel )

  print "%15s%15s%15s%8s%s" % ("Generation", "Score", "Mut Rate", " ", "Value")
  print "%15s%15s%15s%8s%s" % ("----------", "-----", "--------", " ", "-----")

  print "%15d%15d%15.2f%8s%s" % (population[0][0], population[0][2], population[0][3], " ", population[0][1] )

  bestFit = population[0][1]

  while population[0][2] != 0:
    generation += 1
    breeder_max = int(int(options.size) / 10)
    
    for x in xrange(breeder_max,int(options.size)):
      candidate, mutRate = mutate( population[x%breeder_max][1] )
      population[x] = (generation, candidate, score( candidate, target ), mutRate)
    population.sort( cmpWeasel )
    if population[0][1] != bestFit:
      bestFit = population[0][1]
      print "%15d%15d%15.2f%8s%s" %( population[0][0], population[0][2], population[0][3], " ", population[0][1])

if __name__ == '__main__':
  main()

