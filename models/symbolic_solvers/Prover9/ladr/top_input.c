
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

#include "top_input.h"

#include "nonport.h"
#include "clausify.h"
#include "definitions.h"
#include <ctype.h>  /* for toupper, tolower */

/* Private definitions and types */

typedef struct readlist * Readlist;

struct readlist {
  char *name;      /* name, as it appears in the input file */
  int type;        /* FORMULAS, TERMS */
  BOOL auxiliary;
  Plist *p;        /* *pointer* to the Plist */
  Readlist next;
};

static Readlist Input_lists = NULL;

static Plist Lex_function_list = NULL;
static Plist Lex_predicate_list = NULL;
static Plist Skolem_list = NULL;  /* temp store of user-declared skolems */

static char *Program_name = "";

/*
 * memory management
 */

#define PTRS_READLIST PTRS(sizeof(struct readlist))
static unsigned Readlist_gets, Readlist_frees;

/*************
 *
 *   Readlist get_readlist()
 *
 *************/

static
Readlist get_readlist(void)
{
  Readlist p = get_cmem(PTRS_READLIST);
  Readlist_gets++;
  return(p);
}  /* get_readlist */

/*************
 *
 *    free_readlist()
 *
 *************/

#if 0
static
void free_readlist(Readlist p)
{
  free_mem(p, PTRS_READLIST);
  Readlist_frees++;
}  /* free_readlist */
#endif

/*************
 *
 *   fprint_top_input_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the top_input package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_top_input_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct readlist);
  fprintf(fp, "readlist (%4d)      %11u%11u%11u%9.1f K\n",
          n, Readlist_gets, Readlist_frees,
          Readlist_gets - Readlist_frees,
          ((Readlist_gets - Readlist_frees) * n) / 1024.);

}  /* fprint_top_input_mem */

/*************
 *
 *   p_top_input_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the top_input package.
*/

/* PUBLIC */
void p_top_input_mem()
{
  fprint_top_input_mem(stdout, TRUE);
}  /* p_top_input_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   init_standard_ladr()
 *
 *************/

/* DOCUMENTATION
This routine initializes various LADR packaages.
*/

/* PUBLIC */
void init_standard_ladr(void)
{
  init_wallclock();
  init_paramod();
  init_basic_paramod();
  init_maximal();
  declare_base_symbols();
  declare_standard_parse_types();
  translate_neg_equalities(TRUE);
  init_standard_options();
}  /* init_standard_ladr */

/*************
 *
 *   fatal_input_error()
 *
 *************/

static
void fatal_input_error(FILE *fout, char *msg, Term t)
{
  bell(stderr);
  if (t) {
    fprintf(fout, "\n%%%%ERROR: %s:\n\n", msg);
    fprintf(fout, "%%%%START ERROR%%%%\n");
    fwrite_term_nl(fout, t);
    fprintf(fout, "%%%%END ERROR%%%%\n");
    fwrite_term_nl(stderr, t);
  }
  else
    fprintf(fout, "%%%%ERROR: %s.\n\n", msg);
  fatal_error(msg);
}  /* fatal_input_error */

/*************
 *
 *   set_program_name()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_program_name(char *name)
{
  Program_name = name;
}  /* set_program_name */

/*************
 *
 *   condition_is_true()
 *
 *************/

static
BOOL condition_is_true(Term t)
{
  return is_term(t, Program_name, 0);
}  /* condition_is_true */

/*************
 *
 *   process_op2()
 *
 *************/

static
void process_op2(FILE *fout, Term t, int prec, Term type_term, Term symb_term)
{
  if (ARITY(symb_term) != 0) {
    fatal_input_error(fout, "Bad symbol in op command (quotes needed?)", t);
  }
  else {
    Parsetype pt = NOTHING_SPECIAL;
    if (is_constant(type_term, "infix"))
      pt = INFIX;
    else if (is_constant(type_term, "infix_left"))
      pt = INFIX_LEFT;
    else if (is_constant(type_term, "infix_right"))
      pt = INFIX_RIGHT;
    else if (is_constant(type_term, "prefix"))
      pt = PREFIX;
    else if (is_constant(type_term, "prefix_paren"))
      pt = PREFIX_PAREN;
    else if (is_constant(type_term, "postfix"))
      pt = POSTFIX;
    else if (is_constant(type_term, "postfix_paren"))
      pt = POSTFIX_PAREN;
    else if (is_constant(type_term, "ordinary"))
      pt = NOTHING_SPECIAL;
    else
      fatal_input_error(fout, "Bad parse-type in op command", t);
    declare_parse_type(sn_to_str(SYMNUM(symb_term)), prec, pt);
  }
}  /* process_op2 */

