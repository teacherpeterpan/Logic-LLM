/*  Copyright (C) 2006, 2007 William McCune

    This file is part of the LADR Deduction Library.

    The LADR Deduction Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    The LADR Deduction Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the LADR Deduction Library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifdef SOLO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#else
#include "complex.h"
#endif

/* Private definitions and types */

#ifdef SOLO
/*************
 *
 *   complexity1()
 *
 *   This is translated from some Matlab code I found:
 *
 *   FUNCTION: kolmogorov.m
 *   DATE: 9th Feb 2005
 *   AUTHOR: Stephen Faul (stephenf@rennes.ucc.ie)
 * 
 *   Function for estimating the Kolmogorov Complexity as per:
 *   "Easily Calculable Measure for the Complexity of Spatiotemporal Patterns"
 *   by F Kaspar and HG Schuster, Physical Review A, vol 36, num 2 pg 842
 *
 *   (WWM) I believe this is desiend for binary strings.
 *
 *************/

static
double complexity1(int *s, int n)
{
  /* NOTE: s is indexed from 1 .. n.  (not 0 .. n-1) */
  if (n == 1)
    return 1.0;  /* this case not in Faul's code */
  else {
    int c = 1;
    int l = 1;
    int i = 0;
    int k = 1;
    int kmax = 1;
    int stop = 0;
    while (!stop) {
      if (s[i+k] != s[l+k]) {
	kmax = IMAX(k, kmax);
	i++;
	if (i == l) {
	  c++;
	  /* This must be designed for binary strings: we jump over
	     (don't look at) the character following a match.  If binary,
	     we know what the skipped character is.
	  */
	  l += kmax;
	  if (l+1 > n)
	    stop = 1;
	  else {
	    i = 0;
	    k = 1;
	    kmax = 1;
	  }
	}
	else
	  k = 1;
      }
      else {
	k++;
	if (l + k > n) {
	  /* Faul's code increments c here, but that doesn't make sense to
	     me.  If incremented, abb has a different complexity from bba. */
	  /* c++;  */
	  stop = 1;
	}
      }
    }
    /* Normalize so that 0 < result <= 1.
       Faul's code normalizes: return c / (n / log2(n))
    */

    return ((double) c) / n;
  }
}  /* complexity1 */
#endif

/*************
 *
 *   complexity2() - a recoding of complexity1
 *
 *************/

static
double complexity2(int *s, int n)
{
  /* NOTE: s is indexed from 0 .. n-1. */
  int c = 1;
  int b = 1;
  int a, k, kmax;
  a = k = kmax = 0;
  while (b+k < n) {
    if (s[a+k] == s[b+k]) {
      k++;
    }
    else {
      kmax = IMAX(k, kmax);
      a++;
      if (a != b)
	k = 0;
      else {
	c++;
	b += (kmax+1);
	a = k = kmax = 0;
      }
    }
  }
  return ((double) c) / n;
}  /* complexity2 */

/*************
 *
 *   complexity3() - similar to complexity2, but it makes more sense to me.
 *
 *   complexity1 (also complexity2) gives the same answer for
 *     ABBA
 *     ABBC
 *   (I believe that's because they are designed for binary strings,
 *   where the problem doesn't arise.)
 *
 *   complexity3 is designed for arbitrary strings.
 *
 *************/

static
double complexity3(int *s, int n, int adjustment)
{
  /* NOTE: s is indexed from 0 .. n-1. */
  int c = 1;
  int b = 1;
  int x = 0;
  while (b < n) {
    int a = 0;
    int fmax = 0;
    while (a < b) {
      int f = 0;
      while (b+f < n && s[a+f] == s[b+f])
	f++;
      fmax = IMAX(f, fmax);
      a++;
    }
    b += IMAX(1,fmax);
    c++;
    if (fmax > 0)
      x += ((adjustment+1) * fmax) - adjustment;
  }
  return 1 - (((double) x) / (((adjustment+1) * (n - 1)) - adjustment));
}  /* complexity3 */

#ifndef SOLO

/*************
 *
 *   complex4_devar() - change variables to constants in a kludgey way
 *
 *************/

static
Term complex4_devar(Term t, int base)
{
  if (VARIABLE(t)) {
    Term new = get_rigid_term_dangerously(base + VARNUM(t), 0);
    free_term(t);
    return new;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = complex4_devar(ARG(t,i), base);
    return t;
  }
}  /* complex4_devar */

