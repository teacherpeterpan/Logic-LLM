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

#include "ivy.h"

/* Private definitions and types */

#define DICT_SIZE 6

/* The following maps symbols that are unacceptable to IVY. */

static char *Dict[DICT_SIZE][2] = {{"0",  "zero_for_ivy"},
				   {"1",  "one_for_ivy"},
				   {"'",  "quote_for_ivy"},
				   {"\\", "backslash_for_ivy"},
				   {"@",  "at_for_ivy"},
				   {"^",  "meet_for_ivy"}};
/*************
 *
 *   dict_lookup()
 *
 *************/

static
char *dict_lookup(char *key)
{
  int i;
  for (i = 0; i < DICT_SIZE; i++) {
    if (str_ident(Dict[i][0], key))
      return Dict[i][1];
  }
  return NULL;
}  /* dict_lookup */

/*************
 *
 *   ivy_term_trans()
 *
 *************/

static
void ivy_term_trans(Term t)
{
  if (VARIABLE(t))
    return;
  else {
    int i;
    char *s = dict_lookup(sn_to_str(SYMNUM(t)));
    if (s)
      t->private_symbol = -str_to_sn(s, ARITY(t));
    for (i = 0; i < ARITY(t); i++)
      ivy_term_trans(ARG(t,i));
  }
}  /* ivy_term_trans */

/*************
 *
 *   ivy_clause_trans()
 *
 *************/

static
void ivy_clause_trans(Topform c)
{
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next) {
    ivy_term_trans(lit->atom);
  }
}  /* ivy_clause_trans */

/*************
 *
 *   sb_ivy_write_term()
 *
 *************/

static
void sb_ivy_write_term(String_buf sb, Term t)
{
  if (VARIABLE(t)) {
    sb_append(sb, "v");
    sb_append_int(sb, VARNUM(t));
  }
  else {
    int i;
    sb_append(sb, "(");
    sb_append(sb, sn_to_str(SYMNUM(t)));
    for (i = 0; i < ARITY(t); i++) {
      sb_append(sb, " ");
      sb_ivy_write_term(sb, ARG(t,i));
    }
    sb_append(sb, ")");
  }
}  /* sb_ivy_write_term */

/*************
 *
 *   sb_ivy_write_pair()
 *
 *************/

static
void sb_ivy_write_pair(String_buf sb, Term pair)
{
  Term v = ARG(pair,0);
  Term t = ARG(pair,1);
  sb_append(sb, "(");
  sb_ivy_write_term(sb, v);
  sb_append(sb, " . ");
  sb_ivy_write_term(sb, t);
  sb_append(sb, ")");
}  /* sb_ivy_write_pair */

/*************
 *
 *   sb_ivy_write_pairs()
 *
 *************/

static
void sb_ivy_write_pairs(String_buf sb, Plist pairs)
{
  Plist p;
  sb_append(sb, "(");
  for (p = pairs; p; p = p->next) {
    sb_ivy_write_pair(sb, p->v);
    if (p->next)
      sb_append(sb, " ");	
  }
  sb_append(sb, ")");
}  /* sb_ivy_write_pairs */

/*************
 *
 *   sb_ivy_write_position()
 *
 *************/

static
void sb_ivy_write_position(String_buf sb, Ilist position)
{
  Ilist p;
  sb_append(sb, "(");
  for (p = position; p; p = p->next) {
    sb_append_int(sb, p->i);
    if (p->next)
      sb_append(sb, " ");	
  }
  sb_append(sb, ")");
}  /* sb_ivy_write_position */

/*************
 *
 *   sb_ivy_write_lit()
 *
 *************/

static
void sb_ivy_write_lit(String_buf sb, Literals lit)
{
  if (lit->sign == FALSE) {
    sb_append(sb, "(not ");
    sb_ivy_write_term(sb, lit->atom);
    sb_append(sb, ")");
  }
  else
    sb_ivy_write_term(sb, lit->atom);
}  /* sb_ivy_write_lit */

/*************
 *
 *   sb_ivy_write_literals()
 *
 *************/

static
void sb_ivy_write_literals(String_buf sb, Literals lits)
{
  if (lits == NULL)
    sb_append(sb, "false");
  else if (lits->next == NULL)
    sb_ivy_write_lit(sb, lits);
  else {
    sb_append(sb, "(or ");
    sb_ivy_write_lit(sb, lits);
    sb_append(sb, " ");
    sb_ivy_write_literals(sb, lits->next);
    sb_append(sb, ")");
  }
}  /* sb_ivy_write_literals */

