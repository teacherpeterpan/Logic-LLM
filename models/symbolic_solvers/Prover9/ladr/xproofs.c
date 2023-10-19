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

#include "xproofs.h"
#include "demod.h"

/* #define DEBUG_EXPAND */

/* Private definitions and types */

/*************
 *
 *   check_parents_and_uplinks_in_proof()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void check_parents_and_uplinks_in_proof(Plist proof)
{
  Ilist seen = NULL;
  Plist p;
  for (p = proof; p; p = p->next) {
    Topform c = p->v;
    Ilist parents = get_parents(c->justification, FALSE);
    if (!check_upward_clause_links(c)) {
      printf("bad uplinks: "); fprint_clause(stdout, c);
      fatal_error("check_parents_and_uplinks_in_proof, bad uplinks");
    }
    if (!ilist_subset(parents, seen)) {
      printf("seen:    "); p_ilist(seen);
      printf("parents: "); p_ilist(parents);
      fatal_error("check_parents_and_uplinks_in_proof, parents not seen");
    }
    seen = ilist_prepend(seen, c->id);
    zap_ilist(parents);
  }
  zap_ilist(seen);
}  /* check_parents_and_uplinks_in_proof */

/*************
 *
 *   xx_res2()
 *
 *************/

static
Topform xx_res2(Topform c, int n)
{
  Literals lit = ith_literal(c->literals, n);

  if (lit == NULL ||
      lit->sign == TRUE ||
      !eq_term(lit->atom))
    return NULL;
  else {
    Context subst = get_context();
    Trail tr = NULL;
    Topform res;

    if (unify(ARG(lit->atom,0), subst, ARG(lit->atom,1), subst, &tr)) {
      Literals l2;
      res = get_topform();
      for (l2 = c->literals; l2; l2 = l2->next)
	if (l2 != lit)
	  res->literals = append_literal(res->literals, apply_lit(l2,  subst));

      res->attributes = cat_att(res->attributes,
			     inheritable_att_instances(c->attributes, subst));

      res->justification = xxres_just(c, n);
      upward_clause_links(res);
      renumber_variables(res, MAX_VARS);
      undo_subst(tr);
    }
    else {
      res = NULL;
    }
    free_context(subst);
    return res;
  }
}  /* xx_res2 */

/*************
 *
 *   xx_simp2()
 *
 *************/

static
void xx_simp2(Topform c, int n)
{
  Literals lit = ith_literal(c->literals, n);
  Term a = lit->atom;

  if ((!lit->sign && eq_term(a) && term_ident(ARG(a,0), ARG(a,1))) ||
      eq_removable_literal(c, lit)) {
    zap_term(lit->atom);
    lit->atom = NULL;
    c->literals = remove_null_literals(c->literals);
    c->justification = append_just(c->justification, xx_just(n));
  }
  else {
    printf("\nERROR, literal %d in clause cannot be removed: ", n);
    fprint_clause(stdout, c);
    fatal_error("xx_simp2, bad literal");
  }
}  /* xx_simp2 */

/*************
 *
 *   factor()
 *
 *************/

static
Topform factor(Topform c, int n1, int n2)
{
  Topform fac;
  Literals l1 = ith_literal(c->literals, n1);
  Literals l2 = ith_literal(c->literals, n2);
  Context subst = get_context();

  Trail tr = NULL;

  if (l1->sign == l2->sign && unify(l1->atom, subst, l2->atom, subst, &tr)) {
    Literals lit;
    fac = get_topform();
    for (lit = c->literals; lit; lit = lit->next)
      if (lit != l2)
	fac->literals = append_literal(fac->literals, apply_lit(lit,  subst));

    fac->attributes = cat_att(fac->attributes,
			      inheritable_att_instances(c->attributes, subst));

    fac->justification = factor_just(c, n1, n2);
    upward_clause_links(fac);
    renumber_variables(fac, MAX_VARS);
    undo_subst(tr);
  }
  else
    fac = NULL;
  free_context(subst);
  return fac;
}  /* factor */

/*************
 *
 *   merge1()
 *
 *************/

