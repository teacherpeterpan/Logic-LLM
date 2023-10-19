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

#include "msearch.h"

extern int Domain_size;
extern Term *Domain;
extern Plist Ground_clauses;
extern struct mace_stats Mstats;

extern struct cell *Cells;

extern int Eq_sn;
extern int Or_sn;
extern int Not_sn;
extern int Relation_flag;
extern int Negation_flag;

/*
 * memory management
 */

#define PTRS_MCLAUSE CEILING(sizeof(struct mclause),BYTES_POINTER)
static unsigned Mclause_gets, Mclause_frees;
static unsigned Mclause_use, Mclause_high;

/*************
 *
 *   Mclause get_mclause(arity)
 *
 *************/

static
Mclause get_mclause(int numlits)
{
  /* This is a little tricky.  The pointers to the literals are
     in an array (p->lits) that is just after (contiguous with)
     the mclause.
   */
  Mclause p = get_mem(PTRS_MCLAUSE + numlits);
  p->numlits = numlits;
  if (numlits == 0)
    p->lits = NULL;  /* not sure if this can happen */
  else {
    void **v = (void **) p;
    p->lits = (Term *) (v + PTRS_MCLAUSE);  /* just after the struct */
  }
  /* initialization */	
  p->subsumed = FALSE;
  p->u.active = -1;
  p->next = NULL;

  Mclause_gets++;
  Mclause_use += (PTRS_MCLAUSE + numlits) * BYTES_POINTER;
  Mclause_high = IMAX(Mclause_use, Mclause_high);
  return(p);
}  /* get_mclause */

/*************
 *
 *    free_mclause()
 *
 *************/

static
void free_mclause(Mclause p)
{
  Mclause_frees++;
  Mclause_use -= (PTRS_MCLAUSE + p->numlits) * BYTES_POINTER;
  free_mem(p, PTRS_MCLAUSE + p->numlits);
}  /* free_mclause */

/* end of get and free routines for each type */

/*************
 *
 *   fprint_mclause_mem()
 *
 *************/

void fprint_mclause_mem(FILE *fp, int heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct mclause);
  fprintf(fp, "mclause (%4d)      %11u%11u%11u%9.1f K (%.1f K high)\n",
	  n, Mclause_gets, Mclause_frees, Mclause_gets - Mclause_frees,
	  Mclause_use / 1024.0, Mclause_high / 1024.0);
  
  /* end of printing for each type */
  
}  /* fprint_mclause_mem */

/*************
 *
 *   p_mclause_mem()
 *
 *************/

void p_mclause_mem()
{
  fprint_mclause_mem(stdout, 1);
}  /* p_mclause_mem */

/*************
 *
 *   zap_mterm()
 *
 *   Do not free variable terms (that is, domain elements).
 *
 *************/

void zap_mterm(Term t)
{
  if (!VARIABLE(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++)
      zap_mterm(ARG(t,i));
    free_term(t);
  }
}  /* zap_mterm */

/*************
 *
 *   zap_mclause()
 *
 *************/

void zap_mclause(Mclause c)
{
  int i;
  for (i = 0; i < c->numlits; i++)
    zap_mterm(LIT(c,i));
  free_mclause(c);
}  /* zap_mclause */

/*************
 *
 *   lit_position()
 *
 *************/

int lit_position(Mclause parent, Term child)
{
  int i;
  for (i = 0; i < parent->numlits; i++) {
    if (LIT(parent,i) == child)
      return i;
  }
  return -1;
}  /* lit_position */

/*************
 *
 *   set_parent_pointers()
 *
 *   Make each term, except for variables, point to its parent.
 *
 *************/

static
void set_parent_pointers(Term t)
{
  if (!VARIABLE(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (VARIABLE(ARG(t,i)))
	ARG(t,i)->container = NULL;
      else
	ARG(t,i)->container = t;
      set_parent_pointers(ARG(t,i));
    }
  }
}  /* set_parent_pointers */

/*************
 *
 *   check_parent_pointers()
 *
 *   Check that each term, except for variables, points to its parent.
 *   If not, print a message to stdout.
 *
 *************/

void check_parent_pointers(Term t)
{
  if (!VARIABLE(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (VARIABLE(ARG(t,i))) {
	if (ARG(t,i)->container != NULL) {
	  printf("check_parent_pointers, bad link: "); p_term(t);
	}
      }
      else {
	if (ARG(t,i)->container != t) {
	  printf("check_parent_pointers, bad link: "); p_term(t);
	}
      }
      set_parent_pointers(ARG(t,i));
    }
  }
}  /* check_parent_pointers */

/*************
 *
 *   containing_mclause()
 *
 *************/