/*************
 *
 *   sb_ivy_write_just()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void sb_ivy_write_just(String_buf sb, Ivyjust j, I3list map)
{
  if (j->type == INPUT_JUST) {
    sb_append(sb, "(input)");
  }
  else if (j->type == PROPOSITIONAL_JUST) {
    sb_append(sb, "(propositional ");
    sb_append_id(sb, j->parent1, map);
    sb_append(sb, ")");
  }
  else if (j->type == NEW_SYMBOL_JUST) {
    sb_append(sb, "(new_symbol ");
    sb_append_id(sb, j->parent1, map);
    sb_append(sb, ")");
  }
  else if (j->type == FLIP_JUST) {
    sb_append(sb, "(flip ");
    sb_append_id(sb, j->parent1, map);
    sb_append(sb, " ");
    sb_ivy_write_position(sb, j->pos1);
    sb_append(sb, ")");
  }
  else if (j->type == INSTANCE_JUST) {
    sb_append(sb, "(instantiate ");
    sb_append_id(sb, j->parent1, map);
    sb_append(sb, " ");
    sb_ivy_write_pairs(sb, j->pairs);
    sb_append(sb, ")");
  }
  else if (j->type == BINARY_RES_JUST ||
	   j->type == PARA_JUST) {
    if (j->type == BINARY_RES_JUST)
      sb_append(sb, "(resolve ");
    else
      sb_append(sb, "(paramod ");
    sb_append_id(sb, j->parent1, map);
    sb_append(sb, " ");
    sb_ivy_write_position(sb, j->pos1);
    sb_append(sb, " ");
    sb_append_id(sb, j->parent2, map);
    sb_append(sb, " ");
    sb_ivy_write_position(sb, j->pos2);
    sb_append(sb, ")");
  }
  else
    fatal_error("sb_ivy_write_just, bad ivy justification");
}  /* sb_ivy_write_just */

/*************
 *
 *   sb_ivy_write_clause_jmap()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void sb_ivy_write_clause_jmap(String_buf sb, Topform c, I3list map)
{
  if (c->justification->type != IVY_JUST) {
    printf("not ivy just: "); fprint_clause(stdout, c);
    fatal_error("sb_ivy_write_clause_jmap, not IVY_JUST");
  }

  sb_append(sb, "(");                                   /* start */
  sb_append_id(sb, c->id, map);                         /* ID */
  sb_append(sb, " ");
  sb_ivy_write_just(sb, c->justification->u.ivy, map);  /* justification */
  sb_append(sb, " ");
  sb_ivy_write_literals(sb, c->literals);               /* literals */
  sb_append(sb, " NIL)\n");                             /* end  */

}  /* sb_ivy_write_clause_jmap */

/*************
 *
 *   instantiate_inference()
 *
 *************/

static
Topform instantiate_inference(Topform c, Context subst)
{
  Topform d = instantiate_clause(c, subst);
  Plist pairs = context_to_pairs(varnums_in_clause(c->literals), subst);
  d->justification = ivy_just(INSTANCE_JUST, c->id, NULL, 0, NULL, pairs);
  inherit_attributes(c, subst, NULL, NULL, d);
  upward_clause_links(d);
  return d;
}  /* instantiate_inference */

/*************
 *
 *   renumber_inference() - return NULL if renumbering does nothing
 *
 *************/

static
Topform renumber_inference(Topform c)
{
  Plist pairs;
  Topform child = copy_clause(c);
  pairs = renum_vars_map(child);
  if (pairs == NULL) {
    zap_topform(child);
    return NULL;
  }
  else {
    child->justification = ivy_just(INSTANCE_JUST, c->id, NULL, 0, NULL, pairs);
    child->attributes = copy_attributes(c->attributes);
    return child;
  }
}  /* renumber_inference */

/*************
 *
 *   ivy_lit_position()
 *
 *************/

static
Ilist ivy_lit_position(int n, int number_of_lits)
{
  /* Build an Ivy-style position for a literal.  Ivy clauses are
     binary trees of ORs.  Ivy accepts any association, but the
     ones we are using are always right associated.
     To build the position, we have to know if the designated
     literal is the last, e.g.,
        Designated     If last       If not last
            1              ()                (1)
            2             (2)              (2 1)
            3           (2 2)            (2 2 1)
            4         (2 2 2)          (2 2 2 1)
   */   
  int i;
  Ilist pos = NULL;
  if (n != number_of_lits)
    pos = ilist_prepend(pos, 1);
  for (i = 1; i < n; i++)
    pos = ilist_prepend(pos, 2);
  return pos;
}  /* ivy_lit_position */