/*************
 *
 *   process_op()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void process_op(Term t, BOOL echo, FILE *fout)
{
  Term prec_term, type_term, symb_term;
  int prec;
  BOOL ok;

  if (ARITY(t) == 3) {
    prec_term = ARG(t,0);
    type_term = ARG(t,1);
    symb_term = ARG(t,2);
    ok = term_to_int(prec_term, &prec);
  }
  else {
    type_term = ARG(t,0);
    symb_term = ARG(t,1);
    if (!is_constant(type_term, "ordinary"))
      fatal_input_error(fout,"If no precedence, type must be \"ordinary\"",t);

    ok = TRUE;
    prec = MIN_PRECEDENCE;  /* checked, but not used */
  }
  
  if (echo)
    fwrite_term_nl(fout, t);

  if (!ok || prec < MIN_PRECEDENCE || prec > MAX_PRECEDENCE) {
    bell(stderr);
    fatal_input_error(fout, "Precedence in op command is out of range", t);
  }
  else if (proper_listterm(symb_term)) {
    while (cons_term(symb_term)) {
      process_op2(fout, t, prec, type_term, ARG(symb_term, 0));
      symb_term = ARG(symb_term, 1);
    }
  }
  else
    process_op2(fout, t, prec, type_term, symb_term);
}  /* process_op */

/*************
 *
 *   process_redeclare()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void process_redeclare(Term t, BOOL echo, FILE *fout)
{
  if (ARITY(t) != 2) {
    fatal_input_error(fout, "The redeclare command takes 2 arguments "
		      "(symbol, operation)", t);
    }
  else {
    Term operation = ARG(t, 0);
    Term symbol  = ARG(t, 1);
    if (ARITY(symbol) != 0 || ARITY(operation) != 0) {
      fatal_input_error(fout, "The redeclare command takes 2 arguments "
			"(symbol, operation)", t);
    }
    else {
      BOOL ok;
      if (echo)
	fwrite_term_nl(fout, t);
      ok = redeclare_symbol_and_copy_parsetype(sn_to_str(SYMNUM(operation)),
					       sn_to_str(SYMNUM(symbol)),
					       echo, fout);
      if (!ok) {
	fatal_input_error(fout, "The new symbol for the redeclared operation"
			  " is already in use", t);
      }
    }
  }
}  /* process_redeclare */

/*************
 *
 *   execute_unknown_action()
 *
 *************/

static
void execute_unknown_action(FILE *fout, int unknown_action, Term t, char *message)
{
  if (unknown_action == KILL_UNKNOWN) {
    fatal_input_error(fout, message, t);
  }
  else if (unknown_action == WARN_UNKNOWN) {
    bell(stderr);
    fprintf(fout, "WARNING, %s: ", message);
    fwrite_term_nl(fout,   t);
    fprintf(stderr, "WARNING, %s: ", message);
    fwrite_term_nl(stderr, t);
  }
  else if (unknown_action == NOTE_UNKNOWN) {
    fprintf(fout,   "NOTE, %s: ", message);
    fwrite_term_nl(fout,   t);
  }
}  /* execute_unknown_action */

/*************
 *
 *   flag_handler()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void flag_handler(FILE *fout, Term t, BOOL echo, int unknown_action)
{
  int flag = str_to_flag_id(sn_to_str(SYMNUM(ARG(t,0))));
  if (flag == -1)
    execute_unknown_action(fout, unknown_action, t, "Flag not recognized");
  else {
    update_flag(fout, flag, is_term(t, "set", 1), echo);
    if (is_term(ARG(t,0), "arithmetic", 0)) {
      if (is_term(t,"set",1)) {
	printf("\n    %% Declaring Mace4 arithmetic parse types.\n");
	declare_parse_type("+",   490, INFIX_RIGHT);
	declare_parse_type("*",   470, INFIX_RIGHT);
	declare_parse_type("/",   460, INFIX);
	declare_parse_type("mod", 460, INFIX);
      }
    }
  }
}  /* flag_handler */

