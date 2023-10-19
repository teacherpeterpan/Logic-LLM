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

#include "formula.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_FORMULA PTRS(sizeof(struct formula))
static unsigned Formula_gets, Formula_frees;

static unsigned Arg_mem;  /* memory (pointers) for arrays of args */

/*************
 *
 *   Formula get_formula()
 *
 *************/

static
Formula get_formula(int arity)
{
  Formula p = get_cmem(PTRS_FORMULA);
  p->kids = get_cmem(arity);
  p->arity = arity;
  Formula_gets++;
  Arg_mem += arity;
  return(p);
}  /* get_formula */

/*************
 *
 *    free_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_formula(Formula p)
{
  if (p->excess_refs != 0)
    fatal_error("free_formula: freeing shared formula");
  free_mem(p->kids, p->arity);
  Arg_mem -= p->arity;
  free_mem(p, PTRS_FORMULA);
  Formula_frees++;
}  /* free_formula */

/*************
 *
 *   fprint_formula_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the formula package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_formula_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct formula);
  fprintf(fp, "formula (%4d)      %11u%11u%11u%9.1f K\n",
          n, Formula_gets, Formula_frees,
          Formula_gets - Formula_frees,
          ((Formula_gets - Formula_frees) * n) / 1024.);

  fprintf(fp, "    formula arg arrays:                              %9.1f K\n",
	  Arg_mem * BYTES_POINTER / 1024.); 

}  /* fprint_formula_mem */

/*************
 *
 *   p_formula_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the formula package.
*/

/* PUBLIC */
void p_formula_mem()
{
  fprint_formula_mem(stdout, TRUE);
}  /* p_formula_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   formula_megs()
 *
 *************/

/* DOCUMENTATION
Return the approximate number of megabytes in use for storage of formulas.
*/

/* PUBLIC */
unsigned formula_megs(void)
{
  unsigned bytes =
    (Formula_gets - Formula_frees) * sizeof(struct formula)
    +
    Arg_mem * BYTES_POINTER;

  return bytes / (1024 * 1024);
}  /* formula_megs */

/*************
 *
 *   formula_get()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula formula_get(int arity, Ftype type)
{
  Formula f = get_formula(arity);
  f->type = type;
  return f;
}  /* formula_get */

/*************
 *
 *   zap_formula()
 *
 *************/

/* DOCUMENTATION
Free a formula, including all of its subformulas, including its atoms.
If a subformula as excess references, the refcount is decremented instead.
*/

/* PUBLIC */
void zap_formula(Formula f)
{
  if (f == NULL)
    return;
  else if (f->excess_refs > 0)
    f->excess_refs--;
  else {
    if (f->type == ATOM_FORM)
      zap_term(f->atom);
    else {
      int i;
      for (i = 0; i < f->arity; i++)
	zap_formula(f->kids[i]);
    }
    if (f->attributes)
      zap_attributes(f->attributes);
    free_formula(f);
  }
}  /* zap_formula */

/*************
 *
 *   logic_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL logic_term(Term t)
{
  return (is_term(t, true_sym(), 0) ||
	  is_term(t, false_sym(), 0) ||
	  is_term(t, not_sym(), 1) ||
	  is_term(t, and_sym(), 2) ||
	  is_term(t, or_sym(), 2) ||
	  is_term(t, imp_sym(), 2) ||
	  is_term(t, impby_sym(), 2) ||
	  is_term(t, iff_sym(), 2) ||
	  is_term(t, quant_sym(), 3));
}  /* logic_term */

/*************
 *
 *   gather_symbols_in_term()
 *
 *************/

static
void gather_symbols_in_term(Term t, I2list *rsyms, I2list *fsyms)
{
  if (!VARIABLE(t)) {
    if (is_term(t, "if", 3)) {
      gather_symbols_in_formula_term(ARG(t,0), rsyms, fsyms);
      gather_symbols_in_term(ARG(t,1), rsyms, fsyms);
      gather_symbols_in_term(ARG(t,2), rsyms, fsyms);
    }
    else {
      int i;
      *fsyms = multiset_add(*fsyms, SYMNUM(t));
      for (i = 0; i < ARITY(t); i++) {
	gather_symbols_in_term(ARG(t,i), rsyms, fsyms);
      }
    }
  }
}  /* gather_symbols_in_term */

/*************
 *
 *   gather_symbols_in_formula_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void gather_symbols_in_formula_term(Term t, I2list *rsyms, I2list *fsyms)
{
  if (logic_term(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      if (is_term(t, quant_sym(), 3) && i != 3)
	;  /* skip quantifier and quantified variable */
      else
	gather_symbols_in_formula_term(ARG(t,i), rsyms, fsyms);
    }
  }
  else {
    int i;
    *rsyms = multiset_add(*rsyms, SYMNUM(t));
    for (i = 0; i < ARITY(t); i++)
      gather_symbols_in_term(ARG(t,i), rsyms, fsyms);
  }
}  /* gather_symbols_in_formula_term */

