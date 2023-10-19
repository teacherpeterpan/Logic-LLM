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

#include "paramod.h"

/* Private definitions and types */

static BOOL  Ordered_inference = FALSE;
static BOOL  Positive_inference = FALSE;
static BOOL  Para_from_vars = TRUE;
static BOOL  Para_into_vars = FALSE;
static BOOL  Para_from_small = FALSE;
static BOOL  Check_instances   = FALSE;  /* non-oriented from lits */

static int Para_instance_prunes = 0;     /* counter */
static int Basic_prunes = 0;             /* counter */

/*************
 *
 *   paramodulation_options()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */

void paramodulation_options(BOOL ordered_inference,
			    BOOL check_instances,
			    BOOL positive_inference,
			    BOOL basic_paramodulation,
			    BOOL para_from_vars,
			    BOOL para_into_vars,
			    BOOL para_from_small)
{
  Ordered_inference = ordered_inference;
  Para_from_vars = para_from_vars;
  Para_into_vars = para_into_vars;
  Para_from_small = para_from_small;
  Check_instances = check_instances;
  Positive_inference = positive_inference;
  set_basic_paramod(basic_paramodulation);
  Para_instance_prunes = 0;
  Basic_prunes = 0;
}  /* paramodulation_options */

/*************
 *
 *   para_instance_prunes()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int para_instance_prunes()
{
  return Para_instance_prunes;
}  /* para_instance_prunes */

/*************
 *
 *   basic_paramodulation_prunes()
 *
 *************/

/* DOCUMENTATION
How many paramodulants were killed because they failed the "basic" test.
*/

/* PUBLIC */
int basic_paramodulation_prunes(void)
{
  return Basic_prunes;
}  /* basic_paramodulation_prunes */

/*************
 *
 *   basic_check()
 *
 *************/

static
BOOL basic_check(Term into_term)
{
  if (basic_paramod() && nonbasic_term(into_term)) {
    Basic_prunes++;
    return FALSE;
  }
  else
    return TRUE;
}  /* basic_check */

/*************
 *
 *   apply_lit_para()
 *
 *************/

static
Literals apply_lit_para(Literals lit, Context c)
{
  if (basic_paramod())
    return new_literal(lit->sign, apply_basic(lit->atom, c));
  else
    return new_literal(lit->sign, apply(lit->atom, c));
}  /* apply_lit_para */

/*************
 *
 *   apply_substitute_para()
 *
 *************/

#if 0
static
Term apply_substitute_para(Term t, Term beta, Context from_subst,
			   Term into, Context into_subst)
{
  if (basic_paramod())
    return apply_basic_substitute(t, beta, from_subst, into, into_subst);
  else
    return apply_substitute(t, beta, from_subst, into, into_subst);
}  /* apply_substitute_para */
#endif

/*************
 *
 *   paramodulate()
 *
 *   Construct a paramodulant.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform paramodulate(Literals from_lit, int from_side, Context from_subst,
		     Topform into_clause, Ilist into_pos, Context into_subst)
{
  Topform from_clause = from_lit->atom->container;
  Topform p = get_topform();
  Term beta = ARG(from_lit->atom, from_side == 0 ? 1 : 0);
  Literals into_lit = ith_literal(into_clause->literals, into_pos->i);
  Literals lit;
  for (lit = from_clause->literals; lit != NULL; lit = lit->next) {
    if (lit != from_lit)
      p->literals = append_literal(p->literals,
				   apply_lit_para(lit, from_subst));
  }
  for (lit = into_clause->literals; lit != NULL; lit = lit->next) {
    if (lit != into_lit)
      p->literals = append_literal(p->literals,
				   apply_lit_para(lit, into_subst));
    else
      p->literals = 
	append_literal(p->literals,
		       new_literal(lit->sign,
				   apply_substitute2(lit->atom,
						     beta,
						     from_subst,
						     into_pos->next,
						     into_subst)));
  }
  inherit_attributes(from_clause, from_subst, into_clause, into_subst, p);
  upward_clause_links(p);
  return p;
}  /* paramodulate */

