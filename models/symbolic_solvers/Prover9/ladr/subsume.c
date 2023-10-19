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

#include "subsume.h"

/* Private definitions and types */

static int Nonunit_subsumption_tests = 0;

/*************
 *
 *   nonunit_subsumption_tests(void)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int nonunit_subsumption_tests(void)
{
  return Nonunit_subsumption_tests;
}  /* nonunit_subsumption_tests */

/*************
 *
 *   subsume_literals()
 *
 *   This is a recursive routine that tries to map a list of literals
 *   into a clause d.  If successful, the trail is updated; if failure,
 *   the trail is unchanged.
 *
 *   This version uses ordinary unification.
 *
 *************/

static
BOOL subsume_literals(Literals clit, Context subst, Topform d, Trail *trp)
{
  BOOL subsumed = FALSE;
  Literals dlit;

  if (clit == NULL)
    return TRUE;
  else {
    for (dlit = d->literals; !subsumed && dlit != NULL; dlit = dlit->next) {
      if (clit->sign == dlit->sign) {
	Trail mark = *trp;
	if (match(clit->atom, subst, dlit->atom, trp)) {
	  if (subsume_literals(clit->next, subst, d, trp))
	    subsumed = TRUE;
	  else {
	    undo_subst_2(*trp, mark);
	    *trp = mark;
	  }
	}
      }
    }
    return subsumed;
  }
}  /* subsume_literals */

/*************
 *
 *   subsume_bt_literals()
 *
 *   This version uses backtrack unification.
 *
 *************/

static
BOOL subsume_bt_literals(Literals clit, Context subst,
			 Topform d, Plist *gp)
{
  if (clit == NULL)
    return TRUE;
  else {
    BOOL subsumed = FALSE;
    Literals dlit;
    *gp = plist_prepend(*gp, NULL);

    for (dlit=d->literals; !subsumed && dlit!=NULL; dlit=dlit->next) {
      if (clit->sign == dlit->sign) {
	Btm_state bt = match_bt_first(clit->atom, subst, dlit->atom, FALSE);
	while (bt != NULL && !subsumed) {
	  (*gp)->v = bt;
	  if (subsume_bt_literals(clit->next, subst, d, gp))
	    subsumed = TRUE;
	  else {
	    bt = match_bt_next(bt);
	  }
	}
      }
    }
    if (subsumed)
      return TRUE;
    else {
      *gp = plist_pop(*gp);
      return FALSE;
    }
  }
}  /* Subsume_bt_literals */

/*************
 *
 *   subsumes()
 *
 *************/

/* DOCUMENTATION
This routine checks if Topform c subsumes Topform d.
Ordinary unification is used; in particular, symmetry of
equality is not built in.
<P>
*/

/* PUBLIC */
BOOL subsumes(Topform c, Topform d)
{
  Context subst = get_context();
  Trail tr = NULL;
  BOOL subsumed = subsume_literals(c->literals, subst, d, &tr);
  if (subsumed)
    undo_subst(tr);
  free_context(subst);
  Nonunit_subsumption_tests++;
  return subsumed;
}  /* subsumes */

/*************
 *
 *   subsumes_bt()
 *
 *************/

/* DOCUMENTATION
This routine checks if Topform c subsumes Topform d.
Backtrack unification is used; in particular, AC and
commutative/symmetric matching are applied where appropriate.
*/

/* PUBLIC */
BOOL subsumes_bt(Topform c, Topform d)
{
  Context subst = get_context();
  Plist g = NULL;
  int rc = subsume_bt_literals(c->literals, subst, d, &g);
  if (rc) {
    /* Cancel the list (stack) of btm_states */
    while (g != NULL) {
      Btm_state bt = g->v;
      match_bt_cancel(bt);
      g = plist_pop(g);
    }
  }
  free_context(subst);
  return rc;
}  /* subsumes_bt */

/*************
 *
 *   forward_subsume()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform forward_subsume(Topform d, Lindex idx)
{
  Literals dlit;
  Topform subsumer = NULL;
  Context subst = get_context();
  int nd = number_of_literals(d->literals);

  /* We have to consider all literals of d, because when d is
     subsumed by c, not all literals of d have to match with
     a literal in c.  c is indexed on the first literal only.
   */

  for (dlit=d->literals; dlit!=NULL && subsumer==NULL; dlit=dlit->next) {
    Mindex mdx = dlit->sign ? idx->pos : idx->neg;
    Mindex_pos pos;
    Term catom = mindex_retrieve_first(dlit->atom, mdx, GENERALIZATION,
				       NULL, subst, FALSE, &pos);
    BOOL backtrack = lindex_backtrack(idx);
    while (catom != NULL && subsumer == NULL) {
      Topform c = catom->container;
      if (atom_number(c->literals, catom) == 1) {
	int nc = number_of_literals(c->literals);
	/* If c is a unit then we already know it subsumes d; otherwise,
	 * do a full subsumption check on the clauses.  (We don't let
	 * a clause subsume a shorter one, because that would cause
	 * factors to be deleted.)
	 */
	if (nc == 1 || (nc <= nd && (backtrack
				     ? subsumes_bt(c,d)
				     : subsumes(c,d)))) {
	  subsumer = c;
	  mindex_retrieve_cancel(pos);
	}
      }
      if (subsumer == NULL)
	catom = mindex_retrieve_next(pos);
    }
  }
  free_context(subst);
  return subsumer;
}  /* forward_subsume */

