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

#include "flatdemod.h"

/* Private definitions and types */

static int Fdemod_attempts = 0;
static int Fdemod_rewrites = 0;

/*************
 *
 *    fapply_demod()
 *
 *************/

/* DOCUMENTATION
Special-purpose apply for Flatterm demodulation.

Apply a substitution to a (ordinary) Term, building a Flatterm.
Assumptions:
(1) the terms in the substitution are Flatterms;
(2) every variable in the term is bound.

In the result, Flatterms that are copied from the substitution
have the "reduced_flag" set.
*/

/* PUBLIC */
Flatterm fapply_demod(Term t, Context c)
{
  Flatterm f;

  if (VARIABLE(t)) {
    f = copy_flatterm((Flatterm) c->terms[VARNUM(t)]);
    f->reduced_flag = TRUE;
  }
  else {
    int n = 1;
    int i;
    Flatterm end;
    f = get_flatterm();
    f->private_symbol = t->private_symbol;
    ARITY(f) = ARITY(t);
    end = f;

    for (i = 0; i < ARITY(t); i++) {
      Flatterm a = fapply_demod(ARG(t,i), c);
      n += a->size;
      end->next = a;
      a->prev = end;
      end = a->end;
    }
    f->size = n;
    f->end = end;
  }
  return f;
}  /* fapply_demod */

enum { GO, BACKTRACK };

/*************
 *
 *   discrim_flat_retrieve_leaf()
 *
 *************/

#if 0
static
void maybe_unbind(Flatterm f, Context subst)
{
  if (f->varnum_bound_to >= 0) {
    subst->terms[f->varnum_bound_to] = NULL;
    f->varnum_bound_to = -1;
  }
}  /* maybe_unbind */
#endif

#define MAYBE_UNBIND(f,c)  if (f->varnum_bound_to >= 0) { c->terms[f->varnum_bound_to] = NULL; f->varnum_bound_to = -1; }

/* DOCUMENTATION
*/

/* PUBLIC */
Plist discrim_flat_retrieve_leaf(Flatterm fin, Discrim root,
				 Context subst, Flatterm *ppos)
{
  Flatterm f = NULL;
  Discrim d = NULL;
  BOOL status = GO;

  if (root) {  /* first call */
    d = root->u.kids;
    f = fin;
    if (d == NULL)
      return NULL;
    else
      status = GO;
  }
  else {
    f = *ppos;
    status = BACKTRACK;
  }

  while (TRUE) {

    if (status == BACKTRACK) {
      while (f != fin && f->alternative == NULL) {
	MAYBE_UNBIND(f, subst);
	f = f->prev;
      }
      if (f == fin)
	return NULL;  /* fail */
      else {
	MAYBE_UNBIND(f, subst);
	d = f->alternative;
	f->alternative = NULL;
	status = GO;
      }
    }  /* backtrack */

    if (DVAR(d)) {
      int varnum = d->symbol;
      Flatterm b = (Flatterm) subst->terms[varnum];
      if (b) {
	f->alternative = d->next;
	if (flatterm_ident(f, b))
	  f = f->end;  /* already bound to identical term */
	else
	  status = BACKTRACK;  /* already bound to something else */
      }
      else {
	subst->terms[varnum] = (Term) f;
	f->varnum_bound_to = varnum;
	f->alternative = d->next;
	f = f->end;
      }
    }
    else if (VARIABLE(f))
      status = BACKTRACK;
    else {
      int symnum = SYMNUM(f);
      Discrim dd = NULL;
      while (d && d->symbol < symnum) {
	dd = d;
	d = d->next;
      }
      if (!d || d->symbol != symnum)
	status = BACKTRACK;
    }
    
    if (status == GO) {
      if (f == fin->end) {
	*ppos = f;
	return d->u.data;  /* succeed */
      }
      else {
	f = f->next;
	d = d->u.kids;
      }
    }  /* go */
  }  /* while */
}  /* discrim_flat_retrieve_leaf */

