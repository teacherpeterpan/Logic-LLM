#include "formula.h"

/* Private definitions and types */

/*
 * memory management
 */


static unsigned Formula_gets, Formula_frees;

#define BYTES_FORMULA sizeof(struct formula)
#define PTRS_FORMULA BYTES_FORMULA%BPP == 0 ? BYTES_FORMULA/BPP : BYTES_FORMULA/BPP + 1

/*************
 *
 *   Formula get_formula()
 *
 *************/

static
Formula get_formula(int arity)
{
  Formula p = get_mem(PTRS_FORMULA);
  p->kids = get_mem(arity);
  p->arity = arity;
  Formula_gets++;
  return(p);
}  /* get_formula */

/*************
 *
 *    free_formula()
 *
 *************/

static
void free_formula(Formula p)
{
  free_mem(p->kids, p->arity);
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

  n = BYTES_FORMULA;
  fprintf(fp, "formula (%4d)      %11u%11u%11u%9.1f K\n",
          n, Formula_gets, Formula_frees,
          Formula_gets - Formula_frees,
          ((Formula_gets - Formula_frees) * n) / 1024.);

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
 *   formula_get()
 *
 *************/

static
Formula formula_get(int arity, Ftype type)
{
  Formula f = get_formula(arity);
  f->type = type;
  return f;
}  /* formula_get */

/*************
 *
 *   quant_form()
 *
 *************/

static
BOOL quant_form(Formula f)
{
  return (f->type == ALL_FORM || f->type == EXISTS_FORM);
}  /* quant_form */

/*************
 *
 *   zap_formula()
 *
 *************/

/* DOCUMENTATION
Free a formula, including all of its subformulas, including its atoms.
*/

/* PUBLIC */
void zap_formula(Formula f)
{
  if (f == NULL)
    return;
  if (f->type == ATOM_FORM)
    zap_term(f->atom);
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      zap_formula(f->kids[i]);
  }
  free_formula(f);
}  /* zap_formula */

/*************
 *
 *   flatten_top()
 *
 *************/

static
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
This routine (recursively) flattens all AND and OR formulas.
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
 *   term_to_formula()
 *
 *************/

/* DOCUMENTATION
<P>
Assume that no subterm (of t) representing a formula is a
term of type VARIABLE.
*/

/* PUBLIC */
Formula term_to_formula(Term t)
{
  Formula f;
  Ftype type;
  char *str = sn_to_str(SYMNUM(t));

  if (ARITY(t) == 1 && str_ident(str, QUANT_SYM)) {
    /* example: [all,x,exists,y,all,z,form] */
    Term l = t->args[0];
    int n = listterm_length(l);
    if (n % 2 != 1) {
      fatal_error("term_to_formula, bad quantified formula.");
    }
    f = term_to_formula(listterm_i(l, n));
    for (n = n-2; n > 0; n = n - 2) {
      Term quant = listterm_i(l, n);
      Term var   = listterm_i(l, n+1);
      Formula g;
      Ftype qtype;
      qtype = (is_symbol(SYMNUM(quant),ALL_SYM,0) ? ALL_FORM : EXISTS_FORM);
      g = formula_get(1, qtype);
      g->kids[0] = f;
      g->qvar = sn_to_str(SYMNUM(var));
      f = g;
    }
  }
  else {
    if (ARITY(t) == 0 && str_ident(str, TRUE_SYM))
      type = AND_FORM;
    else if (ARITY(t) == 0 && str_ident(str, FALSE_SYM))
      type = OR_FORM;
    else if (ARITY(t) == 1 && str_ident(str, NOT_SYM))
      type = NOT_FORM;
    else if (ARITY(t) == 2 && str_ident(str, AND_SYM))
      type = AND_FORM;
    else if (ARITY(t) == 2 && str_ident(str, OR_SYM))
      type = OR_FORM;
    else if (ARITY(t) == 2 && str_ident(str, IFF_SYM))
      type = IFF_FORM;
    else if (ARITY(t) == 2 && str_ident(str, IMP_SYM))
      type = IMP_FORM;
    else if (ARITY(t) == 2 && str_ident(str, IMPBY_SYM))
      type = IMPBY_FORM;
    else
      type = ATOM_FORM;

    if (type == ATOM_FORM) {
      f = formula_get(0, ATOM_FORM);
      f->atom = copy_term(t);
    }
    else if (type == NOT_FORM) {
      f = formula_get(1, NOT_FORM);
      f->kids[0] = term_to_formula(t->args[0]);
    }
    else if (ARITY(t) == 0) {
      f = formula_get(0, type);
    }
    else {
      f = formula_get(2, type);
      f->kids[0] = term_to_formula(t->args[0]);
      f->kids[1] = term_to_formula(t->args[1]);
    }
  }
  return flatten_top(f);
}  /* term_to_formula */

