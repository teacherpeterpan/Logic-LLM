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

#include "topform.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_TOPFORM PTRS(sizeof(struct topform))
static unsigned Topform_gets, Topform_frees;

/*************
 *
 *   Topform get_topform()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform get_topform(void)
{
  Topform p = get_cmem(PTRS_TOPFORM);
  Topform_gets++;
  return(p);
}  /* get_topform */

/*************
 *
 *    free_topform()
 *
 *************/

static
void free_topform(Topform p)
{
  free_mem(p, PTRS_TOPFORM);
  Topform_frees++;
}  /* free_topform */

/*************
 *
 *   fprint_topform_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the clause package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_topform_mem(FILE *fp, int heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct topform);
  fprintf(fp, "topform (%4d)      %11u%11u%11u%9.1f K\n",
          n, Topform_gets, Topform_frees,
          Topform_gets - Topform_frees,
          ((Topform_gets - Topform_frees) * n) / 1024.);

}  /* fprint_topform_mem */

/*************
 *
 *   p_topform_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the clause package.
*/

/* PUBLIC */
void p_topform_mem()
{
  fprint_topform_mem(stdout, 1);
}  /* p_topform_mem */

/*
 *  end of memory management
 */

/*************
 *
 *    zap_topform(c)
 *
 *************/

/* DOCUMENTATION
This routine frees a Topform (but not any justification list).
The caller should make sure that nothing (e.g., indexes)
refer to the clause or any of its subterms.
<P>
If the clause has a justification or an official ID,
use the higher-level routine delete_clause(c) instead.
*/

/* PUBLIC */
void zap_topform(Topform tf)
{
  zap_literals(tf->literals);
  zap_formula(tf->formula);
  zap_attributes(tf->attributes);
  free_topform(tf);
}  /* zap_topform */

/*************
 *
 *   fprint_clause()
 *
 *************/

/* DOCUMENTATION
This routine prints a clause to a file.
*/

/* PUBLIC */
void fprint_clause(FILE *fp, Topform c)
{
  if (c == NULL)
    fprintf(fp, "fprint_clause: NULL clause\n");
  else {
    Literals lit;

    if (c->id > 0)
      fprintf(fp, "%d: ", c->id);

    if (c->literals == NULL)
      fprintf(fp, "%s", false_sym());
    else {
      for (lit = c->literals; lit != NULL; lit = lit->next) {
	if (!lit->sign)
	  fprintf(fp, "%s", not_sym());
	fprint_term(fp, lit->atom);
#if 0
	if (maximal_literal_check(lit))
	  fprintf(fp, "[max]");
#endif
	if (lit->next != NULL)
	  fprintf(fp, " %s ", or_sym());
      }
    }
    fprintf(fp, ".\n");
  }
  fflush(fp);
}  /* fprint_clause */

/*************
 *
 *   p_clause()
 *
 *************/

/* DOCUMENTATION
This routine prints a clause to stdout.
*/

/* PUBLIC */
void p_clause(Topform c)
{
  fprint_clause(stdout, c);
}  /* p_clause */

/*************
 *
 *   term_to_clause()
 *
 *************/

/* DOCUMENTATION
This routine takes a Term t (presumably a disjunction with binary
symbol or_sym()), and constructs a Topform.  The Topform is entirely new.
<P>
The main use of this routine is intended to be as follows: a
Term representing a clause is parsed (using mixfix notation)
from the input, then here it is copied translated into
a Topform data structure.
*/

/* PUBLIC */
Topform term_to_clause(Term t)
{
  Topform c = get_topform();
  Term t_start;

  if (is_term(t, attrib_sym(), 2)) {
    c->attributes = term_to_attributes(ARG(t,1), attrib_sym());
    t_start = ARG(t,0);
  }
  else
    t_start = t;

  c->literals = term_to_literals(t_start, NULL);
  upward_clause_links(c);
  return(c);
}  /* term_to_clause */

/*************
 *
 *   term_to_topform()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform term_to_topform(Term t, BOOL is_formula)
{
  Topform c = get_topform();
  Term t_start;

  if (is_term(t, attrib_sym(), 2)) {
    c->attributes = term_to_attributes(ARG(t,1), attrib_sym());
    t_start = ARG(t,0);
  }
  else
    t_start = t;

  c->is_formula = is_formula;
  if (is_formula)
    c->formula = term_to_formula(t_start);
  else {
    c->literals = term_to_literals(t_start, NULL);
    upward_clause_links(c);
  }
  return(c);
}  /* term_to_topform */

/*************
 *
 *   topform_to_term()
 *
 *************/

/* DOCUMENTATION
This routine takes a Topform and returns an entirely new Term
which represents the clause.  The disjunction symbol for the
term is binary or_sym(), and the negation symbols is not_sym().
The attributes are included, but not the id or justifiction.
*/

