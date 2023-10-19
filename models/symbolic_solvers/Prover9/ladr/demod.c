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

#include "demod.h"
#include "dollar.h"

/* Private definitions and types */

/* These are just statistics. */

static int Demod_attempts = 0;
static int Demod_rewrites = 0;

/*************
 *
 *   demodulator_type()
 *
 *************/

/* DOCUMENTATION
Return NOT_DEMODULATOR, ORIENTED, LEX_DEP_LR, LEX_DEP_RL, or LEX_DEP_BOTH.
*/

/* PUBLIC */
int demodulator_type(Topform c, int lex_dep_demod_lim, BOOL sane)
{
  if (!pos_eq_unit(c->literals))
    return NOT_DEMODULATOR;
  else {
    Term atom = c->literals->atom;
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    int n_alpha = symbol_count(alpha);
    int n_beta = symbol_count(beta);
      
    if (oriented_eq(atom))
      return ORIENTED;
    else if (sane && n_alpha != n_beta)
      return NOT_DEMODULATOR;
    else if (lex_dep_demod_lim != -1 && 
	     n_alpha + n_beta + 1 > lex_dep_demod_lim)
      return NOT_DEMODULATOR;
    else {
      Plist alpha_vars = set_of_variables(alpha);
      Plist beta_vars = set_of_variables(beta);
      BOOL lr = plist_subset(beta_vars, alpha_vars) && !VARIABLE(alpha);
      BOOL rl = !renamable_flip_eq(atom) &&
		plist_subset(alpha_vars, beta_vars) && !VARIABLE(beta);
	
      zap_plist(alpha_vars);
      zap_plist(beta_vars);

      if (lr && rl)
	return LEX_DEP_BOTH;
      else if (lr)
	return LEX_DEP_LR;
      else if (rl)
	return LEX_DEP_RL;
      else
	return NOT_DEMODULATOR;
    }
  }
}  /* demodulator_type */

/*************
 *
 *   idx_demodulator()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void idx_demodulator(Topform c, int type, Indexop operation, Mindex idx)
{
  Term atom = c->literals->atom;
  Term alpha = ARG(atom, 0);
  Term beta  = ARG(atom, 1);

  if (type == ORIENTED ||
      type == LEX_DEP_LR ||
      type == LEX_DEP_BOTH)
    mindex_update(idx, alpha, operation);  /* index for left->right */

  if (type == LEX_DEP_RL ||
      type == LEX_DEP_BOTH)
    mindex_update(idx, beta, operation);   /* index for right->left */
}  /* idx_demodulator */

/*************
 *
 *   demod_attempts()
 *
 *************/

/* DOCUMENTATION
Return the number of rewrite attempts so far (for the whole process).
*/

/* PUBLIC */
int demod_attempts()
{
  return Demod_attempts;
}  /* demod_attempts */

/*************
 *
 *   demod_rewrites()
 *
 *************/

/* DOCUMENTATION
Return the number of successful rewrites so far (for the whole process).
*/

/* PUBLIC */
int demod_rewrites()
{
  return Demod_rewrites;
}  /* demod_rewrites */

/*************
 *
 *   demod()
 *
 *   For non-AC terms.
 *
 *************/

static
Term demod(Term t, Mindex demods, int flag, Ilist *just_head,
	   BOOL lex_order_vars)
{
  if (term_flag(t, flag) || VARIABLE(t))
    ;  /* leave it alone */
  else {
    Discrim_pos dpos;
    Term found;
    Context c;  /* allocate after we demoulate the subterms */
    int i;

    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = demod(ARG(t,i), demods, flag, just_head, lex_order_vars);

    c = get_context();
    Demod_attempts++;
    found = discrim_bind_retrieve_first(t, demods->discrim_tree, c, &dpos);
				  
    while (found != NULL) {
      Topform demodulator = found->container;
      Term atom = demodulator->literals->atom;
      Term alpha = ARG(atom, 0);
      Term beta = ARG(atom, 1);
      BOOL match_left = (found == alpha);
      Term other = (match_left ? beta : alpha);
      Term contractum = apply_demod(other, c, flag);
      BOOL ok;

      if (oriented_eq(atom))
	ok = TRUE;
      else
	ok = term_greater(t, contractum, lex_order_vars);  /* LPO, RPO, KBO */
      
      if (ok) {
	discrim_bind_cancel(dpos);
	Demod_rewrites++;
	zap_term(t);
	*just_head = ilist_prepend(*just_head, demodulator->id);
	*just_head = ilist_prepend(*just_head, match_left ? 1 : 2);
	t = demod(contractum, demods, flag, just_head, lex_order_vars);
	found = NULL;
      }
      else {
	zap_term(contractum);
	found = discrim_bind_retrieve_next(dpos);
      }
    }
    free_context(c);
    term_flag_set(t, flag);  /* Mark as fully demodulated. */
  }
  return(t);
}  /* demod */