/*************
 *
 *   formula_to_term()
 *
 *************/

/* DOCUMENTATION
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
    t = get_rigid_term(NOT_SYM, 1);
    t->args[0] = formula_to_term(f->kids[0]);
    break;
  case IFF_FORM:
    t = get_rigid_term(IFF_SYM, 2);
    t->args[0] = formula_to_term(f->kids[0]);
    t->args[1] = formula_to_term(f->kids[1]);
    break;
  case IMP_FORM:
    t = get_rigid_term(IMP_SYM, 2);
    t->args[0] = formula_to_term(f->kids[0]);
    t->args[1] = formula_to_term(f->kids[1]);
    break;
  case IMPBY_FORM:
    t = get_rigid_term(IMPBY_SYM, 2);
    t->args[0] = formula_to_term(f->kids[0]);
    t->args[1] = formula_to_term(f->kids[1]);
    break;
  case AND_FORM:
  case OR_FORM:
    if (f->arity == 0)
      t = get_rigid_term(f->type == AND_FORM ? TRUE_SYM : FALSE_SYM, 0);
    else {
      int i = f->arity-1;
      t = formula_to_term(f->kids[i]);
      for (i--; i >= 0; i--) {
	Term t1 = get_rigid_term(f->type == AND_FORM ? AND_SYM : OR_SYM, 2);
	t1->args[0] = formula_to_term(f->kids[i]);
	t1->args[1] = t;
	t = t1;
      }
    }
    break;
  case ALL_FORM:
  case EXISTS_FORM:
    {
      Term l = get_nil_term();
      Formula g;
      for (g = f; quant_form(g); g = g->kids[0]) {
	Term quant, var;
	quant = get_rigid_term(g->type == ALL_FORM ? ALL_SYM : EXISTS_SYM, 0);
	var = get_rigid_term(g->qvar, 0);
	l = listterm_append(l, quant);
	l = listterm_append(l, var);
      }
      l = listterm_append(l, formula_to_term(g));
      t = get_rigid_term(QUANT_SYM, 1);
      t->args[0] = l;
    }
    break;
  }
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
    // fprintf(fp, "(");
    fprint_term(fp, f->atom);
    // fprintf(fp, ")");
  }
  else if (f->type == NOT_FORM) {
    // fprintf(fp, "(%s ", NOT_SYM);
    fprintf(fp, "%s", NOT_SYM);
    fprint_formula(fp, f->kids[0]);
    // fprintf(fp, ")");
  }
  else if (f->type == IFF_FORM) {
    fprintf(fp, "(");
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, " %s ", IFF_SYM);
    fprint_formula(fp, f->kids[1]);
    fprintf(fp, ")");
  }
  else if (f->type == IMP_FORM) {
    fprintf(fp, "(");
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, " %s ", IMP_SYM);
    fprint_formula(fp, f->kids[1]);
    fprintf(fp, ")");
  }
  else if (f->type == IMPBY_FORM) {
    fprintf(fp, "(");
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, " %s ", IMPBY_SYM);
    fprint_formula(fp, f->kids[1]);
    fprintf(fp, ")");
  }
  else if (quant_form(f)) {
    fprintf(fp, "(%s %s ", f->type==ALL_FORM ? ALL_SYM : EXISTS_SYM, f->qvar);
    fprint_formula(fp, f->kids[0]);
    fprintf(fp, ")");
  }
  else if (f->type == AND_FORM || f->type == OR_FORM) {
    if (f->arity == 0)
      fprintf(fp, "%s", f->type == AND_FORM ? TRUE_SYM : FALSE_SYM);
    else {
      int i;
      fprintf(fp, "(");
      for (i = 0; i < f->arity; i++) {
	fprint_formula(fp, f->kids[i]);
	if (i < f->arity-1)
	  fprintf(fp, " %s ", f->type == AND_FORM ? AND_SYM : OR_SYM);
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
    return 0;
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

static
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
Change a formula into its dual.
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

static
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

static
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

static
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

static
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
Do not refer to the given formula after the call; a good way to call
this routine is <TT>f = nnf2(f, CONJUNCTION)</TT>.
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
Do not refer to the given formula after the call; a good way to call
this routine is <TT>f = nnf(f)</TT>.
*/

