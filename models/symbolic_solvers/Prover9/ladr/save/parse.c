#include "parse.h"

/* TO DO
 * 1. term_to_terms should not have to call binary_parse_type and 
 *    sn_to_str when backtracking.  Store that info in pterm.
 * 2. less use of strbuf?
 * 3. DONE.  Keep allowing applications without parens?  NO
 * 4. think about check_arity.
 * 5. reduce some term copying?
 * 6. less space on output?
 * 7. more parens on output?
 * 8. another symbol type for single-character tokens? (comma, single quote)
 */

/* Private definitions and types */

/* Token types */

typedef enum {TOK_UNKNOWN,  /* probably a garbage binary char */
	      TOK_NAME,     /* see name_char() */
	      TOK_SYMBOL,   /* see symbol_char() */
	      TOK_STRING,   /* see quote_char() */
	      TOK_COMMENT,  /* see comment_char() */
	      TOK_PUNC      /* see punctutation_char() */
} Toktype;

/* Return codes from read_buf() */

typedef enum {READ_BUF_OK,READ_BUF_EOF,
	      READ_BUF_ERROR,READ_BUF_QUOTE_ERROR} Read_rc;

/* A list of tokens */

typedef struct token * Token;

struct token {
  Toktype    type;
  char       c;        /* for punctuation & unknown tokens */
  String_buf sb;       /* for other tokens */
  int        buf_pos;  /* position of this token in buffer */
  Token      next;
};

/* A list of terms with some other data. */

typedef struct pterm * Pterm;

struct pterm {
  Term       t;
  int        possible_application;  /* avoids "a b" being parsed as a(b) */
  Pterm      prev, next;
};

/* Token position */

typedef struct tok_pos *Tok_pos;

struct tok_pos {
  Token tok;
  char *error_message;
};

/* Private variables */

static BOOL Translate_neg_equalities = FALSE;

/*
 * memory management
 */

static unsigned Token_gets, Token_frees;
static unsigned Pterm_gets, Pterm_frees;

#define BYTES_TOKEN sizeof(struct token)
#define PTRS_TOKEN BYTES_TOKEN%BPP == 0 ? BYTES_TOKEN/BPP : BYTES_TOKEN/BPP + 1

#define BYTES_PTERM sizeof(struct pterm)
#define PTRS_PTERM BYTES_PTERM%BPP == 0 ? BYTES_PTERM/BPP : BYTES_PTERM/BPP + 1

/*************
 *
 *   Token get_token()
 *
 *************/

static
Token get_token(void)
{
  Token p = get_mem(PTRS_TOKEN);
  Token_gets++;
  return(p);
}  /* get_token */

/*************
 *
 *    free_token()
 *
 *************/

static
void free_token(Token p)
{
  free_mem(p, PTRS_TOKEN);
  Token_frees++;
}  /* free_token */

/*************
 *
 *   Pterm get_pterm()
 *
 *************/

static
Pterm get_pterm(void)
{
  Pterm p = get_mem(PTRS_PTERM);
  Pterm_gets++;
  return(p);
}  /* get_pterm */

/*************
 *
 *    free_pterm()
 *
 *************/

static
void free_pterm(Pterm p)
{
  free_mem(p, PTRS_PTERM);
  Pterm_frees++;
}  /* free_pterm */

/*************
 *
 *   fprint_parse_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the parse package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_parse_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_TOKEN;
  fprintf(fp, "token (%4d)        %11u%11u%11u%9.1f K\n",
          n, Token_gets, Token_frees,
          Token_gets - Token_frees,
          ((Token_gets - Token_frees) * n) / 1024.);

  n = BYTES_PTERM;
  fprintf(fp, "pterm (%4d)        %11u%11u%11u%9.1f K\n",
          n, Pterm_gets, Pterm_frees,
          Pterm_gets - Pterm_frees,
          ((Pterm_gets - Pterm_frees) * n) / 1024.);

}  /* fprint_parse_mem */

/*************
 *
 *   p_parse_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the parse package.
*/

/* PUBLIC */
void p_parse_mem()
{
  fprint_parse_mem(stdout, TRUE);
}  /* p_parse_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   translate_neg_equalities()
 *
 *************/

/* DOCUMENTATION
This routine sets or clears the flag which tells the parser
to automatically translate alpha!=beta to ~(alpha=beta).
This happens in read_term(), which is called by
read_clause(), read_formula(), and read_term_list().
*/

/* PUBLIC */
void translate_neg_equalities(BOOL flag)
{
  Translate_neg_equalities = flag;
}  /* translate_neg_equalities */

/*************
 *
 *   translate_neg_eq()
 *
 *************/