/*************
 *
 *   gather_symbols_in_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void gather_symbols_in_formula(Formula f, I2list *rsyms, I2list *fsyms)
{
  if (f->type == ATOM_FORM) {
    if (is_term(f->atom, "if", 3)) {
      gather_symbols_in_formula_term(ARG(f->atom,0), rsyms, fsyms);
      gather_symbols_in_formula_term(ARG(f->atom,1), rsyms, fsyms);
      gather_symbols_in_formula_term(ARG(f->atom,2), rsyms, fsyms);
    }
    else
      gather_symbols_in_formula_term(f->atom, rsyms, fsyms);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      gather_symbols_in_formula(f->kids[i], rsyms, fsyms);
  }
}  /* gather_symbols_in_formula */

/*************
 *
 *   gather_symbols_in_formulas()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void gather_symbols_in_formulas(Plist lst, I2list *rsyms, I2list *fsyms)
{
  Plist p;
  for (p = lst; p; p = p->next)
    gather_symbols_in_formula(p->v, rsyms, fsyms);
}  /* gather_symbols_in_formulas */

/*************
 *
 *   function_symbols_in_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist function_symbols_in_formula(Formula f)
{
  Ilist p;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  gather_symbols_in_formula(f, &rsyms, &fsyms);
  p = multiset_to_set(fsyms);
  zap_i2list(rsyms);
  zap_i2list(fsyms);
  return p;
}  /* function_symbols_in_formula */

/*************
 *
 *   relation_symbols_in_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist relation_symbols_in_formula(Formula f)
{
  Ilist p;
  I2list rsyms = NULL;
  I2list fsyms = NULL;
  gather_symbols_in_formula(f, &rsyms, &fsyms);
  p = multiset_to_set(rsyms);
  zap_i2list(rsyms);
  zap_i2list(fsyms);
  return p;
}  /* relation_symbols_in_formula */

/*************
 *
 *   relation_symbol_in_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL relation_symbol_in_formula(int sn, Formula f)
{
  Ilist p = relation_symbols_in_formula(f);
  BOOL found = ilist_member(p, sn);
  zap_ilist(p);
  return found;
}  /* relation_symbol_in_formula */

/*************
 *
 *   term_to_formula()
 *
 *************/

/* DOCUMENTATION
Assume that no subterm (of t) representing a formula is a
term of type VARIABLE.  The given Term is not changed.
*/

/* PUBLIC */
Formula term_to_formula(Term t)
{
  Formula f = NULL;
  Ftype type;
  Attribute attributes = NULL;

  if (is_term(t, attrib_sym(), 2)) {
    attributes = term_to_attributes(ARG(t,1), attrib_sym());
    t = ARG(t,0);
  }

  if (is_term(t, quant_sym(), 3)) {
    /* example: $quantified(all,x,p) */
    Term quant = ARG(t,0);
    Term var = ARG(t,1);
    Ftype qtype = (is_term(quant, all_sym(), 0) ? ALL_FORM : EXISTS_FORM);
    f = formula_get(1, qtype);
    f->kids[0] = term_to_formula(ARG(t,2));
    f->qvar = sn_to_str(SYMNUM(var));
  }
  else {
    if (is_term(t, true_sym(), 0))
      type = AND_FORM;
    else if (is_term(t, false_sym(), 0))
      type = OR_FORM;
    else if (is_term(t, not_sym(), 1))
      type = NOT_FORM;
    else if (is_term(t, and_sym(), 2))
      type = AND_FORM;
    else if (is_term(t, or_sym(), 2))
      type = OR_FORM;
    else if (is_term(t, iff_sym(), 2))
      type = IFF_FORM;
    else if (is_term(t, imp_sym(), 2))
      type = IMP_FORM;
    else if (is_term(t, impby_sym(), 2))
      type = IMPBY_FORM;
    else
      type = ATOM_FORM;

    if (type == ATOM_FORM) {
      f = formula_get(0, ATOM_FORM);
      f->atom = copy_term(t);
    }
    else if (type == NOT_FORM) {
      f = formula_get(1, NOT_FORM);
      f->kids[0] = term_to_formula(ARG(t,0));
    }
    else if (ARITY(t) == 0) {
      f = formula_get(0, type);
    }
    else {
      f = formula_get(2, type);
      f->kids[0] = term_to_formula(ARG(t,0));
      f->kids[1] = term_to_formula(ARG(t,1));
    }
  }
  f = flatten_top(f);
  f->attributes = attributes;
  return f;
}  /* term_to_formula */

/*************
 *
 *   formula_to_term()
 *
 *************/

/* DOCUMENTATION
Returns an entirely new term.
*/