/* PUBLIC */
Formula nnf(Formula f)
{
  return nnf2(f, CONJUNCTION);
}  /* nnf */

/*************
 *
 *   complementary()
 *
 *************/

static
BOOL complementary(Formula a, Formula b)
{
  return
    (a->type == NOT_FORM && formula_ident(a->kids[0], b))
    ||
    (b->type == NOT_FORM && formula_ident(a, b->kids[0]));
}  /* complementary */

/*************
 *
 *   contains_complements()
 *
 *   Assume AND_FORM or OR_FORM.
 *
 *************/

static
BOOL contains_complements(Formula f)
{
  int i, j;
  for (i = 0; i < f->arity-1; i++) {
    for (j = i+1; j < f->arity; j++) {
      if (complementary(f->kids[i], f->kids[j]))
	return TRUE;
    }
  }
  return FALSE;
}  /* contains_complements */

/*************************************************************************/

/*************
 *
 *   prop_member()
 *
 *************/

static
BOOL prop_member(Formula f, Formula g)
{
  int i;
  for (i = 0; i < g->arity; i++)
    if (formula_ident(f, g->kids[i]))
      return TRUE;
  return FALSE;
}  /* prop_member */

/*************
 *
 *   prop_subset()
 *
 *************/

static
BOOL prop_subset(Formula f, Formula g)
{
  int i;
  for (i = 0; i < f->arity; i++)
    if (!prop_member(f->kids[i], g))
      return FALSE;
  return TRUE;
}  /* prop_subset */

/*************
 *
 *   prop_subsume()
 *
 *   Assume disjunctions, atomic, TRUE, or FALSE
 *
 *************/

static
BOOL prop_subsume(Formula f, Formula g)
{
  if (FALSE_FORMULA(f))
    return TRUE;
  else if  (TRUE_FORMULA(g))
    return TRUE;
  else if (g->type == OR_FORM) {
    if (f->type == OR_FORM)
      return prop_subset(f, g);
    else
      return prop_member(f, g);
  }
    return formula_ident(f, g);
}  /* prop_subsume */

/*************
 *
 *   remove_subsumed()
 *
 *   Assume flat conjunction.  Always return conjunction.
 *
 *************/

