#!/usr/bin/python

import sys

counts = {}

for line in sys.stdin:
    line = line.strip()
    if line in counts.keys():
        counts[line] += 1
    else:
        counts[line] = 1

for k in counts.keys():
    print '%s    %3d' % (k,counts[k])