/* PUBLIC */
Term topform_to_term(Topform tf)
{
  Term t;

  if (tf->is_formula)
    t = formula_to_term(tf->formula);
  else {
    if (tf->literals == NULL)
      t = get_rigid_term(false_sym(), 0);
    else
      t = literals_to_term(tf->literals);
  }

  if (tf->attributes == NULL)
    return t;
  else
    return build_binary_term(str_to_sn(attrib_sym(), 2),
			     t,
			     attributes_to_term(tf->attributes, attrib_sym()));
}  /* topform_to_term */

/*************
 *
 *   topform_to_term_without_attributes()
 *
 *************/

/* DOCUMENTATION
This routine takes a Topform and returns an entirely new Term
which represents the clause.  The disjunction symbol for the
term is binary or_sym(), and the negation symbols is not_sym().
The attributes, id, and justifiction are NOT included.
*/

/* PUBLIC */
Term topform_to_term_without_attributes(Topform tf)
{
  Term t;
  if (tf->is_formula)
    t = formula_to_term(tf->formula);
  else {
    if (tf->literals == NULL)
      t = get_rigid_term(false_sym(), 0);
    else
      t = literals_to_term(tf->literals);
  }
  return t;
}  /* topform_to_term_without_attributes */

/*************
 *
 *   clause_set_variables()
 *
 *************/

/* DOCUMENTATION
This routine traverses a clause and changes the constants
that should be variables, into variables.  On input, the clause
should have no variables.  The new variables are numbered
0, 1, 2 ... according the the first occurrence, reading from the
left.
<P>
A fatal error occurs if there are more than max_vars variables.
<P>
The intended is use is for input clauses that
are built without regard to variable/constant distinction.
*/