static
Formula remove_subsumed(Formula f)
{
  if (f->type != AND_FORM)
    return f;
  else {
    Formula h;
    int new_arity = f->arity;
    int i, j;
    for (i = 0; i < f->arity; i++) {
      for (j = i+1; j < f->arity; j++) {
	if (f->kids[i] && f->kids[j] &&
	    prop_subsume(f->kids[i], f->kids[j])) {
	  zap_formula(f->kids[j]);
	  f->kids[j] = NULL;
	  new_arity--;
	}
	else if (f->kids[i] && f->kids[j] &&
		 prop_subsume(f->kids[j], f->kids[i])) {
	  zap_formula(f->kids[i]);
	  f->kids[i] = NULL;
	  new_arity--;
	}
      }
    }
    h = formula_get(new_arity, AND_FORM);
    j = 0;
    for (i = 0; i < f->arity; i++) {
      if (f->kids[i])
	h->kids[j++] = f->kids[i];
    }
    free_formula(f);
    return h;
  }
}  /* remove_subsumed */

/*************
 *
 *   make_conjunction()
 *
 *************/

static
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

static
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
 *   disjoin_flatten_simplify(a, b)   a OR b
 *
 *   Remove duplicates; if it contains complements, return TRUE.
 *
 *************/

static
Formula disjoin_flatten_simplify(Formula a, Formula b)
{
  Formula c;
  int new_arity, i, j;
  a = make_disjunction(a);
  b = make_disjunction(b);
  new_arity = a->arity + b->arity;
  for (i = 0; i < a->arity; i++) {
    for (j = 0; j < b->arity; j++) {
      if (b->kids[j] != NULL) {
	if (complementary(a->kids[i], b->kids[j])) {
	  zap_formula(a);
	  zap_formula(b);  /* this can handle NULL kids */
	  return formula_get(0, AND_FORM);  /* TRUE formula */
	}
	else if (formula_ident(a->kids[i], b->kids[j])) {
	  /* Note that this makes b non-well-formed. */
	  zap_formula(b->kids[j]);  /* really FALSE */
	  b->kids[j] = NULL;
	  new_arity--;
	}
      }
    }
  }
  c = formula_get(new_arity, OR_FORM);
  j = 0;
  for (i = 0; i < a->arity; i++)
    c->kids[j++] = a->kids[i];
  for (i = 0; i < b->arity; i++)
    if (b->kids[i] != NULL)
      c->kids[j++] = b->kids[i];
  free_formula(a);
  free_formula(b);
  return c;
}  /* disjoin_flatten_simplify */

/*************
 *
 *   simplify_and()
 *
 *   Assume flattened conjunction, and all kids are simplified flat
 *   disjunctions (or atomic, TRUE, FALSE).
 *   
 *
 *************/

static
Formula simplify_and(Formula f)
{
  if (f->type != AND_FORM)
    return f;
  else {
    f = remove_subsumed(f);  /* still AND */
    if (f->arity == 1) {
      Formula g = f->kids[0];
      free_formula(f);
      return g;
    }
    else if (contains_complements(f)) {
      zap_formula(f);
      return formula_get(0, OR_FORM);  /* FALSE */
    }
    else
      return f;
  }
}  /* simplify_and */

/*************
 *
 *   distribute_top()
 *
 *   Assume it's a binary disjunction.
 *
 *************/

static
Formula distribute_top(Formula h)
{
  Formula f = h->kids[0];
  Formula g = h->kids[1];
  int arity, new_arity, i, j, k;
  Formula a;
  free_formula(h);
  /* If not conjunctions, make them so. */
  f = make_conjunction(f);
  g = make_conjunction(g);

  /* printf("DT: %5d x %5d\n", f->arity, g->arity); fflush(stdout); */

  arity = f->arity * g->arity;
  new_arity = arity;
  a = formula_get(arity, AND_FORM);
  k = 0;
  for (i = 0; i < f->arity; i++) {
    for (j = 0; j < g->arity; j++) {
      Formula fi = formula_copy(f->kids[i]);
      Formula gj = formula_copy(g->kids[j]);
      a->kids[k++] = disjoin_flatten_simplify(fi, gj);
    }
  }
  zap_formula(f);
  zap_formula(g);
  a = simplify_and(a);
  return a;
}  /* distribute_top */

/*************
 *
 *   distribute()
 *
 *************/

