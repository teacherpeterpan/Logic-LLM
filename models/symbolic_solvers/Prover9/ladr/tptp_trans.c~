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

#include "tptp_trans.h"

/* Private definitions and types */

/*

  From Geoff Sutcliffe, Feb 13, 2008:

  %----These are used in the TPTP and need to exist before the
  %----transformations and formats are loaded. They are also declared at
  %----runtime in tptp2X/4.
  :-op(99,fx,'$').
  :-op(100,fx,++).
  :-op(100,fx,--).
  %----Postfix for !=
  :-op(100,xf,'!').
  %---- .. used for range in tptp2X. Needs to be stronger than :
  :-op(400,xfx,'..').
  %----! and ? are of higher precedence than : so ![X]:p(X) is :(![X],p(X))
  %----Otherwise ![X]:![Y]:p(X,Y) cannot be parsed.
  %----! is fy so Prolog can read !! (blah) as !(!(blah)) and it gets fixed
  :-op(400,fy,!).
  :-op(400,fx,?).
  :-op(400,fx,^).
  :-op(400,fx,:=).
  :-op(400,fx,'!!').
  :-op(400,fx,'??').
  %----= must bind more tightly than : for ! [X] : a = X. = must binder looser
  %----than quantifiers for otherwise X = ! [Y] : ... is a syntax error (the =
  %----grabs the quantifier). That means for thf it is necessary to bracket
  %----formula terms, e.g., a = (! [X] : p(X))
  :-op(405,xfx,'=').
  %---!= not possible because ! is special - see postfix cheat :-op(405,xfx,'!=').
  :-op(440,xfy,>).     %----Type arrow
  %----Need : stronger than binary connectives for ! [X] : p(X) <=> !Y ...
  %----Need ~ and : equal and right-assoc for ![X] : ~p and for ~![X] : ...
  :-op(450,xfy,:).
  :-op(450,fy,~).
  :-op(501,yfx,@).
  :-op(502,xfy,'|').
  :-op(502,xfy,'~|').
  :-op(502,xfy,'+').   %----Fision for rfof
  :-op(503,xfy,&).
  :-op(503,xfy,~&).
  :-op(503,xfy,'*').   %----Fusion for rfof
  :-op(504,xfy,=>).
  :-op(504,xfy,<=).
  :-op(505,xfy,<=>).
  :-op(505,xfy,<~>).
  %----Must be weak to allow any formulae on RHS
  :-op(550,xfy,:=).

  --------------------------------------------------------

  Correction from Geoff, May 14, 2008:

    op(502,xfx,'~|'),
    op(503,xfx,~&),
    op(504,xfx,=>),
    op(504,xfx,<=),
    op(505,xfx,<=>),
    op(505,xfx,<~>),
    op(550,xfx,:=).

  --------------------------------------------------------

  From Geoff, October 6, 2008:

  I would be quite happy if both [input and output] were equally strict,
  conforming to the
  TPTP BNF. However, your other users might be happier if the input were
  a little more forgiving, and that's why my Prolog operator defs are
  like they are - more forgiving than the BNF.
 
  I guess you have the following at the moment, which are the "forgiving"
  operator defs ...

    op(400,fy,!),
    op(400,fx,?),
    op(405,xfx,'='),
    op(450,xfy,:),
    op(450,fy,~),
    op(502,xfy,'|'),
    op(502,xfx,'~|'),
    op(503,xfy,&),
    op(503,xfx,~&),
    op(504,xfx,=>),
    op(504,xfx,<=),
    op(505,xfx,<=>),
    op(505,xfx,<~>),

  Indeed, I solved the current issue by reading your output with my Prolog
  tool, whose parsing uses those defs, but whose output routines are hand 
  rolled to output in strict TPTP format. For output, or generally strict, 
  you can set all the binary connectives to the same precedence, e.g., ...

    op(502,xfy,'|'),
    op(502,xfx,'~|'),
    op(502,xfy,&),
    op(502,xfx,~&),
    op(502,xfx,=>),
    op(502,xfx,<=),
    op(502,xfx,<=>),
    op(502,xfx,<~>),

  Note the | and & are xfy, which allows, e.g., (a | b | c | d).

  --------------------------------------------------------

  WWM: Ok, I'll separate the types for parsing TPTP and printing TPTP.
  Parsing TPTP will be flexible, pretty much as before.  Printing
  will be conservative, using Geoff's second set of op defs above,
  with the exception that & and | will be INFIX instead of INFIX_RIGHT,
  (because LADR does not allow different parse types with same precedence).
  So we'll print, e.g.,  (a & (b & c)) => (d | (e | f)), whereas
  the TPTP string a & b & c => d | e | f wil be parsed correctly as the
  same term.

  Also, for ladr_to_tptp, I'll add an command-line option -p that will
  cause ALL expressions to be parenthesized.

*/