/*************
 *
 *   para_from_right()
 *
 *   Should we paramoulate from the right side?
 *   If it is oriented, then NO;
 *   else if it is a renamable_flip unit, then NO;
 *   else YES.
 *
 *************/

static
BOOL para_from_right(Term atom)
{
  /* Assume atom is an eq_atom. */
  if (Para_from_small)
    return TRUE;
  if (oriented_eq(atom))
    return FALSE;
  else if (renamable_flip_eq(atom) &&
	   unit_clause(((Topform) atom->container)->literals))
    return FALSE;
  else
    return TRUE;
}  /* para_from_right */

/*************
 *
 *   from_parent_test()
 *
 *************/

static
BOOL from_parent_test(Literals from_lit, int check)
{
  Topform from_parent = from_lit->atom->container;
  if (Positive_inference)
    return
      pos_eq(from_lit) &&
      positive_clause(from_parent->literals) &&
      (!Ordered_inference ||
       maximal_literal(from_parent->literals, from_lit, check));
  else
    return
      pos_eq(from_lit) &&
      !exists_selected_literal(from_parent->literals) &&
      (!Ordered_inference ||
       maximal_literal(from_parent->literals, from_lit, check));
}  /* from_parent_test */

/*************
 *
 *   into_parent_test()
 *
 *************/

static
BOOL into_parent_test(Literals into_lit, int check)
{
  Topform into_parent = into_lit->atom->container;
  if (into_lit->sign) {
    /* into positive literal */
    if (Positive_inference)
      return
	positive_clause(into_parent->literals) &&
	(!Ordered_inference ||
	 maximal_literal(into_parent->literals, into_lit, check));
    else
      return
	!exists_selected_literal(into_parent->literals) &&
	(!Ordered_inference ||
	 maximal_literal(into_parent->literals, into_lit, check));
  }
  else {
    /* into negative literal */
    if (Positive_inference) {
      if (exists_selected_literal(into_parent->literals))
	return selected_literal(into_lit);
      else
	return (!Ordered_inference ||
		maximal_signed_literal(into_parent->literals,into_lit,check));
    }
    else {
      if (exists_selected_literal(into_parent->literals))
	return selected_literal(into_lit);
      else
	return (!Ordered_inference ||
		maximal_literal(into_parent->literals, into_lit, check));
    }
  }
}  /* into_parent_test */

/*************
 *
 *   check_instance()
 *
 *************/

static
BOOL check_instance(Literals lit, Context subst, BOOL is_from_parent)
{
  Topform c = lit->atom->container;
  if (number_of_maximal_literals(c->literals, FLAG_CHECK) == 1 ||
      variable_substitution(subst))
    return TRUE;
  else {
    Literals a;
    BOOL ok;
    int n = literal_number(c->literals, lit);
    Topform d = instantiate_clause(c, subst);
    copy_selected_literal_marks(c->literals, d->literals);
    a = ith_literal(d->literals, n);

    if (is_from_parent)
      ok = from_parent_test(a, FULL_CHECK);
    else
      ok = into_parent_test(a, FULL_CHECK);
    zap_topform(d);
    if (!ok)
      Para_instance_prunes++;
    return ok;
  }
}  /* check_instance */

/*************
 *
 *   check_instances()
 *
 *************/

static
BOOL check_instances(Literals from_lit, int from_side, Context cf,
		     Literals into_lit, Term into, Context ci)
{
  if (!Check_instances)
    return TRUE;
  else {
    return
      check_instance(from_lit, cf, TRUE) &&
      check_instance(into_lit, ci, FALSE);
  }
}  /* check_instances */

/*************
 *
 *   para_into()
 *
 *   Paramodulate from a given side of a given literal into a given
 *   term and/or its subterms.
 *
 *************/