/*************
 *
 *   back_subsume()
 *
 *************/

/* DOCUMENTATION
Look in the index and return the list of clauses subsumed by c.
*/

/* PUBLIC */
Plist back_subsume(Topform c, Lindex idx)
{
  int nc = number_of_literals(c->literals);

  if (nc == 0)
    return NULL;
  else {
    Plist subsumees = NULL;
    Context subst = get_context();
    Literals clit = c->literals;

    /* We only have to consider the first literal of c, because when
       c subsumes a clause d, all literals of c have to map into d.
       All literals of d are indexed.
     */

    Mindex mdx = clit->sign ? idx->pos : idx->neg;
    Mindex_pos pos;

    Term datom = mindex_retrieve_first(clit->atom, mdx, INSTANCE,
				       subst, NULL, FALSE, &pos);
    BOOL backtrack = lindex_backtrack(idx);
    while (datom != NULL) {
      Topform d = datom->container;
      if (d != c) {  /* in case c is already in idx */
	int nd = number_of_literals(d->literals);
	/* If c is a unit the we already know it subsumes d; otherwise,
	 * do a full subsumption check on the clauses.  (We don't let
	 * a clause subsume a shorter one.)
	 */
	if (nc == 1 || (nc <= nd && (backtrack
				     ? subsumes_bt(c, d)
				     : subsumes(c, d))))
	  subsumees = insert_clause_into_plist(subsumees, d, FALSE);
      }
      datom = mindex_retrieve_next(pos);
    }
    free_context(subst);
    return subsumees;
  }
}  /* back_subsume */

/*************
 *
 *   back_subsume_one()
 *
 *************/

/* DOCUMENTATION
Look in the index for a clause subsumed by c.
The first one found is returned.  (It is not
necessarily the first of the subsumees that
was inserted into the index.)
*/

/* PUBLIC */
Topform back_subsume_one(Topform c, Lindex idx)
{
  int nc = number_of_literals(c->literals);

  if (nc == 0)
    return NULL;
  else {
    Context subst = get_context();
    Literals clit = c->literals;

    Mindex mdx = clit->sign ? idx->pos : idx->neg;
    Mindex_pos pos;

    Term datom = mindex_retrieve_first(clit->atom, mdx, INSTANCE,
				       subst, NULL, FALSE, &pos);
    BOOL backtrack = lindex_backtrack(idx);
    BOOL found = FALSE;
    Topform d = NULL;

    while (datom != NULL && !found) {
      d = datom->container;
      if (d != c) {  /* in case c is already in idx */
	int nd = number_of_literals(d->literals);
	/* If c is a unit the we already know it subsumes d; otherwise,
	 * do a full subsumption check on the clauses.  (We don't let
	 * a clause subsume a shorter one.)
	 */
	if (nc == 1 || (nc <= nd && (backtrack
				     ? subsumes_bt(c, d)
				     : subsumes(c, d)))) {
	  found = TRUE;
	  mindex_retrieve_cancel(pos);
	}
	else
	  datom = mindex_retrieve_next(pos);
      }
    }
    free_context(subst);
    return found ? d : NULL;
  }
}  /* back_subsume_one */

/*************
 *
 *   atom_conflict()
 *
 *************/

static
void atom_conflict(BOOL flipped, Topform c, BOOL sign,
		    Term a, Lindex idx, void (*empty_proc) (Topform))
{
  int n = 0;
  Context subst1 = get_context();
  Context subst2 = get_context();
  Mindex mdx = sign ? idx->neg : idx->pos;
  Mindex_pos pos;
  Term b = mindex_retrieve_first(a, mdx, UNIFY,
				 subst1, subst2, FALSE, &pos);
  while (b) {
    Topform d = b->container;
    if (number_of_literals(d->literals) == 1) {
      Topform conflictor = d;
      Topform empty = get_topform();

      if (c->id == 0)
	assign_clause_id(c);  /* so that justification makes sense */
      c->used = TRUE;         /* so it won't be discarded */

      empty->justification = binary_res_just(c, 1, conflictor,
					     flipped ? -1 : 1);
      inherit_attributes(c, subst1, conflictor, subst2, empty);
      n++;
      (*empty_proc)(empty);
      b = mindex_retrieve_next(pos);
    }
    else 
      b = mindex_retrieve_next(pos);
  }
  free_context(subst1);
  free_context(subst2);
}  /* atom_conflict */