/*************
 *
 *   ivy_para_position()
 *
 *************/

static
Ilist ivy_para_position(Ilist pos1, BOOL sign, int number_of_lits)
{
  /* Given a LADR-style position for a term within a clause,
     build an Ivy-style position for a term within a clause.
     See ivy_lit_position.
   */   
  Ilist pos2 = ivy_lit_position(pos1->i, number_of_lits);

  if (!sign)  /* Ivy position considers sign, LADR's doesn't. */
    pos2 = ilist_append(pos2, 1);
  pos2 = ilist_cat(pos2, copy_ilist(pos1->next));
  return pos2;
}  /* ivy_para_position */

/*************
 *
 *   paramod2_instances()
 *
 *************/

/* DOCUMENTATION
Paramodulate, if possible, two clauses at the positions given.
Renumber vars, include justification, transfer inheritable
attributes, assign an ID.
<P>
*/

static
Plist paramod2_instances(Topform from, Ilist from_pos,
			 Topform into, Ilist into_pos, int *next_id)
{
  Context subst_from = get_context();
  Context subst_into = get_context();
  Trail tr = NULL;
  Plist steps = NULL;  /* build sequence of inferences (backward) */
  BOOL demod_like;

  Literals from_lit = ith_literal(from->literals, from_pos->i);
  Literals into_lit = ith_literal(into->literals, into_pos->i);
  BOOL left_to_right = from_pos->next->i == 1;
  Term alpha = ARG(from_lit->atom, left_to_right ? 0 : 1);
  Term beta  = ARG(from_lit->atom, left_to_right ? 1 : 0);
  Term into_term = term_at_pos(into_lit->atom, into_pos->next);
  if (into_term == NULL)
    fatal_error("paramod2_instances, term does not exist");

  demod_like =
    unit_clause(from->literals) &&
    variables_subset(beta, alpha) &&
    match(alpha, subst_from, into_term, &tr);

  if (demod_like || unify(alpha, subst_from, into_term, subst_into, &tr)) {
    Topform from_instance, into_instance, para, para_renum;

    if (ground_clause(from->literals))
      from_instance = from;
    else {
      from_instance = instantiate_inference(from, subst_from);
      from_instance->id = (*next_id)++;
      steps = plist_prepend(steps, from_instance);
    }

    if (demod_like || ground_clause(into->literals))
      into_instance = into;
    else {
      into_instance = instantiate_inference(into, subst_into);
      into_instance->id = (*next_id)++;
      steps = plist_prepend(steps, into_instance);
    }

    undo_subst(tr);

    /* Positions in instances are same as positions in originals. */

    from_lit = ith_literal(from_instance->literals, from_pos->i);
    into_lit = ith_literal(into_instance->literals, into_pos->i);

    para = paramodulate(from_lit, left_to_right ? 0 : 1, NULL,
			into_instance, into_pos, NULL);
    para->justification =
      ivy_just(PARA_JUST,
	       from_instance->id,
	       ivy_para_position(from_pos,
				 TRUE,  /* sign of literal */
				 number_of_literals(from_instance->literals)),
	       into_instance->id,
	       ivy_para_position(into_pos,
				 into_lit->sign,
				 number_of_literals(into_instance->literals)),
	       NULL);
    para->id = (*next_id)++;
    steps = plist_prepend(steps, para);
    para_renum = renumber_inference(para);
    if (para_renum) {
      para_renum->id = (*next_id)++;
      steps = plist_prepend(steps, para_renum);
    }
  }
  else
    steps = NULL;
  
  free_context(subst_from);
  free_context(subst_into);
  return steps;
}  /* paramod2_instances */

/*************
 *
 *   flip_inference()
 *
 *************/

static
Topform flip_inference(Topform c, int n)
{
  Topform child = copy_clause(c);
  Literals lit = ith_literal(child->literals, n);
  Term atom = lit->atom;
  Term t;
  if (!eq_term(atom))
    fatal_error("flip_inference, literal not equality");
  t = ARG(atom, 0);
  ARG(atom, 0) = ARG(atom, 1);
  ARG(atom, 1) = t;

  child->justification =
    ivy_just(FLIP_JUST,
	     c->id,
	     ivy_lit_position(n,
			      number_of_literals(c->literals)),
	     0, NULL, NULL);
  child->attributes = copy_attributes(c->attributes);
  return child;
}  /* flip_inference */

