#!/usr/bin/perl
use strict;
use warnings;

#print "Hello, world!\n";

my $MIN = 2;

open(INFILE, "/dev/stdin") or die "Can't open the input file...\n";
while (<INFILE>) {
#   print "% Here's the line: $_";
  my $total = 0;
  my $string = $_; # foobarfoobar
  my $length = (length ($string)) - 2;
  my $index = $MIN;
#  print "Will check from index $index to $length.\n";
#  print "Length of string is $length.\n";
  while($index <= $length) { # index of where the search begins
     my $maxlook = ($length - $index) + 2;
     if ($maxlook > $index) { $maxlook = $index; }# don't check for occurences longer than head of string
#     print "Checking at index: $index.\n";
#     print "Will check strings of length $MIN through $maxlook.\n";
     my $flen = 0; # length of found redundancy
     for(my $window = $MIN; $window <= $maxlook; $window++) { # index of length of lookahead to check
        my $tocheck = substr( $string, $index, $window );
#         print "Want to know if $tocheck occurs in positions 0 until $index.\n";
        my $ind = index($string, $tocheck, 0);
        if( $ind < $index ) {$flen = $window; }
     } # end lookahead buffer loop
     if($flen == 0) { $index++; }
     else           { $index = $index + $flen; $total = $total + $flen }
#      print "total: $total\n";
#      print "index: $index\n";
#      print "length: $length\n";
#      print "MIN: $MIN\n";
  } # end outer while loop
#   print "DONE.\n";
  my $kolmogorov = ($total / ($length-1));
#   print "TOTAL REDUNDANCY: $total\n";
  print "$kolmogorov\n"

} # end of while loop

close(INFILE);
