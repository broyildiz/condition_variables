BINARIES = prodcons

CC = gcc
CFLAGS = -Wall -g -c
LDLIBS = -lpthread

all: $(BINARIES)

clean:
	rm -f *.0 $(BINARIES)

prodcons: prodcons.o
prodcons.o: prodcons.c prodcons.h