/* PUBLIC */
Term formula_to_term(Formula f)
{
  Term t = NULL;

  switch (f->type) {
  case ATOM_FORM:
    t = copy_term(f->atom);
    break;
  case NOT_FORM:
    t = get_rigid_term(not_sym(), 1);
    ARG(t,0) = formula_to_term(f->kids[0]);
    break;
  case IFF_FORM:
    t = get_rigid_term(iff_sym(), 2);
    ARG(t,0) = formula_to_term(f->kids[0]);
    ARG(t,1) = formula_to_term(f->kids[1]);
    break;
  case IMP_FORM:
    t = get_rigid_term(imp_sym(), 2);
    ARG(t,0) = formula_to_term(f->kids[0]);
    ARG(t,1) = formula_to_term(f->kids[1]);
    break;
  case IMPBY_FORM:
    t = get_rigid_term(impby_sym(), 2);
    ARG(t,0) = formula_to_term(f->kids[0]);
    ARG(t,1) = formula_to_term(f->kids[1]);
    break;
  case AND_FORM:
  case OR_FORM:
    if (f->arity == 0)
      t = get_rigid_term(f->type == AND_FORM ? true_sym() : false_sym(), 0);
    else {
      int i = f->arity-1;
      t = formula_to_term(f->kids[i]);
      for (i--; i >= 0; i--) {
	Term t1 = get_rigid_term(f->type == AND_FORM ? and_sym() : or_sym(), 2);
	ARG(t1,0) = formula_to_term(f->kids[i]);
	ARG(t1,1) = t;
	t = t1;
      }
    }
    break;
  case ALL_FORM:
  case EXISTS_FORM:
    {
      /* transform to: $quantified(all,x,f) */
      t = get_rigid_term(quant_sym(), 3);
      ARG(t,0) = get_rigid_term(f->type == ALL_FORM ? all_sym() : exists_sym(), 0);
      ARG(t,1) = get_rigid_term(f->qvar, 0);
      ARG(t,2) = formula_to_term(f->kids[0]);
    }      
    break;
  }

  if (f->attributes)
    t = build_binary_term(str_to_sn(attrib_sym(), 2),
			  t,
			  attributes_to_term(f->attributes, attrib_sym()));
  return t;
}  /* formula_to_term */

/*************
 *
 *   fprint_formula()
 *
 *************/

/* DOCUMENTATION
This routine prints a formula to a file.
If you wish to have a formula printed without extra parentheses,
you can call fprint_formula_term() instead.
*/

/* PUBLIC */
void fprint_formula(FILE *fp, Formula f)
{
  if (f->type == ATOM_FORM) {
    /* fprintf(fp, "("); */
    fprint_term(fp, f->atom);
    /* fprintf(fp, ")"); */
  }
  else if (f->type == NOT_FORM) {
    /* fprintf(fp, "(%s ", not_sym()); */
    fprintf(fp, "%s ", not_sym());
    fprint_formula(fp, f->kids[0]);
    /* fprintf(fp, ")"); */
  }
  else if (f->type == IFF_FORM) {
    fprintf(fp, "(");
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, " %s ", iff_sym());
    fprint_formula(fp, f->kids[1]);
    fprintf(fp, ")");
  }
  else if (f->type == IMP_FORM) {
    fprintf(fp, "(");
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, " %s ", imp_sym());
    fprint_formula(fp, f->kids[1]);
    fprintf(fp, ")");
  }
  else if (f->type == IMPBY_FORM) {
    fprintf(fp, "(");
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, " %s ", impby_sym());
    fprint_formula(fp, f->kids[1]);
    fprintf(fp, ")");
  }
  else if (quant_form(f)) {
    fprintf(fp, "(%s %s ", f->type==ALL_FORM ? all_sym() : exists_sym(), f->qvar);
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, ")");
  }
  else if (f->type == AND_FORM || f->type == OR_FORM) {
    if (f->arity == 0)
      fprintf(fp, "%s", f->type == AND_FORM ? true_sym() : false_sym());
    else {
      int i;
      fprintf(fp, "(");
      for (i = 0; i < f->arity; i++) {
	fprint_formula(fp, f->kids[i]);
	if (i < f->arity-1)
	  fprintf(fp, " %s ", f->type == AND_FORM ? and_sym() : or_sym());
      }
      fprintf(fp, ")");
    }
  }
}  /* fprint_formula */

/*************
 *
 *   p_formula()
 *
 *************/

/* DOCUMENTATION
This routine prints a formula, followed by ".\n" and fflush, to stdout.
If you wish to have a formula printed without extra parentheses,
you can call p_formula_term() instead.
If you don't want the newline, use fprint_formula() instead.
*/

/* PUBLIC */
void p_formula(Formula c)
{
  fprint_formula(stdout, c);
  printf(".\n");
  fflush(stdout);
}  /* p_formula */

/*************
 *
 *   hash_formula()
 *
 *************/

/* DOCUMENTATION
This is a simple hash function for formulas.
It shifts symbols by 3 bits and does exclusive ORs.
*/

/* PUBLIC */
unsigned hash_formula(Formula f)
{
  if (f->type == ATOM_FORM)
    return hash_term(f->atom);
  else if (quant_form(f))
    return (f->type << 3) ^ (unsigned) f->qvar[0];
  else {
    unsigned x = f->type;
    int i;
    for (i = 0; i < f->arity; i++)
      x = (x << 3) ^ hash_formula(f->kids[i]);
    return x;
  }
}  /* hash_formula */

/*************
 *
 *   formula_ident()
 *
 *************/

/* DOCUMENTATION
This Boolean function checks if two formulas are identical.
The routine term_ident() checks identity of atoms.
<P>
The test is for strict identity---it does not consider
renamability of bound variables, permutability of AND or OR,
or symmetry of IFF or equality.
*/