/*************
 *
 *   parm_handler()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void parm_handler(FILE *fout, Term t, BOOL echo, int unknown_action)
{
  char *name   = sn_to_str(SYMNUM(ARG(t,0)));
  Term tval  = ARG(t,1);

  int id = str_to_parm_id(name);
  if (id != -1) {
    int val;
    if (term_to_int(tval, &val))
      assign_parm(id, val, echo);
    else {
      execute_unknown_action(fout, unknown_action, t,
			     "Parm needs integer value");      
    }
  }
  else {
    id = str_to_floatparm_id(name);
    if (id != -1) {
      double val;
      if (term_to_number(tval, &val))
	assign_floatparm(id, val, echo);
      else
	execute_unknown_action(fout, unknown_action, t,
			       "Floatparm needs integer or double value");
    }
    else {
      id = str_to_stringparm_id(name);
      if (id != -1) {
	if (CONSTANT(tval)) {
	  char *s = sn_to_str(SYMNUM(tval));
	  assign_stringparm(id, s, echo);
	}
	else
	  execute_unknown_action(fout, unknown_action, t,
				 "Stringparm needs constant value");
      }
      else
	execute_unknown_action(fout, unknown_action, t,
			       "Parameter not recognized");
    }
  }
}  /* parm_handler */

/*************
 *
 *   process_symbol_list()
 *
 *************/

static
void process_symbol_list(Term t, char *command, Plist terms)
{
  Plist strings = NULL;
  Plist p;
  for (p = terms ; p; p = p->next) {
    Term c = p->v;
    if (!CONSTANT(c))
      fatal_input_error(stdout, "Symbols in this command must not have "
			"arguments (arity is deduced from the clauses)", t);
    strings = plist_append(strings, sn_to_str(SYMNUM(c)));
  }
  if (str_ident(command, "lex") || str_ident(command, "function_order"))
    Lex_function_list = plist_cat(Lex_function_list, strings);
  else if (str_ident(command, "predicate_order"))
    Lex_predicate_list = plist_cat(Lex_predicate_list, strings);
  else if (str_ident(command, "skolem"))
    Skolem_list = plist_cat(Skolem_list, strings);
  else
    fatal_input_error(stdout, "Unknown command", t);
}  /* process_symbol_list */

/*************
 *
 *   readlist_member()
 *
 *************/

static
Readlist readlist_member(Readlist p, char *name, int type)
{
  if (p == NULL)
    return NULL;
  else if (p->type == type && str_ident(p->name, name))
    return p;
  else
    return readlist_member(p->next, name, type);
}  /* readlist_member */

/*************
 *
 *   readlist_member_wild()
 *
 *************/

static
Readlist readlist_member_wild(Readlist p, int type)
{
  if (p == NULL)
    return NULL;
  else if (p->type == type && str_ident(p->name, ""))
    return p;
  else
    return readlist_member_wild(p->next, type);
}  /* readlist_member */

/*************
 *
 *   accept_list()
 *
 *************/

/* DOCUMENTATION
  name:   name of list
  type:   type of list (FORMULAS, TERMS)
  aux:    Is it an auxiliary list?  FALSE: the formulas are part of
          the logical theory.  TRUE: the formulas are not part of the
          logical theory, e.g., hints.  When Skolem symbols are generated,
	  it's ok if the symbols already exist in aux lists.
  l:      pointer to (pointer to) list where objects should be
          placed (later, when they are actually read).
*/

/* PUBLIC */
void accept_list(char *name, int type, BOOL aux, Plist *l)
{
  Readlist p = readlist_member(Input_lists, name, type);
  if (p)
    fatal_error("accept_list, duplicate name/type");
  else {
    p = get_readlist();
    p->name = name;
    p->type = type;
    p->auxiliary = aux;
    *l = NULL;
    p->p = l;

    p->next = Input_lists;
    Input_lists = p;
  }
}  /* accept_list */

