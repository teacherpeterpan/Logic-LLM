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

#include "clash.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_CLASH PTRS(sizeof(struct clash))
static unsigned Clash_gets, Clash_frees;

/*************
 *
 *   Clash get_clash()
 *
 *************/

static
Clash get_clash(void)
{
  Clash p = get_cmem(PTRS_CLASH);
  Clash_gets++;
  return(p);
}  /* get_clash */

/*************
 *
 *    free_clash()
 *
 *************/

static
void free_clash(Clash p)
{
  free_mem(p, PTRS_CLASH);
  Clash_frees++;
}  /* free_clash */

/*************
 *
 *   fprint_clash_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the clash package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_clash_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct clash);
  fprintf(fp, "clash (%4d)        %11u%11u%11u%9.1f K\n",
          n, Clash_gets, Clash_frees,
          Clash_gets - Clash_frees,
          ((Clash_gets - Clash_frees) * n) / 1024.);

}  /* fprint_clash_mem */

/*************
 *
 *   p_clash_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the clash package.
*/

/* PUBLIC */
void p_clash_mem()
{
  fprint_clash_mem(stdout, TRUE);
}  /* p_clash_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   append_clash()
 *
 *************/

/* DOCUMENTATION
This routine simply allocates a new clash node, links it in after the
given node, and returns the new node.
*/

/* PUBLIC */
Clash append_clash(Clash p)
{
  Clash q = get_clash();
  if (p != NULL)
    p->next = q;
  return q;
}  /* append_clash */

/*************
 *
 *   zap_clash()
 *
 *************/

/* DOCUMENTATION
Free a clash list.  Contexts in clashable nodes (which are assumed to exist
and be empty) are freed as well.
*/

/* PUBLIC */
void  zap_clash(Clash p)
{
  while (p != NULL) {
    Clash q = p;
    p = p->next;
    if (q->clashable && !q->clashed)
      free_context(q->sat_subst);
    free_clash(q);
  }
}  /* zap_clash */

/*************
 *
 *   atom_to_literal()
 *
 *************/

/* DOCUMENTATION
This routine takes an atom and returns its parent literal.
*/

/* PUBLIC */
Literals atom_to_literal(Term atom)
{
  Topform c = atom->container;
  Literals lit = (c == NULL ? NULL : c->literals);
  while (lit != NULL && lit->atom != atom)
    lit = lit->next;
  return lit;
}  /* atom_to_literal */

/*************
 *
 *   apply_lit()
 *
 *************/

/* DOCUMENTATION
This routine applies a substitution to a literal and returns the
instance.  The given literal is not changed.
*/

/* PUBLIC */
Literals apply_lit(Literals lit, Context c)
{
  return new_literal(lit->sign, apply(lit->atom, c));
}  /* apply_lit */

/*************
 *
 *   lit_position()
 *
 *************/

static
int lit_position(Topform c, Literals lit)
{
  int i = 1;
  Literals l = c->literals;
  while (l != NULL && l != lit) {
    i++;
    l = l->next;
  }
  if (l == lit)
    return i;
  else
    return -1;
}  /* lit_position */

/*************
 *
 *   resolve() - construct a clash-resolvent (for hyper or UR)
 *
 *************/

static
Topform resolve(Clash first, Just_type rule)
{
  Topform r = get_topform();
  Topform nuc =  first->nuc_lit->atom->container;
  Ilist j = ilist_append(NULL, nuc->id);
  Clash p;
  int n;

  /* First, include literals in the nucleus. */
  for (p = first; p != NULL; p = p->next, n++) {
    if (!p->clashed)
      r->literals = append_literal(r->literals,
				   apply_lit(p->nuc_lit, p->nuc_subst));
  }

  r->attributes = cat_att(r->attributes,
			  inheritable_att_instances(nuc->attributes,
						    first->nuc_subst));

  /* Next, include literals in the satellites. */

  n = 1;  /* n-th nucleus literal, starting with 1 */
  for (p = first; p != NULL; p = p->next, n++) {
    if (p->clashed) {
      if (p->sat_lit == NULL) {
	/* special code for resolution with x=x */
	j = ilist_append(j, n);
	j = ilist_append(j, 0);
	j = ilist_append(j, 0);
      }
      else {
	Literals lit;
	Topform sat = p->sat_lit->atom->container;
	int sat_pos = lit_position(sat, p->sat_lit);
	j = ilist_append(j, n);
	j = ilist_append(j, sat->id);
	j = ilist_append(j, p->flipped ? -sat_pos : sat_pos);
	for (lit = sat->literals; lit != NULL; lit = lit->next) {
	  if (lit != p->sat_lit)
	    r->literals = append_literal(r->literals, 
					 apply_lit(lit,  p->sat_subst));
	}
	r->attributes = cat_att(r->attributes,
				inheritable_att_instances(sat->attributes,
							  p->sat_subst));
      }
    }
  }
  r->justification = resolve_just(j, rule);
  upward_clause_links(r);
  return r;
}  /* resolve */