static
Formula distribute(Formula f)
{
  if (f->type != OR_FORM)
    return f;
  else {
    if (f->arity != 2)
      fatal_error("distribute: not binary");
    f->kids[0] = distribute(f->kids[0]);
    f->kids[1] = distribute(f->kids[1]);
    f = distribute_top(f);
    return f;
  }
}  /* distribute */

/*************
 *
 *   rbt()
 *
 *   Take a flat OR or AND, and make it into a
 *   right-associated binary tree.
 *
 *************/

static
Formula rbt(Formula f)
{
  if (f->type != AND_FORM && f->type != OR_FORM) {
    fatal_error("rbt: not AND or OR");
    return NULL;  /* to please the compiler (won't happen) */
  }
  else if (f->arity == 1) {
    Formula g = f->kids[0];
    free_formula(f);
    return g;
  }
  else if (f->arity == 2)
    return f;
  else {
    int arity = f->arity;
    int i;
    Formula a = f->kids[arity-1];  /* working formula */
    for (i = arity-2; i >= 0; i--) {
      Formula b = formula_get(2, f->type);
      b->kids[0] = f->kids[i];
      b->kids[1] = a;
      a = b;
    }
    free_formula(f);
    return a;
  }
}  /* rbt */

/*************
 *
 *   cnf()
 *
 *   Assume NNF and flattened.
 *
 *   This does not go below quantifiers; that is,
 *   quantified formulas are treated as atomic.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula cnf(Formula f)
{
  if (f->type != AND_FORM && f->type != OR_FORM)
    return f;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = cnf(f->kids[i]);
    
    if (f->type == AND_FORM) {
      f = flatten_top(f);
      f = simplify_and(f);
      return f;
    }
    else {  /* OR_FORM */
      /* printf("\nkids done: "); p_formula(f); */
      f = dual(remove_subsumed(dual(f)));
      /* printf("simplify:  "); p_formula(f); */

      f = rbt(f);  /* make the top OR-tree binary */

      f = distribute(f);
      /* printf("\ncnf:       "); p_formula(f); */
      return f;
    }
  }
}  /* cnf */

/*************
 *
 *   formula_test()
 *
 *************/

int formula_test(Term t)
{
  Formula f = term_to_formula(t);

  printf("\nform: "); p_formula(f);

  f = nnf(f);
  printf("\nnnf : "); p_formula(f);

  f = skolemize(f);
  printf("\nskol: "); p_formula(f);

  f = unique_quantified_vars(f);
  printf("\nunqu: "); p_formula(f);

  f = remove_universal_quantifiers(f);
  printf("\nrmqu: "); p_formula(f);

  f = formula_flatten(f);
  printf("\nflat: "); p_formula(f);

  f = cnf(f);
  printf("\ncnf : "); p_formula(f);

  zap_formula(f);

  p_formula_mem();

  return 0;
}  /* formula_test */

/*************************************************************************/

/*************
 *
 *   dnf()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula dnf(Formula f)
{
  return dual(cnf(dual(f)));
}  /* dnf */

/*************
 *
 *   subst_free_var()
 *
 *************/

static
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
 *   skolem()
 *
 *************/