/*************
 *
 *    contract_bt
 *
 *************/

static
Term contract_bt(Term t, Mindex demods, int flag, Topform *demodulator_ptr)
{
  Mindex_pos pos;
  Term contractum, alpha;
  Topform demodulator = NULL;
  Context c = get_context();

  alpha = mindex_retrieve_first(t,demods,GENERALIZATION,NULL,c,TRUE,&pos);
				  
  if (alpha == NULL)
    contractum = NULL;
  else {
    Term atom;
    demodulator = alpha->container;
    atom = demodulator->literals->atom;
    contractum = apply_demod(ARG(atom,1), c, flag);
    if (c->partial_term) {
      /* Get copy, including marks that indicate normal terms. */
      Term partial = apply_demod(c->partial_term, NULL, flag);
      contractum = build_binary_term(SYMNUM(t), contractum, partial);
    }
    mindex_retrieve_cancel(pos);
  }
  free_context(c);
  *demodulator_ptr = demodulator;
  return(contractum);
}  /* contract_bt */

/*************
 *
 *    demod_bt
 *
 *************/

static
Term demod_bt(Term t, Mindex demods, int psn, int flag,
	      Ilist *just_head)
{
  if (term_flag(t, flag) || VARIABLE(t))
    ;  /* leave it alone */
  else {
    int i;
    int sn = SYMNUM(t);
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = demod_bt(ARG(t,i), demods, sn, flag, just_head);

    if (sn == psn && is_assoc_comm(sn))
      ;  /* t is part of an AC term, so leave it alone. */
    else {
      Term contractum;
      Topform demodulator;
      ac_canonical(t, flag);
      Demod_attempts++;
      contractum = contract_bt(t, demods, flag, &demodulator);
      if (contractum) {
	Demod_rewrites++;
	zap_term(t);
	ac_canonical(contractum, flag);
	*just_head = ilist_prepend(*just_head, demodulator->id);
	t = demod_bt(contractum, demods, psn, flag, just_head);
      }
    }
  }
  /* Mark as fully demodulated.  This also means ac_canonical. */
  term_flag_set(t, flag);
  return(t);
}  /* demod_bt */

/*************
 *
 *   demodulate()
 *
 *************/

/* DOCUMENTATION
This routine demodulates a term.  ID numbers of demodulators
are put on the front of just_head, so you'll probably want
to reverse the list before putting it into the clause justification.
*/

/* PUBLIC */
Term demodulate(Term t, Mindex demods, Ilist *just_head, BOOL lex_order_vars)
{
  int flag = claim_term_flag();
  Term result;

  if (demods->unif_type == ORDINARY_UNIF)
    result = demod(t, demods, flag, just_head, lex_order_vars);
  else
    result = demod_bt(t, demods, -1, flag, just_head);
  term_flag_clear_recursively(result, flag);
  release_term_flag(flag);
  return result;
}  /* demodulate */

/*************
 *
 *   demod1_recurse()
 *
 *   For non-AC terms.  Similar to demod(), but do at most one rewrite
 *   and return a position.
 *
 *************/

static
Term demod1_recurse(Term top, Term t, Topform demodulator, int direction,
		    Ilist *ipos, BOOL lex_order_vars)
{
  if (VARIABLE(t))
    ;  /* leave it alone */
  else {
    int i;

    for (i = 0; i < ARITY(t) && *ipos == NULL; i++)
      ARG(t,i) = demod1_recurse(top,ARG(t,i),demodulator,direction,ipos,
				lex_order_vars);

    if (*ipos == NULL) {
      Context c1 = get_context();
      Trail tr = NULL;
      Term atom = demodulator->literals->atom;
      BOOL match_left = (direction == 1);
      Term t1 = ARG(atom, match_left ? 0 : 1);
      Term t2 = ARG(atom, match_left ? 1 : 0);

      if (match(t1, c1, t, &tr)) {
	Term contractum = apply_demod(t2, c1, -1);
	BOOL ok;
	
	if (oriented_eq(atom))
	  ok = TRUE;
	else
	  ok = term_greater(t, contractum, lex_order_vars); /* LPO, RPO, KBO */
	
	if (ok) {
	  undo_subst(tr);
	  *ipos = position_of_subterm(top, t);
	  zap_term(t);
	  t = contractum;
	}
	else
	  zap_term(contractum);
      }
      free_context(c1);
    }
  }
  return(t);
}  /* demod1_recurse */