/*************
 *
 *   unit_conflict_by_index()
 *
 *************/

/* DOCUMENTATION
Look in idx for unit conflicts
*/

/* PUBLIC */
void unit_conflict_by_index(Topform c, Lindex idx, void (*empty_proc) (Topform))
{
  if (number_of_literals(c->literals) == 1) {
    Literals lit = c->literals;
    Term atom = lit->atom;
    atom_conflict(FALSE, c, lit->sign, atom, idx, empty_proc);
    /* maybe try the flip */
    if (eq_term(atom) && !renamable_flip_eq(atom)) {
      Term flip = top_flip(atom);
      atom_conflict(TRUE, c, lit->sign, flip, idx, empty_proc);
      zap_top_flip(flip);
    }
  }
}  /* unit_conflict_by_index */

/*************
 *
 *   try_unit_conflict()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform try_unit_conflict(Topform a, Topform b)
{
  Context c1 = get_context();
  Context c2 = get_context();
  Trail tr = NULL;
  Topform empty = NULL;
  if (unit_clause(a->literals) && unit_clause(b->literals) &&
      a->literals->sign != b->literals->sign &&
      unify(a->literals->atom, c1, b->literals->atom, c2, &tr)) {
    empty = get_topform();
    empty->justification = binary_res_just(a, 1, b, 1);
    inherit_attributes(a, c1, b, c2, empty);
    undo_subst(tr);
  }
  free_context(c1);
  free_context(c2);
  return empty;
}  /* try_unit_conflict */

/*************
 *
 *   unit_delete()
 *
 *************/

/* DOCUMENTATION
Given a clause and a literal index, remove the literals that
can be removed by "unit deletion" with units in the index.
Update the clause's justification for each removed literal.
*/

/* PUBLIC */
void unit_delete(Topform c, Lindex idx)
{
  Context subst = get_context();
  Literals l;
  int i;
  BOOL null_literals = FALSE;

  for (l = c->literals, i = 1; l; l = l->next, i++) {
    Mindex mdx = l->sign ? idx->neg : idx->pos;
    Mindex_pos pos;
    Term datom = mindex_retrieve_first(l->atom, mdx, GENERALIZATION,
				       NULL, subst, FALSE, &pos);
    BOOL ok = FALSE;
    while (datom && !ok) {
      Topform d = datom->container;
      if (unit_clause(d->literals)) {
	ok = TRUE;
	c->justification = append_just(c->justification,
				       unit_del_just(d, i));
	c->attributes = cat_att(c->attributes,
				inheritable_att_instances(d->attributes,
							  subst));
	mindex_retrieve_cancel(pos);
	zap_term(l->atom);
	l->atom = NULL;  /* remove it below */
	null_literals = TRUE;
      }
    }
    /* If still there and equality, try flipping it. */
    if (l->atom && eq_term(l->atom)) {
      Term flip = top_flip(l->atom);
      Term datom = mindex_retrieve_first(flip, mdx, GENERALIZATION,
					 NULL, subst, FALSE, &pos);
      BOOL ok = FALSE;
      while (datom && !ok) {
	Topform d = datom->container;
	if (unit_clause(d->literals)) {
	  ok = TRUE;
	  mindex_retrieve_cancel(pos);
	  c->justification = append_just(c->justification,
					 unit_del_just(d, -i));
	  c->attributes = cat_att(c->attributes,
				  inheritable_att_instances(d->attributes,
							    subst));
	  zap_term(l->atom);
	  l->atom = NULL;  /* remove it below */
	  null_literals = TRUE;
	}
      }
      zap_top_flip(flip);
    }  /* eq_atom */
  }  /* foreach literal */
  if (null_literals) {
    c->literals = remove_null_literals(c->literals);
    c->normal_vars = FALSE;  /* removing literals can make vars non-normal */
  }
  free_context(subst);
}  /* unit_delete */

/*************
 *
 *   back_unit_del_by_index()
 *
 *************/

/* DOCUMENTATION
Given a unit clause and a literal index, return the Plist of
clauses containing literals that are instances
of the negation of the unit clause.
<P>
Such clauses can be "back unit deleted".
*/

