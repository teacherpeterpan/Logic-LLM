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

#include "features.h"

/* Private definitions and types */

static Ilist Feature_symbols;  /* list of featured symbols (symnums)*/

/* The following are work arrays, indexed by symnum, used for calculating
   the features of a clause.  They are allocated by init_features() and
   left in place throughout the process.
*/

static int Work_size;         /* size of following arrays */
static int *Pos_occurrences;
static int *Neg_occurrences;
static int *Pos_maxdepth;
static int *Neg_maxdepth;

/*************
 *
 *   init_features()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_features(Ilist fsyms, Ilist rsyms)
{
  Work_size = greatest_symnum() + 1;

  /* printf("init_features: size=%d\n", Work_size); */

  Feature_symbols = ilist_cat(ilist_copy(rsyms), ilist_copy(fsyms));
  Pos_occurrences = calloc(Work_size, sizeof(int));
  Neg_occurrences = calloc(Work_size, sizeof(int));
  Pos_maxdepth    = calloc(Work_size, sizeof(int));
  Neg_maxdepth    = calloc(Work_size, sizeof(int));
}  /* init_features */

/*************
 *
 *   fill_in_arrays()
 *
 *************/

static
void fill_in_arrays(Term t, BOOL sign, int depth)
{
  if (!VARIABLE(t)) {
    int sn = SYMNUM(t);
    int i;
    if (sn >= Work_size) {
      /* Assume it's a symbol that was added after the start of
	 the search.  If we ignore symbols in features, we may get
	 less discrimination, but all answers should be returned.
       */
      ;  
    }
    else if (sign) {
      Pos_occurrences[sn]++;
      Pos_maxdepth[sn] = IMAX(depth, Pos_maxdepth[sn]);
    }
    else {
      Neg_occurrences[sn]++;
      Neg_maxdepth[sn] = IMAX(depth, Neg_maxdepth[sn]);
    }
    for (i = 0; i < ARITY(t); i++)
      fill_in_arrays(ARG(t,i), sign, depth+1);
  }
}  /* fill_in_arrays */

/*************
 *
 *   features()
 *
 *************/

/* DOCUMENTATION
Given a clause, build the feature vector.

Features:
  positive literals
  negative literals
  foreach relation symbol
     positive occurrences
     negative occurrences
  foreach function symbol
     positive occurrences
     negative occurrences
     positive maxdepth
     negative maxdepth
*/

/* PUBLIC */
Ilist features(Literals lits)
{
  Ilist f = NULL;
  Ilist p;
  Literals lit;

  /* Build it backwards, then reverse it. */

  f = ilist_prepend(f, positive_literals(lits));
  f = ilist_prepend(f, negative_literals(lits));

  for (lit = lits; lit; lit = lit->next) {
    fill_in_arrays(lit->atom, lit->sign, 0);
  }

  for (p = Feature_symbols; p; p = p->next) {
    f = ilist_prepend(f, Pos_occurrences[p->i]);
    f = ilist_prepend(f, Neg_occurrences[p->i]);

    if (function_symbol(p->i)) {
      f = ilist_prepend(f, Pos_maxdepth[p->i]);
      f = ilist_prepend(f, Neg_maxdepth[p->i]);
    }

    Pos_occurrences[p->i] = 0;
    Neg_occurrences[p->i] = 0;
    Pos_maxdepth[p->i] = 0;
    Neg_maxdepth[p->i] = 0;
  }
  f = reverse_ilist(f);
#if 0
  printf("Features for clause "); f_clause(c);
  p_features(f);
#endif
  return f;
}  /* features */

/*************
 *
 *   feature_length()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int feature_length(void)
{
  int n = 2;  /* pos lits, neg lits */
  Ilist p;
  for (p = Feature_symbols; p; p = p->next) {
    n += 2;
    if (function_symbol(p->i))
      n += 2;
  }
  return n;
}  /* feature_length */

/*************
 *
 *   features_less_or_equal()
 *
 *************/

/* DOCUMENTATION
Return TRUE if Ilists c and d are thr same length and
each member of c is <= the corresponding member of d.
*/

/* PUBLIC */
BOOL features_less_or_equal(Ilist c, Ilist d)
{
  while (c && d && c->i <= d->i) {
    c = c->next;
    d = d->next;
  }
  return !c && !d;
}  /* features_less_or_equal */

/*************
 *
 *   p_features()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_features(Ilist f)
{
  Ilist p;
  printf("  pos_lits=%d, neg_lits=%d\n", f->i, f->next->i);
  f = f->next->next;

  for (p = Feature_symbols; p; p = p->next) {
    printf("  symbol %s: ", sn_to_str(p->i));
    printf("pos_occ=%d, neg_occ=%d", f->i, f->next->i);
    f = f->next->next;
    if (function_symbol(p->i)) {
      printf(", pos_max=%d, neg_max=%d", f->i, f->next->i);
      f = f->next->next;
    }
    printf("\n");
  }
}  /* p_features */

