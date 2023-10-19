# LADR/mace4.src

CC = gcc

# XFLAGS can be specified on the command line (see XFLAGS below)

CFLAGS = $(XFLAGS) -O -Wall
# CFLAGS = $(XFLAGS) -O6 -Wall
# CFLAGS = $(XFLAGS) -g  -O -Wall
# CFLAGS = $(XFLAGS) -g  -Wall
# CFLAGS = $(XFLAGS) -pg -O -Wall
# CFLAGS = $(XFLAGS)  -Wall -pedantic

OBJECTS = estack.o util.o print.o syms.o ground.o arithmetic.o select.o \
	propagate.o mstate.o negpropindex.o negprop.o ordercells.o \
	commandline.o msearch.o

all: ladr lib mace4 install

libmace4.a: $(OBJECTS)
	$(AR) rs libmace4.a $(OBJECTS)

##############################################################################

lib libmace4:
	$(MAKE) libmace4.a

ladr:
	cd ../ladr && $(MAKE) libladr.a
	$(MAKE) clean

mace4: libmace4.a mace4.o $(OBJECTS)
	$(CC) $(CFLAGS) -o mace4 mace4.o libmace4.a ../ladr/libladr.a

$(OBJECTS): estack.h syms.h ground.h propagate.h mstate.h msearch.h

tags:
	etags *.c ../ladr/*.c

clean:
	/bin/rm -f *.o

realclean:
	/bin/rm -f *.o *.a mace4

install:
	/bin/mv mace4 ../bin