/*************
 *
 *   demod1()
 *
 *************/

/* DOCUMENTATION
Special purpose demodulation routine.
<P>
Given a clause and a demodulator that rewrites the clause,
rewrite the innermost leftmost subterm to which
the demodulator applies.  Return the rewritten term
and the position vectors of the from and into terms.
*/

/* PUBLIC */
void demod1(Topform c, Topform demodulator, int direction,
	    Ilist *fpos, Ilist *ipos,
	    BOOL lex_order_vars)
{
  Term result;
  Literals lit;
  int n = 0;

  for (lit = c->literals, *ipos = NULL; lit && *ipos == NULL; lit=lit->next) {
    n++;
    result = demod1_recurse(lit->atom, lit->atom, demodulator,direction,ipos,
			    lex_order_vars);
  }

  if (*ipos == NULL)
    fatal_error("demod1, clause not rewritable");
  else {
    *fpos = ilist_prepend(NULL, direction);  /* side of demodulator */
    *fpos = ilist_prepend(*fpos, 1);  /* literal number */
    *ipos = ilist_prepend(*ipos, n);  /* literal number */
  }

}  /* demod1 */

/*************
 *
 *   part_recurse()
 *
 *************/

static
Term part_recurse(Term top, Term t, Topform demod, int target, int direction,
		  int *sequence, Ilist *ipos)
{
  if (VARIABLE(t))
    return t;
  else {
    int i;
    for (i = 0; i < ARITY(t) && *ipos == NULL; i++) {
      ARG(t,i) = part_recurse(top, ARG(t,i), demod, target,
			      direction, sequence, ipos);
    }
    
    if (*ipos != NULL || *sequence >= target)
      return t;
    else {
      (*sequence)++;
      if (*sequence != target)
	return t;
      else {
	Term alpha, beta;
	Context subst = get_context();
	Trail tr = NULL;
	if (direction == 1) {
	  alpha = ARG(demod->literals->atom, 0);
	  beta  = ARG(demod->literals->atom, 1);
	}
	else {
	  alpha = ARG(demod->literals->atom, 1);
	  beta  = ARG(demod->literals->atom, 0);
	}
	if (match(alpha, subst, t, &tr)) {
	  Term result = apply(beta, subst);
	  undo_subst(tr);
	  *ipos = position_of_subterm(top, t);
	  zap_term(t);
	  t = result;
	}
	else {
	  printf("demodulator%s: ", direction == 1 ? "" : " (R)");
	  fprint_clause(stdout, demod);
	  printf("target=%d: ", target); p_term(t); 
	  fatal_error("part_recurse, not rewitable");
	}
	free_context(subst);
	return t;
      }
    }
  }
}  /* part_recurse */

/*************
 *
 *   particular_demod()
 *
 *************/

/* DOCUMENTATION
Special purpose demodulation routine.
<P>
Given a clause and <demodulator,positition,direction> which applies to the
clause, return the rewritten clause and the position vectors of the from
and into terms.
*/

/* PUBLIC */
void particular_demod(Topform c, Topform demodulator, int target, int direction,
		      Ilist *fpos, Ilist *ipos)
{
  Literals lit;
  int n = 0;
  int sequence = 0;

  for (lit = c->literals, *ipos = NULL; lit && *ipos == NULL; lit=lit->next) {
    n++;
    part_recurse(lit->atom, lit->atom, demodulator, target, direction, &sequence, ipos);
  }

  if (*ipos == NULL)
    fatal_error("particular_demod, clause not rewritable");
  else {
    *fpos = ilist_prepend(NULL, direction);  /* side of demodulator */
    *fpos = ilist_prepend(*fpos, 1);  /* literal number */
    *ipos = ilist_prepend(*ipos, n);  /* literal number */
    upward_clause_links(c);
  }
}  /* particular_demod */