/*************
 *
 *    discrim_flat_retrieve_first(t, root, subst, ppos)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void *discrim_flat_retrieve_first(Flatterm f, Discrim root,
				  Context subst, Discrim_pos *ppos)
{
  Plist tp;
  Flatterm f2 = NULL;

  tp = discrim_flat_retrieve_leaf(f, root, subst, &f2);
  if (tp == NULL)
    return NULL;
  else {
    Discrim_pos pos = get_discrim_pos();
    pos->query = f;
    pos->subst = subst;
    pos->data = tp;
    pos->backtrack = f2;
    *ppos = pos;
    return tp->v;
  }
}  /* discrim_flat_retrieve_first */

/*************
 *
 *    discrim_flat_retrieve_next(ppos)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void *discrim_flat_retrieve_next(Discrim_pos pos)
{
  Plist tp;
    
  tp = pos->data->next;
  if (tp != NULL) {  /* if any more terms in current leaf */
    pos->data = tp;
    return tp->v;
  }
  else {  /* try for another leaf */
    tp = discrim_flat_retrieve_leaf(pos->query, NULL, pos->subst,
				    (Flatterm *) &(pos->backtrack));
    if (tp != NULL) {
      pos->data = tp;
      return tp->v;
    }
    else {
      free_discrim_pos(pos);
      return NULL;
    }
  }
}  /* discrim_flat_retrieve_next */

/*************
 *
 *    discrim_flat_cancel(pos)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void discrim_flat_cancel(Discrim_pos pos)
{
  Flatterm f = pos->backtrack;
  Flatterm query = (Flatterm) pos->query;

  while (f != query->prev) {
    if (f->varnum_bound_to >= 0) {
      pos->subst->terms[f->varnum_bound_to] = NULL;
      f->varnum_bound_to = -1;
    }
    f->alternative = NULL;
    f = f->prev;
  }
  free_discrim_pos(pos);
}  /* discrim_flat_cancel */

/*************
 *
 *   fdemod()
 *
 *************/

static
Flatterm fdemod(Flatterm f, Discrim root, Context subst,
		int *step_limit,
		int size_limit,
		int *current_size,
		int *sequence, I3list *just_head, BOOL lex_order_vars)
{
  if (*step_limit == 0 || *current_size > size_limit)
    return f;
  else if (VARIABLE(f))
    return f;
  else if (f->reduced_flag) {
    (*sequence) += flatterm_count_without_vars(f);
    return f;
  }
  else {
    int sequence_save = *sequence;
    { /* demodulate subterms */
      Flatterm arg = f->next;
      Flatterm end = f;
      int n = 1;
      int i;
      for (i = 0; i < ARITY(f); i++) {
	Flatterm next = arg->end->next;
	Flatterm a2 = fdemod(arg, root, subst, step_limit,
			     size_limit, current_size,
			     sequence, just_head, lex_order_vars);
	n += a2->size;
	end->next = a2;
	a2->prev = end;
	end = a2->end;
	arg = next;
      }
      f->size = n;
      f->end = end;
      f->prev = end->next = NULL;  /* helpful for debugging */
    } /* end: demodulate subterms */

    if (*current_size > size_limit)
      return f;  /* size limit has been reached */
    else if (*step_limit == 0)
      return f;  /* step limit has been reached */
    else {
      /* try to rewrite top */
      Discrim_pos dpos;
      Term candidate = discrim_flat_retrieve_first(f, root, subst, &dpos);
      BOOL rewrite = FALSE;

      Fdemod_attempts++;
      (*sequence)++;

      while (candidate && !rewrite) {
	Topform demodulator = candidate->container;
	Term atom = demodulator->literals->atom;
	Term alpha = ARG(atom, 0);
	Term beta = ARG(atom, 1);
	BOOL match_left = (candidate == alpha);
	Term other = (match_left ? beta : alpha);
	Flatterm contractum = fapply_demod(other, subst);

	if (oriented_eq(atom))
	  rewrite = TRUE;
	else if (flat_greater(f, contractum, lex_order_vars)) {
	  rewrite = TRUE;
	  /*
	    printf("kbo=%d ", rewrite); p_flatterm(f);
	    printf("      "    ); p_flatterm(contractum);
	    printf("\n");
	  */
	}
	else
	  rewrite = FALSE;
	
	if (rewrite) {
	  int increase_in_size = contractum->size - f->size;
	  (*current_size) += increase_in_size;  /* likely to be negative */
	  (*step_limit)--;

	  Fdemod_rewrites++;
	  discrim_flat_cancel(dpos);
	  zap_flatterm(f);
	  *just_head = i3list_prepend(*just_head,
				      demodulator->id,
				      *sequence,
				      match_left ? 1 : 2);
	  *sequence = sequence_save;
	  f = fdemod(contractum, root, subst, step_limit,
		     size_limit, current_size,
		     sequence, just_head, lex_order_vars);
	}
	else {
	  zap_flatterm(contractum);
	  candidate = discrim_flat_retrieve_next(dpos);
	}
      }
    } /* end: try to rewrite top */
    f->reduced_flag = TRUE;
    return(f);
  }
}  /* fdemod */

