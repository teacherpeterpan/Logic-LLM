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

#include "clausify.h"

/* Private definitions and types */

/*************
 *
 *   formula_to_literal()
 *
 *************/

/* DOCUMENTATION
This routine takes a Formula f and returns a Literals representation.
If the formula is not an ATOM_FORM or the negation of an ATOM_FORM,
NULL is returned.
<P>
The returned literal is an entirely new copy,
and the given formula is not changed.
*/

/* PUBLIC */
Literals formula_to_literal(Formula f)
{
  if (f->type == NOT_FORM && f->kids[0]->type == ATOM_FORM)
    return new_literal(FALSE, copy_term(f->kids[0]->atom));
  else if (f->type == ATOM_FORM)
    return new_literal(TRUE, copy_term(f->atom));
  else
    return NULL;
}  /* formula_to_literal */

/*************
 *
 *   formula_to_literals()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Literals formula_to_literals(Formula f)
{
  Literals lits = NULL;
  if (f->type == ATOM_FORM || f->type == NOT_FORM)
    lits = append_literal(lits, formula_to_literal(f));
  else if (f->type == OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      lits = append_literal(lits,formula_to_literal(f->kids[i]));
  }
  else {
    fatal_error("formula_to_literals, formula not ATOM, NOT, or OR");
  }
  return lits;
}  /* formula_to_literals */

/*************
 *
 *   formula_to_clause()
 *
 *************/

/* DOCUMENTATION
This routine takes a Formula f and returns a Topform representation.
If f is not an atom, literal, or disjunction of literals, the
returned clause will be NULL or not well formed.
<P>
The returned clause is an entirely new copy, and the given formula
is not changed.
*/

/* PUBLIC */
Topform formula_to_clause(Formula f)
{
  Topform c = get_topform();
  c->literals = formula_to_literals(f);
  upward_clause_links(c);
  return c;
}  /* formula_to_clause */

/*************
 *
 *   formula_to_clauses()
 *
 *************/

/* DOCUMENTATION
This routine takes a Formula f and returns a Plist of clauses
representation.
If f is not an atom, literal, or disjunction of literals, or a
conjunction of those things, the clauses in the returned list may be NULL
or not well formed.
<P>
The returned clauses are entirely new copies, and the given formula
is not changed.
*/

/* PUBLIC */
Plist formula_to_clauses(Formula f)
{
  Plist lst = NULL;
  if (f->type == AND_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      lst = plist_append(lst, formula_to_clause(f->kids[i]));
  }
  else if (f->type == OR_FORM || f->type == NOT_FORM || f->type == ATOM_FORM)
    lst = plist_append(lst, formula_to_clause(f));
  return lst;
}  /* formula_to_clauses */

/*************
 *
 *   clausify_formula()
 *
 *************/

/* DOCUMENTATION
This routine translates a Formula f into a Plist of clauses.
The translation includes Skolemization, so the result should
be unsatisfiable iff f is unsatisfiable.  The variables in
each clause are renumbered.  If there are more than MAX_VARS
variables in a clause, a fatal error occurs.
<P>
Formula f is not changed.
*/

/* PUBLIC */
Plist clausify_formula(Formula f)
{
  Formula g;
  Plist clauses, p;

  g = clausify_prepare(formula_copy(f));
  clauses = formula_to_clauses(g);

  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    renumber_variables(c, MAX_VARS);
  }
  zap_formula(g);
  return clauses;
}  /* clausify_formula */

/*************
 *
 *   vars_to_names()
 *
 *************/

static
Term vars_to_names(Term t)
{
  if (VARIABLE(t)) {
    Term a;
    char *s1 = malloc(25);
    char *s2 = malloc(25);
    Variable_style v = variable_style();
    s2 = int_to_str(VARNUM(t), s2, 25);

    switch (v) {
    case INTEGER_STYLE:   s1 = strcpy(s1, "");      break;
    case STANDARD_STYLE:  s1 = strcpy(s1, "var_");  break;
    case PROLOG_STYLE:    s1 = strcpy(s1, "VAR_");  break;
    }
    s1 = strcat(s1, s2);

    a = get_rigid_term(s1, 0);
    free_term(t);
    free(s1);
    free(s2);
    return a;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = vars_to_names(ARG(t,i));
    return t;
  }
}  /* vars_to_names */

/*************
 *
 *   literal_to_formula()
 *
 *************/

static
Formula literal_to_formula(Literals lit)
{
  Formula a = formula_get(0, ATOM_FORM);
  a->atom = vars_to_names(copy_term(lit->atom));
  if (lit->sign)
    return a;
  else {
    Formula n = formula_get(1, NOT_FORM);
    n->kids[0] = a;
    return n;
  }
}  /* literal_to_formula */

/*************
 *
 *   clause_to_formula()
 *
 *************/

/* DOCUMENTATION
Return an entirely new formula.
*/

/* PUBLIC */
Formula clause_to_formula(Topform c)
{
  if (c->literals == NULL) {
    Formula f = formula_get(0, ATOM_FORM);
    f->atom = get_rigid_term(false_sym(), 0);
    return f;
  }
  else {
    Literals lit;
    int i;
    Formula f = formula_get(number_of_literals(c->literals), OR_FORM);
    for (lit = c->literals, i = 0; lit; lit = lit->next, i++) {
      f->kids[i] = literal_to_formula(lit);
    }
    return f;
  }
}  /* clause_to_formula */

