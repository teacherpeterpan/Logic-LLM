# LADR/apps.src

CC = gcc

# XFLAGS can be specified on the command line (see XFLAGS below)

CFLAGS = $(XFLAGS) -O -Wall
# CFLAGS = $(XFLAGS) -O6 -Wall
# CFLAGS = $(XFLAGS) -g  -O -Wall
# CFLAGS = $(XFLAGS) -g -O0 -Wall
# CFLAGS = $(XFLAGS) -pg -O -Wall
# CFLAGS = $(XFLAGS)  -Wall -pedantic

PROGRAMS = latfilter olfilter clausefilter idfilter renamer unfast clausetester rewriter isofilter0 isofilter isofilter2 dprofiles interpfilter upper-covers miniscope interpformat prooftrans mirror-flip perm3 sigtest directproof test_clause_eval test_complex complex gen_trc_defs

all: ladr apps install realclean

ladr:
	cd ../ladr && $(MAKE) libladr.a

clean:
	/bin/rm -f *.o

realclean:
	/bin/rm -f *.o $(PROGRAMS)

install:
	/bin/mv $(PROGRAMS) ../bin

tags:
	etags *.c ../ladr/*.c

apps: $(PROGRAMS)

test: test.o
	$(CC) $(CFLAGS) -o test test.o ../ladr/libladr.a

interpformat: interpformat.o
	$(CC) $(CFLAGS) -o interpformat interpformat.o ../ladr/libladr.a

prooftrans: prooftrans.o
	$(CC) $(CFLAGS) -o prooftrans prooftrans.o ../ladr/libladr.a

directproof: directproof.o
	$(CC) $(CFLAGS) -o directproof directproof.o ../ladr/libladr.a

test_clause_eval: test_clause_eval.o
	$(CC) $(CFLAGS) -o test_clause_eval test_clause_eval.o ../ladr/libladr.a

test_complex: test_complex.o
	$(CC) $(CFLAGS) -o test_complex test_complex.o ../ladr/libladr.a

complex: complex.o
	$(CC) $(CFLAGS) -o complex complex.o ../ladr/libladr.a

latfilter: latfilter.o
	$(CC) $(CFLAGS) -o latfilter latfilter.o ../ladr/libladr.a

olfilter: olfilter.o
	$(CC) $(CFLAGS) -o olfilter olfilter.o ../ladr/libladr.a

unfast: unfast.o
	$(CC) $(CFLAGS) -o unfast unfast.o ../ladr/libladr.a

upper-covers: upper-covers.o
	$(CC) $(CFLAGS) -o upper-covers upper-covers.o ../ladr/libladr.a

miniscope: miniscope.o
	$(CC) $(CFLAGS) -o miniscope miniscope.o ../ladr/libladr.a

isofilter0: isofilter0.o
	$(CC) $(CFLAGS) -o isofilter0 isofilter0.o ../ladr/libladr.a

isofilter: isofilter.o
	$(CC) $(CFLAGS) -o isofilter isofilter.o ../ladr/libladr.a

isofilter2: isofilter2.o
	$(CC) $(CFLAGS) -o isofilter2 isofilter2.o ../ladr/libladr.a

dprofiles: dprofiles.o
	$(CC) $(CFLAGS) -o dprofiles dprofiles.o ../ladr/libladr.a

renamer: renamer.o
	$(CC) $(CFLAGS) -o renamer renamer.o ../ladr/libladr.a

rewriter: rewriter.o
	$(CC) $(CFLAGS) -o rewriter rewriter.o ../ladr/libladr.a

idfilter: idfilter.o
	$(CC) $(CFLAGS) -o idfilter idfilter.o ../ladr/libladr.a

clausefilter: clausefilter.o
	$(CC) $(CFLAGS) -o clausefilter clausefilter.o ../ladr/libladr.a

interpfilter: interpfilter.o
	$(CC) $(CFLAGS) -o interpfilter interpfilter.o ../ladr/libladr.a

clausetester: clausetester.o
	$(CC) $(CFLAGS) -o clausetester clausetester.o ../ladr/libladr.a

mirror-flip: mirror-flip.o
	$(CC) $(CFLAGS) -o mirror-flip mirror-flip.o ../ladr/libladr.a

perm3: perm3.o
	$(CC) $(CFLAGS) -o perm3 perm3.o ../ladr/libladr.a

sigtest: sigtest.o
	$(CC) $(CFLAGS) -o sigtest sigtest.o ../ladr/libladr.a

rewriter2: rewriter2.o
	$(CC) $(CFLAGS) -o rewriter2 rewriter2.o ../ladr/libladr.a

gen_trc_defs: gen_trc_defs.o
	$(CC) $(CFLAGS) -o gen_trc_defs gen_trc_defs.o ../ladr/libladr.a





