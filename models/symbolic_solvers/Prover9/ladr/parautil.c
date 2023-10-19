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

#include "parautil.h"

/* Private definitions and types */

int Oriented_flag       = -1;  /* termflag to mark oriented equality atoms */
int Renamable_flip_flag = -1;  /* termflag to mark renamable-flip eq atoms */

/*************
 *
 *   init_paramod()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_paramod(void)
{
  if (Oriented_flag != -1 || Renamable_flip_flag != -1)
    fatal_error("init_paramod, called more than once");
  Renamable_flip_flag = claim_term_flag();
  Oriented_flag = claim_term_flag();
}  /* init_paramod */

/*************
 *
 *   mark_renamable_flip()
 *
 *************/

/* DOCUMENTATION
This routine marks an atom as "renamable_flip".
*/

/* PUBLIC */
void mark_renamable_flip(Term atom)
{
  if (Renamable_flip_flag == -1)
    fatal_error("mark_renamable_flip, init_paramod() was not called");
  term_flag_set(atom, Renamable_flip_flag);
}  /* mark_renamable_flip */

/*************
 *
 *   renamable_flip_eq()
 *
 *************/

/* DOCUMENTATION
This function checks if an atom is a renamable_flip equality atom.
(The terms are not actually compared.  Only the mark is checked.)
*/

/* PUBLIC */
BOOL renamable_flip_eq(Term atom)
{
  if (Renamable_flip_flag == -1)
    /* Nothing has ever been marked renamable_flip. */
    return FALSE;
  else
    return term_flag(atom, Renamable_flip_flag);
}  /* renamable_flip_eq */

/*************
 *
 *   renamable_flip_eq_test()
 *
 *************/

/* DOCUMENTATION
Test if a term is a renamable-flip equality atom.
This does not check the flag; it does the complete test.
*/

/* PUBLIC */
BOOL renamable_flip_eq_test(Term atom)
{
  if (eq_term(atom))
    return FALSE;
  else {
    BOOL result;
    Term a1 = copy_term(atom);
    Term a2 = copy_term(atom);
    Term tmp = ARG(a2,0);
    ARG(a2,0) = ARG(a2,1);
    ARG(a2,1) = tmp;
    term_renumber_variables(a1, MAX_VARS);
    term_renumber_variables(a2, MAX_VARS);
    result = term_ident(a1,a2);
    zap_term(a1);
    zap_term(a2);
    return result;
  }
}  /* renamable_flip_eq_test */

/*************
 *
 *   mark_oriented_eq()
 *
 *************/

/* DOCUMENTATION
This routine marks an atom as an oriented equality.
*/

/* PUBLIC */
void mark_oriented_eq(Term atom)
{
  if (Oriented_flag == -1)
    fatal_error("mark_oriented_eq, init_paramod() was not called");
  term_flag_set(atom, Oriented_flag);
}  /* mark_oriented_eq */

/*************
 *
 *   oriented_eq()
 *
 *************/

/* DOCUMENTATION
This function checks if an atom is an oriented equality atom.
(The terms are not actually compared.  Only the mark is checked.)
*/

/* PUBLIC */
BOOL oriented_eq(Term atom)
{
  if (!eq_term(atom))
    return FALSE;
  else if (Oriented_flag == -1)
    /* Nothing has ever been oriented. */
    return FALSE;
  else
    return term_flag(atom, Oriented_flag);
}  /* oriented_eq */

/*************
 *
 *   same_term_structure()
 *
 *************/

/* DOCUMENTATION
Do terms t1 and t2 have the same structure?  That is, if
we rename all variables to x, are t1 and t2 identical?
*/

/* PUBLIC */
BOOL same_term_structure(Term t1, Term t2)
{
  if (VARIABLE(t1) || VARIABLE(t2))
    return VARIABLE(t1) && VARIABLE(t2);
  else if (SYMNUM(t1) != SYMNUM(t2))
    return FALSE;
  else {
    int i;
    for (i = 0; i < ARITY(t1); i++)
      if (!same_term_structure(ARG(t1,i), ARG(t2,i)))
	return FALSE;
    return TRUE;
  }
}  /* same_term_structure */

/*************
 *
 *   flip_eq()
 *
 *************/

/* DOCUMENTATION
Flip an equality and update the justification of the containing clause.
*/

/* PUBLIC */
void flip_eq(Term atom, int n)
{
  Topform c = atom->container;
  Term t = ARG(atom,0);
  ARG(atom,0) = ARG(atom,1);
  ARG(atom,1) = t;
  c->justification = append_just(c->justification, flip_just(n));
}  /* flip_eq */

/*************
 *
 *   orient_equalities()
 *
 *************/

