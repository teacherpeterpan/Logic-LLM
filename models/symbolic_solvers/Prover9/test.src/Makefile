# LADR/apps.src

CC = gcc

# XFLAGS can be specified on the command line (see XFLAGS below)

CFLAGS = $(XFLAGS) -O -Wall
# CFLAGS = $(XFLAGS) -O6 -Wall
# CFLAGS = $(XFLAGS) -g  -O -Wall
CFLAGS = $(XFLAGS) -g  -O0 -Wall
# CFLAGS = $(XFLAGS) -pg -O -Wall
# CFLAGS = $(XFLAGS)  -Wall -pedantic

PROGRAMS = avltest

all: ladr apps

ladr:
	make clean
	cd ../ladr && $(MAKE) libladr.a

clean:
	/bin/rm -f *.o

realclean:
	/bin/rm -f *.o $(PROGRAMS)

tags:
	etags *.c ../ladr/*.c

apps: $(PROGRAMS)

avltest: avltest.o
	$(CC) $(CFLAGS) -o avltest avltest.o ../ladr/libladr.a -lm

tptp_test: tptp_test.o
	$(CC) $(CFLAGS) -o tptp_test tptp_test.o ../ladr/libladr.a -lm

