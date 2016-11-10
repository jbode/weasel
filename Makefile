CC=gcc
CFLAGS=-std=c99 -c -g -Wall -Werror -Wa,-aldh=$(basename $@).lst 
LDFLAGS=-lm
weasel : weasel.o
clean:
	rm -rf weasel *.o