Mclause containing_mclause(Term t)
{
  while (!LITERAL(t))
    t = t->container;
  return t->container;
}  /* containing_mclause */

/*************
 *
 *   containing_mliteral()
 *
 *************/

Term containing_mliteral(Term t)
{
  while (!LITERAL(t))
    t = t->container;
  return t;
}  /* containing_mliteral */

/*************
 *
 *   eterm_index_term()  -  recursive
 *
 *   Insert each eterm into the occurrence list of that cell.
 *
 *************/

static
void eterm_index_term(Term t)
{
  int id;
  if (VARIABLE(t))
    return;
  else if (eterm(t, &id)) {
    t->u.vp = Cells[id].occurrences;
    Cells[id].occurrences = t;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      eterm_index_term(ARG(t,i));
    }
  }
}  /* eterm_index_term */

/*************
 *
 *   simp_tv()
 *
 *   Simplify a term (or atom) with respect to OR, NOT, and
 *   any assignments that have already been made.
 *
 *************/

static
BOOL member(Term x, Term t)
{
  /* This does not assume OR_TERMs are right associated. */
  if (term_ident(x,t))
    return TRUE;
  else if (!OR_TERM(t))
    return FALSE;
  else if (member(x,ARG(t,0)))
    return TRUE;
  else
    return member(x,ARG(t,1));
}

static
Term merge(Term t)
{
  /* This assumes OR_TERMs are right associated. */
  if (!OR_TERM(t))
    return t;
  else {
    ARG(t,1) = merge(ARG(t,1));
    if (!member(ARG(t,0), ARG(t,1)))
      return t;
    else {
      Term t1 = ARG(t,1);
      zap_mterm(ARG(t,0));
      free_term(t);
      return t1;
    }
  }
}

static
Term simp_term(Term t)
{
  if (VARIABLE(t))
    return t;
  else {
    int i, id;
    for (i = 0; i < ARITY(t); i++) 
      ARG(t,i) = simp_term(ARG(t,i));
    if (eterm(t, &id) && Cells[id].value != NULL) {
      zap_mterm(t);
      return Cells[id].value;
    }
    else
      return t;
  }
}

static
Term simp_tv(Term t)
{
  if (true_term(t)) {
    zap_mterm(t);
    return Domain[1];
  }
  else if (false_term(t)) {
    zap_mterm(t);
    return Domain[0];
  }
  else if (OR_TERM(t)) {
    t = merge(t);
    if (!OR_TERM(t))
      return simp_tv(t);
    else {
      int i;
      for (i = 0; i < ARITY(t); i++)
	ARG(t,i) = simp_tv(ARG(t,i));

      if (TRUE_TERM(ARG(t,0)) || TRUE_TERM(ARG(t,1))) {
	zap_mterm(t);
	return Domain[1];
      }
      else if (FALSE_TERM(ARG(t,0))) {
	Term t2 = ARG(t,1);
	zap_mterm(ARG(t,0));
	free_term(t);
	return t2;
      }
      else if (FALSE_TERM(ARG(t,1))) {
	Term t2 = ARG(t,0);
	zap_mterm(ARG(t,1));
	free_term(t);
	return t2;
      }
      else
	return t;
    }
  }  /* end of OR_TERM */
  else if (NOT_TERM(t)) {
    ARG(t,0) = simp_tv(ARG(t,0));
    
    if (TRUE_TERM(ARG(t,0))) {
      zap_mterm(t);
      return Domain[0];
    }
    else if (FALSE_TERM(ARG(t,0))) {
      zap_mterm(t);
      return Domain[1];
    }
    else
      return t;
  }  /* end of NOT_TERM */
  else {
    /* It is an atomic formula. */
    int i, id;
    for (i = 0; i < ARITY(t); i++) 
      ARG(t,i) = simp_term(ARG(t,i));
    if (arith_rel_term(t)) {
      BOOL evaluated;
      int b = arith_eval(t, &evaluated);
      if (evaluated) {
	zap_term(t);
	return (b ? Domain[1] : Domain[0]);
      }
      else
	return t;
    }
    else if (eterm(t, &id) && Cells[id].value != NULL) {
      zap_mterm(t);
      return Cells[id].value;
    }
    else if (EQ_TERM(t)) {
      /* f(4,3)=2; check if 2 has been crossed off of f(4,3) list. */
      int value;
      if (VARIABLE(ARG(t,1)) && eterm(ARG(t,0), &id))
	value = VARNUM(ARG(t,1));
      else if (VARIABLE(ARG(t,0)) && eterm(ARG(t,1), &id))
	value = VARNUM(ARG(t,0));
      else
	return t;
      if (Cells[id].possible[value] == NULL) {
	zap_mterm(t);
	return Domain[0];
      }
      else
	return t;
    }
    else
      return t;
  }
}  /* simp_tv */