/*************
 *
 *   declare_tptp_input_types()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_tptp_input_types(void)
{

  check_for_illegal_symbols(FALSE);  /* get around the "--" problem */

  /* We do not support all of the TPTP operations listed above. */

  declare_parse_type(",",   999, INFIX_RIGHT);     /* LADR requirement */

  declare_parse_type("++",  100, PREFIX_PAREN);
  declare_parse_type("--",  100, PREFIX_PAREN);

  declare_parse_type("!",   400, PREFIX_PAREN);
  declare_parse_type("?",   400, PREFIX_PAREN);

  declare_parse_type("!=",  405, INFIX);           /* added by McCune */
  declare_parse_type("=",   405, INFIX);

  declare_parse_type("~",   410, PREFIX);          /* changed by McCune */

  declare_parse_type(":",   450, INFIX_RIGHT);

  declare_parse_type("|",   502, INFIX_RIGHT);
  declare_parse_type("&",   503, INFIX_RIGHT);

  declare_parse_type("=>",  504, INFIX);
  declare_parse_type("<=",  504, INFIX);
  declare_parse_type("<=>", 505, INFIX);
  declare_parse_type("<~>", 505, INFIX);

  /* Other things */

  set_cons_char('\0');  /* Don't recognize list cons */

}  /* declare_tptp_input_types */

/*************
 *
 *   declare_tptp_output_types()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_tptp_output_types(void)
{

  check_for_illegal_symbols(FALSE);  /* get around the "--" problem */

  /* We do not support all of the TPTP operations listed above. */

  declare_parse_type(",",   999, INFIX_RIGHT);     /* LADR requirement */

  declare_parse_type("++",  100, PREFIX_PAREN);
  declare_parse_type("--",  100, PREFIX_PAREN);

  declare_parse_type("!",   400, PREFIX_PAREN);
  declare_parse_type("?",   400, PREFIX_PAREN);

  declare_parse_type("!=",  405, INFIX);           /* added by McCune */
  declare_parse_type("=",   405, INFIX);

  declare_parse_type("~",   410, PREFIX);          /* changed by McCune */

  declare_parse_type(":",   450, INFIX_RIGHT);

  declare_parse_type("|",   502, INFIX);  /* NOTE: not INFIX_RIGHT (xfy) */
  declare_parse_type("&",   502, INFIX);  /* NOTE: not INFIX_RIGHT (xfy) */

  declare_parse_type("=>",  502, INFIX);
  declare_parse_type("<=",  502, INFIX);
  declare_parse_type("<=>", 502, INFIX);
  declare_parse_type("<~>", 502, INFIX);

  /* Other things */

  set_cons_char('\0');  /* Don't recognize list cons */

}  /* declare_tptp_output_types */

/*****************************************************************************

This first part is for the TPTP to LADR transformation

*****************************************************************************/

/*************
 *
 *   tptp2_to_ladr_term()
 *
 *   Take a TPTP "input_clause" transform it to a LADR formula
 *   (in term form).  The input formula is used up in the transformation.
 *
 *************/

