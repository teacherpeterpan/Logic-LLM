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
#include "propagate.h"

/* External variables defined in mace4.c. */

extern Mace_options Opt;

extern struct cell *Cells;

extern Symbol_data *Sn_to_mace_sn;

extern int Domain_size;
extern Term *Domain;

extern int Relation_flag;  /* term flag */

extern struct mace_stats Mstats;

/*************
 *
 *   nterm_id()
 *
 *   Given an nterm, e.g., f(3,g(2)), find the ID of the eterm
 *   obtained by replacing the subterm with 0, that is, f(3,0).
 *
 *************/

static
int nterm_id(Term t)
{
  int id = Sn_to_mace_sn[SYMNUM(t)]->base;
  int mult = 1;
  int i;
  for (i = ARITY(t)-1; i >= 0; i--) {
    if (VARIABLE(ARG(t,i)))
      id += VARNUM(ARG(t,i)) * mult;
    mult *= Domain_size;
  }
  return id;
}  /* nterm_id */

/*************
 *
 *   negprop_elim()
 *
 *   Given an elimination, e.g., f(1,2)!=3, derive new eliminations
 *   by finding appropriate near assignments in the negprop index.
 *
 *************/

static
void negprop_elim(int id, Term beta, Mstate state)
{
  struct cell *c = Cells + id;
  int arity      = c->symbol->arity;
  int sym        = c->symbol->mace_sn;
  Term alpha     = c->eterm;
  int i;
  for (i = 0; i < arity; i++) {
    Term results = negprop_find_near(1, sym, VARNUM(beta), alpha, i);
    while (results) {
      /* We don't know the orientation. */
      Term found_alpha = (VARIABLE(ARG(results,0)) ?
			  ARG(results,1) : ARG(results,0));
#if 0
      fwrite_term(stdout, alpha);
      printf(" != %d  ELIM  ", VARNUM(beta));
      p_matom(results);
#endif
      if (VARIABLE(found_alpha))
	Mstats.neg_elim_agone++;
      else {
	Term e = ARG(found_alpha, i);
	int subterm_id;
	if (!eterm(e, &subterm_id))
	  Mstats.neg_elim_egone++;
	else {
	  Mstats.neg_elim_attempts++;
	  new_elimination(subterm_id, ARG(alpha,i), state);
	  if (!state->ok)
	    return;  /* contradiction */
	}
      }
      results = results->u.vp;
    }  /* results */
  }  /* position */
}  /* negprop_elim */

/*************
 *
 *   negprop_assign()
 *
 *   Given an assignment, derive new eliminations by finding near
 *   assignments and near eliminations in the negprop index.
 *
 *************/

static
void negprop_assign(int id, Mstate state)
{
  struct cell *c = Cells + id;
  int arity      = c->symbol->arity;
  int sym        = c->symbol->mace_sn;
  Term alpha     = c->eterm;
  Term beta      = c->value;
  int i;

  if (c->symbol->type == FUNCTION) {
    for (i = 0; i < arity; i++) {
      Term results = negprop_find_near(0, sym, VARNUM(beta), alpha, i);
      while (results) {
	/* We don't know the orientation. */
	Term found_alpha = (VARIABLE(ARG(results,0)) ?
			    ARG(results,1) : ARG(results,0));
#if 0
	fwrite_term(stdout, alpha);
	printf(" = %d  ASSIGN1  ", VARNUM(beta));
	p_matom(results);
#endif
	if (VARIABLE(found_alpha))
	  Mstats.neg_assign_agone++;
	else {
	  Term e = ARG(found_alpha, i);
	  int subterm_id;
	  if (!eterm(e, &subterm_id))
	    Mstats.neg_assign_egone++;
	  else {
	    Mstats.neg_assign_attempts++;
	    new_elimination(subterm_id, ARG(alpha,i), state);
	    if (!state->ok)
	      return;  /* contradiction */
	  }
	}
	results = results->u.vp;
      }  /* results */
    }  /* position */
  }  /* FUNCTION */

  /* Now make inferences like  f(3,4)=5,  f(3,g(2))=6  ->  g(2)!=4.
     This applies to nonequations as well as equations, because
     P(0) is handled as P(0)=1, and ~P(0) is handled as P(0)=0.
  */
  
  for (i = 0; i < arity; i++) {
    int j;
    int n = (c->symbol->type == FUNCTION ? Domain_size : 2);
    for (j = 0; j < n; j++) {
      if (j != VARNUM(beta)) {
	Term results = negprop_find_near(1, sym, j, alpha, i);
	/* results can look like:  f(1,2)=3,  3=f(1,2),  P(2),  ~P(3)  */
	while (results) {
	  Term found_alpha;
	  if (c->symbol->type == RELATION)
	    found_alpha = results;
	  else
	    found_alpha = (VARIABLE(ARG(results,0)) ?
			   ARG(results,1) : ARG(results,0));
#if 0
	  fwrite_term(stdout, alpha);
	  printf(" = %d  ASSIGN2  ", VARNUM(beta));
	  p_matom(results);
#endif
	  if (VARIABLE(found_alpha))
	    Mstats.neg_assign_agone++;
	  else {
	    Term e = ARG(found_alpha, i);
	    int subterm_id;
	    if (!eterm(e, &subterm_id))
	      Mstats.neg_assign_egone++;
	    else {
	      Mstats.neg_assign_attempts++;
	      new_elimination(subterm_id, ARG(alpha,i), state);
	      if (!state->ok)
		return;  /* contradiction */
	    }
	  }
	  results = results->u.vp;
	}  /* results */
      }  /* j ok */
    }  /* domain */
  }  /* position */
}  /* negprop_assign */ 

