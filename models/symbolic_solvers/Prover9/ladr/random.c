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

#include "random.h"

/*************
 *
 *   random_term()
 *
 *************/

/* DOCUMENTATION
This routine generates a random term, with depth <= max_depth,
and with subterms of arity <= 3.
The parameters [v, a0, a1, a2, a3] tells how many
variables (named v0,v1,...),
constants (named a0,a1,...),
unary (named g0,g1,...),
binary (named f0,f1,...),
ternary (named h0,h1,...),
symbols to select from.  For example,
<PRE>
random_term(3, 2, 1, 1, 0, 5)
</PRE>
asks for a term, of depth <= 5, with <= 3 variable, <= 2 constant,
<= 1 unary, <=1 binary, and 0 ternary symbols.
*/

/* PUBLIC */
Term random_term(int v, int a0, int a1, int a2, int a3,
		 int max_depth)
{
  int n, arity, j;
  char *s, symbol[MAX_NAME];

  if (max_depth == 0)
    n = rand() % (v+a0);
  else
    n = rand() % (v+a0+a1+a2+a3);

  if (n < v) {
    arity = -1;  /* variable */
    j = n;
  }
  else if (n < v+a0) {
    arity = 0;
    j = n - v;
  }
  else if (n < v+a0+a1) {
    arity = 1;
    j = n - (v+a0);
  }
  else if (n < v+a0+a1+a2) {
    arity = 2;
    j = n - (v+a0+a1);
  }
  else {
    arity = 3;
    j = n - (v+a0+a1+a2);
  }

  if (arity == -1) {
    return get_variable_term(j);
  }
  else {
    int i;
    Term t;

    switch (arity) {
    case 0: s = "a"; break;
    case 1: s = "g"; break;
    case 2: s = "f"; break;
    case 3: s = "h"; break;
    default: s = "?"; break;
    }

    sprintf(symbol, "%s%d", s, j);
    t = get_rigid_term(symbol, arity);
    for (i = 0; i < arity; i++)
      ARG(t,i) = random_term(v, a0, a1, a2, a3, max_depth-1);
    return t;
  }
}  /* random_term */

/*************
 *
 *   random_nonvariable_term()
 *
 *************/

/* DOCUMENTATION
This is like random_term(), except that the returned term will not
be a variable.  Subterms may be variables.
*/

/* PUBLIC */
Term random_nonvariable_term(int v, int a0, int a1, int a2, int a3,
			     int max_depth)
{
  Term t = random_term(v, a0, a1, a2, a3, max_depth);
  /* Let's hope this terminates! */
  while (VARIABLE(t)) {
    zap_term(t);
    t = random_term(v, a0, a1, a2, a3, max_depth);
  }
  return t;
}  /* random_nonvariable_term */

/*************
 *
 *   random_complex_term()
 *
 *************/

/* DOCUMENTATION
This is like random_term(), except that the returned term will not
be a variable or a constant.  Subterms may be variables or constants.
*/

/* PUBLIC */
Term random_complex_term(int v, int a0, int a1, int a2, int a3,
			     int max_depth)
{
  Term t = random_term(v, a0, a1, a2, a3, max_depth);
  /* Let's hope this terminates! */
  while (VARIABLE(t)|| CONSTANT(t)) {
    zap_term(t);
    t = random_term(v, a0, a1, a2, a3, max_depth);
  }
  return t;
}  /* random_complex_term */

/*************
 *
 *   random_path()
 *
 *************/

/* DOCUMENTATION
This routine returns a random-length list of random integers.
The range for the length is [1..length_max], and the range
for the values is [1..value_max].
*/

/* PUBLIC */
Ilist random_path(int length_max, int value_max)
{
  Ilist first, current, new;
  int length = (rand() % length_max) + 1;  /* 1 .. length_max */
  int i, value;

  first = current = NULL;
  for (i = 0; i < length; i++) {
    value = (rand() % value_max) + 1;
    new = get_ilist();
    new->i = value;
    new->next = NULL;
    if (first == NULL)
      first = new;
    else
      current->next = new;
    current = new;
  }
  return first;
}  /* random_path */

/*************
 *
 *   random_permutation()
 *
 *************/

/* DOCUMENTATION
This routine places a random permtation of [0..size-1] into the array a.
(The randomness is not very good.)
*/

/* PUBLIC */
void random_permutation(int *a, int size)
{
  int n, i, x;

  for (i = 0; i < size; i++)
    a[i] = -1;

  n = 0;
  while (n < size) {
    x = rand() % size;
    if (a[x] == -1)
      a[x] = n++;
    else {
      i = x+1;
      while (i < size && a[i] != -1)
	i++;
      if (i < size)
	a[i] = n++;
      else {
	i = x-1;
	while (i >= 0 && a[i] != -1)
	  i--;
	if (i < 0) {
	  fatal_error("random_permutation.");
	}
	a[i] = n++;
      }
    }
  }
}  /* random_permutation */

/*************
 *
 *   random_clause()
 *
 *************/

/* DOCUMENTATION
This routin builds and returns a random clause.
The arguments are like random_term(), with an extra
argument giving the maximum number of literals.
*/

/* PUBLIC */
Topform random_clause(int v, int a0, int a1, int a2, int a3,
		     int max_depth, int max_lits)
{
  Term t;
  Topform c;
  int i, sign;
  int n = (rand() % max_lits) + 1;  /* [1 .. max_lits] */
  
  c = get_topform();
  for (i = 0; i < n; i++) {
    sign = rand() % 2;
    t = random_complex_term(v, a0, a1, a2, a3, max_depth);
    c->literals = append_literal(c->literals, new_literal(sign, t));
  }
  return c;
}  /* random_clause */

/*************
 *
 *   random_op_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term random_op_term(int depth)
{
  static int initialized = 0;
  static int symnum[13];

  if (!initialized) {
    symnum[0] = str_to_sn("a", 0);
    symnum[1] = str_to_sn("->", 2);
    symnum[2] = str_to_sn("|", 2);
    symnum[3] = str_to_sn("&", 2);
    symnum[4] = str_to_sn("~", 1);
    symnum[5] = str_to_sn("=", 2);
    symnum[6] = str_to_sn("+", 2);
    symnum[7] = str_to_sn("*", 2);
    symnum[8] = str_to_sn("\'", 1);
    symnum[9] = str_to_sn("$cons", 2);
    symnum[10] = str_to_sn("$nil", 0);
    symnum[11] = str_to_sn("$quantified", 1);
    symnum[12] = str_to_sn("f", 2);
    initialized = 1;
  }

  if (depth == 0)
    return get_rigid_term("b", 0);
  else {
    Term t;
    int arity, i, sn;
    
    sn = symnum[rand() % 13];
    arity = sn_to_arity(sn);

    if (is_symbol(sn, "$quantified", 1)) {
      Term q = get_rigid_term("$quantified", 1);

      Term a = get_rigid_term("all", 0);
      Term x = get_rigid_term("x", 0);
      Term r = random_op_term(depth-1);

      Term t = get_nil_term();

      t = listterm_cons(r, t);
      t = listterm_cons(x, t);
      t = listterm_cons(a, t);
      
      ARG(q,0) = t;
      return q;
    }
    else {

      t = get_rigid_term(sn_to_str(sn), arity);
      for (i = 0; i < arity; i++)
	ARG(t,i) = random_op_term(depth-1);
      return t;
    }
  }
}  /* random_op_term */
