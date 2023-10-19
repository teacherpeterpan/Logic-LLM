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

#include "../ladr/top_input.h"

#define PROGRAM_NAME    "complex"
#include "../VERSION_DATE.h"

/*************
 *
 *   complex4_compare()
 *
 *************/

static
void complex4_compare(Term a, Term b, int *n)
{
  //printf("compare:  "); p_term(a);
  //printf("       :  "); p_term(b);
  if (VARIABLE(a) && VARIABLE(b)) {
    if (VARNUM(a) == VARNUM(b))
      (*n)++;
  }
  else if (!VARIABLE(a) && !VARIABLE(b)) {
    if (SYMNUM(a) == SYMNUM(b)) {
      int i;
      (*n)++;
      for (i = 0; i < ARITY(a); i++)
	complex4_compare(ARG(a,i), ARG(b,i), n);
    }
  }
}  /* complex4_compare */

/*************
 *
 *   complex4_p2()
 *
 *************/

static
BOOL complex4_p2(Term s, Term t, int *n)
{
  if (s == t)
    return FALSE;
  else {
    int i;
    BOOL go;
    for (i = 0, go = TRUE; i < ARITY(t) && go; i++)
      go = complex4_p2(s, ARG(t,i), n);
    complex4_compare(s, t, n);
    return go;
  }
}  /* complex4_p2 */

/*************
 *
 *   complex4_p1()
 *
 *************/

static
void complex4_p1(Term s, Term t, int *n)
{
  BOOL dummy;
  int i;
  for (i = 0; i < ARITY(s); i++)
    complex4_p1(ARG(s,i), t, n);
  dummy = complex4_p2(s, t, n);
}  /* complex4_p1 */

/*************
 *
 *   complex4()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
double complex4(Term t)
{
  int n = 0;
  double d;
  int size = symbol_count(t);

  complex4_p1(t, t, &n);

  d = ((double) n) / (size*size);
  if (d >= 1.0)
    d = 0.999;

  printf("size= %2d n= %3d  d2=%.3f  ", size, n, d);
	 
  p_term(t);

  return d;
}  /* complex4 */

int main(int argc, char **argv)
{
  Term t;

  init_standard_ladr();
  
  /* Evaluate each clause on stdin. */

  t = read_term(stdin, stderr);

  while (t) {
    double d;
    /* term_set_variables(t, MAX_VARS); */
    d = complex4(t);
    zap_term(t);
    t = read_term(stdin, stderr);
  }
  exit(0);
}  /* main */