/* PUBLIC */
BOOL formula_ident(Formula f, Formula g)
{
  if (f->type != g->type || f->arity != g->arity)
    return FALSE;
  else if (f->type == ATOM_FORM)
    return term_ident(f->atom, g->atom);
  else if (quant_form(f))
    return (str_ident(f->qvar,g->qvar) &&
            formula_ident(f->kids[0],g->kids[0]));
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (!formula_ident(f->kids[i], g->kids[i]))
        return FALSE;
    return TRUE;
  }
}  /* formula_ident */

/*************
 *
 *   formula_copy()
 *
 *************/

/* DOCUMENTATION
This function returns a copy of the given formula.
All subformulas, including the atoms, are copied.
*/

/* PUBLIC */
Formula formula_copy(Formula f)
{
  Formula g = formula_get(f->arity, f->type);

  if (f->type == ATOM_FORM)
    g->atom = copy_term(f->atom);
  else {
    int i;
    if (quant_form(f))
      g->qvar = f->qvar;
    for (i = 0; i < f->arity; i++)
      g->kids[i] = formula_copy(f->kids[i]);
  }
  return g;
}  /* formula_copy */

/*************
 *
 *   dual_type()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL dual_type(int op)
{
  switch (op) {
  case AND_FORM: return OR_FORM;
  case OR_FORM: return AND_FORM;
  case ALL_FORM: return EXISTS_FORM;
  case EXISTS_FORM: return ALL_FORM;
  default: return op;
  }
}  /* dual */

/*************
 *
 *   dual()
 *
 *************/

/* DOCUMENTATION
Change a formula into its dual.  This is destructive.
*/

/* PUBLIC */
Formula dual(Formula f)
{
  int i;
  for (i = 0; i < f->arity; i++)
    f->kids[i] = dual(f->kids[i]);

  f->type = dual_type(f->type);
  return f;
}  /* dual */

/*************
 *
 *   and()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula and(Formula a, Formula b)
{
  Formula f = formula_get(2, AND_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
}  /* and */

/*************
 *
 *   or()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula or(Formula a, Formula b)
{
  Formula f = formula_get(2, OR_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
}  /* or */

/*************
 *
 *   imp()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula imp(Formula a, Formula b)
{
  Formula f = formula_get(2, IMP_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
}  /* imp */

/*************
 *
 *   impby()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula impby(Formula a, Formula b)
{
  Formula f = formula_get(2, IMPBY_FORM);
  f->kids[0] = a;
  f->kids[1] = b;
  return f;
}  /* impby */

/*************
 *
 *   not()
 *
 *************/

static
Formula not(Formula a)
{
  Formula f = formula_get(1, NOT_FORM);
  f->kids[0] = a;
  return f;
}  /* not */

/*************
 *
 *   negate()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula negate(Formula a)
{
  return not(a);
}  /* negate */

/*************
 *
 *   quant_form()  -- is it a quantified formula?
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL quant_form(Formula f)
{
  return (f->type == ALL_FORM || f->type == EXISTS_FORM);
}  /* quant_form */

/*************
 *
 *   flatten_top() -- applies to AND and OR.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula flatten_top(Formula f)
{
  if (f->type != AND_FORM && f->type != OR_FORM)
    return f;
  else {
    BOOL operate = FALSE;
    int n = 0;  /* count new arity */
    int i;
    for (i = 0; i < f->arity; i++) {
      if (f->type != f->kids[i]->type)
	n++;
      else {
	n += (f->kids[i]->arity);
	operate = TRUE;
      }
    }
    if (!operate)
      return f;
    else {
      Formula g = formula_get(n, f->type);
      int i, j;
      j = 0;
      for (i = 0; i < f->arity; i++) {
	if (f->kids[i]->type != f->type)
	  g->kids[j++] = f->kids[i];
	else {
	  int k;
	  for (k = 0; k < f->kids[i]->arity; k++)
	    g->kids[j++] = f->kids[i]->kids[k];
	  free_formula(f->kids[i]);
	}
      }
      free_formula(f);
      /* If the new formula has just one argument, return that argument. */
      if (g->arity == 1) {
	Formula h = g->kids[0];
	free_formula(g);
	return h;
      }
      else
	return g;
    }
  }
}  /* flatten_top */

/*************
 *
 *   formula_flatten()
 *
 *************/

/* DOCUMENTATION
This routine (recursively) flattens all AND and OR subformulas.
*/

/* PUBLIC */
Formula formula_flatten(Formula f)
{
  int i;
  for (i = 0; i < f->arity; i++)
    f->kids[i] = formula_flatten(f->kids[i]);
  return flatten_top(f);
}  /* flatten */
  
/*************
 *
 *   nnf2()
 *
 *************/

/* DOCUMENTATION
Transform a formula into negation normal form (NNF).  (NNF means
that all propositional connectives have been rewritten in terms of
AND, OR and NOT, and all negation signs ar up against atomic formulas).
<P>
The argument "pref" should be either CONJUNCTION or DISJUNCTION,
and it specifies the preferred form to use when translating IFFs.
<P>
This rouine is destructive; a good way to call
it is <TT>f = nnf2(f, CONJUNCTION)</TT>.
*/