static
void merge1(Topform c, int n)
{
  Literals target = ith_literal(c->literals, n);
  Literals prev = ith_literal(c->literals, n-1);
  Literals lit = c->literals;
  BOOL go = TRUE;

  while (go) {
    if (lit->sign == target->sign && term_ident(lit->atom, target->atom))
      go = FALSE;
    else
      lit = lit->next;
  }
  if (lit == target)
    fatal_error("merge1, literal does not merge");
  prev->next = target->next;
  zap_literal(target);
  c->justification = append_just(c->justification, merge_just(n));
}  /* merge1 */

/*************
 *
 *   proof_id_to_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform proof_id_to_clause(Plist proof, int id)
{
  Plist p = proof;
  while (p && ((Topform) p->v)->id != id)
    p = p->next;
  if (p == NULL)
    return NULL;
  else
    return p->v;
}  /* proof_id_to_clause */

/*************
 *
 *   greatest_id_in_proof()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int greatest_id_in_proof(Plist proof)
{
  if (proof == NULL)
    return INT_MIN;
  else {
    int x = greatest_id_in_proof(proof->next);
    Topform c = proof->v;
    return (c->id > x ? c->id : x);
  }
}  /* greatest_id_in_proof */

/*************
 *
 *   expand_proof()
 *
 *************/

/* DOCUMENTATION
Given a proof, return a more detailed proof, entirely new, leaving the
given proof unchanged.  Also returned is an I3list mapping new IDs to
pairs <old_id, n>.  The n compnent identifies the sub-steps arising
from the expansions, e.g.,  66 -> <23,4> means that step 66 in the new
proof corresponds to the 4th substep in expanding step 23 of the old proof.

Clauses in the new proof that match clauses in the old proof retain
the IDs from the old proof, and there is no entry in the map for them.
*/