static
void para_into(Literals from_lit, int from_side, Context cf, Ilist from_pos,
	       Topform into_clause, Literals into_lit, Term into, Context ci,
	       Ilist into_pos,
	       BOOL skip_top,
	       void (*proc_proc) (Topform))
{
  if ((!VARIABLE(into) | Para_into_vars) && basic_check(into)) {
    int i;
    if (COMPLEX(into)) {
      Ilist last = ilist_last(into_pos);
      Ilist new = get_ilist();
      last->next = new;
      new->i = 0;
      for (i = 0; i < ARITY(into); i++) {
	new->i += 1;
	para_into(from_lit, from_side, cf, from_pos,
		  into_clause, into_lit, ARG(into,i), ci, into_pos,
		  FALSE,
		  proc_proc);
      }
      free_ilist(new);
      last->next = NULL;
    }
    if (!skip_top) {
      Trail tr = NULL;
      Term alpha = ARG(from_lit->atom, from_side);
      if (unify(alpha, cf, into, ci, &tr)) {
	if (check_instances(from_lit, from_side, cf, into_lit, into, ci)) {
	  Topform p = paramodulate(from_lit, from_side, cf,
				   into_clause, into_pos, ci);
	  p->justification = para_just(PARA_JUST,
				       from_lit->atom->container,
				       copy_ilist(from_pos),
				       into_clause,
				       copy_ilist(into_pos));
	  (*proc_proc)(p);
	}
	undo_subst(tr);
      }
    }
  }
}  /* para_into */

/*************
 *
 *   para_into_lit()
 *
 *************/

static
void para_into_lit(Literals from_lit, int from_side, Context cf,
		   Literals into_lit, Context ci,
		   BOOL check_top,
		   void (*proc_proc) (Topform))
{
  Term alpha = ARG(from_lit->atom, from_side);
  if (!VARIABLE(alpha) || Para_from_vars) {
    /* Position vectors are constructed FORWARD. */
    Ilist from_pos = ilist_prepend(ilist_prepend(NULL,0),0);
    Ilist into_pos = ilist_prepend(ilist_prepend(NULL,0),0);
    Term into_atom = into_lit->atom;
    Term from_atom = from_lit->atom;
    int i;
    Topform from_clause = from_atom->container;
    Topform into_clause = into_atom->container;
    BOOL positive_equality = pos_eq(into_lit);

    from_pos->i = literal_number(from_clause->literals, from_lit);
    from_pos->next->i = from_side+1;  /* arg of from_lit, counts from 1 */
    into_pos->i = literal_number(into_clause->literals, into_lit);
    for (i = 0; i < ARITY(into_atom); i++) {
      BOOL skip_top = (check_top &&
		       positive_equality &&
		       (i == 0 ||
			(i == 1 && para_from_right(into_lit->atom))));

      into_pos->next->i += 1;  /* increment arg number */
      para_into(from_lit, from_side, cf, from_pos,
		into_clause, into_lit, ARG(into_atom,i), ci, into_pos,
		skip_top, proc_proc);
    }
    zap_ilist(from_pos);
    zap_ilist(into_pos);
  }
}  /* para_into_lit */

/*************
 *
 *   para_from_into()
 *
 *************/

/* DOCUMENTATION
Paramodulate from one clause into another (non-backtrack unification version).
<P>
For oriented equality atoms, we go from left sides only
and into both sides.
For nonoriented equality atoms, we go from and into both sides.
*/

/* PUBLIC */
void para_from_into(Topform from, Context cf,
		    Topform into, Context ci,
		    BOOL check_top,
		    void (*proc_proc) (Topform))
{
  if (exists_selected_literal(from->literals))
    return;  /* cannot para from clause with selected literals */
  else {
    Literals from_lit;
    for (from_lit = from->literals; from_lit; from_lit = from_lit->next) {
      if (from_parent_test(from_lit, FLAG_CHECK)) {
	Literals into_lit;
	for (into_lit = into->literals; into_lit; into_lit = into_lit->next) {
	  if (into_parent_test(into_lit, FLAG_CHECK)) {
	    para_into_lit(from_lit,0,cf,into_lit,ci,check_top,proc_proc);  /* from L */
	    if (para_from_right(from_lit->atom))
	      para_into_lit(from_lit,1,cf,into_lit,ci,check_top,proc_proc); /* from R */
	  }
	}
      }
    }
  }
}  /* para_from_into */