/* PUBLIC */
Formula nnf2(Formula f, Fpref pref)
{
  if (f->type == ATOM_FORM)
    return f;
  else if (quant_form(f)) {
    f->kids[0] = nnf2(f->kids[0], pref);
    return f;
  }
  else if (f->type == AND_FORM || f->type == OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = nnf2(f->kids[i], pref);
    return f;
  }
  else if (f->type == IMP_FORM) {
    Formula g = nnf2(or(not(f->kids[0]), f->kids[1]), pref);
    free_formula(f);
    return g;
  }
  else if (f->type == IMPBY_FORM) {
    Formula g = nnf2(or(f->kids[0], not(f->kids[1])), pref);
    free_formula(f);
    return g;
  }
  else if (f->type == IFF_FORM) {
    Formula g;
    Formula a = f->kids[0];
    Formula b = f->kids[1];
    Formula ac = formula_copy(a);
    Formula bc = formula_copy(b);

    if (pref == CONJUNCTION)
      g = nnf2(and(imp(a,b), impby(ac,bc)), pref);
    else
      g = nnf2(or(and(a,b),and(not(ac),not(bc))), pref);
	     
    free_formula(f);
    return g;
  }

  /* NOT */

  else if (f->type == NOT_FORM) {
    Formula h = f->kids[0];
    if (h->type == ATOM_FORM)
      return f;
    else if (h->type == NOT_FORM) {
      Formula g = nnf2(h->kids[0], pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (quant_form(h)) {
      Formula g = formula_get(1, dual_type(h->type));
      g->qvar = h->qvar;
      g->kids[0] = nnf2(not(h->kids[0]), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == AND_FORM || h->type == OR_FORM) {
      Formula g = formula_get(h->arity, dual_type(h->type));
      int i;
      for (i = 0; i < h->arity; i++)
	g->kids[i] = nnf2(not(h->kids[i]), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == IMP_FORM) {
      Formula g = nnf2(and(h->kids[0], not(h->kids[1])), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == IMPBY_FORM) {
      Formula g = nnf2(and(not(h->kids[0]), h->kids[1]), pref);
      free_formula(h);
      free_formula(f);
      return g;
    }
    else if (h->type == IFF_FORM) {
      Formula g;
      Formula a = h->kids[0];
      Formula b = h->kids[1];
      Formula ac = formula_copy(a);
      Formula bc = formula_copy(b);

      if (pref == CONJUNCTION)
	g = nnf2(and(or(a,b),or(not(ac),not(bc))), pref);
      else
	g = nnf2(or(and(a,not(b)),and(not(ac),bc)), pref);

      free_formula(h);
      free_formula(f);
      return g;
    }
    else
      return f;
  }  /* NOT */
  else
    return f;
}  /* nnf2 */

/*************
 *
 *   nnf()
 *
 *************/

/* DOCUMENTATION
Transform a formula into negation normal form (NNF).  (NNF means
that all propositional connectives have been rewritten in terms of
AND, OR and NOT, and all negation signs ar up against atomic formulas).
<P>
This routine is destructive; a good way to call
it is <TT>f = nnf(f)</TT>.
*/

/* PUBLIC */
Formula nnf(Formula f)
{
  return nnf2(f, CONJUNCTION);
}  /* nnf */

/*************
 *
 *   make_conjunction()
 *
 *************/

/* DOCUMENTATION
If the formula is not a conjunction, make it so.
*/

/* PUBLIC */
Formula make_conjunction(Formula f)
{
  if (f->type == AND_FORM)
    return f;
  else {
    Formula h = formula_get(1, AND_FORM);
    h->kids[0] = f;
    return h;
  }
}  /* make_conjunction */

/*************
 *
 *   make_disjunction()
 *
 *************/

/* DOCUMENTATION
If the formula is not a dismunction, make it so.
*/

/* PUBLIC */
Formula make_disjunction(Formula f)
{
  if (f->type == OR_FORM)
    return f;
  else {
    Formula h = formula_get(1, OR_FORM);
    h->kids[0] = f;
    return h;
  }
}  /* make_disjunction */

/*************
 *
 *   formula_canon_eq()
 *
 *************/

/* DOCUMENTATION
For each equality in the formula, if the right side greater
according to "term_compare_ncv", flip the equality.
*/

/* PUBLIC */
void formula_canon_eq(Formula f)
{
  if (f->type == ATOM_FORM) {
    Term a = f->atom;
    if (eq_term(a)) {
      Term left = ARG(a,0);
      Term right = ARG(a,1);
      if (term_compare_ncv(left, right) == LESS_THAN) {
	ARG(a,0) = right;
	ARG(a,1) = left;
      }
    }
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      formula_canon_eq(f->kids[i]);
  }
}  /* formula_canon_eq */

/*************
 *
 *   formula_size()
 *
 *************/

/* DOCUMENTATION
How many nodes are in the formula.  (Atomic formulae count as 1.)
*/

/* PUBLIC */
int formula_size(Formula f)
{
  if (f->type == ATOM_FORM)
    return 1;
  else {
    int i;
    int n = 0;
    for (i = 0; i < f->arity; i++)
      n += formula_size(f->kids[i]);
    return n+1;
  }
}  /* formula_size */

/*************
 *
 *   greatest_qvar()
 *
 *************/

/* DOCUMENTATION
Return the greatest SYMNUM of a quantified variable in Formula f.
<P>
Recall that in Formulas, a quantified variable is represented
as a constant (which is bound by the quantifier).
If the formula has no quantified variables, return -1.
*/

/* PUBLIC */
int greatest_qvar(Formula f)
{
  if (quant_form(f)) {
    int sn = str_to_sn(f->qvar, 0);
    int max_sub = greatest_qvar(f->kids[0]);
    return (sn > max_sub ? sn : max_sub);
  }
  else {
    int max = -1;
    int i;
    for (i = 0; i < f->arity; i++) {
      int max_sub = greatest_qvar(f->kids[i]);
      max = (max_sub > max ? max_sub : max);
    }
    return max;
  }
}  /* greatest_qvar */

/*************
 *
 *   greatest_symnum_in_formula()
 *
 *************/

/* DOCUMENTATION
Return the greatest SYMNUM of a any subterm.  This includes quantifed
variables that don't occur in any term.
<P>
This routine is intended to be used if you need malloc an array
for indexing by SYMNUM.
*/

/* PUBLIC */
int greatest_symnum_in_formula(Formula f)
{
  if (f->type == ATOM_FORM) {
    return greatest_symnum_in_term(f->atom);
  }
  if (quant_form(f)) {
    int sn = str_to_sn(f->qvar, 0);
    int max_sub = greatest_symnum_in_formula(f->kids[0]);
    return (sn > max_sub ? sn : max_sub);
  }
  else {
    int max = -1;
    int i;
    for (i = 0; i < f->arity; i++) {
      int max_sub = greatest_symnum_in_formula(f->kids[i]);
      max = (max_sub > max ? max_sub : max);
    }
    return max;
  }
}  /* greatest_symnum_in_formula */

/*************
 *
 *   subst_free_var()
 *
 *************/

/* DOCUMENTATION
In formula f, substitute free occurrences of target
with replacement.  The function term_ident() is used,
and the target can be any term.
*/

/* PUBLIC */
void subst_free_var(Formula f, Term target, Term replacement)
{
  if (f->type == ATOM_FORM)
    f->atom = subst_term(f->atom, target, replacement);
  else if (quant_form(f) && str_ident(sn_to_str(SYMNUM(target)), f->qvar)) {
    ; /* Do nothing, because we have a quantified variable of the same name. */
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      subst_free_var(f->kids[i], target, replacement);
  }
}  /* subst_free_var */

/*************
 *
 *   elim_rebind()
 *
 *************/

static
Formula elim_rebind(Formula f, Ilist uvars)
{
  if (quant_form(f)) {
    Term var = get_rigid_term(f->qvar, 0);
    Ilist uvars_plus;
  
    if (ilist_member(uvars, SYMNUM(var))) {
      /* We are in the scope of another variable with this name, so
       * rename this variable.
       */
      int sn = gen_new_symbol("y", 0, uvars);
      Term newvar = get_rigid_term(sn_to_str(sn), 0);
      subst_free_var(f->kids[0], var, newvar);
      f->qvar = sn_to_str(sn);
      free_term(var);
      var = newvar;
    }

    uvars_plus = ilist_prepend(uvars, SYMNUM(var));
    f->kids[0] = elim_rebind(f->kids[0], uvars_plus);
    free_term(var);
    free_ilist(uvars_plus);  /* frees first node only; uvars still good */
    return f;
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = elim_rebind(f->kids[i], uvars);
    return f;
  }
}  /* elim_rebind */

/*************
 *
 *   eliminate_rebinding()
 *
 *************/

/* DOCUMENTATION
This routine renames quantified variables so that
no quantified variable occurs in the scope of a quantified variable
with the same name.
<P>
If you wish to rename variables so that each
quantifer has a unique variable,
you can use the routine unique_quantified_vars() instead.
<P>
The argument f is "used up" during the procedure.
<P>
(This could be a void routine, because
none of the formula nodes is changed; I made it return the Formula
so that it is consistent with its friends.)
*/

/* PUBLIC */
Formula eliminate_rebinding(Formula f)
{
  f = elim_rebind(f, NULL);
  return f;
}  /* eliminate_rebinding */

/*************
 *
 *   free_vars()
 *
 *************/

static
Plist free_vars(Formula f, Plist vars)
{
  if (f->type == ATOM_FORM)
    vars = free_vars_term(f->atom, vars);
  else if (quant_form(f)) {
    Term var = get_rigid_term(f->qvar, 0);
    Plist vars2 = free_vars(f->kids[0], NULL);
    vars2 = tlist_remove(var, vars2);
    vars = tlist_union(vars, vars2);
    zap_term(var);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      vars = free_vars(f->kids[i], vars);
  }
  return vars;
}  /* free_vars */

/*************
 *
 *   closed_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL closed_formula(Formula f)
{
  Plist vars = free_vars(f, NULL);  /* deep (returns new terms) */
  BOOL ok = (vars == NULL);
  zap_tlist(vars);
  return ok;
}  /* closed_formula */

/*************
 *
 *   get_quant_form()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula get_quant_form(Ftype type, char *qvar, Formula subformula)
{
  Formula f = formula_get(1, type);
  f->qvar = qvar;
  f->kids[0] = subformula;
  return f;
}  /* get_quant_form */

/*************
 *
 *   uni_close()
 *
 *************/

static
Formula uni_close(Formula f, Plist vars)
{
  if (vars == NULL)
    return f;
  else {
    Formula g = uni_close(f, vars->next);
    Term v = vars->v;
    return get_quant_form(ALL_FORM, sn_to_str(SYMNUM(v)), g);
  }
}  /* uni_close */

/*************
 *
 *   universal_closure()
 *
 *************/

/* DOCUMENTATION
Construct the universal closure of Formula f.  The Formula
is consumed during the construction.
*/

/* PUBLIC */
Formula universal_closure(Formula f)
{
  Plist vars = free_vars(f, NULL);  /* deep (returns new terms) */
  f = uni_close(f, vars);
  zap_tlist(vars);
  return f;
}  /* universal_closure */

/*************
 *
 *   free_var()
 *
 *************/

static
BOOL free_var(char *svar, Term tvar, Formula f)
{
  if (f->type == ATOM_FORM)
    return occurs_in(tvar, f->atom);
  else if (quant_form(f) && str_ident(svar, f->qvar)) {
    return FALSE;
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++) {
      if (free_var(svar, tvar, f->kids[i]))
	return TRUE;
    }
    return FALSE;
  }
}  /* free_var */

/*************
 *
 *   free_variable()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL free_variable(char *svar, Formula f)
{
  Term tvar = get_rigid_term(svar, 0);
  BOOL free = free_var(svar, tvar, f);
  free_term(tvar);
  return free;
}  /* free_variable */

/*************
 *
 *   formulas_to_conjunction
 *
 *************/

/* DOCUMENTATION
Given a Plist of formulas, form a conjunction of the members.
The formulas are not copied, and the Plist is not freed, so
you may wish to call zap_plist after the call to this routine.
<p>
Note that the empty conjunction is TRUE.
*/

/* PUBLIC */
Formula formulas_to_conjunction(Plist formulas)
{
  Plist p;
  int n = plist_count(formulas);
  Formula f = formula_get(n, AND_FORM);
  int i = 0;
  for (p = formulas; p; p = p->next) {
    f->kids[i++] = p->v;
  }
  return f;
}  /* formulas_to_conjunction */

/*************
 *
 *   formulas_to_disjunction
 *
 *************/

/* DOCUMENTATION
Given a Plist of formulas, form a disjunction of the members.
The formulas are not copied, and the Plist is not freed, so
you may wish to call zap_plist after the call to this routine.
<p>
Note that the empty disjunction is FALSE.
*/

/* PUBLIC */
Formula formulas_to_disjunction(Plist formulas)
{
  Plist p;
  int n = plist_count(formulas);
  Formula f = formula_get(n, OR_FORM);
  int i = 0;
  for (p = formulas; p; p = p->next) {
    f->kids[i++] = p->v;
  }
  return f;
}  /* formulas_to_disjunction */

/*************
 *
 *   copy_plist_of_formulas()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist copy_plist_of_formulas(Plist formulas)
{
  if (formulas == NULL)
    return NULL;
  else {
    Plist tail = copy_plist_of_formulas(formulas->next);
    Plist head = get_plist();
    head->v = formula_copy(formulas->v);
    head->next = tail;
    return head;
  }
}  /* copy_plist_of_formulas */

/*************
 *
 *   literal_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL literal_formula(Formula f)
{
  if (f->type == ATOM_FORM)
    return TRUE;
  else if (f->type == NOT_FORM)
    return f->kids[0]->type == ATOM_FORM;
  else
    return FALSE;
}  /* literal_formula */

/*************
 *
 *   clausal_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL clausal_formula(Formula f)
{
  if (f->type == OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++) {
      if (!clausal_formula(f->kids[i]))
	return FALSE;
    }
    return TRUE;
  }
  else
    return literal_formula(f);
}  /* clausal_formula */

/*************
 *
 *   formula_set_vars_recurse()
 *
 *************/

static
void formula_set_vars_recurse(Formula f, char *vnames[], int max_vars)
{
  if (f->type == ATOM_FORM)
    f->atom = set_vars_recurse(f->atom, vnames, max_vars);
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      formula_set_vars_recurse(f->kids[i], vnames, max_vars);
  }
}  /* formula_set_vars_recurse */

/*************
 *
 *   formula_set_variables()
 *
 *************/

/* DOCUMENTATION
This routine traverses a formula and changes the constants
that should be variables, into variables.  On input, the formula
should have no variables.  The new variables are numbered
0, 1, 2 ... according the the first occurrence, reading from the
left.
<P>
A fatal error occurs if there are more than max_vars variables.
<P>
The intended is use is for input formulas that
are built without regard to variable/constant distinction.
*/

/* PUBLIC */
void formula_set_variables(Formula f, int max_vars)
{
  char *a[MAX_VARS], **vmap;
  int i;

  if (max_vars > MAX_VARS)
    vmap = malloc((max_vars * sizeof(char *)));
  else
    vmap = a;

  for (i = 0; i < max_vars; i++)
    vmap[i] = NULL;

  formula_set_vars_recurse(f, vmap, max_vars);

  /* Now do any answer attributes (with the same vmap). */

  if (f->attributes) {
    set_vars_attributes(f->attributes, vmap, max_vars);
#if 0
    /* Make sure that answer vars also occur in formula. */
    Plist attr_vars = vars_in_attributes(c->attributes);
    Plist formula_vars = vars_in_formula(c);
    if (!plist_subset(attr_vars, formula_vars)) {
      Plist p;
      printf("Variables in answers must also occur ordinary literals:\n");
      p_formula(c);
      for (p = attr_vars; p; p = p->next) {
	if (!plist_member(formula_vars, p->v)) {
	  Term t = p->v;
	  printf("Answer variable not in ordinary literal: %s.\n",
		 vmap[VARNUM(t)]);
	}
      }
      fatal_error("formula_set_variables, answer variable not in literal");
    }
    zap_plist(formula_vars);
    zap_plist(attr_vars);
#endif
  }
  
  if (max_vars > MAX_VARS)
    free(vmap);

}  /* formula_set_variables */

/*************
 *
 *   positive_formula()
 *
 *************/

/* DOCUMENTATION
Ignoring quantifiers, does the formula consist of an atomic
formula or the conjunction of atomic formulas?
*/

/* PUBLIC */
BOOL positive_formula(Formula f)
{
  Formula g = f;
  while (quant_form(g))
    g = g->kids[0];
  if (g->type == ATOM_FORM)
    return TRUE;
  else if (g->type != AND_FORM)
    return FALSE;
  else {
    int i;
    for (i = 0; i < g->arity; i++)
      if (!positive_formula(g->kids[i]))
	return FALSE;
    return TRUE;
  }
}  /* positive_formula */

/*************
 *
 *   formula_contains_attributes()
 *
 *************/

/* DOCUMENTATION
Does the formula or any of its subformulas contain attributes?
*/

/* PUBLIC */
BOOL formula_contains_attributes(Formula f)
{
  if (f->attributes != NULL)
    return TRUE;
  else if (f->type == ATOM_FORM)
    return FALSE;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (formula_contains_attributes(f->kids[i]))
	return TRUE;
    return FALSE;
  }
}  /* formula_contains_attributes */

/*************
 *
 *   subformula_contains_attributes()
 *
 *************/

/* DOCUMENTATION
Does any proper subformula contain attributes?
*/

/* PUBLIC */
BOOL subformula_contains_attributes(Formula f)
{
  if (f->type == ATOM_FORM)
    return FALSE;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (formula_contains_attributes(f->kids[i]))
	return TRUE;
    return FALSE;
  }
}  /* subformula_contains_attributes */