/*************
 *
 *   input_symbols()
 *
 *************/

static
void input_symbols(int type, BOOL aux, Ilist *fsyms, Ilist *rsyms)
{
  I2list fsyms_multiset = NULL;
  I2list rsyms_multiset = NULL;
  Readlist p;
  for (p = Input_lists; p; p = p->next) {
    if (p->type == type && p->auxiliary == aux) {
      gather_symbols_in_formulas(*(p->p), &rsyms_multiset, &fsyms_multiset);
    }
  }
  *rsyms = multiset_to_set(rsyms_multiset);
  *fsyms = multiset_to_set(fsyms_multiset);
  *fsyms = remove_variable_symbols(*fsyms);
  zap_i2list(fsyms_multiset);
  zap_i2list(rsyms_multiset);

  if (FALSE) {
    Ilist p;
    printf("RSYMS: ");
    for (p = *rsyms; p; p = p->next)
      printf(" %s", sn_to_str(p->i));
    printf("\n");
    printf("FSYMS: ");
    for (p = *fsyms; p; p = p->next)
      printf(" %s", sn_to_str(p->i));
    printf("\n");
  }
}  /* input_symbols */

/*************
 *
 *   symbol_check_and_declare()
 *
 *   Make sure that symbols are being used in reasonable ways.
 *
 *   Look at the clauses and formulas in the Input_lists.
 *   Make sure that (1) no symbol is used as both a relation
 *   and a function symbol, and (2) that no symbol has multiple
 *   arities.  Also tell the symbol package which symbols are
 *   relations and which are functions.
 *
 *************/

static
void symbol_check_and_declare(void)
{
  Ilist fsyms_theory, fsyms_aux, fsyms_all, fsyms_aux_only;
  Ilist rsyms_theory, rsyms_aux, rsyms_all;
  Ilist bad;

  input_symbols(FORMULAS,  FALSE,  &fsyms_theory, &rsyms_theory);
  input_symbols(FORMULAS,  TRUE,   &fsyms_aux,    &rsyms_aux);

  fsyms_all = ilist_union(fsyms_theory, fsyms_aux);
  rsyms_all = ilist_union(rsyms_theory, rsyms_aux);

  fsyms_aux_only = ilist_subtract(fsyms_aux, fsyms_theory);

  declare_aux_symbols(fsyms_aux_only);

  /* Check for variables in rsyms. */

  bad = variable_symbols(rsyms_all);
  if (bad) {
    Ilist g;
    String_buf sb = init_string_buf("The following symbols cannot be used as"
				    " atomic formulas, because they are"
				    " variables: ");
    for (g = bad; g; g = g->next) {
      sb_append(sb, sn_to_str(g->i));
      if (g->next)
	sb_append(sb, ", ");
    }
    fatal_input_error(stdout, sb_to_malloc_string(sb), NULL);
  }

  /* Check if any symbol is used as both a relation and function symbol. */

  bad = ilist_intersect(fsyms_all, rsyms_all);
  if (bad) {
    Ilist g;
    String_buf sb = init_string_buf("The following symbols/arities are used as"
				    " both relation and function symbols: ");
    for (g = bad; g; g = g->next) {
      sb_append(sb, sn_to_str(g->i));
      sb_append_char(sb, '/');
      sb_append_int(sb, sn_to_arity(g->i));
      if (g->next)
	sb_append(sb, ", ");
    }
    fatal_input_error(stdout, sb_to_malloc_string(sb), NULL);
  }

  /* Check if any symbol is used with multiple arities. */

  bad = arity_check(fsyms_all, rsyms_all);
  if (bad) {
    Ilist g;
    String_buf sb = init_string_buf("The following symbols are used with"
				    " multiple arities: ");
    for (g = bad; g; g = g->next) {
      sb_append(sb, sn_to_str(g->i));
      sb_append_char(sb, '/');
      sb_append_int(sb, sn_to_arity(g->i));
      if (g->next)
	sb_append(sb, ", ");
    }
    fatal_input_error(stdout, sb_to_malloc_string(sb), NULL);
  }

  /* Tell the symbol package the functions and relations.
     (needed for ordering the symbols) */

  declare_functions_and_relations(fsyms_all, rsyms_all);

  process_skolem_list(Skolem_list, fsyms_theory);
  zap_plist(Skolem_list);
  Skolem_list = NULL;

  process_lex_list(Lex_function_list, fsyms_all, FUNCTION_SYMBOL);
  process_lex_list(Lex_predicate_list, rsyms_all, PREDICATE_SYMBOL);

  zap_plist(Lex_function_list);
  zap_plist(Lex_predicate_list);
  Lex_function_list = NULL;
  Lex_predicate_list = NULL;

  /*
  p_syms();
  printf("fsyms_theory: "); p_ilist(fsyms_theory);
  printf("fsyms_aux: "); p_ilist(fsyms_aux);
  printf("fsyms_all: "); p_ilist(fsyms_all);
  printf("fsyms_aux_only: "); p_ilist(fsyms_aux_only);
  printf("rsyms_theory: "); p_ilist(rsyms_theory);
  printf("rsyms_aux: "); p_ilist(rsyms_aux);
  printf("rsyms_all: "); p_ilist(rsyms_all);
  */

  zap_ilist(fsyms_theory);
  zap_ilist(fsyms_aux);
  zap_ilist(fsyms_all);
  zap_ilist(fsyms_aux_only);

  zap_ilist(rsyms_theory);
  zap_ilist(rsyms_aux);
  zap_ilist(rsyms_all);

}  /* symbol_check_and_declare */

