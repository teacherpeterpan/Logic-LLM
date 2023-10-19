OUTPUTS = qg.out3 qg.iso3 MOL-cand.238  OL.8 OL.8.out OL.6 BA-sheffer.out \
	lattice-sax.rewritten meet-join-equations.out mjc01s-equations.out \
	OL.8.upper-covers

test:
	$(MAKE) backup
	./run-all

clean:
	/bin/rm -rf $(OUTPUTS) backup

backup:
	/bin/rm -rf backup
	mkdir backup && mv $(OUTPUTS) backup