static
Term tptp2_to_ladr_term(Term t)
{
  Term f = NULL;
  if (!proper_listterm(t)) {
    p_term(t);
    fatal_error("tptp2_to_ladr_term, expected list of literals");
  }

  t = listterm_reverse(t);  /* so the result is in the same order */

  while (cons_term(t)) {
    Term a0 = ARG(t,0);
    Term lit = NULL;

    if (is_term(a0, "++", 1) || is_term(a0, "--", 1)) {
      Term atom = ARG(a0, 0);
      if (is_term(atom, "equal", 2)) {
	Term tmp = build_binary_term_safe(eq_sym(), ARG(atom,0), ARG(atom,1));
	free_term(atom);  /* shallow */
	atom = tmp;
      }
      if (is_term(a0, "++", 1))
	lit = atom;
      else
	lit = build_unary_term_safe(not_sym(), atom);
    }
    else {
      p_term(a0);
      fatal_error("tptp2_to_ladr_term, unknown literal");
    }

    if (f == NULL)
      f = lit;
    else
      f = build_binary_term_safe(or_sym(), lit, f);

    t = ARG(t,1);
  }
  return f;
}  /* tptp2_to_ladr_term */

/*************
 *
 *   tptp3_to_ladr_term()
 *
 *   Take a TPTP "cnf" or "fof", and transform it to a LADR formula
 *   (in term form).  The input formula is used up in the transformation.
 *
 *************/

