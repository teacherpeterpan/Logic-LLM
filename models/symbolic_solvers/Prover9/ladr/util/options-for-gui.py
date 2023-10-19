#!/usr/bin/python

import re, sys

print '    dependencies = ['

for line in sys.stdin.readlines():

    line = line.replace('automatic', 'auto')
    line = line.replace('TRUE', 'True')
    line = line.replace('FALSE', 'False')
    line = line.replace('INT_MAX', 'sys.maxint')
    line = line.replace('"', "'")
    
    (_,type,opt,a,b,c,_) = re.split('_dependency\(\w+->|,\s*\w+->|,\s*|\);\n|\s*', line)

    if (opt in ['default_parts', 'default_output'] or
        b in ['lrs_ticks', 'echo_input', 'quiet']):
        com = '#'
    else:
        com = ''
    
    if type in ['flag_flag', 'flag_parm', 'flag_stringparm']:
        print "%s        (('%s', %s), ('%s', %s))," % (com, opt, a, b, c)
    elif type == 'parm_parm':
        if c == 'True':
            v2 = "('multiply', %s)" % b
        else:
            v2 = b
        print "%s        (('%s', 'any'), ('%s', %s))," % (com, opt, a, v2)
    else:
        print '# not handled: line'
    
print '        ]'
