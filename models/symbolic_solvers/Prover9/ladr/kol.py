#!/usr/bin/python

import sys

# print "Hello, world!"

MIN = 2

def kolmogorov(string):
   print "string=%s" % string
   total = 0
   length = len(string) - 2
   index = MIN
   print "Will check from index %d to %d." % (index,length)
   print "Length of string is %d." % length
   while index <= length:  # index of where the search begins
       maxlook = (length - index) + 2
       # don't check for occurences longer than head of string
       if maxlook > index:
           maxlook = index
       print "Checking at index: %d." % index
       print "Will check strings of length %d through %d." % (MIN,maxlook)
       flen = 0 # length of found redundancy
       # index of length of lookahead to check
       for window in range(MIN, maxlook+1):  # note: range(2,5) is [2,3,4]
           tocheck = string[ index : index+window ]  # note: "abcde"[1:3] is "bc"
           print "Want to know if %s occurs in pos 0 until %d." % (tocheck,index)
           ind = string.find(tocheck)
           if ind < index:
               flen = window

       if flen == 0:
           index += 1
       else:
           index += flen
           total += flen
       print "total=%d, index=%d, length=%d, MIN=%d" % (total,index,length,MIN)
   print "DONE.";
   print "TOTAL REDUNDANCY: %d" % total
   return float(total) / (length-1)

while 1:
   string = sys.stdin.readline()
   if not string:
       break;
   print str(kolmogorov(string))