/*************
 *
 *   fdemodulate()
 *
 *************/

/* DOCUMENTATION
This routine demodulates a term.  ID numbers of demodulators
are put on the front of just_head, so you'll probably want
to reverse the list before putting it into the clause justification.
<P>
This version uses flatterm retrieval.
*/

/* PUBLIC */
Term fdemodulate(Term t, Discrim root,
		 int *step_limit, int *increase_limit, int *sequence,
		 I3list *just_head, BOOL lex_order_vars)
{
  Flatterm f = term_to_flatterm(t);
  Context subst = get_context();
  int current_size = f->size;
  int size_limit = (*increase_limit==INT_MAX) ? INT_MAX : current_size + *increase_limit;
  Flatterm f2 = fdemod(f, root, subst, step_limit,
		       size_limit, &current_size,
		       sequence, just_head, lex_order_vars);
  free_context(subst);

  if (current_size > size_limit)
    *increase_limit = -1;  /* lets callers know that limit has been exceeded */

  if (*just_head == NULL) {
    zap_flatterm(f2);
    return t;
  }
  else {
    Term t2 = flatterm_to_term(f2);
    zap_flatterm(f2);
    zap_term(t);
    return t2;
  }
}  /* fdemodulate */

/*************
 *
 *   fdemod_attempts()
 *
 *************/

/* DOCUMENTATION
Return the number of flatterm rewrite attempts so far.
*/

/* PUBLIC */
int fdemod_attempts()
{
  return Fdemod_attempts;
}  /* fdemod_attempts */

/*************
 *
 *   fdemod_rewrites()
 *
 *************/

/* DOCUMENTATION
Return the number of successful flatterm rewrites so far.
*/

/* PUBLIC */
int fdemod_rewrites()
{
  return Fdemod_rewrites;
}  /* fdemod_rewrites */

/*************
 *
 *   fdemod_clause()
 *
 *************/

/* DOCUMENTATION
Demodulate Topform c, using demodulators in Mindex idx.
If any rewriting occurs, the justification is appended to
the clause's existing justification.
<P>
This version uses flatterm retrievel.
*/

/* PUBLIC */
void fdemod_clause(Topform c, Mindex idx,
		   int *step_limit, int *increase_limit, BOOL lex_order_vars)
{
  if (mindex_empty(idx))
    return;
  else {
    Literals lit;
    I3list steps = NULL;
    /* The "sequence" variable counts the (nonvariable) subterms that
       are visited as we demodulate.  Bottom-up, left-to-right.  When
       a term is rewritten, the justification is a triple of integers:
       <demodulator-id, sequence-number, direction>.  The sequence number
       n means that the n-th node is rewritten, counting as if this
       is the only demodulation step applied to the clause.
     */
    int sequence = 0;
    for (lit = c->literals; lit != NULL; lit = lit->next)
      lit->atom = fdemodulate(lit->atom, idx->discrim_tree,
			      step_limit, increase_limit,
			      &sequence, &steps, lex_order_vars);
			      
    upward_clause_links(c);
    if (steps != NULL) {
      steps = reverse_i3list(steps);
      c->justification = append_just(c->justification, demod_just(steps));
    }
  }
}  /* fdemod_clause */

