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

#include "flatterm.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_FLATTERM PTRS(sizeof(struct flatterm))
static unsigned Flatterm_gets, Flatterm_frees;

/*************
 *
 *   Flatterm get_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Flatterm get_flatterm(void)
{
  Flatterm p = get_mem(PTRS_FLATTERM);  /* get uninitialized memory */
  Flatterm_gets++;

  p->prev = NULL;
  p->next = NULL;
  p->varnum_bound_to = -1;
  p->alternative = NULL;
  p->reduced_flag = FALSE;
  p->size = 0;

  /* end, arity, private_symbol not initilized */
  return(p);
}  /* get_flatterm */

/*************
 *
 *    free_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_flatterm(Flatterm p)
{
  free_mem(p, PTRS_FLATTERM);
  Flatterm_frees++;
}  /* free_flatterm */

/*************
 *
 *   fprint_flatterm_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the flatterm package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_flatterm_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct flatterm);
  fprintf(fp, "flatterm (%4d)     %11u%11u%11u%9.1f K\n",
          n, Flatterm_gets, Flatterm_frees,
          Flatterm_gets - Flatterm_frees,
          ((Flatterm_gets - Flatterm_frees) * n) / 1024.);

}  /* fprint_flatterm_mem */

/*************
 *
 *   p_flatterm_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the flatterm package.
*/

/* PUBLIC */
void p_flatterm_mem()
{
  fprint_flatterm_mem(stdout, TRUE);
}  /* p_flatterm_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   flatterm_ident()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL flatterm_ident(Flatterm a, Flatterm b)
{
  Flatterm ai, bi;
  for (ai = a, bi = b; ai != a->end->next; ai = ai->next, bi = bi->next)
    if (ai->private_symbol != bi->private_symbol)
      return FALSE;
  return TRUE;
}  /* flatterm_ident */

/*************
 *
 *   zap_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void zap_flatterm(Flatterm f)
{
  Flatterm fi = f;
  while (fi != f->end->next) {
    Flatterm tmp = fi;
    fi = fi->next;
    free_flatterm(tmp);
  }
}  /* zap_flatterm */

/*************
 *
 *   term_to_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Flatterm term_to_flatterm(Term t)
{
  Flatterm f = get_flatterm();
  f->private_symbol = t->private_symbol;
  ARITY(f) = ARITY(t);

  if (VARIABLE(t)) {
    f->end = f;
    f->size = 1;
    return f;
  }
  else {
    int n = 1;
    int i;
    Flatterm end = f;
    for (i = 0; i < ARITY(t); i++) {
      Flatterm arg = term_to_flatterm(ARG(t,i));
      n += arg->size;
      end->next = arg;
      arg->prev = end;
      end = arg->end;
    }
    f->end = end;
    f->size = n;
    return f;
  }
}  /* term_to_flatterm */

/*************
 *
 *   flatterm_to_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term flatterm_to_term(Flatterm f)
{
  if (VARIABLE(f))
    return get_variable_term(VARNUM(f));
  else {
    Term t = get_rigid_term_dangerously(SYMNUM(f),ARITY(f));
    int i;
    Flatterm g = f->next;
    for (i = 0; i < ARITY(f); i++) {
      ARG(t,i) = flatterm_to_term(g);
      g = g->end->next;
    }
    return t;
  }
}  /* flatterm_to_term */

/*************
 *
 *   copy_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Flatterm copy_flatterm(Flatterm f)
{
  int i;
  int n = 1;
  Flatterm g = get_flatterm();
  Flatterm end = g;
  Flatterm arg = f->next;

  g->private_symbol = f->private_symbol;
  ARITY(g) = ARITY(f);

  for (i = 0; i < ARITY(f); i++) {
    Flatterm b = copy_flatterm(arg);
    n += b->size;
    end->next = b;
    b->prev = end;
    end = b->end;
    arg = arg->end->next;
  }
  g->end = end;
  g->size = n;
  return g;
}  /* copy_flatterm */

/*************
 *
 *   print_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void print_flatterm(Flatterm f)
{
  if (VARIABLE(f)) {
    if (VARNUM(f) < 3)
      printf("%c", 'x' + VARNUM(f));
    else if (VARNUM(f) < 6)
      printf("%c", 'r' + VARNUM(f));
    else
      printf("v%d", VARNUM(f));
  }
  else if (CONSTANT(f))
    printf("%s", sn_to_str(SYMNUM(f)));
  else {
    int i;
    Flatterm g = f->next;
    printf("%s(", sn_to_str(SYMNUM(f)));
    for (i = 0; i < ARITY(f); i++) {
      print_flatterm(g);
      if (i < ARITY(f) - 1)
	printf(",");
      g = g->end->next;
    }
    printf(")");
  }
}  /* print_flatterm */

/*************
 *
 *   flatterm_symbol_count()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int flatterm_symbol_count(Flatterm f)
{
  if (VARIABLE(f))
    return 1;
  else {
    int n = 1;
    int i;
    Flatterm g = f->next;
    for (i = 0; i < ARITY(f); i++) {
      n += flatterm_symbol_count(g);
      g = g->end->next;
    }
    return n;
  }
}  /* flatterm_symbol_count */

/*************
 *
 *   p_flatterm()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_flatterm(Flatterm f)
{
  print_flatterm(f);
  printf("\n");
  fflush(stdout);
}  /* p_flatterm */

/*************
 *
 *   flat_occurs_in()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL flat_occurs_in(Flatterm t1, Flatterm t2)
{
  Flatterm t2i;
  for (t2i = t2; t2i != t2->end->next; t2i = t2i->next)
    if (flatterm_ident(t1, t2i))
      return TRUE;
  return FALSE;
}  /* flat_occurs_in */

/*************
 *
 *   flat_multiset_vars()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I2list flat_multiset_vars(Flatterm f)
{
  I2list vars = NULL;
  Flatterm fi;
  for (fi = f; fi != f->end->next; fi = fi->next)
    if (VARIABLE(fi))
      vars = multiset_add(vars, VARNUM(fi));
  return vars;
}  /* flat_multiset_vars */

/*************
 *
 *   flat_variables_multisubset()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL flat_variables_multisubset(Flatterm a, Flatterm b)
{
  I2list a_vars = flat_multiset_vars(a);
  I2list b_vars = flat_multiset_vars(b);
  BOOL ok = i2list_multisubset(a_vars, b_vars);
  zap_i2list(a_vars);
  zap_i2list(b_vars);
  return ok;
}  /* flat_variables_multisubset */

/*************
 *
 *   flatterm_count_without_vars()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int flatterm_count_without_vars(Flatterm f)
{
  if (VARIABLE(f))
    return 0;
  else {
    int n = 1;
    int i;
    Flatterm g = f->next;
    for (i = 0; i < ARITY(f); i++) {
      n += flatterm_count_without_vars(g);
      g = g->end->next;
    }
    return n;
  }
}  /* flatterm_count_without_vars */