/* DOCUMENTATION
For each equality literal (positive or negative) of Topform c,
compare the arguments; if the left is greater, mark the
atom as oriented, and if the the right is greater, flip the
arguments (add an entry to the justification),
and mark the atom as oriented.
*/

/* PUBLIC */
void orient_equalities(Topform c, BOOL allow_flips)
{
  Literals lit;
  int i;
  for (lit = c->literals, i = 1; lit != NULL; lit = lit->next, i++) {
    Term atom = lit->atom;
    if (eq_term(atom)) {
      Term alpha = ARG(atom,0);
      Term beta  = ARG(atom,1);
      if (!term_ident(alpha, beta)) {
	Ordertype ord = NOT_COMPARABLE;
	/* Check if it is a pos_eq_unit that should be unfolded. */
	if (i == 1 && lit->next == NULL && lit->sign)
	  ord = unfold_order(alpha, beta);
	/* If not oriented, check the primary ordering (LPO, RPO, KBO). */
	if (ord == NOT_COMPARABLE)
	  ord = term_order(alpha, beta);

	if (ord == GREATER_THAN)
	  mark_oriented_eq(atom);
	else if (ord == LESS_THAN) {
	  if (allow_flips) {
	    flip_eq(atom, i);
	    mark_oriented_eq(atom);
	  }
	  else {
#if 0  /* disable, because this happens regularly during uncompression */
	    fprintf(stderr, "WARNING: orient_equalities, backward eq1:");
	    fwrite_term_nl(stderr, atom);
#endif
	  }
	}
	else {
	  /* Not orientable by the primary ordering. */
	  /* We could call renamable_flip_eq_test, but we might need
	     a1 and a2 for the secondary test. */
	  Term a1 = copy_term(atom);
	  Term a2 = copy_term(atom);
	  Term tmp = ARG(a2,0);
	  ARG(a2,0) = ARG(a2,1);
	  ARG(a2,1) = tmp;
	  term_renumber_variables(a1, MAX_VARS);
	  term_renumber_variables(a2, MAX_VARS);
	  if (term_ident(a1,a2))
	    /* The renumbered flip is identical to the renumbered original. */
	    mark_renamable_flip(atom);  /* We won't para from both sides. */
	  else {
	    /* Flip if the right side is greater in the secondary ordering. */
	    ord = term_compare_vcp(a1, a2);
	    if (ord == LESS_THAN) {
	      if (allow_flips)
		flip_eq(atom, i);
	      else {
		fprintf(stderr, "WARNING: orient_equalities, backward eq2:");
		fwrite_term_nl(stderr, atom);
	      }
	    }
	  }
	  zap_term(a1);
	  zap_term(a2);
	}
      }  /* not identical */
    }  /* eq_atom */
  }  /* foreach literal */
}  /* orient_equalities */

/*************
 *
 *   eq_tautology()
 *
 *************/

/* DOCUMENTATION
This routine returns TRUE if the clause has any literals of the form t=t.
*/

/* PUBLIC */
BOOL eq_tautology(Topform c)
{
  Literals l1;
  for (l1 = c->literals; l1; l1 = l1->next) {
    Term a = l1->atom;
    if (l1->sign && eq_term(a) && term_ident(ARG(a,0), ARG(a,1)))
      return TRUE;
  }
  return FALSE;
}  /* eq_tautology */

/*************
 *
 *   top_flip()
 *
 *************/

/* DOCUMENTATION
Given a binary term (or atom), return the flip.
The two arguments are not copied.
When done with it, call zap_top_flip(t) instead of
zap_term so that the arguments are not zapped.
*/

/* PUBLIC */
Term top_flip(Term a)
{
  Term b;
  if (ARITY(a) != 2)
    fatal_error("top_flip, arity != 2");
  b = get_rigid_term_like(a);
  ARG(b,0) = ARG(a,1);
  ARG(b,1) = ARG(a,0);
  b->private_flags = a->private_flags;
  return b;
}  /* top_flip */

/*************
 *
 *   zap_top_flip()
 *
 *************/

/* DOCUMENTATION
Free a term created by top_flip.
*/

/* PUBLIC */
void zap_top_flip(Term a)
{
  free_term(a);
}  /* zap_top_flip */

/*************
 *
 *   literal_flip()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literals literal_flip(Literals a)
{
  Literals b = get_literals();
  b->sign = a->sign;
  b->atom = top_flip(a->atom);
  return b;
}  /* literal_flip */

/*************
 *
 *   zap_literal_flip()
 *
 *************/

/* DOCUMENTATION
Free a literal created by literal_flip.
*/