/*************
 *
 *   clash_recurse()
 *
 *************/

static
void clash_recurse(Clash first,
		   Clash p,
		   BOOL (*sat_test) (Literals),
		   Just_type rule,
		   void (*proc_proc) (Topform))
{
  if (p == NULL) {
    /* All clashable literals have been mated, so construct the resolvent. */
    Topform resolvent = resolve(first, rule);
    (*proc_proc)(resolvent);
  }
  else if (!p->clashable | p->clashed)
    clash_recurse(first, p->next, sat_test, rule, proc_proc);
  else {
    Term fnd_atom;
    fnd_atom = mindex_retrieve_first(p->nuc_lit->atom, p->mate_index, UNIFY,
				     p->nuc_subst, p->sat_subst,
				     FALSE, &(p->mate_pos));
    while (fnd_atom != NULL) {
      Literals slit = atom_to_literal(fnd_atom);
      if ((*sat_test)(slit)) {
	p->clashed = TRUE;
	p->flipped = FALSE;
	p->sat_lit = slit;
	clash_recurse(first, p->next, sat_test, rule, proc_proc);
	p->clashed = FALSE;
      }
      fnd_atom = mindex_retrieve_next(p->mate_pos);
    }
    /* If the literal is an equality, try flipping it. */
    if (eq_term(p->nuc_lit->atom)) {
      Term flip = top_flip(p->nuc_lit->atom);
      fnd_atom = mindex_retrieve_first(flip, p->mate_index, UNIFY,
				       p->nuc_subst, p->sat_subst,
				       FALSE, &(p->mate_pos));
      while (fnd_atom != NULL) {
	Literals slit = atom_to_literal(fnd_atom);
	if ((*sat_test)(slit)) {
	  p->clashed = TRUE;
	  p->flipped = TRUE;
	  p->sat_lit = slit;
	  clash_recurse(first, p->next, sat_test, rule, proc_proc);
	  p->clashed = FALSE;
	}
	fnd_atom = mindex_retrieve_next(p->mate_pos);
      }
      zap_top_flip(flip);
    }
    /* Built-in resolution with x=x. */
    if (neg_eq(p->nuc_lit)) {
      Term alpha = ARG(p->nuc_lit->atom,0);
      Term beta  = ARG(p->nuc_lit->atom,1);
      Trail tr = NULL;
      if (unify(alpha, p->nuc_subst, beta, p->nuc_subst, &tr)) {
	p->clashed = TRUE;
	p->sat_lit = NULL;
	clash_recurse(first, p->next, sat_test, rule, proc_proc);
	p->clashed = FALSE;
	undo_subst(tr);
      }
    }
  }
}  /* clash_recurse */

/*************
 *
 *   clash()
 *
 *************/

/* DOCUMENTATION
This routine takes a complete clash list and computes the
resolvents.
<UL>
<LI>clash -- a complete clash list corresponding to the nucleus.
<LI>sat_test -- a Boolean function on clauses which identifies
potential satellites (e.g., positive clauses for hyperresolution).
<LI>rule -- the name of the inference rule for the justification list
(see just.h).
<LI>proc_proc -- procedure for processing resolvents.
</UL>
*/

/* PUBLIC */
void clash(Clash c,
	   BOOL (*sat_test) (Literals),
	   Just_type rule,
	   void (*proc_proc) (Topform))
{
  clash_recurse(c, c, sat_test, rule, proc_proc);
}  /* clash */