static
Term translate_neg_eq(Term t)
{
  if (t != NULL && COMPLEX(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = translate_neg_eq(ARG(t,i));
    if (is_symbol(SYMNUM(t), NEQ_SYM, 2)) {
      Term eq_term = get_rigid_term(EQ_SYM, 2);
      Term not_term = get_rigid_term(NOT_SYM, 1);
      ARG(eq_term,0) = ARG(t, 0);
      ARG(eq_term,1) = ARG(t, 1);
      ARG(not_term,0) = eq_term;
      free_term(t);
      t = not_term;
    }
  }
  return t;
}  /* translate_neg_eq */

/*************
 *
 *   untranslate_neg_eq()
 *
 *************/

static
Term untranslate_neg_eq(Term t)
{
  if (t != NULL && COMPLEX(t)) {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = untranslate_neg_eq(ARG(t,i));
    if (is_symbol(SYMNUM(t), NOT_SYM, 1) &&
	is_symbol(SYMNUM(ARG(t,0)), EQ_SYM, 2)) {
      Term neq_term = get_rigid_term(NEQ_SYM, 2);
      ARG(neq_term,0) = ARG(ARG(t,0), 0);
      ARG(neq_term,1) = ARG(ARG(t,0), 1);
      free_term(ARG(t,0));
      free_term(t);
      t = neq_term;
    }
  }
  return t;
}  /* untranslate_neg_eq */

/*************
 *
 *   free_pterm_list()
 *
 *************/

static
void free_pterm_list(Pterm p)
{
  Pterm p1;
  while (p != NULL) {
    if (p->t != NULL)
      zap_term(p->t);
    p1 = p;
    p = p->next;
    free_pterm(p1);
  }
}  /* free_pterm_list */

/*************
 *
 *   free_token_list()
 *
 *************/

static
void free_token_list(Token p)
{
  Token p1;
  while (p != NULL) {
    p1 = p;
    p = p->next;
    if (p1->sb != NULL)
      zap_string_buf(p1->sb);
    free_token(p1);
  }
}  /* free_token_list */

/*************
 *
 *    end_char()
 *
 *************/

static
BOOL end_char(char c)
{
    return (c == '.');
}  /* end_char */

/*************
 *
 *    comment_char()
 *
 *************/

static
BOOL comment_char(char c)
{
    return (c == '%');
}  /* comment_char */

/*************
 *
 *    quote_char()
 *
 *************/

static
BOOL quote_char(char c)
{
    return (c == '\"');
}  /* quote_char */

/*************
 *
 *    punctuation_char()
 *
 *************/

static
BOOL punctuation_char(char c)
{
    return (c == ',' ||
	    c == ':' ||
	    c == '(' || c == ')' ||
	    c == '[' || c == ']' ||
	    c == '{' || c == '}' ||
	    
	    c == '.');
}  /* punctuation_char */

/*************
 *
 *    name_char()
 *
 *************/

static
BOOL name_char(char c)
{
    return ((c >= '0' && c <= '9') ||
	    (c >= 'a' && c <= 'z') ||
	    (c >= 'A' && c <= 'Z') ||
	    c == '_' ||
	    c == '$');
}  /* name_char */

/*************
 *
 *    symbol_char(c)
 *
 *************/

static
BOOL symbol_char(char c)
{
  /* This allows us to have special chars in the list below. */
  if (quote_char(c) || end_char(c) || comment_char(c))
    return 0;
  else
    return (c == '+' ||
	    c == '-' ||
	    c == '*' ||
	    c == '/' ||
	    c == '\\' ||
	    c == '^' ||
	    c == '<' ||
	    c == '>' ||
	    c == '=' ||
	    c == '`' ||
	    c == '~' ||
	    c == '?' ||
	    c == '@' ||
	    c == '&' ||
	    c == '|' ||
	    c == '!' ||
	    c == '#' ||
	    c == '%' ||
	    c == '\'' ||
	    c == '\"' ||
	    c == '.' ||
	    c == ';'    );
}  /* symbol_char */

/*************
 *
 *   white_char()
 *
 *************/

static
BOOL white_char(char c)
{
  return (c == ' '  ||
	  c == '\t' ||  /* tab */
	  c == '\n' ||  /* newline */
	  c == '\v' ||  /* vertical tab */
	  c == '\r' ||  /* carriage return */
	  c == '\f');   /* form feed */
}  /* white_char */

/*************
 *
 *   white_or_comment()
 *
 *************/

static
BOOL white_or_comment(String_buf sb)
{
  int i = 0;
  BOOL ok = TRUE;
  int n = sb_size(sb);

  while (ok && i < n) {
    char c = sb_char(sb, i);
    if (white_char(c))
      i++;
    else if (comment_char(c)) {
      /* skip over comment */
      i++;
      while (i < n && sb_char(sb, i) != '\n')
	i++;
      if (i < n)
	i++;
    }
    else
      ok = FALSE;
  }
  return ok;
}  /* white_or_comment */

/*************
 *
 *   read_buf()
 *
 *   Read characters into buffer until one of the following:
 *      1.  END_CHAR is reached (which goes into the buffer)
 *          (except if in quoted string or comment).
 *      2.  EOF is reached (everything still goes into buffer).
 *
 *   Return:
 *       READ_BUF_OK
 *       READ_BUF_EOF           possible white space or comment, then EOF
 *       READ_BUF_ERROR         non-white space, noncomment then EOF
 *       READ_BUF_QUOTE_ERROR   no end to quoted string (EOF)
 *
 *************/

static
int read_buf(FILE *fp, String_buf sb)
{
  int c, end, eof, eof_q;

  end = eof = eof_q = 0;  /* stop conditions */

  while (!end && !eof && !eof_q) {

    c = getc(fp);

    if (c == EOF)
      eof = 1;
    else {
      sb_append_char(sb, c);
      if (end_char(c))
	end = 1;
      else if (quote_char(c)) {
	int qc = c;
	c = getc(fp);
	while (c != qc && c != EOF) {
	  sb_append_char(sb, c);
	  c = getc(fp);
	}
	if (c == EOF)
	  eof_q = 1;
	else
	  sb_append_char(sb, c);
      }
      else if (comment_char(c)) {
	c = getc(fp);
	while (c != '\n' && c != EOF) {
	  sb_append_char(sb, c);
	  c = getc(fp);
	}
	if (c == EOF)
	  eof = 1;
	else
	  sb_append_char(sb, c);
      }
    }
  }

  if (end)
    return READ_BUF_OK;
  else if (eof_q)
    return READ_BUF_QUOTE_ERROR;
  else {
    /* eof */
    if (white_or_comment(sb))
      return READ_BUF_EOF;
    else
      return READ_BUF_ERROR;
  }
}  /* read_buf */

/*************
 *
 *   tokenize()
 *
 *   Break up a string into a sequence of tokens.
 *
 *************/

static
Token tokenize(String_buf sb)
{
  int i = 0;
  char c = sb_char(sb, i);
  Token first, last, tok;
  first = last = NULL;

  while (!end_char(c) && c != '\0') {
    tok = get_token();  /* delete if not needed, i.e., white space */
    tok->buf_pos = i;

    /* Make sure that each case, when finished, sets c to the next char. */

    if (white_char(c)) {
      do {
	c = sb_char(sb, ++i);
      } while (white_char(c));
      free_token(tok);
      tok = NULL;
    }
    else if (punctuation_char(c)) {
      tok->type = TOK_PUNC;
      tok->c = c;
      c = sb_char(sb, ++i);
    }
    else if (name_char(c)) {
      tok->type = TOK_NAME;
      tok->sb = get_string_buf();
      while (name_char(c)) {
	sb_append_char(tok->sb, c);
	c = sb_char(sb, ++i);
      }
    }
    else if (symbol_char(c)) {
      tok->type = TOK_SYMBOL;
      tok->sb = get_string_buf();
      while (symbol_char(c)) {
	sb_append_char(tok->sb, c);
	c = sb_char(sb, ++i);
      }
    }
    else if (comment_char(c)) {
      tok->type = TOK_COMMENT;
      tok->sb = get_string_buf();
      while (c != '\n' && c != '\0') {
	sb_append_char(tok->sb, c);
	c = sb_char(sb, ++i);
      }
    }
    else if (quote_char(c)) {
      char qc = c;
      tok->type = TOK_STRING;
      tok->sb = get_string_buf();
      sb_append_char(tok->sb, c);
      do {
	c = sb_char(sb, ++i);
	sb_append_char(tok->sb, c);
      } while (c != qc && c != '\0');
      if (c == qc)
	c = sb_char(sb, ++i);
    }
    else {
      tok->type = TOK_UNKNOWN;
      tok->c = c;
      c = sb_char(sb, ++i);
    }

    if (tok != NULL) {
      if (first == NULL)
	first = tok;
      else
	last->next = tok;
      last = tok;
    }
  }  /* while */
  return first;
}  /* tokenize */

/*************
 *
 *   comma_terms()
 *
 *************/

static
int comma_terms(Term t)
{
  if (ARITY(t) == 0 || !is_symbol(SYMNUM(t), ",", 2))
    return 1;
  else
    return comma_terms(t->args[0]) + comma_terms(t->args[1]);
}  /* comma_terms */

/*************
 *
 *   transfer_comma_term()
 *
 *************/

static
void transfer_comma_term(Term t, Term dest, int *p)
{
  if (ARITY(t) == 0 || !is_symbol(SYMNUM(t), ",", 2)) {
    dest->args[*p] = copy_term(t);
    (*p)++;
  }
  else {
    transfer_comma_term(t->args[0], dest, p);
    transfer_comma_term(t->args[1], dest, p);
  }
}  /* transfer_comma_term */

/*************
 *
 *   quantifier()
 *
 *************/

static
BOOL quantifier(Term t)
{
  return (is_symbol(SYMNUM(t), ALL_SYM,    0) ||
	  is_symbol(SYMNUM(t), EXISTS_SYM, 0));
}  /* quantifier */

/* reference for mutual recursion */

static Term terms_to_term(Pterm start, Pterm end, int m);

/*************
 *
 *   terms_to_quant_term()
 *
 *   Try to build a quantified term from a sequence of terms.
 *   A quantified term is like this $quantified([all,x,all,y,exists,z,term]).
 *   Why the "$quantified"?  Because we wish
 *   to recognize it as a special term for printing and transformations.
 *   Note that, different from previous versions, each variable needs
 *   its own quantifier.
 *
 *************/

static
Term terms_to_quant_term(Pterm start, Pterm end)
{
  Pterm curr = start;
  int vars_ok = 1;
  while (curr != NULL && quantifier(curr->t) && vars_ok) {
    curr = curr->next;
    if (curr != NULL && ARITY(curr->t) == 0)
      curr = curr->next;
    else
      vars_ok = 0;
  }
  if (vars_ok && curr != NULL) {
    Term formula = terms_to_term(curr, end, 1000);
    if (formula == NULL)
      return NULL;
    else {
      /* We have a good quantifier prefix and formula.
       * Build and return the quantified term.
       */
      Pterm p;
      Term q = get_rigid_term("$quantified", 1);
      Term t = get_nil_term();

      /* build list from the back */
      t = listterm_cons(formula, t);
      for (p = curr->prev; p != NULL; p = p->prev) {
	t = listterm_cons(copy_term(p->t), t);
      }
      q->args[0] = t;
      return q;
    }
  }
  else
    return NULL;
}  /* terms_to_quant_term */

/*************
 *
 *   terms_to_term()
 *
 *   This routine attempts to construct a term
 *   starting with start, ending with end, with precedence <= m.
 *   On success, the resulting term is an entirely new copy.
 *
 *************/

static
Term terms_to_term(Pterm start, Pterm end, int m)
{
  if (start == end) {
    if (is_symbol(SYMNUM(start->t), ",", 0))
      return NULL;  /* don't allow commas as constants */
    else
      return copy_term(start->t);
  }
  else {
    int rc, prec;
    Parsetype type;
    char *str;

    /* Try for prefix op; return if successful. */
    if (ARITY(start->t) == 0) {
      str = sn_to_str(SYMNUM(start->t));
      rc = unary_parse_type(str, &prec, &type);
      if (rc && prec <= m &&
	  (type == PREFIX_PAREN || type == PREFIX)) {
	int p = (type == PREFIX_PAREN ? prec-1 : prec);
	Term t1 = terms_to_term(start->next, end, p);
	if (t1 != NULL) {
	  Term t = get_rigid_term(str, 1);
	  t->args[0] = t1;
	  return t;
	}
      }
    }

    /* Try for postfix op; return if successful. */
    if (ARITY(end->t) == 0) {
      str = sn_to_str(SYMNUM(end->t));
      rc = unary_parse_type(str, &prec, &type);
      if (rc && prec <= m &&
	  (type == POSTFIX_PAREN || type == POSTFIX)) {
	int p = (type == POSTFIX_PAREN ? prec-1 : prec);
	Term t1 = terms_to_term(start, end->prev, p);
	if (t1 != NULL) {
	  Term t = get_rigid_term(str, 1);
	  t->args[0] = t1;
	  return t;
	}
      }
    }

    /* Try for an application; return if successful. */
    if (start->next == end && end->possible_application &&
	ARITY(start->t) == 0 && !is_symbol(SYMNUM(start->t), ",", 0)) {
      Term t;
      int arity = comma_terms(end->t);
      int argnum = 0;
      t = get_rigid_term(sn_to_str(SYMNUM(start->t)), arity);
      transfer_comma_term(end->t, t, &argnum);
      return t;
    }

    /* Try for quantified formula; return if successful.
     * Allow this only if we are not in a recursive call.
     */
    if (start->next != end && m == 1000) {
      if (quantifier(start->t)) {
	Term t = terms_to_quant_term(start, end);
	if (t != NULL)
	  return t;
      }
    }

    /* Try for infix op; return if successful. */
    if (start->next != end) {
      /* Try each possible infix op, until success or exhausted. */
      Pterm op;
      int backward = 0;

      /* If we parse a long left-associated expression left-to-right,
       * it ends up trying all the different associations before finding
       * the correct one.  Therefore, as a heuristic, if the second
       * symbol is INFIX_LEFT, then we try to parse backward.  This
       * doesn't always work well, for example, with & right and + left,
       * a&...&a -> a+...+a (symmetric shape) is slow both forward
       * and backward.  To speed things up, the user can include
       * parentheses, i.e., (a&...&a) -> (a+...+a).
       */

      if (ARITY(start->next->t) == 0) {
	str = sn_to_str(SYMNUM(start->next->t));
	rc = binary_parse_type(str, &prec, &type);
	backward = (rc && type == INFIX_LEFT);
      }

      op = (backward ? end->prev : start->next);
      while (backward ? op != start : op != end) {
	if (ARITY(op->t) == 0) {
	  str = sn_to_str(SYMNUM(op->t));
	  rc = binary_parse_type(str, &prec, &type);
	  if (rc && prec <= m) {
	    Term t1, t2;
	    int p1 = (type == INFIX || type == INFIX_RIGHT ? prec-1 : prec);
	    int p2 = (type == INFIX || type == INFIX_LEFT  ? prec-1 : prec);
	    
	    t1 = terms_to_term(start, op->prev, p1);
	    if (t1 != NULL) {
	      t2 = terms_to_term(op->next, end, p2);
	      if (t2 == NULL)
		zap_term(t1);
	      else {
		Term t = get_rigid_term(str, 2);
		t->args[0] = t1;
		t->args[1] = t2;
		return t;
	      }
	    }
	  }
	}  /* arity 0 */
	op = (backward ? op->prev : op->next);
      }  /* while (binary attempts) */
    }
    /* nothing works */
    return NULL;
  }  /* start != end */
}  /* terms_to_term */

/*************
 *
 *   next_token()
 *
 *   This routine is called when it's time to move to the next token.
 *   The current token (including any sb) is deleted.  Don't call this
 *   routine if an error is found; instead, set the error message ane
 *   return NULL (from whereever you are).
 *
 *************/

static
void next_token(Tok_pos p)
{
  Token tok = p->tok;
  p->tok = p->tok->next;
  if (tok->sb != NULL)
    zap_string_buf(tok->sb);
  free_token(tok);
}  /* next_token */

/* reference for mutual recursion */

static Term toks_to_term(Tok_pos p);

/*************
 *
 *   toks_to_set()
 *
 *************/

static
Term toks_to_set(Tok_pos p)
{
  p->error_message = "set parsing not done";
  return NULL;
}  /* toks_to_set */

/*************
 *
 *   make_a_list()
 *
 *   Prepend, to tail, copies of comma-elements in t.
 *
 *************/

static
Term make_a_list(Term t, Term tail)
{
  if (ARITY(t) == 0 || !is_symbol(SYMNUM(t), ",", 2)) {
    return listterm_cons(copy_term(t), tail);
  }
  else {
    Term l = make_a_list(t->args[1], tail);
    return make_a_list(t->args[0], l);
  }
}  /* make_a_list */

/*************
 *
 *   toks_to_list()
 *
 *   On entry, current token is [.
 *   On successful exit, current token should be ].
 *
 *************/

static
Term toks_to_list(Tok_pos p)
{
  /* Assume current token is "[". */
  next_token(p);
  if (p->tok != NULL && p->tok->c == ']') {
    return get_nil_term();
  }
  else {
    Term cterm = toks_to_term(p);  /* a comma-term */
    if (cterm == NULL)
      return NULL;
    else if (p->tok == NULL ||
	     p->tok->type != TOK_PUNC ||
	     (p->tok->c != ']' && p->tok->c != ':')) {
      p->error_message = "\']\' or \':\' expected in list";
      zap_term(cterm);
      return NULL;
    }
    else if (p->tok->type == TOK_PUNC && p->tok->c == ':') {
      Term tail;
      next_token(p);
      tail = toks_to_term(p);
      if (tail == NULL)
	return NULL;
      else if (p->tok == NULL ||
	       p->tok->type != TOK_PUNC || p->tok->c != ']') {
	p->error_message = "\']\' expected in list";
	zap_term(cterm);
	zap_term(tail);
	return NULL;
      }
      else {
	Term list = make_a_list(cterm, tail);
	zap_term(cterm);
	return list;
      }
    }
    else {
      /* current token is ']' */
      Term list = make_a_list(cterm, get_nil_term());
      zap_term(cterm);
      return list;
    }
  }
}  /* toks_to_list */

/*************
 *
 *   toks_to_terms()
 *
 *************/

static
Pterm toks_to_terms(Tok_pos p)
{
  Term t;
  int done = 0;
  int error = 0;
  int possible_application;
  int comma_constant;
  Pterm first = NULL;
  Pterm last = NULL;
  Pterm new;

  while (!done && !error) {
    possible_application = 0;
    comma_constant = 0;
    t = NULL;
    
    if (p->tok->type == TOK_NAME ||
	p->tok->type == TOK_SYMBOL ||
	p->tok->type == TOK_STRING ) {
      char *str = sb_to_new_string(p->tok->sb);
      t = get_rigid_term(str, 0);
      free(str);
    }
    else if (p->tok->type == TOK_PUNC && p->tok->c == ',') {
      /* Special case: comma is both punctuation and operator. */
      t = get_rigid_term(",", 0);
      comma_constant = 1;
    }
    else if (p->tok->type == TOK_PUNC) {
      if (p->tok->c == '(') {
	next_token(p);
	t = toks_to_term(p);
	if (t == NULL)
	  error = 1;
	else if (p->tok == NULL || p->tok->c != ')') {
	  p->error_message = "closing parenthesis expected";
	  zap_term(t);
	  t = NULL;
	  error = 1;
	}
	else
	  possible_application = 1;
      }
      else if (p->tok->c == '[') {
	t = toks_to_list(p);
	error = (t == NULL);
      }
      else if (p->tok->c == '{') {
	t = toks_to_set(p);
	error = (t == NULL);
      }
      else {
	/* bad punctuation */
	p->error_message = "bad punctuation character";
	error = 1;
      }
    }
    else if (p->tok->type == TOK_COMMENT) {
      ;  /* do nothing */
    }
    else if (p->tok->type == TOK_UNKNOWN) {
      p->error_message = "bad character";
      error = 1;
    }

    if (t != NULL) {
      /* Add a node to the terms list. */
      new = get_pterm();
      new->prev = last;
      if (first == NULL)
	first = new;
      else
	last->next = new;
      new->t = t;
      new->possible_application = possible_application;
      last = new;
    }

    if (!error) {
      /*  */
      next_token(p);
      if (p->tok == NULL)
	done = 1;
      else if(p->tok->type == TOK_PUNC &&
	      (p->tok->c == ')' || p->tok->c == ']' ||
	       p->tok->c == '}' || p->tok->c == ':'))
	done = 1;
    }

  }  /* while */

  if (error) {
    free_pterm_list(first);
    return NULL;
  }
  else
    return first;
}  /* toks_to_terms */

/*************
 *
 *   toks_to_term()
 *
 *************/

static
Term toks_to_term(Tok_pos p)
{
  Term t;
  Pterm terms;

  terms = toks_to_terms(p);
  if (terms == NULL)
    t = NULL;
  else {
    Pterm end;
    for (end = terms; end->next != NULL; end = end->next);
    t = terms_to_term(terms, end, 1000);
    free_pterm_list(terms);
    if (t == NULL) {
      p->error_message = "terms/ops could not be combined into a term";
    }
  }
  return t;
}  /* toks_to_term */

/*************
 *
 *   fprint_parse_error()
 *
 *************/

static
void fprint_parse_error(FILE *fp, char *msg, String_buf sb, int position)
{
  int i;
  int n = sb_size(sb);
  fprintf(fp, "Error parsing input string, %s:\n", msg);
  for (i = 0; i < position; i++)
    fprintf(fp, "%c", sb_char(sb, i));
  fprintf(fp, " **HERE** ");
  for (i = position; i < n; i++)
    fprintf(fp, "%c", sb_char(sb, i));
  fprintf(fp, "\n");
}  /* fprint_parse_error */

/*************
 *
 *   sread_term()
 *
 *************/

/* DOCUMENTATION
This routine reads a term (from String_buf *sb).  The term may be
in readable form, that is with infix operations and without
extra parentheses.
<P>
If there is no term to be read, NULL is returned.  If an error
occurs, a message is sent to FILE *fout, and fatal error occurs.
<P>
See the documentation on mixfix terms and the routine set_parse_type().
*/

/* PUBLIC */
Term sread_term(String_buf sb, FILE *fout)
{
  Token tokens;
  Term t;
  struct tok_pos tp;
  tokens = tokenize(sb);
  if (tokens == NULL)
    fatal_error("sread_term, empty term (too many periods?)");
  tp.tok = tokens;
  tp.error_message = "";
  t = toks_to_term(&tp);
  if (t == NULL || tp.tok != NULL) {
    int pos;
    if (t != NULL)
      tp.error_message = "characters after complete term";

    pos = (tp.tok != NULL ? tp.tok->buf_pos : sb_size(sb)-1);
    fprint_parse_error(fout, tp.error_message, sb, pos);
    free_token_list(tp.tok);
    fatal_error("sread_term error");
  }

  return t;
}  /* sread_term */

/*************
 *
 *   read_term()
 *
 *************/

/* DOCUMENTATION
This routine reads a term (from FILE *fin).  The term may be
in readable form, that is with infix operations and without
extra parentheses.
<P>
If there is no term to be read, NULL is returned.  If an error
occurs, a message is sent to FILE *fout, and fatal error occurs.
<P>
See the documentation on mixfix terms and the routine set_parse_type().
*/

/* PUBLIC */
Term read_term(FILE *fin, FILE *fout)
{
  Read_rc rc;
  String_buf sb = get_string_buf();

  rc = read_buf(fin, sb);

  if (rc == READ_BUF_EOF) {
    zap_string_buf(sb);
    return NULL;
  }
  else if (rc != READ_BUF_OK) {
    char *msg;
    switch (rc) {
    case READ_BUF_ERROR:
      msg = "EOF found while reading term (missing period?)";
      break;
    case READ_BUF_QUOTE_ERROR:
      msg = "EOF found while reading quoted string";
      break;
    default:
      msg = "error reading characters from file";
      break;
    }
    fprint_parse_error(fout, msg, sb, sb_size(sb));
    zap_string_buf(sb);
    fatal_error("read_term error");
    return NULL;  /* to please the complier */
  }
  else {
    Term t = sread_term(sb, fout);
    zap_string_buf(sb);
    if (Translate_neg_equalities)
      t = translate_neg_eq(t);
    return t;
  }
}  /* read_term */

/*************
 *
 *   quantified_term()
 *
 *************/

static
BOOL quantified_term(Term t)
{
  return (COMPLEX(t) &&
	  is_symbol(SYMNUM(t), "$quantified", 1) &&
	  listterm_length(t->args[0]) >= 3);
}  /* quantified_term */

/*************
 *
 *   arrange_term()
 *
 *************/

static
void arrange_term(String_buf sb, Term t, int par_prec)
{
  if (t == NULL)
    sb_append(sb, "arrange_term gets NULL term");
  else if (VARIABLE(t)) {
    char str[100];
    if (variable_style() == INTEGER_STYLE)
      sprintf(str, "%d", VARNUM(t));
    else if (variable_style() == PROLOG_STYLE) {
      /* A,B,C,D,E,F,V6,V7,V8,... */
      if (VARNUM(t) < 6)
	sprintf(str, "%c", 'A' + VARNUM(t));
      else
	sprintf(str, "V%d", VARNUM(t));
    }
    else {
      /* x,y,z,u,v,w,v6,v7,v8,... */
      if (VARNUM(t) < 3)
	sprintf(str, "%c", 'x' + VARNUM(t));
      else if (VARNUM(t) < 6)
	sprintf(str, "%c", 'r' + VARNUM(t));
      else
	sprintf(str, "v%d", VARNUM(t));
    }
    sb_append(sb, str);
  }  /* variable */
  else if (CONSTANT(t)) {
    if (nil_term(t))
      sb_append(sb, "[]");
    else
      sb_append(sb, sn_to_str(SYMNUM(t)));
  }  /* constant */

  else if (quantified_term(t)) {
    Term t1 = t->args[0];
    sb_append(sb, "(");
    while (cons_term(t1->args[1])) {
      arrange_term(sb, t1->args[0], 1000);
      sb_append(sb, " ");
      t1 = t1->args[1];
    }
    sb_append(sb, "(");
    arrange_term(sb, t1->args[0], 1000);
    sb_append(sb, ")");
    sb_append(sb, ")");
  }  /* quantified */

  else if (cons_term(t)) {
    Term t1 = t;
    sb_append(sb, "[");
    while (cons_term(t1)) {
      arrange_term(sb, t1->args[0], 1000);
      t1 = t1->args[1];
      if (cons_term(t1))
	sb_append(sb, ",");
    }
    if (!nil_term(t1)) {
      sb_append(sb, ":");
      arrange_term(sb, t1, 1000);
    }
    sb_append(sb, "]");
  }  /* cons_term */

  else {
    Parsetype type;
    int op_prec;
    if (ARITY(t) == 2 &&
	binary_parse_type(sn_to_str(SYMNUM(t)), &op_prec, &type)) {
      int p1 = (type == INFIX || type == INFIX_RIGHT ? op_prec-1 : op_prec);
      int p2 = (type == INFIX || type == INFIX_LEFT  ? op_prec-1 : op_prec);
		
      if (op_prec > par_prec)
	sb_append(sb, "(");
      arrange_term(sb, t->args[0], p1);
      sb_append(sb, " ");
      sb_append(sb, sn_to_str(SYMNUM(t)));
      sb_append(sb, " ");
      arrange_term(sb, t->args[1], p2);
      if (op_prec > par_prec)
	sb_append(sb, ")");
    }  /* binary infix */
    else if (ARITY(t) == 1 &&
	     unary_parse_type(sn_to_str(SYMNUM(t)), &op_prec, &type)) {

      int p1 = (type == PREFIX_PAREN ||
		type == POSTFIX_PAREN ? op_prec-1 : op_prec);

      if (op_prec > par_prec)
	sb_append(sb, "(");

      if (type == PREFIX_PAREN || type == PREFIX) {
	sb_append(sb, sn_to_str(SYMNUM(t)));
	sb_append(sb, " ");
	arrange_term(sb, t->args[0], p1);
      }  /* prefix */
      else {
	arrange_term(sb, t->args[0], p1);
	sb_append(sb, " ");
	sb_append(sb, sn_to_str(SYMNUM(t)));
      }  /* postfix */

      if (op_prec > par_prec)
	sb_append(sb, ")");

    }  /* unary prefix or postfix */
    else {
      /* ordinary application */
      int i;
      sb_append(sb, sn_to_str(SYMNUM(t)));
      sb_append(sb, "(");
      for (i = 0; i < ARITY(t); i++) {
	arrange_term(sb, t->args[i], 1000);
	if (i < ARITY(t)-1)
	  sb_append(sb, ",");
      }
      sb_append(sb, ")");
    }  /* ordinary application */
  }
}  /* arrange_term */

/*************
 *
 *   fwrite_term()
 *
 *************/

/* DOCUMENTATION
This routine prints a term (to FILE *fp) in readable form,
that is, infix where appropriate and without extra parentheses.
A period and newline are <I>not</I> printed.
<P>
See the documentation on mixfix terms and the routine set_parse_type().
*/

/* PUBLIC */
void fwrite_term(FILE *fp, Term t)
{
  String_buf sb = get_string_buf();
  if (Translate_neg_equalities) {
    Term temp_term = untranslate_neg_eq(copy_term(t));
    arrange_term(sb, temp_term, 1000);
    zap_term(temp_term);
  }
  else
    arrange_term(sb, t, 1000);
  fprint_sb(fp, sb);
  zap_string_buf(sb);
}  /* fwrite_term */

/*************
 *
 *   fwrite_term_nl()
 *
 *************/

/* DOCUMENTATION
This routine prints a term (to FILE *fp) in readable form,
that is, infix where appropriate and without extra parentheses.
Also printed is ".\n".
<P>
See the documentation on mixfix terms and the routine set_parse_type().
*/

/* PUBLIC */
void fwrite_term_nl(FILE *fp, Term t)
{
  fwrite_term(fp, t);
  fprintf(fp, ".\n");
}  /* fwrite_term_nl */

/*************
 *
 *   declare_standard_parse_types()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_standard_parse_types(void)
{
  set_parse_type(",",        999, INFIX_RIGHT);  /* essential */
			    
  set_parse_type(ATTRIB_SYM, 810, INFIX_RIGHT);
			    
  set_parse_type(IFF_SYM,    800, INFIX);
  set_parse_type(IMP_SYM,    800, INFIX);
  set_parse_type(IMPBY_SYM,  800, INFIX);
  set_parse_type(OR_SYM,     790, INFIX_RIGHT);
  set_parse_type(AND_SYM,    780, INFIX_RIGHT);
			    
  set_parse_type(EQ_SYM,     700, INFIX);
  set_parse_type(NEQ_SYM,    700, INFIX);
  set_parse_type("==",       700, INFIX);
  set_parse_type("<",        700, INFIX);
  set_parse_type("<=",       700, INFIX);
  set_parse_type(">",        700, INFIX);
  set_parse_type(">=",       700, INFIX);
			    
  set_parse_type("+",        600, INFIX);
  set_parse_type("*",        500, INFIX);

  set_parse_type("/",        500, INFIX);
  set_parse_type("\\",       500, INFIX);
  set_parse_type("^",        500, INFIX);
  set_parse_type("v",        500, INFIX);

  set_parse_type(NOT_SYM,    400, PREFIX);
  set_parse_type("\'",       300, POSTFIX);

}  /* declare_standard_parse_types */