static
Term tptp3_to_ladr_term(Term t)
{
  if (is_term(t, "$true", 0)) {
    free_term(t);
    return get_rigid_term(true_sym(), 0);
  }
  else if (is_term(t, "$false", 0)) {
    free_term(t);
    return get_rigid_term(false_sym(), 0);
  }
  else if (is_term(t, "~", 1)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term t2 = build_unary_term_safe(not_sym(), a0);
    free_term(t);
    return t2;
  }
  else if (is_term(t, "|", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(or_sym(), a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, "&", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(and_sym(), a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, "=>", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(imp_sym(), a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, "<=", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(impby_sym(), a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, "<=>", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(iff_sym(), a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, "~&", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(and_sym(), a0, a1);
    Term t3 = build_unary_term_safe(not_sym(), t2);
    free_term(t);
    return t3;
  }
  else if (is_term(t, "~|", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(or_sym(), a0, a1);
    Term t3 = build_unary_term_safe(not_sym(), t2);
    free_term(t);
    return t3;
  }
  else if (is_term(t, "<~>", 2)) {
    Term a0 = tptp3_to_ladr_term(ARG(t,0));
    Term a1 = tptp3_to_ladr_term(ARG(t,1));
    Term t2 = build_binary_term_safe(iff_sym(), a0, a1);
    Term t3 = build_unary_term_safe(not_sym(), t2);
    free_term(t);
    return t3;
  }
  else if (is_term(t, ":", 2)) {
    /* Assume it's well formed (don't check for errors). */
    Term q = ARG(t,0);
    Term form = tptp3_to_ladr_term(ARG(t,1));
    Term vars = listterm_reverse(ARG(q,0));
    char *quant;

    if (!proper_listterm(vars))
      fatal_error("tptp_to_ladr: bad quantified variable(s)");

    quant = (is_term(q,"!",1) ? all_sym() : exists_sym());

    while (cons_term(vars)) {
      Term v = ARG(vars, 0);
      Term new = get_rigid_term(quant_sym(), 3);
      ARG(new,0) = get_rigid_term(quant, 0);
      ARG(new,1) = copy_term(v);
      ARG(new,2) = form;
      form = new;
      vars = ARG(vars, 1);
    }
    zap_term(vars);
    free_term(t);
    free_term(q);
    return form;
  }
  else
    return t;
}  /* tptp3_to_ladr_term */

/*************
 *
 *   tptp_input_to_ladr_formula()
 *
 *   The input Term t is not modified by the call.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Formula tptp_input_to_ladr_formula(Term t)
{
  char *type = sn_to_str(SYMNUM(t));

  Term name = ARG(t,0);
  Term role = ARG(t,1);
  Term form = NULL;
  Term a, b;
  Formula f;

  if (str_ident(type, "fof") || str_ident(type, "cnf"))
    form = tptp3_to_ladr_term(copy_term(ARG(t,2)));
  else if (str_ident(type, "input_clause"))
    form = tptp2_to_ladr_term(copy_term(ARG(t,2)));
  else {
    p_term(t);
    fatal_error("tptp_formlua: unknown type");
  }

  a = build_binary_term_safe(attrib_sym(),
			     build_unary_term_safe("label", copy_term(name)),
			     build_unary_term_safe("label", copy_term(role)));
  b = build_binary_term_safe(attrib_sym(), form, a);
  f = term_to_formula(b);
  zap_term(b);
  return f;
}  /* tptp_input_to_ladr_formula */

/*****************************************************************************

This first part is for the LADR to TPTP transformation

*****************************************************************************/

/*************
 *
 *   rename_vars_to_upper()
 *
 *************/

static
void rename_vars_to_upper(Formula f)
{
  int i;
  if (quant_form(f)) {
    Term var = get_rigid_term(f->qvar, 0);
    int sn = fresh_symbol("X", 0);
    Term newvar = get_rigid_term(sn_to_str(sn), 0);
    subst_free_var(f->kids[0], var, newvar);
    f->qvar = sn_to_str(sn);
    free_term(var);
    free_term(newvar);
  }
  for (i = 0; i < f->arity; i++)
    rename_vars_to_upper(f->kids[i]);
}  /* rename_vars_to_upper */

/*************
 *
 *   ladr_term_to_tptp_term()
 *
 *************/

static
Term ladr_term_to_tptp_term(Term t)
{
  if (is_term(t, true_sym(), 0)) {
    free_term(t);
    return get_rigid_term("$true", 0);
  }
  else if (is_term(t, false_sym(), 0)) {
    free_term(t);
    return get_rigid_term("$false", 0);
  }
  else if (is_term(t, not_sym(), 1)) {
    Term a0 = ladr_term_to_tptp_term(ARG(t,0));
    Term t2 = build_unary_term_safe("~", a0);
    free_term(t);
    return t2;
  }
  else if (is_term(t, or_sym(), 2)) {
    Term a0 = ladr_term_to_tptp_term(ARG(t,0));
    Term a1 = ladr_term_to_tptp_term(ARG(t,1));
    Term t2 = build_binary_term_safe("|", a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, and_sym(), 2)) {
    Term a0 = ladr_term_to_tptp_term(ARG(t,0));
    Term a1 = ladr_term_to_tptp_term(ARG(t,1));
    Term t2 = build_binary_term_safe("&", a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, imp_sym(), 2)) {
    Term a0 = ladr_term_to_tptp_term(ARG(t,0));
    Term a1 = ladr_term_to_tptp_term(ARG(t,1));
    Term t2 = build_binary_term_safe("=>", a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, impby_sym(), 2)) {
    Term a0 = ladr_term_to_tptp_term(ARG(t,0));
    Term a1 = ladr_term_to_tptp_term(ARG(t,1));
    Term t2 = build_binary_term_safe("<=", a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, iff_sym(), 2)) {
    Term a0 = ladr_term_to_tptp_term(ARG(t,0));
    Term a1 = ladr_term_to_tptp_term(ARG(t,1));
    Term t2 = build_binary_term_safe("<=>", a0, a1);
    free_term(t);
    return t2;
  }
  else if (is_term(t, quant_sym(), 3)) {
    Term quant = ARG(t,0);
    Term var   = ARG(t,1);
    Term form  = ladr_term_to_tptp_term(ARG(t,2));

    char *q = is_term(quant, "all", 0) ? "!" : "?";

    /* build the term :(quant([var]),form) */

    Term vlist = listterm_cons(var, get_nil_term());
    Term qterm = build_unary_term_safe(q, vlist);
    Term top   = build_binary_term_safe(":", qterm, form);

    free_term(quant);  /* shallow */

    return top;
  }
  else
    return t;
}  /* ladr_term_to_tptp_term */

/*************
 *
 *   ladr_list_to_tptp_list()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist ladr_list_to_tptp_list(Plist in, char *name, char *type)
{
  BOOL goal = str_ident(type, "conjecture");  /* goals always become fof */
  Plist out = NULL;
  Plist p;
  for (p = in; p; p = p->next) {
    Formula f = p->v;
    f->attributes = NULL;  /* always remove all attributes */
    if (clausal_formula(f) && !goal) {
      Term t, cnf;
      Topform c = formula_to_clause(f);
      zap_formula(f);
      clause_set_variables(c, MAX_VARS);
      t = ladr_term_to_tptp_term(topform_to_term(c));
      zap_topform(c);
      cnf = get_rigid_term("cnf", 3);
      ARG(cnf,0) = get_rigid_term(name, 0);
      ARG(cnf,1) = get_rigid_term(type, 0);
      ARG(cnf,2) = t;
      out = plist_prepend(out, cnf);
    }
    else {
      /* f is a non-clausal formula */
      Term t, fof;
      f = universal_closure(f);
      rename_vars_to_upper(f);

      t = ladr_term_to_tptp_term(formula_to_term(f));

      fof = get_rigid_term("fof", 3);
      ARG(fof,0) = get_rigid_term(name, 0);
      ARG(fof,1) = get_rigid_term(type, 0);
      ARG(fof,2) = t;
      out = plist_prepend(out, fof);
    }
  }
  return reverse_plist(out);
}  /* ladr_list_to_tptp_list */

/*************
 *
 *   syms_in_form()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ilist syms_in_form(Term t, BOOL clausal)
{
  if (clausal) {
    if (is_term(t, "~", 1))
      return syms_in_form(ARG(t,0), TRUE);
    else if (is_term(t,  "|", 2)) {
      Ilist a = syms_in_form(ARG(t,0), TRUE);
      Ilist b = syms_in_form(ARG(t,1), TRUE);
      return ilist_union(a,b);
    }
    else if (is_term(t, "=", 2)) {
      Ilist a = syms_in_form(ARG(t,0), TRUE);
      Ilist b = syms_in_form(ARG(t,1), TRUE);
      return ilist_union(a,b);
    }
    else
      return fsym_set_in_term(t);
  }
  else {
    /* non-clausal */
    if (is_term(t, "~", 1))
      return syms_in_form(ARG(t,0), FALSE);
    else if (is_term(t,  "|", 2) ||
	     is_term(t,  "&", 2) ||
	     is_term(t, "=>", 2) ||
	     is_term(t, "<=", 2) ||
	     is_term(t, "<=>", 2)) {
      Ilist a = syms_in_form(ARG(t,0), FALSE);
      Ilist b = syms_in_form(ARG(t,1), FALSE);
      return ilist_union(a,b);
    }
    else if (is_term(t, ":", 2)) {
      /* This assumes exactly one universally quantified variable! */
      Ilist a = syms_in_form(ARG(t,1), FALSE);
      a = ilist_removeall(a, SYMNUM(ARG(ARG(ARG(t,0),0),0)));
      return a;
    }
    else if (is_term(t, "=", 2)) {
      Ilist a = syms_in_form(ARG(t,0), FALSE);
      Ilist b = syms_in_form(ARG(t,1), FALSE);
      return ilist_union(a,b);
    }
    else
      return fsym_set_in_term(t);
  }
}  /* syms_in_form */

/*************
 *
 *   good_tptp_sym()
 *
 *************/

static
BOOL good_tptp_sym(char *s)
{
  /* [a-z][a-zA-Z0-9_]* */
  if (strlen(s) == 0)
    return FALSE;
  else {
    if (!(s[0] >= 'a' && s[0] <= 'z'))
      return FALSE;
    else {
      int i;
      for (i = 1; i < strlen(s); i++) {
	char c = s[i];
	if (!((c >= 'a' && c <= 'z') ||
	      (c >= 'A' && c <= 'Z') ||
	      (c >= '0' && c <= '9') ||
	      c == '_'))
	  return FALSE;
      }
      return TRUE;
    }
  }
}  /* good_tptp_sym */

/*************
 *
 *   good_ladr_sym()
 *
 *************/

static
BOOL good_ladr_sym(char *s)
{
  return s[0] != '\'';
}  /* good_ladr_sym */

/*************
 *
 *   map_for_bad_tptp_syms()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I2list map_for_bad_tptp_syms(Ilist syms, BOOL quote_bad_syms)
{
  if (syms == NULL)
    return NULL;
  else {
    I2list map = map_for_bad_tptp_syms(syms->next, quote_bad_syms);
    int old_sn = syms->i;
    char *s = sn_to_str(old_sn);
    if (!good_tptp_sym(s)) {
      int new_sn;
      if (quote_bad_syms) {
	char *escaped = escape_char(s, '\'');
	int n = strlen(escaped);
	char *new_str = malloc(n+3);
	new_str[0] = '\'';
	strcpy(new_str+1, escaped);
	new_str[n+1] = '\'';
	new_str[n+2] = '\0';
	new_sn = str_to_sn(new_str, sn_to_arity(old_sn));
	free(new_str);
	free(escaped);
      }
      else
	new_sn = fresh_symbol("tptp", sn_to_arity(old_sn));
      map = i2list_append(map, old_sn, new_sn);
    }
    return map;
  }
}  /* map_for_bad_tptp_syms */

/*************
 *
 *   map_for_bad_ladr_syms()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
I2list map_for_bad_ladr_syms(Ilist syms, BOOL quote_bad_syms)
{
  if (syms == NULL)
    return NULL;
  else {
    I2list map = map_for_bad_ladr_syms(syms->next, quote_bad_syms);
    int old_sn = syms->i;
    char *s = sn_to_str(old_sn);
    if (!good_ladr_sym(s)) {
      int new_sn;
      if (quote_bad_syms) {
	char *escaped = escape_char(s, '"');
	int n = strlen(escaped);
	char *new_str = malloc(n+3);
	new_str[0] = '"';
	strcpy(new_str+1, escaped);
	new_str[n+1] = '"';
	new_str[n+2] = '\0';
	new_sn = str_to_sn(new_str, sn_to_arity(old_sn));
	free(new_str);
	free(escaped);
      }
      else
	new_sn = fresh_symbol("ladr", sn_to_arity(old_sn));
      map = i2list_append(map, old_sn, new_sn);
    }
    return map;
  }
}  /* map_for_bad_ladr_syms */

/*************
 *
 *   replace_bad_syms_term()
 *
 *************/

/* DOCUMENTATION
The given term is used up.
*/

/* PUBLIC */
Term replace_bad_syms_term(Term t, I2list map)
{
  if (VARIABLE(t))
    return t;
  else {
    int i;
    int old_sn = SYMNUM(t);
    int new_sn = assoc(map, old_sn);
    if (new_sn != INT_MIN) {
      Term new = get_rigid_term(sn_to_str(new_sn), sn_to_arity(new_sn));
      for (i = 0; i < ARITY(t); i++)
	ARG(new,i) = ARG(t,i);
      free_term(t);  /* shallow */
      t = new;
    }
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = replace_bad_syms_term(ARG(t,i), map);
    return t;
  }
}  /* replace_bad_syms_term */

/*************
 *
 *   replace_bad_tptp_syms_form()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term replace_bad_tptp_syms_form(Term t, BOOL clausal, I2list map)
{
  if (is_term(t, "~", 1)) {
    ARG(t,0) = replace_bad_tptp_syms_form(ARG(t,0), clausal, map);
    return t;
  }
  else if (clausal && is_term(t, "|", 2)) {
    ARG(t,0) = replace_bad_tptp_syms_form(ARG(t,0), TRUE, map);
    ARG(t,1) = replace_bad_tptp_syms_form(ARG(t,1), TRUE, map);
    return t;
  }
  else if (!clausal && (is_term(t, "|", 2) ||
			is_term(t, "&", 2) ||
			is_term(t, "=>", 2) ||
			is_term(t, "<=", 2) ||
			is_term(t, "<=>", 2))) {
    ARG(t,0) = replace_bad_tptp_syms_form(ARG(t,0), FALSE, map);
    ARG(t,1) = replace_bad_tptp_syms_form(ARG(t,1), FALSE, map);
    return t;
  }
  else if (!clausal && is_term(t, ":", 2)) {
    /* We do not have to worry about quantified variables being
       replaced---they are disjoint from all other symbols, because
       they were generted by fresh_symbol().
     */
    ARG(t,1) = replace_bad_tptp_syms_form(ARG(t,1), FALSE, map);
    return t;
  }
  else
    return replace_bad_syms_term(t, map);
}  /* replace_bad_tptp_syms_form */


