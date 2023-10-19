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

#include "definitions.h"

/* Private definitions and types */

/*
 * memory management
 */

/*************
 *
 *   is_definition()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL is_definition(Formula f)
{
  if (!closed_formula(f))
    return FALSE;
  else {
    Plist vars = NULL;  /* leading universal quantifiers */
    BOOL ok = TRUE;
    Formula g;
    for (g = f; g->type == ALL_FORM; g = g->kids[0]) {
      Term t = get_rigid_term(g->qvar, 0);
      vars = plist_append(vars, t);
    }
    if (g->type != IFF_FORM || g->kids[0]->type != ATOM_FORM)
      ok = FALSE;
    else {
      Term atom = g->kids[0]->atom;
      Plist args = plist_of_subterms(atom);
      ok = (ARITY(atom) == plist_count(vars) &&
	    tlist_set(vars) &&
	    tlist_subset(vars, args) &&
	    tlist_subset(args, vars));
      zap_plist(args);  /* shallow */
    }
    zap_plist_of_terms(vars);  /* deep */

    if (ok) {
      /* check that the defined symbol does not occur in the right side */
      ok = !relation_symbol_in_formula(SYMNUM(g->kids[0]->atom), g->kids[1]);
    }

    return ok;
  }
}  /* is_definition */

/*************
 *
 *   subst_terms()
 *
 *************/

static
Term subst_terms(Term t, Plist vars, Plist terms, Plist qvars)
{
  /* In Term t, replace members of vars (really constants) with
     (copies of) the corresponding members of terms.
     EXCEPT if the var (constant) is a member of qvars.
     Note that "vars" and "terms" are lists of terms, and "qvars"
     is a list of strings.
   */
  if (CONSTANT(t)) {
    if (!string_member_plist(sn_to_str(SYMNUM(t)), qvars)) {
      int i = position_of_term_in_tlist(t, vars);
      if (i != -1) {
	Term corresponding_term = ith_in_plist(terms, i);
	zap_term(t);
	t = copy_term(corresponding_term);
      }
    }
    return t;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = subst_terms(ARG(t,i), vars, terms, qvars);
    return t;
  }
}  /* subst_terms */

/*************
 *
 *   subst_free_vars()
 *
 *************/

static
void subst_free_vars(Formula f, Plist vars, Plist terms, Plist qvars)
{
  if (f->type == ATOM_FORM)
    f->atom = subst_terms(f->atom, vars, terms, qvars);
  else if (quant_form(f)) {
    Plist qvars2 = plist_prepend(qvars, f->qvar);
    subst_free_vars(f->kids[0], vars, terms, qvars2);
    free_plist(qvars2);  /* first_node only, leaves qvars as it was */
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      subst_free_vars(f->kids[i], vars, terms, qvars);
  }
}  /* subst_free_vars */

/*************
 *
 *   subst_atomic_formula()
 *
 *************/

static
Formula subst_atomic_formula(Formula f, Formula target, Formula replacement)
{
  /* We have to do *simultaneous* replacement, instead of making
     a sequence of calls to subst_free_var.  For example, consider
     f=p(y,z), target=p(x,y), replacement=r(x,y).  Sequential
     replacement gives r(z,z) instead of r(y,z).
   */
  if (f->type == ATOM_FORM) {
    if (SYMNUM(f->atom) == SYMNUM(target->atom)) {
      Formula new = formula_copy(replacement);
      Plist vars  = plist_of_subterms(target->atom);
      Plist terms = plist_of_subterms(f->atom);
      /* We have to prevent capture of constants in f by quantifiers
	 in the repalcement. */
      Ilist constants_in_f = constants_in_formula(f);
      rename_these_bound_vars(new, constants_in_f);
      subst_free_vars(new, vars, terms, NULL);
      zap_plist(vars);
      zap_plist(terms);
      zap_ilist(constants_in_f);
      zap_term(f->atom);
      return new;
    }
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = subst_atomic_formula(f->kids[i], target, replacement);
  }
  return f;
}  /* subst_atomic_formula */

/*************
 *
 *   strip_quantifiers()
 *
 *************/

static
Formula strip_quantifiers(Formula f)
{
  while (quant_form(f))
    f = f->kids[0];
  return f;
}  /* strip_quantifiers */