/* PUBLIC */
void zap_literal_flip(Literals a)
{
  free_term(a->atom);
  free_literals(a);
}  /* zap_literal_flip */

/*************
 *
 *   clause_with_new_constant()
 *
 *************/

static
Topform clause_with_new_constant(Topform c, Term arg, int new_constant_sn)
{
  Term atom = get_rigid_term(eq_sym(), 2);
  Literals lit = get_literals();
  Topform new = get_topform();
  lit->sign = TRUE;
  lit->atom = atom;
  ARG(atom,0) = copy_term(arg);
  ARG(atom,1) = get_rigid_term_dangerously(new_constant_sn, 0);
  new->literals = append_literal(new->literals, lit);
  upward_clause_links(new);
  new->justification = new_symbol_just(c);
  return new;
}  /* clause_with_new_constant */

/*************
 *
 *   new_constant()
 *
 *************/

/* DOCUMENTATION
If the Topform is a positive equality unit a(x) = b(y) with two variables
in which one variable occurs in each side, infer a clause
a(x) = c with a new constant c.  We could also infer b(y)=c, but
that will come by other inference mechanisms.
<p>
If new_sn == INT_MAX, a new symbol will be generated; otherwise,
new_sn will be used as the symbol number of the new symbol.
*/

/* PUBLIC */
Topform new_constant(Topform c, int new_sn)
{
  if (!pos_eq_unit(c->literals))
    return NULL;
  else {
    Term a = ARG(c->literals->atom,0);
    Term b = ARG(c->literals->atom,1);
    Plist va = set_of_variables(a);
    Plist vb = set_of_variables(b);
    if (va && !va->next && vb && !vb->next && !term_ident(va->v, vb->v)) {
      if (new_sn == INT_MAX) {
	new_sn = fresh_symbol("c_", 0);
	new_constant_properties(new_sn);  /* type, weights, prec */
      }
      return clause_with_new_constant(c, a, new_sn);
    }
    else
      return NULL;
  }
}  /* new_constant */

/*************
 *
 *   fold_denial()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform fold_denial(Topform c, int alpha_max)
{
  static BOOL done = FALSE;
  if (done || !neg_eq_unit(c->literals) || !ground_clause(c->literals))
    return NULL;
  else {
    /* assume it's already oriented */
    Term alpha = ARG(c->literals->atom,0);
    Term beta =  ARG(c->literals->atom,1);

    if (!CONSTANT(beta) && 
	(alpha_max == -1 || symbol_count(alpha) <= alpha_max)) {
      int new_constant_sn = fresh_symbol("c_", 0);
      Topform ca = clause_with_new_constant(c, alpha, new_constant_sn);
      new_constant_properties(new_constant_sn);  /* type, weights, prec */
      done = TRUE;
      return ca;
    }
    else
      return NULL;
  }
}  /* fold_demial */

/*************
 *
 *   equational_def_2()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL equational_def_2(Term alpha, Term beta)
{
  return (!VARIABLE(alpha) && 
	  args_distinct_vars(alpha) &&
	  variables_subset(beta, alpha) &&
	  symbol_occurrences(beta, SYMNUM(alpha)) == 0);
}  /* equational_def_2 */

/*************
 *
 *   equational_def()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int equational_def(Topform c)
{
  if (!pos_eq_unit(c->literals))
    return 0;
  else {
    Term left = ARG(c->literals->atom,0);
    Term right  = ARG(c->literals->atom,1);
    if (equational_def_2(left, right))
      return 1;
    else if (equational_def_2(right, left))
      return 2;
    else
      return 0;
  }
}  /* equational_def */

/*************
 *
 *   unfold_order()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ordertype unfold_order(Term alpha, Term beta)
{
  if (!VARIABLE(alpha) &&
      is_unfold_symbol(SYMNUM(alpha)) &&
      equational_def_2(alpha, beta))
    return GREATER_THAN;

  else if (!VARIABLE(beta) &&
	   is_unfold_symbol(SYMNUM(beta)) &&
	   equational_def_2(beta, alpha))
    return LESS_THAN;
  else
    return NOT_COMPARABLE;
}  /* unfold_order */

/*************
 *
 *   build_reflex_eq()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform build_reflex_eq(void)
{
  Term alpha = get_variable_term(0);
  Term beta  = get_variable_term(0);
  Term atom  = build_binary_term(str_to_sn(eq_sym(), 2), alpha, beta);
  Literals lit = get_literals();
  Topform c = get_topform();
  lit->sign = TRUE;
  lit->atom = atom;
  c->literals = append_literal(c->literals, lit);
  upward_clause_links(c);
  return c;
}  /* build_reflex_eq */