/* PUBLIC */
Plist expand_proof(Plist proof, I3list *pmap)
{
  Plist new_proof = NULL; /* build it backward, reverse at end */
  I3list map = NULL;     /* map new IDs to <old-id,n> for intermediate steps */
  int old_id, old_id_n;  /* for mapping new steps to old */
  int next_id;
  Plist p;

  /* Start numbering the new proof where the old one ends. */

  next_id = greatest_id_in_proof(proof) + 1;

  for (p = proof; p; p = p->next) {
    Topform c = p->v;         /* the clause we're expanding */
    Topform current = NULL;   /* by substeps, this becomes identical to c */
    Just j;

    j = c->justification;
    old_id = c->id;
    old_id_n = 0;  /* this counts substeps of the expansion */

#ifdef DEBUG_EXPAND
    printf("\nexpanding: "); fprint_clause(stdout, c);
#endif

    if (j->next == NULL &&
	j->type != HYPER_RES_JUST &&
	j->type != UR_RES_JUST) {

      /* No expansion is necessary for this step.
	 We take a shortcut by just copying the clause.
      */

      current = copy_clause_ija(c);
      new_proof = plist_prepend(new_proof, current);

      /* The next 2 steps get undone below.  They are performed here
	 so that the state is consistent with the cases in which
	 some expansion occurs.
       */

      map = alist2_insert(map, next_id, old_id, old_id_n++);
      current->id = next_id++;
    }
    else {
      /* To adjust literal numbers when literals disappear. */
      int merges = 0;
      int unit_deletes = 0;
      int xx_simplify = 0;
      /* primary inference */

      if (j->type == COPY_JUST ||
	  j->type == BACK_DEMOD_JUST ||
	  j->type == PROPOSITIONAL_JUST ||
	  j->type == BACK_UNIT_DEL_JUST) {
	/* Note that we get "current" directly from the new proof.
	   This prevents an unnecessary "copy" inference.
	   This assumes there is some secondary justification.
	*/
	current = proof_id_to_clause(new_proof, j->u.id);
      }
      else if (j->type == HYPER_RES_JUST ||
	       j->type == UR_RES_JUST ||
	       j->type == BINARY_RES_JUST) {
	/* c ncn ncn ncn ... (length is 3m+1) */
	Ilist p = j->u.lst;
	Topform c1 = proof_id_to_clause(proof, p->i);
	int j = 0;  /* literals resolved; subtract from nucleus position */
	p = p->next;
	while (p != NULL) {
	  Topform resolvent;
	  int n1 = p->i - j;  /* literal number in c1 */
	  int sat_id = p->next->i;
	  if (sat_id == 0)
	    resolvent = xx_resolve2(c1, n1, TRUE);
	  else {
	    Topform c2 = proof_id_to_clause(proof, sat_id);
	    int n2 = p->next->next->i;
	    resolvent = resolve2(c1, n1, c2, n2, TRUE);
	    if (resolvent == NULL) {
	      printf("Lit %d: ",n1); fprint_clause(stdout, c1);
	      printf("Lit %d: ",n2); fprint_clause(stdout, c2);
	      fatal_error("expand_step, clauses don't resolve");
	    }
	  }
	  map = alist2_insert(map, next_id, old_id, old_id_n++);
	  resolvent->id = next_id++;
	  new_proof = plist_prepend(new_proof, resolvent);
	  c1 = resolvent;
	  j++;
	  p = p->next->next->next;
	}
	current = c1;
      }
      else if (j->type == PARA_JUST) {
	Topform from = proof_id_to_clause(proof, j->u.para->from_id);
	Topform into = proof_id_to_clause(proof, j->u.para->into_id);
	Ilist from_pos = j->u.para->from_pos;
	Ilist into_pos = j->u.para->into_pos;
	current = para_pos(from, from_pos, into, into_pos);  /* does just. */
	map = alist2_insert(map, next_id, old_id, old_id_n++);
	current->id = next_id++;
	new_proof = plist_prepend(new_proof, current);
      }
      else if (j->type == FACTOR_JUST) {
	Ilist p = j->u.lst;
	Topform parent = proof_id_to_clause(proof, p->i);
	int lit1 = p->next->i;
	int lit2 = p->next->next->i;

	current = factor(parent, lit1, lit2);
	map = alist2_insert(map, next_id, old_id, old_id_n++);
	current->id = next_id++;
	new_proof = plist_prepend(new_proof, current);
      }
      else if (j->type == XXRES_JUST) {
	Ilist p = j->u.lst;
	Topform parent = proof_id_to_clause(proof, p->i);
	int lit = p->next->i;

	current = xx_res2(parent, lit);
	map = alist2_insert(map, next_id, old_id, old_id_n++);
	current->id = next_id++;
	new_proof = plist_prepend(new_proof, current);
      }
      else if (j->type == NEW_SYMBOL_JUST) {
	Topform parent = proof_id_to_clause(proof, j->u.id);
	/* Assume EQ unit with right side constant. */
	int sn = SYMNUM(ARG(c->literals->atom,1));
	current = new_constant(parent, sn);
	map = alist2_insert(map, next_id, old_id, old_id_n++);
	current->id = next_id++;
	new_proof = plist_prepend(new_proof, current);
      }
      else {
	printf("expand_step, unknown primary justification\n");
	new_proof = plist_prepend(new_proof, copy_clause_ija(c));
      }

#ifdef DEBUG_EXPAND
      printf("primary: "); fprint_clause(stdout, current);
#endif

      /* secondary inferences */

      for (j = j->next; j; j = j->next) {
	if (j->type == DEMOD_JUST) {
	  /* list of triples: <ID, position, direction> */
	  I3list p;
	  for (p = j->u.demod; p; p = p->next) {
	    Topform demod = proof_id_to_clause(proof, p->i);
	    int position = p->j;
	    int direction = p->k;
	    Ilist from_pos, into_pos;
	    Topform work = copy_clause(current);
	    map = alist2_insert(map, next_id, old_id, old_id_n++);
	    work->id = next_id++;
	    particular_demod(work, demod, position, direction,
			     &from_pos, &into_pos);
	    work->justification = para_just(PARA_JUST,
					    demod, from_pos,
					    current, into_pos);
	    current = work;
	    new_proof = plist_prepend(new_proof, current);
#ifdef DEBUG_EXPAND
	    printf("demod: "); fprint_clause(stdout, current);
#endif
	  }
	}
	else if (j->type == FLIP_JUST) {
	  Term atom;
	  int n = j->u.id;
	  Topform work = copy_inference(current);
	  current = work;
	  map = alist2_insert(map, next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  atom = ith_literal(current->literals, n)->atom;
	  if (!eq_term(atom))
	    fatal_error("expand_step, cannot flip nonequality");
	  flip_eq(atom, n);  /* updates justification */
	  new_proof = plist_prepend(new_proof, current);
#ifdef DEBUG_EXPAND
	  printf("flip: "); fprint_clause(stdout, current);
#endif
	}
	else if (j->type == MERGE_JUST) {
	  int n = j->u.id - merges;
	  Topform work = copy_inference(current);
	  current = work;
	  map = alist2_insert(map, next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  merge1(current, n);  /* updates justification */
	  new_proof = plist_prepend(new_proof, current);
#ifdef DEBUG_EXPAND
	  printf("merge: "); fprint_clause(stdout, current);
#endif
	  merges++;
	}
	else if (j->type == UNIT_DEL_JUST) {
	  Ilist p = j->u.lst;
	  int n = p->i - unit_deletes;
	  Topform unit = proof_id_to_clause(proof, p->next->i);
	  Topform work = resolve2(unit, 1,current, n, TRUE);
	  if (work == NULL) {
	    printf("Lit %d: ",n); fprint_clause(stdout, current);
	    printf("Lit %d: ",1); fprint_clause(stdout, unit);
	    fatal_error("expand_step, clauses don't unit_del");
	  }
	  current = work;
	  map = alist2_insert(map, next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  new_proof = plist_prepend(new_proof, current);
	  unit_deletes++;
#ifdef DEBUG_EXPAND
	  printf("unit_del: "); fprint_clause(stdout, current);
#endif
	}
	else if (j->type == XX_JUST) {
	  int n = j->u.id - xx_simplify;
	  Topform work = copy_inference(current);
	  current = work;
	  map = alist2_insert(map, next_id, old_id, old_id_n++);
	  current->id = next_id++;
	  xx_simp2(current,n);
	  new_proof = plist_prepend(new_proof, current);
	  xx_simplify++;
#ifdef DEBUG_EXPAND
	  printf("xx_simplify: "); fprint_clause(stdout, current);
#endif
	}
	else {
	  printf("expand_step, unknown secondary justification\n");
	  new_proof = plist_prepend(new_proof, current);
	}
      }

      renumber_variables(current, MAX_VARS);

#ifdef DEBUG_EXPAND
      printf("secondary: "); fprint_clause(stdout, current);
#endif

      }

    /* Okay.  Now current should be identical to c. */
    
    if (current == c)
      fatal_error("expand_proof, current == c");
    else if (!clause_ident(current->literals, c->literals)) {
      fprint_clause(stdout, c);
      fprint_clause(stdout, current);
      fatal_error("expand step, result is not identical");
    }
    else {
      /* Now we undo the numbering of the last substep (including
	 the cases in which no expansion is done).  This is so that
	 the clauses in the expanded proof that match the clauses
	 in the original proof have the same IDs.  That is, only
	 the clauses introduced by expansion (e.g., intermedediate
	 demodulants) get new IDs.
       */
      current->id = c->id;
      next_id--;
      map = alist2_remove(map, next_id);
#ifdef DEBUG_EXPAND
      printf("end: "); fprint_clause(stdout, current);
#endif
    }  /* expand */
  }  /* process proof step c */

  *pmap = map;  /* make available to caller */

  new_proof = reverse_plist(new_proof);
  check_parents_and_uplinks_in_proof(new_proof);
  return new_proof;
}  /* expand_proof */

/*************
 *
 *   renumber_proof()
 *
 *************/

/* DOCUMENTATION
We assume that every clause occurs after its parents.
*/

/* PUBLIC */
void renumber_proof(Plist proof, int start)
{
  I2list map = NULL;         /* map old IDs to new IDs */
  int n = start;            /* for numbering the steps */
  Plist p;

  for (p = proof; p; p = p->next) {
    Topform c = p->v;
    int old_id = c->id;
    c->id = n++;
    map_just(c->justification, map);
    map = alist_insert(map, old_id, c->id);
  }
  zap_i2list(map);

  check_parents_and_uplinks_in_proof(proof);
}  /* renumber_proof */

/*************
 *
 *   copy_and_renumber_proof()
 *
 *************/

/* DOCUMENTATION
We assume that every clause occurs after its parents.
*/

/* PUBLIC */
Plist copy_and_renumber_proof(Plist proof, int start)
{
  Plist workproof = copy_clauses_ija(proof);
  renumber_proof(workproof, start);
  return workproof;
}  /* copy_and_renumber_proof */

/*************
 *
 *   proof_to_xproof()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist proof_to_xproof(Plist proof)
{
  I3list map;
  Plist xproof = expand_proof(proof, &map);
  return xproof;
}  /* proof_to_xproof */