/*************
 *
 *   read_from_file()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void read_from_file(FILE *fin, FILE *fout, BOOL echo, int unknown_action)
{
  int if_depth = 0;  /* for conditional inclusion */
  Term t = read_term(fin, fout);

  while (t != NULL) {
    if (is_term(t, "set", 1) || is_term(t, "clear", 1)) {
      /********************************************************** set, clear */
      if (echo)
	fwrite_term_nl(fout, t);
      flag_handler(fout, t, echo, unknown_action);

    }
    else if (is_term(t, "assign", 2)) {
      /************************************************************** assign */
      if (echo)
	fwrite_term_nl(fout, t);
      parm_handler(fout, t, echo, unknown_action);
    }
    else if (is_term(t, "assoc_comm", 1) ||
             is_term(t, "commutative", 1)) {
      /************************************************************ AC, etc. */
      Term f = ARG(t,0);
      if (!CONSTANT(f)) {
	fatal_input_error(fout, "Argument must be symbol only", t);
      }
      else {
	if (echo)
	  fwrite_term_nl(fout, t);
	if (is_term(t, "assoc_comm", 1))
	  set_assoc_comm(sn_to_str(SYMNUM(f)), TRUE);
	else
	  set_commutative(sn_to_str(SYMNUM(f)), TRUE);
      }
    }
    else if (is_term(t, "op", 3) || is_term(t, "op", 2)) {
      /****************************************************************** op */
      /* e.g., op(300, infix, +).  op(ordinary, *) */
      process_op(t, echo, fout);
    }
    else if (is_term(t, "redeclare", 2)) {
      /*********************************************************** redeclare */
      /* e.g., redeclare(~, negation). */
      process_redeclare(t, echo, fout);
    }
    else if (is_term(t, "lex", 1) || is_term(t, "predicate_order", 1) ||
	     is_term(t, "function_order", 1) || is_term(t, "skolem", 1)) {
      /********************************************************* lex, skolem */
      char *command = sn_to_str(SYMNUM(t));
      Plist p = listterm_to_tlist(ARG(t,0));
      if (p == NULL) {
	fatal_input_error(fout, "Function_order/predicate_order/skolem command"
			  "must contain a list, e.g., [a,b,c]", t);
      }
      else {
	if (echo)
	  fwrite_term_nl(fout, t);
	process_symbol_list(t, command, p);
	zap_plist(p);
      }
    }
    else if (is_term(t, "formulas", 1) ||
	     is_term(t, "clauses", 1) ||
	     is_term(t, "terms", 1) ||
	     is_term(t, "list", 1)) {
      /***************************************************** list of objects */
      int type = (is_term(t, "formulas", 1) || is_term(t, "clauses", 1)
		  ? FORMULAS : TERMS);
      char *name = term_symbol(ARG(t,0));
      Plist objects = NULL;
      int echo_id = str_to_flag_id("echo_input");
      BOOL echo_objects = (echo_id == -1 ? TRUE : flag(echo_id));

      if (is_term(t, "clauses", 1)) {
	bell(stderr);
	fprintf(stderr,
		"\nWARNING: \"clauses(...)\" should be replaced with \"formulas(...)\".\n"
		"Please update your input files.  Future versions will not accept \"clauses(...)\".\n\n");
      }
      else if (is_term(t, "terms", 1)) {
	bell(stderr);
	fprintf(stderr,
		"\nWARNING: \"terms(...)\" should be replaced with \"list(...)\".\n"
		"Please update your input files.  Future versions will not accept \"terms(...)\".\n\n");
      }

      objects = read_term_list(fin, fout);
      if (type == FORMULAS) {
	Plist p;
	for (p = objects; p; p = p->next) {
	  Term t = p->v;
	  p->v = term_to_formula(t);
	  zap_term(t);
	}
      }

      if (echo) {
	if (echo_objects) {
	  if (type == FORMULAS)
	    fwrite_formula_list(fout, objects, name);
	  else
	    fwrite_term_list(fout, objects, name);
	}
	else {
	  fprintf(fout, "\n%% ");
	  fwrite_term(fout, t);
	  fprintf(fout, ".  %% not echoed (%d %s)\n",
		  plist_count(objects),
		  type == FORMULAS ? "formulas" : "terms");
	}
      }
      /* Find the correct list, and append the objects to it. */
      
      {
	Readlist r = readlist_member(Input_lists, name, type);
	if (r == NULL)
	  r = readlist_member_wild(Input_lists, type);
	if (r == NULL) {
	  fatal_input_error(fout, "List name/type not recognized", t);
	}
	else {
	  *(r->p) = plist_cat(*(r->p), objects);
	}
      }
    }  /* list of formulas or terms */
    else if (is_term(t, "if", 1)) {
      /***************************************************** if() ... end_if */
      Term condition = ARG(t,0);
      if (echo)
	fwrite_term_nl(fout, t);
      if (condition_is_true(condition)) {
	if (echo)
	  fprintf(fout, "%% Conditional input included.\n");
	if_depth++;
      }
      else {
	/* skip to matching end_if */
	int depth = 1;  /* if-depth */
	Term t2;
	do {
	  t2 = read_term(fin, fout);
	  if (!t2)
	    fatal_input_error(fout, "Missing end_if (condition is false)", t);
	  else if (is_term(t2, "if", 1))
	    depth++;
	  else if (is_term(t2, "end_if", 0))
	    depth--;
	  zap_term(t2);
	}
	while (depth > 0);

	if (echo)
	  fprintf(fout, "%% Conditional input omitted.\nend_if.\n");
      }
    }  /* if() ... end_if. */
    else if (is_term(t, "end_if", 0)) {
      /***************************************************** end_if (true case) */
      if_depth--;
      if (echo)
	fwrite_term_nl(fout, t);
      if (if_depth < 0)
	fatal_input_error(fout, "Extra end_if", t);
    }
    else
      fatal_input_error(fout, "Unrecognized command or list", t);
    zap_term(t);
    t = read_term(fin, fout);
  }
  if (if_depth != 0) {
    fatal_input_error(fout, "Missing end_if (condition is true)", t);
  }
}  /* read_from_file */