/*************
 *
 *   constants_in_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist constants_in_formula(Formula f)
{
  Ilist p = function_symbols_in_formula(f);
  p = symnums_of_arity(p, 0);
  return p;
}  /* constants_in_formula */

/*************
 *
 *   relation_in_formula()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL relation_in_formula(Formula f, int symnum)
{
  if (f->type == ATOM_FORM)
    return SYMNUM(f->atom) == symnum;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      if (relation_in_formula(f->kids[i], symnum))
	return TRUE;
    return FALSE;
  }
}  /* relation_in_formula */

/*************
 *
 *   rename_all_bound_vars()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void rename_all_bound_vars(Formula f)
{
  if (quant_form(f)) {
    Term var = get_rigid_term(f->qvar, 0);
    int sn = fresh_symbol("x", 0);
    Term newvar = get_rigid_term(sn_to_str(sn), 0);
    subst_free_var(f->kids[0], var, newvar);
    f->qvar = sn_to_str(sn);
    free_term(var);
    free_term(newvar);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      rename_all_bound_vars(f->kids[i]);
  }
}  /* rename_all_bound_vars */

/*************
 *
 *   rename_these_bound_vars()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void rename_these_bound_vars(Formula f, Ilist vars)
{
  /* Rename each quantified variable in "vars" to a new symbol. */
  if (quant_form(f)) {
    Term var = get_rigid_term(f->qvar, 0);
    if (ilist_member(vars, SYMNUM(var))) {
      /* Rename this variable. */
      int sn = fresh_symbol("x", 0);
      Term newvar = get_rigid_term(sn_to_str(sn), 0);
      subst_free_var(f->kids[0], var, newvar);
      f->qvar = sn_to_str(sn);
      free_term(var);
    }
    rename_these_bound_vars(f->kids[0], vars);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      rename_these_bound_vars(f->kids[i], vars);
  }
}  /* rename_these_bound_vars */



