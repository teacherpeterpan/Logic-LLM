help:
	@echo See README.make

all:
	cd ladr         && $(MAKE) lib
	cd mace4.src    && $(MAKE) all
	cd provers.src  && $(MAKE) all
	cd apps.src     && $(MAKE) all
	/bin/cp -p utilities/* bin
	@echo ""
	@echo "**** Now try 'make test1'. ****"
	@echo ""

ladr lib:
	cd ladr         && $(MAKE) lib

test1:
	bin/prover9 -f prover9.examples/x2.in | bin/prooftrans parents_only
	@echo ""
	@echo "**** If you see a proof, prover9 is probably okay. ****"
	@echo "**** Next try 'make test2'. ****"
	@echo ""

test2:
	bin/mace4 -v0 -f mace4.examples/group2.in | bin/interpformat tabular
	@echo ""
	@echo "**** If you see a group table, mace4 is probably okay. ****"
	@echo "**** Next try 'make test3'. ****"
	@echo ""

test3:
	bin/mace4 -n3 -m -1 < apps.examples/qg.in | bin/interpformat | bin/isofilter
	@echo ""
	@echo "*** If you see 5 interpretations, the apps are probably okay. ***"
	@echo ""
	@echo "*** All of the programs are in ./bin, and they can be copied anywhere you like. ***"
	@echo ""

clean:	
	cd ladr             && $(MAKE) realclean
	cd apps.src         && $(MAKE) realclean
	cd mace4.src        && $(MAKE) realclean
	cd provers.src      && $(MAKE) realclean

realclean:
	$(MAKE) clean
	/bin/rm -f bin/*


# The following cleans up, then makes a .tar.gz file of the current
# directory, leaving it in the parent directory.  (Gnu make only.)

DIR = $(shell basename $(PWD))

dist:
	$(MAKE) realclean
	cd .. && tar cvf $(DIR).tar $(DIR)
	gzip -f ../$(DIR).tar
	ls -l ../$(DIR).tar.gz