/*************
 *
 *   complex4_revar() - restore variables after call to complex4_devar
 *
 *************/

static
Term complex4_revar(Term t, int base)
{
  if (CONSTANT(t) && SYMNUM(t) >= base) {
    Term var = get_variable_term(SYMNUM(t) - base);
    free_term(t);
    return var;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = complex4_revar(ARG(t,i), base);
    return t;
  }
}  /* complex4_revar */

/*************
 *
 *   complex4_compare()
 *
 *************/

static
void complex4_compare(Term a, Term b, int *n)
{
  /* There should be no variables in a or b.   Recurse through a and b
     in paralle, incrementing *n for each node where they agree.  Don't
     look below nodes where they disagree. */
  if (SYMNUM(a) == SYMNUM(b)) {
    int i;
    (*n)++;
    for (i = 0; i < ARITY(a); i++)
      complex4_compare(ARG(a,i), ARG(b,i), n);
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
  /* Recurse through t, stopping when we get to s. */
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
  /* Traverse the term and call complex4_p2 for each. 
     We will consider all pairs of distinct subterms.  */
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
A tree-based complexity measure.  For each pair of (distinct) subterms,
compare, starting at the roots; increment the counter for each position
where they agree (do not look past positions that disagree).

The result must be between 0 and 1.  If there are n nodes in the term,
the only bound I know for the counter is n^3.  However, the worst cases
I can come up with are a bit over n^2.  Therefore, we'll divide the counter
by n^2, and if the result is >= 1, we'll make it .999.

We want more complex terms to be greater, so we'll subtract from 1.
*/

/* PUBLIC */
double complex4(Term t)
{
  double d;
  int count = 0;
  int size = symbol_count(t);  /* for changing vars to constants */
  int base = greatest_symnum()+1;  /* for changing vars to constants */

  /* For technical reasons having to do with sharing of variables,
     we have to temporarily change variables to constants. */

  t = complex4_devar(t, base);  /* change variables to constants */
  complex4_p1(t, t, &count);
  t = complex4_revar(t, base);  /* restore variables */

  d = ((double) count) / (size*size);

  /* make sure 0 < d < 1 */

  if (d >= 1.0)
    d = 0.999;
  else if (d == 0)
    d = 0.001;

  return 1.0 - d;
}  /* complex4 */

/*************
 *
 *   term_to_ints()
 *
 *************/

static
void term_to_ints(Term t, int *a, int *i)
{
  if (VARIABLE(t)){
    a[(*i)++] = -VARNUM(t);
  }
  else {
    int j;
    a[(*i)++] = SYMNUM(t);
    for (j = 0; j < ARITY(t); j++)
      term_to_ints(ARG(t,j), a, i);
  }
}  /* term_to_ints */

/*************
 *
 *   term_complexity()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
double term_complexity(Term t, int func, int adjustment)
{
  if (func == 4) {
    return complex4(t);
  }
  else {
    int n = symbol_count(t);
    int *a = malloc(n * sizeof(int));
    int i = 0;
    double x = 0.0;
    term_to_ints(t, a, &i);
    if (func == 2)
      x = complexity2(a, n);
    else if (func == 3)
      x = complexity3(a, n, adjustment);
    else
      fatal_error("term_complexity, bad func");
    free(a);
    return x;
  }
}  /* term_complexity */

/*************
 *
 *   clause_complexity()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
double clause_complexity(Literals lits, int func, int adjustment)
{
  Term t = lits_to_term(lits);   /* shallow */
  double x = term_complexity(t, func, adjustment);
  free_lits_to_term(t);          /* shallow */
  return x;
}  /* clause_complexity */

#endif  /* not SOLO */

#ifdef SOLO
int main(int argc, char **argv)
{
  char s[1000];
  int a[1001];
  char *t;
  int i, n;

  while (1) {
    /* printf("\nenter string: "); */
    t = fgets(s, 1000, stdin);
    if (t == NULL)
      exit(0);
    n = strlen(s);
    if (n > 0)
      s[n] = '\0';  /* get rid of newline */
    n--;

    for (i = 0; i < n; i++)
      a[i+1] = s[i];

    printf("complex1: %.5f\n", complexity1(a, n));
    printf("complex2: %.5f\n", complexity2(a+1, n));
    printf("complex3: %.5f\n", complexity3(a+1, n, 1));
  }
}
#endif  /* SOLO */