/*************
 *
 *   para_pos()
 *
 *************/

/* DOCUMENTATION
Construct a paramodulant from the given data.  A fatal error
occurs if it does not exist.  In building the justification,
the position vectors are copied.
*/

/* PUBLIC */
Topform para_pos(Topform from_clause, Ilist from_pos,
		 Topform into_clause, Ilist into_pos)
{
  Context cf = get_context();
  Context ci = get_context();
  Trail tr = NULL;
  Topform paramodulant;
  BOOL ok;

  Literals from_lit = ith_literal(from_clause->literals, from_pos->i);
  Literals into_lit = ith_literal(into_clause->literals, into_pos->i);
  int from_side = (from_pos->next->i == 1 ? 0 : 1);
  Term alpha = ARG(from_lit->atom, from_side);
  Term into_term = term_at_pos(into_lit->atom, into_pos->next);
  if (into_term == NULL)
    fatal_error("paramod2_instances, term does not exist");

  ok = unify(alpha, cf, into_term, ci, &tr);
  if (!ok)
    fatal_error("para_pos, terms do not unify");

  paramodulant = paramodulate(from_lit, from_side, cf,
			      into_clause, into_pos, ci);

  paramodulant->justification = para_just(PARA_JUST,
					  from_clause, copy_ilist(from_pos),
					  into_clause, copy_ilist(into_pos));
  renumber_variables(paramodulant, MAX_VARS);
  undo_subst(tr);
  free_context(cf);
  free_context(ci);
  return paramodulant;
}  /* para_pos */

/*************
 *
 *   para_pos2()
 *
 *************/

/* DOCUMENTATION
Construct a paramodulant from the given data.  A fatal error
occurs if it does not exist.  In building the justification,
the position vectors are copied.

This is similar to para_pos(), except that it allows the
into_term to be a variable.
*/

/* PUBLIC */
Topform para_pos2(Topform from, Ilist from_pos, Topform into, Ilist into_pos)
{
  Context from_subst = get_context();
  Context into_subst = get_context();
  Trail tr = NULL;
  BOOL ok;
  int from_side;
  Term alpha, into_term;
  Topform p;
  Term beta;
  Literals lit;

  Literals from_lit = ith_literal(from->literals, from_pos->i);
  Literals into_lit = ith_literal(into->literals, into_pos->i);
  from_side = (from_pos->next->i == 1 ? 0 : 1);
  alpha = ARG(from_lit->atom, from_side);
  into_term = term_at_pos(into_lit->atom, into_pos->next);
  if (into_term == NULL)
    fatal_error("paramod2_instances, term does not exist");

  ok = unify(alpha, from_subst, into_term, into_subst, &tr);
  if (!ok)
    fatal_error("para_pos2, terms do not unify");

  p = get_topform();
  beta = ARG(from_lit->atom, from_side == 0 ? 1 : 0);
  for (lit = from->literals; lit; lit = lit->next) {
    if (lit != from_lit)
      p->literals = append_literal(p->literals,
				   apply_lit_para(lit, from_subst));
  }
  for (lit = into->literals; lit; lit = lit->next) {
    if (lit != into_lit) {
      p->literals = append_literal(p->literals,
				   apply_lit_para(lit, into_subst));
    }
    else {
      p->literals = 
	append_literal(p->literals,
		       new_literal(lit->sign,
				   apply_substitute2(lit->atom,
						     beta,
						     from_subst,
						     into_pos->next,
						     into_subst)));
    }
  }
  inherit_attributes(from, from_subst, into, into_subst, p);
  upward_clause_links(p);

  p->justification = para_just(PARA_JUST,
			       from, copy_ilist(from_pos),
			       into, copy_ilist(into_pos));
  renumber_variables(p, MAX_VARS);
  undo_subst(tr);
  free_context(from_subst);
  free_context(into_subst);
  return p;
}  /* para_pos2 */

