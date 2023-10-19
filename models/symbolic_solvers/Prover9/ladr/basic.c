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

#include "basic.h"

/*
  In "Basic Paramodlation", we do not paramodulate into terms that
  arise by instantiation.  Another way to look at it (and the way
  it is rigorously defined), is that substitutions (instantiations) 
  are kept separate from clauses.  The skeleton of the clause
  stays the same (except that paramoulation substitutes the 
  skeleton from the "from" parent), and all instantiations occur
  in the associationed substitution.

  In this implementation, we do not keep separate substitutions.
  Instead, we mark the subterms that arose by substitution; that
  is the "nonbasic" subterms which are inadmissible "into" terms.

  Basic paramodulation is complete; but more important, it can
  be powerful (e.g., the Robbins proof).  The paper [McCune, "33
  Basic Test Problems"] reports on experiments showing that it
  can be useful.

  Things to be aware of:

  1. The terms that are marked are the "nonbasic" terms, that is,
  the inadmissible "into" terms.  Variables are always inadmissible,
  and are never marked.

  2. Things get messy for subsumption and demodulation, and I'm not
  clear on what's exactly what's required for completeness.

  3. Forward subsumption.  If the subsumer is less basic than the
  new clause, the subsumer should acquire some of the nonbasic
  marks of the new clause.  Not implemented.  Similar for back
  subsumption

  4. Should demodulation be applied to nonbasic terms?  I think yes,
  from a practical point of view (I don't know about completeness).
  If it is, then a nonbasic term can have a basic subterm.  Should
  we paramodulate into those?  I think not.

  5. Basic paramodulation can require a greater max_weight.

*/
  
/* Private definitions and types */

static BOOL Basic_paramodulation = FALSE; /* Is basic paramod enabled? */
static int Nonbasic_flag         = -1;    /* termflag to mark nonbasic terms */

/*************
 *
 *   init_basic_paramod(void)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_basic_paramod(void)
{
  if (Nonbasic_flag != -1)
    fatal_error("init_basic_paramod, called more than once");
  Nonbasic_flag = claim_term_flag();  /* allocate a termflag */
}  /* init_basic_paramod */

/*************
 *
 *   set_basic_paramod()
 *
 *************/

/* DOCUMENTATION
Set or clear basic paramodulation.
*/

/* PUBLIC */
void set_basic_paramod(BOOL flag)
{
  Basic_paramodulation = flag;
}  /* set_basic_paramod */

/*************
 *
 *   basic_paramod()
 *
 *************/

/* DOCUMENTATION
Is basic paramodulation enabled?
*/

/* PUBLIC */
BOOL basic_paramod(void)
{
  return Basic_paramodulation;
}  /* basic_paramod */

/*************
 *
 *   mark_term_nonbasic()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mark_term_nonbasic(Term t)
{
  if (Nonbasic_flag == -1)
    fatal_error("mark_term_nonbasic: init_basic() was not called");
  term_flag_set(t, Nonbasic_flag);
}  /* mark_term_nonbasic */

/*************
 *
 *   mark_all_nonbasic()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void mark_all_nonbasic(Term t)
{
  /* Even though variables are nonbasic, don't mark them. */
  if (!VARIABLE(t)) {
    int i;
    mark_term_nonbasic(t);
    for (i = 0; i < ARITY(t); i++)
      mark_all_nonbasic(ARG(t,i));
  }
}  /* mark_all_nonbasic */

/*************
 *
 *   nonbasic_term()
 *
 *************/

/* DOCUMENTATION
Check if a term is nonbasic.  This simply checks the "nonbasic" mark.
*/

/* PUBLIC */
BOOL nonbasic_term(Term t)
{
  return term_flag(t, Nonbasic_flag);
}  /* nonbasic_term */

/*************
 *
 *   basic_term()
 *
 *************/

/* DOCUMENTATION
Check if a term is basic.  This simply checks the "nonbasic" mark.
*/

/* PUBLIC */
BOOL basic_term(Term t)
{
  return !term_flag(t, Nonbasic_flag);
}  /* basic_term */

/*************
 *
 *   nonbasic_flag()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int nonbasic_flag(void)
{
  return Nonbasic_flag;
}  /* nonbasic_flag */

/*************
 *
 *    Term apply_basic(term, context) -- Apply a substitution to a term.
 *
 *************/

/* DOCUMENTATION
This is similar to apply(), but it makes "nonbasic" marks for
"basic paramodulation".
*/

/* PUBLIC */
Term apply_basic(Term t, Context c)
{
  Term raw = t;  /* save original (nondereferenced) term */
  DEREFERENCE(t, c)

  /* A NULL context is ok.  It happens when c is built by match. */
  /* If the context is NULL, then apply just copies the term.    */
    
  if (VARIABLE(t)) {
    if (!c)
      return get_variable_term(VARNUM(t));
    else
      return get_variable_term(c->multiplier * MAX_VARS + VARNUM(t));
  }
  else {  /* constant or complex term */
    Term t2 = get_rigid_term_like(t);
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t2,i) = apply_basic(ARG(t,i), c);
    /* If the raw term is a variable, the result and all of its nonvariable
       subterms are marked as nonbasic; if the dereferenced term is nonbasic,
       the rood of the result is marked as nonbasic (subterms have already
       been marked as nonbasic; recall that all nonvariable subterms of
       a nonbasic term are nonbasic).
    */
    if (VARIABLE(raw))
      mark_all_nonbasic(t2);
    else if (nonbasic_term(t))
      mark_term_nonbasic(t2);
    return t2;
  }
}  /* apply_basic */

/*************
 *
 *    apply_basic_substitute()
 *
 *************/

/* DOCUMENTATION
This is similar to apply_substitute(), but it makes "nonbasic" marks for
"basic paramodulation".
*/

/* PUBLIC */
Term apply_basic_substitute(Term t, Term beta, Context c_from,
			    Term into_term, Context c_into)
{
  if (t == into_term)
    return apply_basic(beta, c_from);
  else if (VARIABLE(t))
    return apply_basic(t, c_into);
  else {
    int i;
    Term t2 = get_rigid_term_like(t);
    if (nonbasic_term(t))
      mark_term_nonbasic(t2);
    for (i = 0; i < ARITY(t); i++)
      ARG(t2,i) = apply_basic_substitute(ARG(t,i), beta, c_from,
					   into_term, c_into); 
    return t2;
  }
}  /* apply_basic_substitute */

/*************
 *
 *   clear_all_nonbasic_marks()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void clear_all_nonbasic_marks(Term t)
{
  /* recall that variables never have nonbasic marks */
  if (!VARIABLE(t)) {
    int i;
    term_flag_clear(t, Nonbasic_flag);
    for (i = 0; i < ARITY(t); i++)
      clear_all_nonbasic_marks(ARG(t,i));
  }
}  /* clear_all_nonbasic_marks */

/*************
 *
 *   p_term_basic()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_term_basic(Term t)
{
  if (VARIABLE(t))
    printf("v%d", VARNUM(t));
  else {
    if (basic_term(t))
      printf("#");
    fprint_sym(stdout, SYMNUM(t));
    if (COMPLEX(t)) {
      int i;
      printf("(");
      for (i = 0; i < ARITY(t); i++) {
	p_term_basic(ARG(t,i));
	if (i < ARITY(t)-1)
	  printf(",");
      }
      printf(")");
    }
  }
  fflush(stdout); 
}  /* p_term_basic */