/*************
 *
 *   read_all_input()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void read_all_input(int argc, char **argv, FILE *fout,
		    BOOL echo, int unknown_action)
{
  int n = which_string_member("-f", argv, argc);
  if (n == -1) {
    read_from_file(stdin, fout, echo, unknown_action);
  }
  else {
    int i;
    for (i = n+1; i < argc; i++) {
      FILE *fin = fopen(argv[i], "r");
      if (fin == NULL) {
	char s[100];
	sprintf(s, "read_all_input, file %s not found", argv[i]);
	fatal_error(s);
      }
      if (echo)
	printf("\n%% Reading from file %s\n\n", argv[i]);
      read_from_file(fin, fout, echo, unknown_action);
      fclose(fin);
    }
  }
  process_standard_options();
  symbol_check_and_declare();
}  /* read_all_input */

/*************
 *
 *   check_formula_attributes()
 *
 *************/

static
void check_formula_attributes(Formula f)
{
  if (subformula_contains_attributes(f)) {
    Term t = formula_to_term(f);
    fatal_input_error(stdout, "Subformulas cannot contain attributes", t);
  }
  else if (!clausal_formula(f) && 
	   attributes_contain_variables(f->attributes)) {
    Term t = formula_to_term(f);
    fatal_input_error(stdout, "Answer attributes on non-clausal formulas"
		      " cannot contain variables", t);
  }
}  /* check_formula_attributes */

