/*  Copyright (C) 2006, 2007 William McCune

    This file is part of the LADR Deduction Library.

    The LADR Deduction Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License,
    version 2.

    The LADR Deduction Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the LADR Deduction Library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef TP_FASTPARSE_H
#define TP_FASTPARSE_H

#include "topform.h"

/* INTRODUCTION
This package is meant for use when there are a great number
of terms to be read and written.  I wrote it when I was
looking for a single axiom for lattice theory and was
filtering trillions (really) of equations.
<P>
The ordinary parsing is very general and rather slow.
<P>
This package reads and writes prefix terms, without parentheses
commas, or spaces.  Each symbol is one character.
Each term to be read starts on a new line and ends with a period.
The user has to declare the arities of symbols
by calling fast_set_symbol().
<P>
Here's an example to read lattice equations in fast-parse form and
write them in ordinary form.
<PRE>
int main(int argc, char **argv)
{
  Term t;
  fast_set_symbol('=', 2);
  fast_set_symbol('m', 2);
  fast_set_symbol('j', 2);
  t = fast_read_term(stdin, stderr);
  while (t != NULL) {
    fwrite_term_nl(stdout, t);
    zap_term(t);
    t = fast_read_term(stdin, stderr);
  }
  exit(0);
}
</PRE>
<P>
There are also routines for reading and writing fast-parse clauses.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from fastparse.c */

void fast_set_symbol(char c, int arity);

void fast_set_defaults(void);

Term fast_read_term(FILE *fin, FILE *fout);

void fast_fwrite_term_nl(FILE *fp, Term t);

Topform fast_read_clause(FILE *fin, FILE *fout);

void fast_fwrite_clause(FILE *fp, Topform c);

#endif  /* conditional compilation of whole file */