/*************
 *
 *   definition_applies()
 *
 *************/

static
BOOL definition_applies(Formula f, Formula def)
{
  int symnum;
  if (!is_definition(def)) {
    p_formula(def);
    fatal_error("definition_applies received non-definition");
  }
  def = strip_quantifiers(def);
  symnum = SYMNUM(def->kids[0]->atom);
  return relation_in_formula(f, symnum);
}  /* definition_applies */

/*************
 *
 *   expand_with_definition()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula expand_with_definition(Formula f, Formula def)
{
  Formula g = formula_copy(f);
  Formula result;
  Ilist constants_in_def;
  if (!is_definition(def)) {
    p_formula(def);
    fatal_error("expand_with_definition received non-definition");
  }
  /* We have to prevent capture of constants in the definition
     by quantifiers in the formula being expanded. */
  constants_in_def = constants_in_formula(def);
  rename_these_bound_vars(g, constants_in_def);
  zap_ilist(constants_in_def);
  def = strip_quantifiers(def);
  result = subst_atomic_formula(g, def->kids[0], def->kids[1]);
  return result;
}  /* expand_with_definition */

/*************
 *
 *   first_definition()
 *
 *************/

static
Topform first_definition(Plist p)
{
  if (p == NULL)
    return NULL;
  else {
    Topform tf = p->v;
    if (is_definition(tf->formula))
      return tf;
    else
      return first_definition(p->next);
  }
}  /* first_definition */

/*************
 *
 *   process_definitions()
 *
 *************/

/* DOCUMENTATION
All arguments are plists of Topforms containing formulas.
*/

/* PUBLIC */
void process_definitions(Plist formulas,
			 Plist *results,
			 Plist *defs,
			 Plist *rewritten)
{
  Plist work = formulas;
  Topform def;
  *defs = NULL;
  *rewritten = NULL;

  def = first_definition(work);
  while (def) {
    Plist p;
    work = plist_remove(work, def);
    *defs = plist_append(*defs, def);
    for (p = work; p; p = p->next) {
      Topform old = p->v;
      if (definition_applies(old->formula, def->formula)) {
	Topform new = get_topform();
	new->is_formula = TRUE;
	new->formula = expand_with_definition(old->formula, def->formula);
	assign_clause_id(new);
	new->justification = expand_def_just(old, def);
	*rewritten = plist_append(*rewritten, old);
	p->v = new;
      }
    }
    def = first_definition(work);
  }
  *results = work;
}  /* process_definitions */

/*************
 *
 *   expand_with_definitions()
 *
 *************/

/* DOCUMENTATION
All arguments are plists of Topforms containing formulas.
*/

/* PUBLIC */
void expand_with_definitions(Plist formulas,
			     Plist defs,
			     Plist *results,
			     Plist *rewritten)
{
  Plist p1;

  for (p1 = formulas; p1; p1 = p1->next) {
    Topform work = p1->v;
    Plist p2;
    for (p2 = defs; p2; p2 = p2->next) {
      Topform def = p2->v;
      if (definition_applies(work->formula, def->formula)) {
	Topform new = get_topform();
	new->is_formula = TRUE;
	new->formula = expand_with_definition(work->formula, def->formula);
	assign_clause_id(work);
	new->justification = expand_def_just(work, def);
	*rewritten = plist_prepend(*rewritten, work);
	work = new;
      }
    }
    *results = plist_prepend(*results, work);
  }

  zap_plist(formulas);  /* shallow */
  *results = reverse_plist(*results);

}  /* expand_with_definitions */

/*************
 *
 *   separate_definitions()
 *
 *************/

/* DOCUMENTATION
All arguments are plists of Topforms containing formulas.
*/

/* PUBLIC */
void separate_definitions(Plist formulas,
			  Plist *defs,
			  Plist *nondefs)
{
  Plist p;
  *defs = NULL;
  *nondefs = NULL;

  for (p = formulas; p; p = p->next) {
    Topform tf = p->v;
    if (is_definition(tf->formula))
      *defs = plist_prepend(*defs, tf);
    else
      *nondefs = plist_prepend(*nondefs, tf);
  }

  zap_plist(formulas);  /* shallow */

  *defs = reverse_plist(*defs);
  *nondefs = reverse_plist(*nondefs);

}  /* separate_definitions */