/*************
 *
 *   resolve2_instances()
 *
 *************/

/* DOCUMENTATION
Resolve, if possible, two clauses on the literals (specified
by literals, counting from 1).
Renumber vars, include justification, transfer inheritable
attributes, assign an ID.
<P>
The inference is done as a sequence of 2 -- 4 steps.  (1,2) the parents
are instantiated, (3) then resolution occurs on identical literals,
(4) then the variables in the resolvent are renumbered.  If any of
the instantiation steps is vacuous, it is skipped.
<P>
if n2 < 0, then the literal is abs(n2), and it should be flipped.
*/

static
Plist resolve2_instances(Topform c1, int n1, Topform c2, int n2, int *next_id)
{
  Literals l1 = ith_literal(c1->literals, n1);
  Literals l2 = ith_literal(c2->literals, abs(n2));
  Term a1 = l1->atom;
  Term a2 = l2->atom;
  Context s1 = get_context();
  Context s2 = get_context();
  Trail tr = NULL;
  Term a2x;
  Plist steps = NULL;

  if (n2 < 0)
    a2x = top_flip(a2);  /* temporary flipped equality */
  else
    a2x = a2;

  if (l1->sign != l2->sign && unify(a1, s1, a2x, s2, &tr)) {
    Literals l1i, l2i, lit;
    Topform res, res_renum, c1i, c2i;

    /* instantiate parents if not ground */

    if (ground_clause(c1->literals))
      c1i = c1;
    else {
      c1i = instantiate_inference(c1, s1);
      c1i->id = (*next_id)++;
      steps = plist_prepend(steps, c1i);
    }

    if (ground_clause(c2->literals))
      c2i = c2;
    else {
      c2i = instantiate_inference(c2, s2);
      c2i->id = (*next_id)++;
      steps = plist_prepend(steps, c2i);
    }

    if (n2 < 0) {
      c2i = flip_inference(c2i, abs(n2));
      c2i->id = (*next_id)++;
      steps = plist_prepend(steps, c2i);
    }

    undo_subst(tr);
    
    l1i = ith_literal(c1i->literals, n1);
    l2i = ith_literal(c2i->literals, abs(n2));
    
    /* construct the resolvent */

    res = get_topform();
    for (lit = c1i->literals; lit; lit = lit->next)
      if (lit != l1i)
	res->literals = append_literal(res->literals, copy_literal(lit));
    for (lit = c2i->literals; lit; lit = lit->next)
      if (lit != l2i)
	res->literals = append_literal(res->literals, copy_literal(lit));
    inherit_attributes(c1i, NULL, c2i, NULL, res);
    upward_clause_links(res);

    res->justification =
      ivy_just(BINARY_RES_JUST,
	       c1i->id,
	       ivy_lit_position(n1,number_of_literals(c1i->literals)),
	       c2i->id,
	       ivy_lit_position(abs(n2),
				number_of_literals(c2i->literals)),
	       NULL);
    res->id = (*next_id)++;
    steps = plist_prepend(steps, res);
    res_renum = renumber_inference(res);
    if (res_renum) {
      res_renum->id = (*next_id)++;
      steps = plist_prepend(steps, res_renum);
    }
  }
  else
    steps = NULL;

  if (n2 < 0)
    zap_top_flip(a2x);

  free_context(s1);
  free_context(s2);

  return steps;
}  /* resolve2_instances */

/*************
 *
 *   factor2_instances()
 *
 *************/