/* PUBLIC */
void clause_set_variables(Topform c, int max_vars)
{
  char *a[MAX_VARS], **vmap;
  int i;
  Literals lit;

  if (max_vars > MAX_VARS)
    vmap = malloc((max_vars * sizeof(char *)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    vmap[i] = NULL;

  for (lit = c->literals; lit != NULL; lit = lit->next)
    lit->atom = set_vars_recurse(lit->atom, vmap, max_vars);

  /* Now do any answer literals (with the same vmap). */

  if (c->attributes) {
    Plist clause_vars = vars_in_clause(c->literals);
    Plist attr_vars;
    set_vars_attributes(c->attributes, vmap, max_vars);
    attr_vars = vars_in_attributes(c->attributes);
    /* Make sure that answer vars also occur in ordinary literals. */
    if (!plist_subset(attr_vars, clause_vars)) {
      Plist p;
      printf("Variables in answers must also occur ordinary literals:\n");
      p_clause(c);
      for (p = attr_vars; p; p = p->next) {
	if (!plist_member(clause_vars, p->v)) {
	  Term t = p->v;
	  printf("Answer variable not in ordinary literal: %s.\n",
		 vmap[VARNUM(t)]);
	}
      }
      fatal_error("clause_set_variables, answer variable not in literal");
    }
    zap_plist(clause_vars);
    zap_plist(attr_vars);
  }
  
  if (max_vars > MAX_VARS)
    free(vmap);

  c->normal_vars = TRUE;
}  /* clause_set_variables */

/*************
 *
 *   renumber_variables()
 *
 *************/

/* DOCUMENTATION
This routine renumbers the variables of a clause.  The variables are
renumbered 0, 1, 2 ... according the the first occurrence, reading
from the left.
<P>
If there are more than max_vars distinct variables,
a fatal error occurs.
<P>
The intended is use is for inferred clauses that
may contain variable indexes greater than max_vars.
*/

/* PUBLIC */
void renumber_variables(Topform c, int max_vars)
{
  int a[MAX_VARS], *vmap;
  int i;
  Literals lit;

  if (max_vars > MAX_VARS)
    vmap = malloc((max_vars * sizeof(int)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    a[i] = -1;

  for (lit = c->literals; lit != NULL; lit = lit->next) {
    /* There's a special case in which the atom can be null. */
    if (lit->atom)
      lit->atom = renum_vars_recurse(lit->atom, vmap, max_vars);
  }

  /* Now do any inheritable attributes (with the same vmap). */

  renumber_vars_attributes(c->attributes, vmap, max_vars);

  if (max_vars > MAX_VARS)
    free(vmap);

  c->normal_vars = TRUE;
}  /* renumber_variables */

/*************
 *
 *   term_renumber_variables()
 *
 *************/

/* DOCUMENTATION
This routine renumbers the variables of a term.  The variables are
renumbered 0, 1, 2 ... according the the first occurrence, reading
from the left.
<P>
If there are more than max_vars distinct variables,
a fatal error occurs.
<P>
Do not use this to renumber variables of a clause (see renumber_variables).
*/

/* PUBLIC */
void term_renumber_variables(Term t, int max_vars)
{
  int a[MAX_VARS], *vmap;
  int i;

  if (max_vars > MAX_VARS)
    vmap = malloc((max_vars * sizeof(int)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    a[i] = -1;

  t = renum_vars_recurse(t, vmap, max_vars);
  
  if (max_vars > MAX_VARS)
    free(vmap);
}  /* term_renumber_variables */

/*************
 *
 *   renum_vars_map()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist renum_vars_map(Topform c)
{
  int a[MAX_VARS];
  int i;
  Literals lit;
  Plist pairs = NULL;

  for (i = 0; i < MAX_VARS; i++)
    a[i] = -1;

  for (lit = c->literals; lit != NULL; lit = lit->next)
    lit->atom = renum_vars_recurse(lit->atom, a, MAX_VARS);

  /* Now renumber any inheritable attributes (with the same vmap). */

  renumber_vars_attributes(c->attributes, a, MAX_VARS);

  /* Build the list of pairs. */

  for (i = 0; i < MAX_VARS; i++) {
    /* a[i] -> i */
    if (a[i] != -1 && a[i] != i) {
      Term v1 = get_variable_term(a[i]);
      Term v2 = get_variable_term(i);
      Term pair = listterm_cons(v1, v2);
      pairs = plist_append(pairs, pair);
    }
  }
  c->normal_vars = TRUE;
  return pairs;
}  /* renum_vars_map */

/*************
 *
 *   upward_clause_links()
 *
 *************/

/* DOCUMENTATION
In the given Topform c, make the "container" field of each subterm
point to c.
*/

/* PUBLIC */
void upward_clause_links(Topform c)
{
  Literals lit;
  for (lit = c->literals; lit != NULL; lit = lit->next)
    upward_term_links(lit->atom, c);
}  /* upward_clause_links */

/*************
 *
 *   check_upward_clause_links()
 *
 *************/

/* DOCUMENTATION
In the given Topform c, check that the "container" field of each subterm
point to c.
*/

/* PUBLIC */
BOOL check_upward_clause_links(Topform c)
{
  Literals lit;
  for (lit = c->literals; lit != NULL; lit = lit->next) {
    if (!check_upward_term_links(lit->atom, c))
      return FALSE;
  }
  return TRUE;
}  /* check_upward_clause_links */

/*************
 *
 *   copy_clause()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
The container field of each nonvariable subterm points
to the clause.
*/

/* PUBLIC */
Topform copy_clause(Topform c)
{
  Topform c2 = get_topform();
  c2->literals = copy_literals(c->literals);
  upward_clause_links(c2);
  return c2;
}  /* copy_clause */

/*************
 *
 *   copy_clause_with_flags()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
All termflags are copied for all subterms (including atoms,
excluding variables).
*/

/* PUBLIC */
Topform copy_clause_with_flags(Topform c)
{
  Topform c2 = get_topform();
  c2->literals = copy_literals_with_flags(c->literals);
  upward_clause_links(c2);
  return c2;
}  /* copy_clause_with_flags */

/*************
 *
 *   copy_clause_with_flag()
 *
 *************/

/* DOCUMENTATION
This routine builds and returns a copy of a clause.
The given termflag is copied for all subterms (including atoms,
excluding variables).
*/

/* PUBLIC */
Topform copy_clause_with_flag(Topform c, int flag)
{
  Topform c2 = get_topform();
  c2->literals = copy_literals_with_flag(c->literals, flag);
  upward_clause_links(c2);
  return c2;
}  /* copy_clause_with_flag */

/*************
 *
 *   inherit_attributes()
 *
 *************/

/* DOCUMENTATION
This takes two parent clauses and their associated
substitutions, and a child clause.  All inheritable
attributes on the parents are instantiated and
appended to the child's attributes.
<p>
Either parent can be NULL.
*/

/* PUBLIC */
void inherit_attributes(Topform par1, Context s1,
			Topform par2, Context s2,
			Topform child)
{
  Attribute a1 = par1 ? inheritable_att_instances(par1->attributes, s1) : NULL;
  Attribute a2 = par2 ? inheritable_att_instances(par2->attributes, s2) : NULL;
  child->attributes = cat_att(child->attributes, cat_att(a1, a2));
}  /* inherit_attributes */

/*************
 *
 *   gather_symbols_in_topform()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void gather_symbols_in_topform(Topform c, I2list *rsyms, I2list *fsyms)
{
  if (c->is_formula)
    gather_symbols_in_formula(c->formula, rsyms, fsyms);
  else {
    Literals lit;
    for (lit = c->literals; lit; lit = lit->next)
      gather_symbols_in_formula_term(lit->atom, rsyms, fsyms);
  }
}  /* gather_symbols_in_topform */

/*************
 *
 *   gather_symbols_in_topforms()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void gather_symbols_in_topforms(Plist lst, I2list *rsyms, I2list *fsyms)
{
  Plist p;
  for (p = lst; p; p = p->next)
    gather_symbols_in_topform(p->v, rsyms, fsyms);
}  /* gather_symbols_in_topforms */

/*************
 *
 *   fsym_set_in_topforms()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist fsym_set_in_topforms(Plist lst)
{
  Ilist p;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  gather_symbols_in_topforms(lst, &rsyms, &fsyms);
  p = multiset_to_set(fsyms);
  zap_i2list(rsyms);
  zap_i2list(fsyms);
  return p;
}  /* fsym_set_in_topforms */

/*************
 *
 *   rsym_set_in_topforms()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist rsym_set_in_topforms(Plist lst)
{
  Ilist p;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  gather_symbols_in_topforms(lst, &rsyms, &fsyms);
  p = multiset_to_set(rsyms);
  zap_i2list(rsyms);
  zap_i2list(fsyms);
  return p;
}  /* rsym_set_in_topforms */

/*************
 *
 *   min_depth()
 *
 *************/

/* DOCUMENTATION
Does the Literals have minimum depth of all literals the containing clause?
*/

/* PUBLIC */
BOOL min_depth(Literals lit)
{
  Topform c = lit->atom->container;
  int d = term_depth(lit->atom);
  Literals l;
  for (l = c->literals; l; l = l->next) {
    if (term_depth(l->atom) < d)
      return FALSE;
  }
  return TRUE;
}  /* min_depth */

/*************
 *
 *   initial_clause()
 *
 *************/

/* DOCUMENTATION
Is (was) the clause part of the initial sos (after processing input clauses,
before starting search)/
*/

/* PUBLIC */
BOOL initial_clause(Topform c)
{
  return c->initial;
}  /* initial_clause */

/*************
 *
 *   negative_clause_possibly_compressed()
 *
 *************/

/* DOCUMENTATION
Is (was) the clause part of the initial sos (after processing input clauses,
before starting search)/
*/

/* PUBLIC */
BOOL negative_clause_possibly_compressed(Topform c)
{
  if (c->compressed)
    return c->neg_compressed;
  else
    return negative_clause(c->literals);
}  /* negative_clause_possibly_compressed */

/*************
 *
 *   topform_properties()
 *
 *************/

/* DOCUMENTATION
Construct a term containing a list of miscellaneous properties of a Topform.
This is meant to be used as an attribute on Topforms for debugging.
*/

/* PUBLIC */
Term topform_properties(Topform c)
{
  Term lst;
  Literals lit;
  int i;
  Term props = get_nil_term();

  /* maximal literals */

  lst = get_nil_term();
  for (lit = c->literals, i = 1; lit; lit = lit->next, i++) {
    if (maximal_literal(c->literals, lit, FLAG_CHECK))
      lst = listterm_cons(nat_to_term(i),lst);
  }
  lst = listterm_reverse(lst);
  props = listterm_cons(build_unary_term_safe("maximal", lst), props);

  /* maximal_signed literals */

  lst = get_nil_term();
  for (lit = c->literals, i = 1; lit; lit = lit->next, i++) {
    if (maximal_signed_literal(c->literals, lit, FLAG_CHECK))
      lst = listterm_cons(nat_to_term(i),lst);
  }
  lst = listterm_reverse(lst);
  props = listterm_cons(build_unary_term_safe("maximal_signed",lst),props);

  /* maximal_signed literals */

  lst = get_nil_term();
  for (lit = c->literals, i = 1; lit; lit = lit->next, i++) {
    if (selected_literal(lit))
      lst = listterm_cons(nat_to_term(i),lst);
  }
  lst = listterm_reverse(lst);
  props = listterm_cons(build_unary_term_safe("selected",lst),props);

  return listterm_reverse(props);

}  /* topform_properties */

/*************
 *
 *   append_label_attribute()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void append_label_attribute(Topform tf, char *s)
{
  tf->attributes = set_string_attribute(tf->attributes, label_att(), s);
}  /* append_label_attribute */

/*************
 *
 *   cl_id_compare()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ordertype cl_id_compare(Topform c1, Topform c2)
{
  if (c1->id < c2->id)
    return LESS_THAN;
  else if (c1->id > c2->id)
    return GREATER_THAN;
  else
    return SAME_AS;
}  /* cl_id_compare */

/*************
 *
 *   cl_wt_id_compare()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ordertype cl_wt_id_compare(Topform c1, Topform c2)
{
  if (c1->weight < c2->weight)
    return LESS_THAN;
  else if (c1->weight > c2->weight)
    return GREATER_THAN;
  else if (c1->id < c2->id)
    return LESS_THAN;
  else if (c1->id > c2->id)
    return GREATER_THAN;
  else
    return SAME_AS;
}  /* cl_wt_id_compare */