static
Formula skolem(Formula f, Term uvars)
{
  if (f->type == ATOM_FORM || f->type == NOT_FORM)
    return f;
  else if (f->type == ALL_FORM) {
    Term var = get_rigid_term(f->qvar, 0);
    Term lst;
    if (listterm_member(var, uvars)) {
      /* We are in the scope of another variable with this name, so
       * rename this variable.
       */
      int sn = fresh_symbol("x", 0);
      Term newvar = get_rigid_term(sn_to_str(sn), 0);
      subst_free_var(f->kids[0], var, newvar);
      f->qvar = sn_to_str(sn);
      free_term(var);
      var = newvar;
    }

    lst = listterm_cons(var, uvars);
    f->kids[0] = skolem(f->kids[0], lst);
    free_term(var);
    free_term(lst);
    return f;
  }
  else if (f->type == EXISTS_FORM) {
    Formula g;
    int n = listterm_length(uvars);
    int sn = next_skolem_symbol(n);
    Term sk = get_rigid_term(sn_to_str(sn), n);
    Term evar = get_rigid_term(f->qvar, 0);
    int i;

    for (i = 0; i < n; i++)
      sk->args[i] = copy_term(listterm_i(uvars, n-i)); /* uvars is backward */

    subst_free_var(f->kids[0], evar, sk);

    zap_term(sk);
    zap_term(evar);

    g = skolem(f->kids[0], uvars);
    free_formula(f);
    return g;
  }
  else if (f->type == AND_FORM || f->type == OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++) {
      f->kids[i] = skolem(f->kids[i], uvars);
    }
    return f;
  }
  else {
    /* Not in NNF!  Let the caller beware! */
    return f;
  }
}  /* skolem */

/*************
 *
 *   skolemize()
 *
 *************/

/* DOCUMENTATION
This routine Skolemizes an NNF formula.
The quantified variables need not be named in any particular way.
If there are universally quantified variables with the same name,
one in the scope of another, the inner variable will be renamed.
(Existential nodes are removed.)
*/

/* PUBLIC */
Formula skolemize(Formula f)
{
  Term uvars = get_nil_term();
  f = skolem(f, uvars);
  free_term(uvars);
  return f;
}  /* skolemize */

/*************
 *
 *   unique_qvars()
 *
 *************/

static
Term unique_qvars(Formula f, Term vars)
{
  if (f->type == ATOM_FORM)
    return vars;
  else if (quant_form(f)) {
    Term var = get_rigid_term(f->qvar, 0);
    Term lst;
    if (listterm_member(var, vars)) {
      /* Rename this variable. */
      int sn = fresh_symbol("x", 0);
      Term newvar = get_rigid_term(sn_to_str(sn), 0);
      subst_free_var(f->kids[0], var, newvar);
      f->qvar = sn_to_str(sn);
      free_term(var);
      var = newvar;
    }
    lst = listterm_cons(var, vars);
    return unique_qvars(f->kids[0], lst);
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++) {
      vars = unique_qvars(f->kids[i], vars);
    }
    return vars;
  }
}  /* unique_qvars */

/*************
 *
 *   unique_quantified_vars()
 *
 *************/

/* DOCUMENTATION
Rename quantified variables, if necessary, so that each is unique.
This works for any formula.
<P>
If you wish to rename a quantified variable only if it occurs in
the scope of of a quantified variable with the same name, you
can use the routine eliminate_rebinding() instead.
<P>
(This could be a void routine, because
none of the formula nodes is changed; I made it return the Formula
so that it is consistent with its friends.)
*/

/* PUBLIC */
Formula unique_quantified_vars(Formula f)
{
  Term uvars = unique_qvars(f, get_nil_term());
  zap_term(uvars);
  return f;
}  /* unique_quantified_vars */

/*************
 *
 *   mark_free_vars_formula()
 *
 *************/

/* Replace all free occurrences of CONSTANT *varname with
 * a VARIABLE of index varnum.
 */

static
void mark_free_vars_formula(Formula f, char *varname, int varnum)
{
  if (f->type == ATOM_FORM)
    f->atom = subst_var_term(f->atom, str_to_sn(varname, 0), varnum);
  else if (quant_form(f) && str_ident(f->qvar, varname))
    return;
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      mark_free_vars_formula(f->kids[i], varname, varnum);
  }
}  /* mark_free_vars_formula */

/*************
 *
 *   remove_uni_quant()
 *
 *************/