/*************
 *
 *   process_input_formulas()
 *
 *************/

/* DOCUMENTATION
Input is Plist of Topforms containing formulas.
Result is a Plist of Topforms containing clauses.
*/

/* PUBLIC */
Plist process_input_formulas(Plist formulas, BOOL echo)
{
  Plist new = NULL;  /* collect Topforms (clauses) to be returned */
  Plist p;

  for (p = formulas; p; p = p->next) {
    Topform tf = p->v;
    if (clausal_formula(tf->formula)) {
      /* just make it into a clause data structure and use the same Topform */
      tf->literals = formula_to_literals(tf->formula);
      upward_clause_links(tf);
      zap_formula(tf->formula);
      tf->formula = NULL;
      tf->is_formula = FALSE;
      clause_set_variables(tf, MAX_VARS);
      new = plist_prepend(new, tf);
    }
    else {
      /* Clausify, collecting new Topforms to be returned. */
      Formula f2;
      Plist clauses, p;
      assign_clause_id(tf);
      f2 = universal_closure(formula_copy(tf->formula));
      clauses = clausify_formula(f2);
      for (p = clauses; p; p = p->next) {
	Topform c = p->v;
	c->attributes = copy_attributes(tf->attributes);
	c->justification = clausify_just(tf);
	new = plist_prepend(new, c);
      }
      append_label_attribute(tf, "non_clause");
      if (echo)
	fwrite_clause(stdout, tf, CL_FORM_STD);
      /* After this point, tf will be accessible only from the ID table. */
    }
  }
  zap_plist(formulas);  /* shallow */
  new = reverse_plist(new);
  return new;
}  /* process_input_formulas */

/*************
 *
 *   process_demod_formulas()
 *
 *************/

/* DOCUMENTATION
Input is Plist of Topforms containing formulas.
Result is a Plist of Topforms containing clauses.
*/

/* PUBLIC */
Plist process_demod_formulas(Plist formulas, BOOL echo)
{
  Plist new = NULL;  /* collect Topforms (clauses) to be returned */
  Plist p;

  for (p = formulas; p; p = p->next) {
    Topform tf = p->v;
    if (clausal_formula(tf->formula)) {
      /* just make it into a clause data structure and use the same Topform */
      tf->literals = formula_to_literals(tf->formula);
      upward_clause_links(tf);
      zap_formula(tf->formula);
      tf->formula = NULL;
      tf->is_formula = FALSE;
      clause_set_variables(tf, MAX_VARS);
      new = plist_prepend(new, tf);
    }
    else if (tf->formula->type == IMP_FORM ||
	     tf->formula->type == IMPBY_FORM ||
	     tf->formula->type == IFF_FORM) {
       new = plist_prepend(new, tf);
    }
    else
      fatal_error("process_demod_formulas: bad demodulator");
  }
  zap_plist(formulas);  /* shallow */
  new = reverse_plist(new);
  return new;
}  /* process_demod_formulas */