/*************
 *
 *   term_to_mclause()
 *
 *************/

Plist term_to_lits(Term t)
{
  if (!OR_TERM(t)) {
    Plist g = get_plist();
    g->v = t;
    return g;
  }
  else {
    Plist g0 = term_to_lits(ARG(t,0));
    Plist g1 = term_to_lits(ARG(t,1));
    free_term(t);  /* the OR node */
    return plist_cat(g0,g1);
  }
}

static
Mclause term_to_mclause(Term t)
{
  Plist g = term_to_lits(t);
  int n = plist_count(g);
  Plist g2;
  int i = 0;
  Mclause c = get_mclause(n);
  c->u.active = n;
  for (g2 = g; g2 != NULL; g2 = g2->next) {
    Term lit = g2->v;
    Term atom;
    if (NOT_TERM(lit)) {
      atom = ARG(lit,0);
      free_term(lit);  /* the NOT node */
      term_flag_set(atom, Negation_flag);
    }
    else
      atom = lit;
    term_flag_set(atom, Relation_flag);
    LIT(c,i) = atom;
    i++;
  }
  zap_plist(g);
  return c;
}  /* term_to_mclause */

/*************
 *
 *   subst_domain_elements_term()
 *
 *************/

static
Term subst_domain_elements_term(Term t, int *vals)
{
  if (VARIABLE(t)) {
    Term t2 = Domain[vals[VARNUM(t)]];
    zap_term(t);
    return t2;
  }
  else {
    int i;
    i = natural_constant_term(t);
    if (i >= 0) { 
      zap_term(t);
      if (i < Domain_size)
	return Domain[i];  /* domain element */
      else
	return get_variable_term(i);  /* natural number of arithmetic only */
    }
    else {
      for (i = 0; i < ARITY(t); i++)
	ARG(t,i) = subst_domain_elements_term(ARG(t,i), vals);
      return t;
    }
  }
}  /* subst_domain_elements_term */

/*************
 *
 *   instances_recurse()
 *
 *************/

static
void instances_recurse(Topform c, int *vals, int *domains,
			int nextvar, int nvars, Mstate state)
{
  if (nextvar == nvars) {
    Term t = topform_to_term_without_attributes(c);
    subst_domain_elements_term(t, vals);
#if 0
    printf("\nbefore: "); fwrite_term_nl(stdout, t); fflush(stdout);
#endif    
    t = simp_tv(t);
#if 0
    printf("after: "); fwrite_term_nl(stdout, t); fflush(stdout);
#endif    
    Mstats.ground_clauses_seen++;
    if (FALSE_TERM(t)) {
      fprintf(stdout, "\nNOTE: unsatisfiability detected on input.\n");
      fprintf(stderr, "\nNOTE: unsatisfiability detected on input.\n");
      state->ok = FALSE;
      return;
    }
    else if (!TRUE_TERM(t)) {
      int i;
      Mclause m = term_to_mclause(t);
      for (i = 0; i < m->numlits; i++) {
	eterm_index_term(LIT(m,i));
	set_parent_pointers(LIT(m,i));
	LIT(m,i)->container = m;
      }
      process_initial_clause(m, state);
      if (!state->ok) {
	fprintf(stdout, "\nNOTE: unsatisfiability detected on input.\n");
	fprintf(stderr, "\nNOTE: unsatisfiability detected on input.\n");
	return;
      }
      Ground_clauses = plist_prepend(Ground_clauses, m);
      Mstats.ground_clauses_kept++;
    }
  }
  else if (domains[nextvar] == -1) {
    /* in case the current variable does not appear in the clause */
    instances_recurse(c, vals, domains, nextvar+1, nvars, state);
    if (!state->ok)
      return;
  }
  else {
    int i;
    for (i = 0; i < domains[nextvar]; i++) {
      vals[nextvar] = i;
      instances_recurse(c, vals, domains, nextvar+1, nvars, state);
      if (!state->ok)
	return;
    }
  }
}  /* instances_recurse */

/*************
 *
 *   generate_ground_clauses()
 *
 *   SIDE EFFECT: global Stack is updated.
 *
 *************/

void generate_ground_clauses(Topform c, Mstate state)
{
  int i, biggest_var, vals[MAX_MACE_VARS], domains[MAX_MACE_VARS];

  biggest_var = greatest_variable_in_clause(c->literals);

  for (i = 0; i <= biggest_var; i++)
    domains[i] = Domain_size;
	
  instances_recurse(c, vals, domains, 0, biggest_var+1, state);
}  /* generate_ground_clauses */