static
Formula remove_uni_quant(Formula f, int *varnum_ptr)
{
  if (f->type == AND_FORM || f->type == OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = remove_uni_quant(f->kids[i], varnum_ptr);
    return f;
  }
  else if (f->type == ALL_FORM) {
    Formula g = f->kids[0];
    mark_free_vars_formula(g, f->qvar, *varnum_ptr);
    *varnum_ptr += 1;
    free_formula(f);
    return remove_uni_quant(g, varnum_ptr);
  }
  else {
    /* If not ATOM_FORM, something's probably wrong,
     * but let the caller beware!
     */
    return f;
  }
}  /* remove_uni_quant */

/*************
 *
 *   remove_universal_quantifiers()
 *
 *************/

/* DOCUMENTATION
For each universally quantified variable in the given formula,
*/

/* PUBLIC */
Formula remove_universal_quantifiers(Formula f)
{
  int varnum = 0;
  return remove_uni_quant(f, &varnum);
}  /* remove_universal_quantifiers */

/*************
 *
 *   clausify_prepare()
 *
 *************/

/* DOCUMENTATION
This routine gets a formula all ready for translation into clauses.
The sequence of transformations is
<UL>
<LI> change to negation normal form;
<LI> propositional simplification;
<LI> skolemize (nothing fancy here);
<LI> make the universally quantified variables unique;
<LI> remove universal quantifiers, changing the
     constants-which-represent-variables into genuine variables;
<LI> change to conjunctive normal form
     (with basic propositional simplification).
</UL>
The caller should not refer to the given formula f after the call;
A good way to call is <TT>f = clausify_prepare(f)</TT>
*/

/* PUBLIC */
Formula clausify_prepare(Formula f)
{
  f = nnf(f);
  f = skolemize(f);
  f = unique_quantified_vars(f);
  f = remove_universal_quantifiers(f);
  f = formula_flatten(f);
  f = cnf(f);
  return f;
}  /* clausify_prepare */

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
 *   elim_rebind()
 *
 *************/

static
Formula elim_rebind(Formula f, Term uvars)
{
  if (quant_form(f)) {
    Term var = get_rigid_term(f->qvar, 0);
    Term lst;
  
    if (listterm_member(var, uvars)) {
      /* We are in the scope of another variable with this name, so
       * rename this variable.
       */
      int sn = fresh_symbol("y", 0);
      Term newvar = get_rigid_term(sn_to_str(sn), 0);
      subst_free_var(f->kids[0], var, newvar);
      f->qvar = sn_to_str(sn);
      free_term(var);
      var = newvar;
    }

    lst = listterm_cons(var, uvars);
    f->kids[0] = elim_rebind(f->kids[0], lst);
    free_term(var);
    free_term(lst);
    return f;
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++) {
      f->kids[i] = elim_rebind(f->kids[i], uvars);
    }
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
(This could be a void routine, because
none of the formula nodes is changed; I made it return the Formula
so that it is consistent with its friends.)
*/

/* PUBLIC */
Formula eliminate_rebinding(Formula f)
{
  Term uvars = get_nil_term();
  f = elim_rebind(f, uvars);
  free_term(uvars);
  return f;
}  /* eliminate_rebinding */

/*************
 *
 *   free_vars_term()
 *
 *************/

static
Plist free_vars_term(Term t, Plist vars)
{
  if (VARIABLE(t))
    fatal_error("free_vars_term, VARIABLE term");

  if (ARITY(t) == 0) {
    char *name = sn_to_str(SYMNUM(t));
    if (variable_name(name)) {
      Term var = get_rigid_term(name, 0);
      if (!tlist_member(var, vars))
	vars = plist_append(vars, var);
    }
    return vars;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++) {
      vars = free_vars_term(ARG(t,i), vars);
    }
    return vars;
  }
}  /* free_vars_term */

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
 *   get_quant_form()
 *
 *************/

static
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
*/

/* PUBLIC */
Formula universal_closure(Formula f)
{
  Plist vars = free_vars(f, NULL);
  f = uni_close(f, vars);
  zap_tlist(vars);
  return f;
}  /* universal_closure */

