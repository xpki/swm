
CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lX11

all: mywm

mywm: mywm.c config.h
	$(CC) $(CFLAGS) -o mywm mywm.c $(LDFLAGS)

clean:
	rm -f mywm