/*************
 *
 *   process_goal_formulas()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist process_goal_formulas(Plist formulas, BOOL echo)
{
  BOOL must_be_positive = (plist_count(formulas) > 1);
  Plist new = NULL;
  Plist p;
  for (p = formulas; p; p = p->next) {
    Topform tf = p->v;
    Formula f2;
    Plist clauses, q;

    f2 = universal_closure(formula_copy(tf->formula));

    if (must_be_positive && !positive_formula(f2)) {
      Term t = formula_to_term(tf->formula);
      fatal_input_error(stdout, "If there are multiple goals, all must be "
			"positive", t);
    }

    f2 = negate(f2);
    clauses = clausify_formula(f2);
    assign_clause_id(tf);
    for (q = clauses; q; q = q->next) {
      Topform c = q->v;
      c->attributes = copy_attributes(tf->attributes);
      c->justification = deny_just(tf);
      new = plist_prepend(new, c);
    }
    append_label_attribute(tf, "non_clause");
    append_label_attribute(tf, "goal");
    if (echo)
      fwrite_clause(stdout, tf, CL_FORM_STD);
    /* After this point, tf will be accessible only from the ID table. */
  }
  zap_plist(formulas);  /* shallow */
  new = reverse_plist(new);
  return new;
}  /* process_goal_formulas */

/*************
 *
 *   read_commands()
 *
 *************/

/* DOCUMENTATION
This is a legacy routine.
*/

/* PUBLIC */
Term read_commands(FILE *fin, FILE *fout, BOOL echo, int unknown_action)
{
  Term t = read_term(fin, fout);
  BOOL go = (t != NULL);

  while (go) {
    BOOL already_echoed = FALSE;
    /************************************************************ set, clear */
    if (is_term(t, "set", 1) || is_term(t, "clear", 1)) {
      if (echo) {
	fwrite_term_nl(fout, t);
	already_echoed = TRUE;
      }
      flag_handler(fout, t, echo, unknown_action);

      /* SPECIAL CASES: these need action right now! */

      if (is_term(ARG(t,0), "prolog_style_variables", 0)) {
	if (is_term(t,"set",1))
	  set_variable_style(PROLOG_STYLE);
	else
	  set_variable_style(STANDARD_STYLE);
      }
    }
    else if (is_term(t, "assign", 2)) {
      /************************************************************** assign */
      if (echo) {
	fwrite_term_nl(fout, t);
	already_echoed = TRUE;
      }
      parm_handler(fout, t, echo, unknown_action);
    }
    else if (is_term(t, "assoc_comm", 1) ||
             is_term(t, "commutative", 1)) {
      /************************************************************ AC, etc. */
      Term f = ARG(t,0);
      if (!CONSTANT(f)) {
	fatal_input_error(fout, "Argument must be symbol only", t);
      }
      else {
	if (is_term(t, "assoc_comm", 1))
	  set_assoc_comm(sn_to_str(SYMNUM(f)), TRUE);
	else
	  set_commutative(sn_to_str(SYMNUM(f)), TRUE);
      }
    }
    else if (is_term(t, "op", 3)) {
      /****************************************************************** op */
      /* e.g., op(300, infix, +); */
      process_op(t, echo, fout);
    }
    else if (is_term(t, "lex", 1)) {
      /***************************************************************** lex */
      Plist p = listterm_to_tlist(ARG(t,0));
      if (p == NULL) {
	fatal_input_error(fout, "Lex command must contain a proper list, "
			  "e.g., [a,b,c]", t);
      }
      else {
	process_symbol_list(t, "lex", p);
	zap_plist(p);
      }
    }
    else {
      /******************************************************** unrecognized */
      /* return this unknown term */
      go = FALSE;
    }

    if (go) {
      if (echo && !already_echoed)
	fwrite_term_nl(fout, t);
      zap_term(t);
      t = read_term(fin, fout);
      go = (t != NULL);
    }
  }
  return t;
}  /* read_commands */

/*************
 *
 *   embed_formulas_in_topforms()
 *
 *************/

/* DOCUMENTATION
The formulas are not copied.
Any attributes on the top of the formula are moved to the Topform.
An ID is not assigned.
Topforms get the justifiction "input".
*/

/* PUBLIC */
Plist embed_formulas_in_topforms(Plist formulas, BOOL assumption)
{
  Plist p;

  for (p = formulas; p; p = p->next) {
    Formula f = p->v;
    Topform tf = get_topform();
    tf->is_formula = TRUE;
    tf->formula = f;
    check_formula_attributes(f);
    tf->justification = (assumption ? input_just() : goal_just());
    tf->attributes = f->attributes;
    f->attributes = NULL;
    p->v = tf;
  }
  return formulas;
}  /* embed_formulas_in_topforms */