static
Plist factor2_instances(Topform c, int n1, int n2, int *next_id)
{
  Literals l1 = ith_literal(c->literals, n1);
  Literals l2 = ith_literal(c->literals, n2);
  Term a1 = l1->atom;
  Term a2 = l2->atom;
  Context subst = get_context();
  Trail tr = NULL;
  Plist steps = NULL;

  if (l1->sign == l2->sign && unify(a1, subst, a2, subst, &tr)) {

    Literals l1i, l2i, lit;
    Topform factor, factor_renum, c_instance;

    c_instance = instantiate_inference(c, subst);
    c_instance->id = (*next_id)++;
    steps = plist_prepend(steps, c_instance);

    undo_subst(tr);

    l1i = ith_literal(c_instance->literals, n1);
    l2i = ith_literal(c_instance->literals, n2);

    /* construct the factor */

    factor = get_topform();
    for (lit = c_instance->literals; lit; lit = lit->next)
      if (lit != l2i)
	factor->literals = append_literal(factor->literals, copy_literal(lit));

    inherit_attributes(c_instance, NULL, NULL, NULL, factor);
    upward_clause_links(factor);

    factor->justification = ivy_just(PROPOSITIONAL_JUST,
				     c_instance->id, NULL,
				     0, NULL,
				     NULL);
    factor->id = (*next_id)++;
    steps = plist_prepend(steps, factor);
    factor_renum = renumber_inference(factor);
    if (factor_renum) {
      factor_renum->id = (*next_id)++;
      steps = plist_prepend(steps, factor_renum);
    }
  }
  else
    steps = NULL;

  free_context(subst);

  return steps;
}  /* factor2_instances */

/*************
 *
 *   copy_proof_and_rename_symbols_for_ivy()
 *
 *************/

static
Plist copy_proof_and_rename_symbols_for_ivy(Plist proof)
{
  Plist new = NULL;
  Plist p;
  for (p = proof; p; p = p->next) {
    Topform c = copy_clause_ija(p->v);
    ivy_clause_trans(c);
    new = plist_prepend(new, c);
  }
  return reverse_plist(new);
}  /* copy_proof_and_rename_symbols_for_ivy */