/* PUBLIC */
Plist back_unit_del_by_index(Topform unit, Lindex idx)
{
  Plist nonunits = NULL; 
  Context subst = get_context();
  Literals clit = unit->literals;

  Mindex mdx = clit->sign ? idx->neg : idx->pos;
  Mindex_pos pos;

  Term datom = mindex_retrieve_first(clit->atom, mdx, INSTANCE,
				     subst, NULL, FALSE, &pos);

  while (datom != NULL) {
    Topform d = datom->container;
    nonunits = insert_clause_into_plist(nonunits, d, FALSE);
    datom = mindex_retrieve_next(pos);
  }

  /* If equality, do the same with the flip. */

  if (eq_term(clit->atom)) {
    Term flip = top_flip(clit->atom);
    Term datom = mindex_retrieve_first(flip, mdx, INSTANCE,
				       subst, NULL, FALSE, &pos);
    while (datom != NULL) {
      Topform d = datom->container;
      nonunits = insert_clause_into_plist(nonunits, d, FALSE);
      datom = mindex_retrieve_next(pos);
    }
    zap_top_flip(flip);
  }

  free_context(subst);
  return nonunits;
}  /* back_unit_del_by_index */

/*************
 *
 *   simplify_literals()
 *
 *************/

/* DOCUMENTATION
Remove any literals t!=t.
*/

/* PUBLIC */
void simplify_literals(Topform c)
{
  Literals l;
  int i;
  BOOL null_literals = FALSE;

  for (l = c->literals, i = 1; l; l = l->next, i++) {
    Term a = l->atom;
    BOOL sign = l->sign;
    if ((!sign && eq_term(a) && term_ident(ARG(a,0), ARG(a,1))) ||
	(!sign && true_term(a)) ||
	(sign && false_term(a))) {

      c->justification = append_just(c->justification, xx_just(i));
      zap_term(l->atom);
      l->atom = NULL;
      null_literals = TRUE;
    }
  }
  if (null_literals)
    c->literals = remove_null_literals(c->literals);
}  /* simplify_literals */

/*************
 *
 *   eq_removable_literal()
 *
 *************/

/* DOCUMENTATION
Can a literal in a clause be removed by resolution with x=x
without instantiating any other literal in the clause?
<p>
If so, instantiate any inheritable (e.g., answer) attributes
with the corresponding substitution.
*/

/* PUBLIC */
BOOL eq_removable_literal(Topform c, Literals lit)
{
  if (lit->sign || !eq_term(lit->atom))
    return FALSE;
  else {
    Term alpha = ARG(lit->atom, 0);
    Term beta  = ARG(lit->atom, 1);
    Context subst = get_context();
    Trail tr = NULL;
    BOOL ok = unify(alpha, subst, beta, subst, &tr);
    if (ok) {
      /* Check if substitution instantiates any other literal. */
      /* Note that other literals may have atom==NULL. */
      Literals l;
      for (l = c->literals; l && ok; l = l->next) {
	if (l != lit && l->atom != NULL)
	  if (subst_changes_term(l->atom, subst))
	    ok = FALSE;
      }
      if (ok)
	instantiate_inheritable_attributes(c->attributes, subst);
      undo_subst(tr);
    }
    free_context(subst);
    return ok;
  }
}  /* eq_removable_literal */

/*************
 *
 *   simplify_literals2()
 *
 *************/

/* DOCUMENTATION
1. If there are any literals t=t, the clause becomes true_sym().
2. Remove any literals t!=t.
3. If there are any literals s!=t, where unify(s,t), without instantiating
   any other literals, remove the literal.
*/

/* PUBLIC */
void simplify_literals2(Topform c)
{
  Literals l;
  int i;
  BOOL null_literals = FALSE;
  BOOL tautological = FALSE;

  if (!c->normal_vars)
    renumber_variables(c, MAX_VARS);

  for (l = c->literals, i = 1; l && !tautological; l = l->next, i++) {
    Term a = l->atom;
    BOOL sign = l->sign;
    if ((!sign && eq_term(a) && term_ident(ARG(a,0), ARG(a,1))) ||
	/* (!sign && true_term(a)) || */
	/* (sign && false_term(a)) || */
	eq_removable_literal(c, l)) {
      /* literal is FALSE, so remove it */
      c->justification = append_just(c->justification, xx_just(i));
      zap_term(l->atom);
      l->atom = NULL;
      null_literals = TRUE;
    }
    else if ((!sign && true_term(a)) ||
	     (sign && false_term(a))) {
      zap_term(l->atom);
      l->atom = NULL;
      null_literals = TRUE;
    }
    else if ((sign && eq_term(a) && term_ident(ARG(a,0), ARG(a,1))) ||
	     (sign && true_term(a)) ||
	     (!sign && false_term(a)))
      tautological = TRUE;
  }

  if (null_literals) {
    c->literals = remove_null_literals(c->literals);
    c->normal_vars = 0;
    renumber_variables(c, MAX_VARS);
  }

  if (tautological || tautology(c->literals)) {
    zap_literals(c->literals);
    c->literals = new_literal(TRUE, get_rigid_term(true_sym(), 0));
    c->literals->atom->container = c;
    /* justification not necessary because clause will disappear??? */
  }
}  /* simplify_literals2 */
