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

/*
  This file is for experimental weighting functions.  To add a new one,
  say test2, look for test1 in this file and do something similar with
  test2.  In the prover9 input, you can then have a weighting rule
  with expressions like call(test2, x) on the right side.
 */

/* This file is set up so that it can be compiled and linked
   by itself for testing.  To do so, use a command like

   % gcc -DSOLO -o weighttest weight2.c

   See "main" at the end of the file.
*/

#ifdef SOLO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#else
#include "weight2.h"
#include "parse.h"
#endif

/*************
 *
 *   char_array_ident()  - identical arrays of characters
 *
 *************/

static
int char_array_ident(char *s, char *t, int n)
{
  int i;
  for (i = 0; i < n; i++)
    if (s[i] != t[i])
      return 0;
  return 1;
}  /* char_array_ident */

/*************
 *
 *   char_array_find() - this is like looking for a substring, except
 *                       that the objects are arrays of characters
 *
 *************/

static
int char_array_find(char *b, int nb, char *a, int na)
{
  if (nb <= na) {
    int i;
    for (i = 0; i <= na - nb; i++) {
      if (char_array_ident(a+i, b, nb))
	return i;  /* b is a subsequence of a at position i */
    }
  }
  return -1;  /* b is not a subsequence */
}  /* char_array_find */

/*************
 *
 *   complexity_1()
 *
 *************/

static
double complexity_1(char *s)
{
  /* This is based on Zac's original perl code.  It's not quite right. */
  int length = strlen(s);
  int min = 2;
  int index = min;
  int total = 0;

  if (length < 3)
    return 0.0;
  
  while (index <= length-2) {  /* index of where the search begins */
    int flen = 0;  /* length of found redundancy */
    int maxlook = (length - index);
    int window;
    /* don't check for occurences longer than head of string */
    if (maxlook > index)
      maxlook = index;
    /* printf("checking at index %d\n", index); */
    /* printf("will chack strings of length %d through %d.\n", min, maxlook); */
    for (window = min; window < maxlook+1; window++) {
      char *tocheck = s + index;
      int ind = char_array_find(tocheck, window, s, length);
       
      /* printf("Want to know if %s occurs in pos 0 until %d.\n", tocheck,index); */
      /* ind = string.find(tocheck); */

      if (ind < index)
	flen = window;
    }

    if (flen == 0)
      index += 1;
    else {
      index += flen;
      total += flen;
    }
  }
  /* printf("total=%d,index=%d,length=%d,MIN=%d\n",total,index,length,min); */
  /* printf("TOTAL REDUNDANCY: %d\n", total); */
  return ((double) total) / (length-2);

}  /* complexity_1 */

#ifndef SOLO

/*************
 *
 *   test1()
 *
 *************/

static
double test1(Term t)
{
  char *s = term_to_string(t);  /* remember to free(s) below */

  /* The complexity function returns a double in the range 0.0--1.0. */
   
  double c = complexity_1(s);
  /* printf("s=%s, complexity=%d:  ", s, c); p_term(t); */
  free(s);
  return c;
}  /* test1 */

/*************
 *
 *   call_weight()
 *
 *************/

/* DOCUMENTATION
This is the entry function, called by the general weighting code.
It receives a string (the name of the function), and a term
(the term to be weighed).
*/

/* PUBLIC */
double call_weight(char *func, Term t)
{
  if (str_ident(func, "test1"))
    return test1(t);
  else {
    fatal_error("call_weight, unknown function");
    return 0;  /* to appease the compiler */
  }
}  /* call_weight */

#endif

#ifdef SOLO
int main(int argc, char **argv)
{
  char s[1000];
  char *t;

  while (1) {

    /* printf("\nenter string: "); */
    t = fgets(s, 1000, stdin);

    if (t == NULL)
      exit(0);

    if (strlen(s) > 0)
      s[strlen(s)-1] = '\0';  /* get rid of newline */

    /* printf("\nchecking %s\n", s); */

    printf("%.5f\n", 1 - complexity_1(s));

  }
}
#endif