/*************
 *
 *   expand_proof_ivy()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist expand_proof_ivy(Plist proof)
{
  Plist work_proof = copy_proof_and_rename_symbols_for_ivy(proof);
  Plist new_proof = NULL; /* build it backward, reverse at end */
  int next_id;
  Plist p;
  BOOL need_reflexivity_of_eq = FALSE;
  Ilist to_be_removed = NULL;  /* clauses to be removed from proof */
  Plist final_proof = NULL;

  /* Build clause x=x, id=0, justification input; delete later if not used. */
  Topform xx = build_reflex_eq();
  xx->id = 0;
  xx->justification = ivy_just(INPUT_JUST, 0, NULL, 0, NULL, NULL);
  new_proof = plist_prepend(new_proof, xx);

  /* Start numbering the new proof where the old one ends. */

  next_id = greatest_id_in_proof(work_proof) + 1;

  for (p = work_proof; p; p = p->next) {
    Topform c = p->v;
    Just j = c->justification;
    Topform new_c = NULL;

    if (j->type == BINARY_RES_JUST || j->type == XXRES_JUST) {
      Ilist p = j->u.lst;

      int id1  = p->i;
      int lit1 = p->next->i;

      int id2  = j->type == XXRES_JUST ? 0 : p->next->next->i;
      int lit2 = j->type == XXRES_JUST ? 1 : p->next->next->next->i;

      Topform c1 = proof_id_to_clause(new_proof, id1);
      Topform c2 = proof_id_to_clause(new_proof, id2);

      Plist new_steps = resolve2_instances(c1, lit1, c2, lit2, &next_id);
      new_c = new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;

      new_proof = plist_cat(new_steps, new_proof);

      if (j->type == XXRES_JUST)
	need_reflexivity_of_eq = TRUE;
    }
    else if (j->type == PARA_JUST) {
      Parajust p = j->u.para;

      Topform from = proof_id_to_clause(new_proof, p->from_id);
      Topform into = proof_id_to_clause(new_proof, p->into_id);

      Plist new_steps = paramod2_instances(from, p->from_pos,
					   into, p->into_pos, &next_id);

      new_c = new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;

      new_proof = plist_cat(new_steps, new_proof);
    }
    else if (j->type == FACTOR_JUST) {
      Ilist p = j->u.lst;

      int id =  p->i;
      int lit1 = p->next->i;
      int lit2 = p->next->next->i;

      Topform parent = proof_id_to_clause(new_proof, id);

      Plist new_steps = factor2_instances(parent, lit1, lit2, &next_id);
      new_c = new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;

      new_proof = plist_cat(new_steps, new_proof);
    }

    else if (j->type == COPY_JUST &&
	     j->next &&
	     j->next->type == FLIP_JUST) {
      int flip_lit = j->next->u.id;
      Topform parent = proof_id_to_clause(new_proof, j->u.id);
      Topform child = flip_inference(parent, flip_lit);
      Topform child_renum;

      child->id = next_id++;
      new_proof = plist_prepend(new_proof, child);

      child_renum = renumber_inference(child);

      if (child_renum) {
	new_proof = plist_prepend(new_proof, child_renum);
	child_renum->id = next_id++;
	new_c = child_renum;
      }
      else
	new_c = child;
      new_c->id = c->id;
      next_id--;
    }
    else if (j->type == COPY_JUST &&
	     j->next &&
	     j->next->type == XX_JUST) {

      int id1  = j->u.id;
      int lit1 = j->next->u.id;

      int id2  = 0;  /* special clause x = x */
      int lit2 = 1;  /* special clause x = x */

      Topform c1 = proof_id_to_clause(new_proof, id1);
      Topform c2 = proof_id_to_clause(new_proof, id2);

      Plist new_steps = resolve2_instances(c1, lit1, c2, lit2, &next_id);
      new_c = new_steps->v;

      /* Give the new clause the ID of the old, so that subsequent
	 steps in the old proof make sense.
      */

      new_c->id = c->id;
      next_id--;
      new_proof = plist_cat(new_steps, new_proof);
      need_reflexivity_of_eq = TRUE;
    }
    else if (j->type == COPY_JUST &&
	     (j->next == NULL || j->next->type == MERGE_JUST)) {
      int parent_id = j->u.id;
      new_c = copy_clause(c);
      new_c->justification = ivy_just(PROPOSITIONAL_JUST,
				      parent_id, NULL,
				      0, NULL,
				      NULL);
      new_c->id = c->id;
      new_proof = plist_prepend(new_proof, new_c);
    }
    else if (j->type == INPUT_JUST ||
	     j->type == CLAUSIFY_JUST ||
	     j->type == EXPAND_DEF_JUST ||
	     j->type == GOAL_JUST ||
	     j->type == DENY_JUST) {
      new_c = copy_clause(c);
      new_c->id = c->id;
      new_c->attributes = copy_attributes(c->attributes);
      new_c->justification = ivy_just(INPUT_JUST, 0, NULL, 0, NULL, NULL);
      new_proof = plist_prepend(new_proof, new_c);
      if (j->type == CLAUSIFY_JUST ||
	  j->type == EXPAND_DEF_JUST ||
	  j->type == DENY_JUST) {
	/* Parents of these must be removed from the
	   proof, because IVY expects ordinary clauses only. */
	Ilist parents = get_parents(j, TRUE);
	to_be_removed = ilist_cat(to_be_removed, parents);
      }
    }
    else if (j->type == NEW_SYMBOL_JUST) {
      int parent_id = j->u.id;
      new_c = copy_clause(c);
      new_c->justification = ivy_just(NEW_SYMBOL_JUST,
				      parent_id, NULL,
				      0, NULL,
				      NULL);
      new_c->id = c->id;
      new_proof = plist_prepend(new_proof, new_c);
      fprintf(stdout, "\n;; WARNING: IVY proof contains unaccepted NEW_SYMBOL justification.\n\n");
      fprintf(stderr, "\n;; WARNING: IVY proof contains unaccepted NEW_SYMBOL justification.\n\n");
    }
    else {
      new_c = copy_clause_ija(c);
      new_proof = plist_prepend(new_proof, new_c);
    }

    if (!subsumes(c, new_c) || !subsumes(new_c, c)) {
      printf("old clause: "); fprint_clause(stdout, c);
      printf("new clause: "); fprint_clause(stdout, new_c);
      fatal_error("expand_proof_ivy, clauses not equivalent");
    }
  }  /* process proof step c */
  
  delete_clauses(work_proof);

  /* The following does 2 things: remove clauses and reverse proof. */

  final_proof = NULL;
  for (p = new_proof; p; p = p->next) {
    Topform c = p->v;
    if (!ilist_member(to_be_removed, c->id))
      final_proof = plist_prepend(final_proof, c);
  }

  zap_plist(new_proof);  /* shallow */
  zap_ilist(to_be_removed);

  if (!need_reflexivity_of_eq) {
    /* We didn't use x=x, so delete it from the proof (it is first). */
    Plist p = final_proof;
    final_proof = final_proof->next;
    zap_topform(p->v);
    free_plist(p);
  }

  check_parents_and_uplinks_in_proof(final_proof);
  return final_proof;
}  /* expand_proof_ivy */
