#include "commands.h"

/* Private definitions and types */

Term List_identifier = NULL;

/*************
 *
 *   process_op()
 *
 *************/

static
void process_op(FILE *fout, Term t, int prec, Term type_term, Term symb_term)
{
  if (ARITY(symb_term) != 0) {
    fwrite_term_nl(fout, t);
    fwrite_term_nl(stderr, t);
    fatal_error("symbols in op command must have no arguments");
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
    else if (is_constant(type_term, "clear"))
      pt = NOTHING_SPECIAL;
    else {
      fwrite_term_nl(fout, t);
      fwrite_term_nl(stderr, t);
      fatal_error("bad parse-type in op command");
    }
    set_parse_type(sn_to_str(SYMNUM(symb_term)), prec, pt);
  }
}  /* process_op */

/*************
 *
 *   flag_handler()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void flag_handler(FILE *fout, Term t, int unknown_action)
{
  int flag = str_to_flag_id(sn_to_str(SYMNUM(ARG(t,0))));
  if (flag == -1) {
    if (unknown_action == KILL_UNKNOWN) {
      fwrite_term_nl(fout, t);
      fwrite_term_nl(stderr, t);
      fatal_error("flag not recognized");
    }
    else if (unknown_action == WARN_UNKNOWN) {
      bell(stderr);
      fprintf(fout,   "WARNING, flag not recognized: ");
      fwrite_term_nl(fout,   t);
      fprintf(stderr, "WARNING, flag not recognized: ");
      fwrite_term_nl(stderr, t);
    }
    else if (unknown_action == NOTE_UNKNOWN) {
      fprintf(fout,   "NOTE: flag not recognized: ");
      fwrite_term_nl(fout,   t);
    }
  }
  else
    update_flag(fout, flag, is_term(t, "set", 1));
}  /* flag_handler */

/*************
 *
 *   parm_handler()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void parm_handler(FILE *fout, Term t, int unknown_action)
{
  int val;
  BOOL ok = term_to_int(ARG(t,1), &val);
  if (!ok) {
    int id = str_to_stringparm_id(sn_to_str(SYMNUM(ARG(t,0))));
    if (id == -1) {
      /* This should take into account unknown_action. */
      fwrite_term_nl(fout, t);
      fwrite_term_nl(stderr, t);
      fatal_error("bad assign command");
    }
    else {
      char *s = sn_to_str(SYMNUM(ARG(t,1)));
      assign_stringparm(id, s);
    }
  }
  else {
    int parm = str_to_parm_id(sn_to_str(SYMNUM(ARG(t,0))));
    if (parm == -1) {
      if (unknown_action == KILL_UNKNOWN) {
	fwrite_term_nl(fout, t);
	fwrite_term_nl(stderr, t);
	fatal_error("parm not recognized");
      }
      else if (unknown_action == WARN_UNKNOWN) {
	bell(stderr);
	fprintf(fout,   "WARNING, parm not recognized: ");
	fwrite_term_nl(fout,   t);
	fprintf(stderr, "WARNING, parm not recognized: ");
	fwrite_term_nl(stderr, t);
      }
      else if (unknown_action == NOTE_UNKNOWN) {
	fprintf(fout,   "NOTE: parm not recognized: ");
	fwrite_term_nl(fout,   t);
      }
    }
    else {
      assign_parm(parm, val);
    }
  }
}  /* parm_handler */

/*************
 *
 *   preliminary_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void preliminary_precedence(Plist p)
{
  int n = 0;
  Plist q;
  reinit_lex_vals();
  for (q = p ; q; q = q->next) {
    Term t = q->v;
    set_lex_val(SYMNUM(t), ++n);
  }
}  /* preliminary_precedence */

/*************
 *
 *   read_commands_from_file()
 *
 *************/

/* DOCUMENTATION
<P>
This routine reads commands from a file.
Echoing the commands to an output file is optional.
Here are examples of the currently supported commands.
<UL>
<LI>  set(verbose).
<LI>  clear(verbose).
<LI>  assign(max_seconds, 1800).
<LI>  assoc_comm(+).
<LI>  commutative(+).
<LI>  op(400, infix, ^).
<LI>  lex([e, a, b, _*_, _', h(_,_)]).
</UL>
<P>
A term is returned:
<UL>
<LI>  NULL: commands were read up to EOF.
<LI>  nonNULL term: commands were read up to this unrecognized term; you
      will probably wish to continue your input processing with this term.
</UL>
<P>
If any error occurs, a message goes to file fout and to stderr,
and a fatal_error() occurs.
*/

/* PUBLIC */
Term read_commands_from_file(FILE *fin, FILE *fout, BOOL echo, int unknown_action)
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
      flag_handler(fout, t, unknown_action);
    }
    else if (is_term(t, "assign", 2)) {
      /************************************************************** assign */
      if (echo) {
	fwrite_term_nl(fout, t);
	already_echoed = TRUE;
      }
      parm_handler(fout, t, unknown_action);
    }
    else if (is_term(t, "assoc_comm", 1) ||
             is_term(t, "commutative", 1)) {
      /************************************************************ AC, etc. */
      Term f = ARG(t,0);
      if (!CONSTANT(f)) {
	bell(stderr);
	fwrite_term_nl(fout, t);
	fwrite_term_nl(stderr, t);
	fatal_error("argument must be symbol only");
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
      Term prec_term = ARG(t,0);
      Term type_term = ARG(t,1);
      Term symb_term = ARG(t,2);
      int prec;
      BOOL ok = term_to_int(prec_term, &prec);
      if (!ok || prec < MIN_PRECEDENCE || prec > MAX_PRECEDENCE) {
	bell(stderr);
	fwrite_term_nl(fout, t);
	fwrite_term_nl(stderr, t);
	fatal_error("bad precedence in op command");
      }
      else if (proper_listterm(symb_term)) {
	while (cons_term(symb_term)) {
	  process_op(fout, t, prec, type_term, ARG(symb_term, 0));
	  symb_term = ARG(symb_term, 1);
	}
      }
      else
	process_op(fout, t, prec, type_term, symb_term);
    }
    else if (is_term(t, "lex", 1)) {
      /***************************************************************** lex */
      Plist p = listterm_to_tlist(ARG(t,0));
      if (p == NULL) {
	bell(stderr);
	fwrite_term_nl(fout, t);
	fwrite_term_nl(stderr, t);
	fatal_error("lex command must contain a proper list, e.g., [a,b,c]");
      }
      else {
	preliminary_precedence(p);
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
}  /* read_commands_from_file */

/*************
 *
 *   read_commands()
 *
 *************/

/* PUBLIC */
void read_commands(int argc, char **argv, FILE *fout, BOOL echo, int unknown_action)
{
  int n = which_string_member("-f", argv, argc);
  if (n == -1) {
    List_identifier = read_commands_from_file(stdin,fout,echo,unknown_action);
  }
  else {
    /* Read from the files until we get to a term that is not recognized
       (which should be a list identifier).  Save that term. */
    int i;
    Term t = NULL;
    for (i = n+1; i < argc && t == NULL; i++) {
      FILE *fin = fopen(argv[i], "r");
      if (fin == NULL) {
	fprintf(stdout, "File not found: %s\n", argv[i]);
	fprintf(stderr, "File not found: %s\n", argv[i]);
	fatal_error("read_commands: file not found");
      }
      t = read_commands_from_file(fin, fout, echo, unknown_action);
      fclose(fin);
      List_identifier = t;
    }
  }
}  /* read_commands */

