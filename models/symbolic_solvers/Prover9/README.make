Compiling Prover9, Mace4, and related programs.
(Tested with Gnu make and GCC 3.3.3, 3.3.5, 4.0.0, 4.0.2., 4.1.1)

    % make all

This compiles 

    ladr library
    mace4 library
    mace4
    prover9              (and related provers)
    miscellaneous apps   (clausefilter, prooftrans, isofilter, etc)

then it moves all the programs and utilities to ./bin.

Quick tests:

   % make test1
   % make test2
   % make test3

Building a package for Debian.  The file libtoolize.patch
(contributed by Peter Collingbourne) can be used to update
the Makefiles so that they build and use a shared LADR
library.  