/*************
 *
 *   negprop_near_elim()
 *
 *   Given a near elimination, derive new eliminations by finding
 *   appropriate assignments in the cell table.
 *
 *************/

static
void negprop_near_elim(int subterm_id, Term alpha, Term beta, int pos,
		       Mstate state)
{
  if (VARIABLE(alpha))
    Mstats.neg_near_elim_agone++;
  else if (VARIABLE(ARG(alpha,pos)))
    Mstats.neg_near_elim_egone++;
  else {
    int i;
    int id = nterm_id(alpha);
    int increment = int_power(Domain_size, (ARITY(alpha) - 1) - pos);
    for (i = 0; i < Domain_size; i++) {
      if (Cells[id].value == beta) {
	Mstats.neg_near_elim_attempts++;
	new_elimination(subterm_id, Domain[i], state);
	if (!state->ok)
	  return;
      }
      id += increment;
    }
  }
}  /* negprop_near_elim */

/*************
 *
 *   negprop_near_assign()
 *
 *   Given a near assignment, derive new eliminations by finding
 *   eliminations and assignments in the cell table.
 *
 *************/

static
void negprop_near_assign(int subterm_id, Term alpha, Term beta, int pos,
			 Mstate state)
{
  if (VARIABLE(alpha))
    Mstats.neg_near_assign_agone++;
  else if (VARIABLE(ARG(alpha,pos)))
    Mstats.neg_near_assign_egone++;
  else {
    int i;
    int base_id = nterm_id(alpha);
    int increment = int_power(Domain_size, (ARITY(alpha) - 1) - pos);
    int id = base_id;

    if (!LITERAL(alpha)) {
      for (i = 0; i < Domain_size; i++) {
	if (Cells[id].possible[VARNUM(beta)] == NULL) {
	  Mstats.neg_near_assign_attempts++;
	  new_elimination(subterm_id, Domain[i], state);
	  if (!state->ok)
	    return;
	}
	id += increment;
      }  /* domain */
    }  /* equation */

    /* Now make inferences like  f(3,g(2))=5,  f(3,4)=6  ->  g(2)!=4.
       This applies to nonequations as well as equations, because
       P(0) is handled as P(0)=1, and ~P(0) is handled as P(0)=0.
    */

    {
      int j;
      int n = (LITERAL(alpha) ? 2 : Domain_size);
      for (j = 0; j < n; j++) {
	if (j != VARNUM(beta)) {
	  id = base_id;
	  for (i = 0; i < Domain_size; i++) {
	    if (Cells[id].value == Domain[j]) {
	      Mstats.neg_near_assign_attempts++;
	      new_elimination(subterm_id, Domain[i], state);
	      if (!state->ok)
		return;
	    }
	    id += increment;
	  }  /* domain */
	}  /* equation */
      }  /* beta */
    }  /* block */
  }
}  /* negprop_near_assign */

/*************
 *
 *   propagate_negative()
 *
 * There are 4 inference rules to derive new eliminations:
 *
 *   NEG_ELIM   - new clause is an elimination: 
 *   NEG_ASSIGN - new clause is a assignment: 
 *   NEG_ELIM_NEAR   - new clause is an near elimination: 
 *   NEG_ASSIGN_NEAR - new clause is a near assignment: 
 *
 *     type             id    alpha       beta     pos      comment
 *
 *   (for the first 2, id gives the alpha, e.g., 32 is g(0))
 *
 *   ASSIGNMENT         32    NULL        NULL      -1   g(0)={in table}
 *   ELIMINATION        32    NULL           2      -1   g(0)!=2
 *
 *   (for the next 4, id gives the eterm subterm, e.g., 32 is g(0))
 *
 *   NEAR_ASSIGNMENT    32    g(g(0))        2       0   g(g(0))=2
 *   NEAR_ASSIGNMENT    32    P(g(0))        1       0   P(g(0))
 *   NEAR_ASSIGNMENT    32    P(g(0))        0       0  ~P(g(0))
 *   NEAR_ELIMINATION   32    g(g(0))        2       0   g(g(0))!=2
 *
 *   Notes:
 *
 *   Eterms and alphas may have been simplified by the time they arrive here.
 *
 *   For ASSIGNMENT, the assignment has already been made
 *   For ELIMINATION, the cross-off has already been done.
 *
 *   If a contradiction is found, set state->ok to FALSE.
 *
 *************/

void propagate_negative(int type, int id, Term alpha, Term beta, int pos,
			Mstate state)
{
  switch (type) {
  case ELIMINATION:
    if (flag(Opt->neg_elim))
      negprop_elim(id, beta, state);
    break;
  case ASSIGNMENT:
    if (flag(Opt->neg_assign))
      negprop_assign(id, state);
    break;
  case NEAR_ELIMINATION:
    if (flag(Opt->neg_elim_near))
      negprop_near_elim(id, alpha, beta, pos, state);
    break;
  case NEAR_ASSIGNMENT:
    if (flag(Opt->neg_assign_near))
      negprop_near_assign(id, alpha, beta, pos, state);
    break;
  }
}  /* propagate_negative */

