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

#include "parse.h"

/* Private definitions and types */

#define COMMENT_CHAR '%'

static char Cons_char = ':';
static char Quote_char = '\"';
static int Quantifier_precedence = 0;  /* see declare_quantifier_precedence */

static BOOL Parenthesize = FALSE;
static BOOL Check_for_illegal_symbols = TRUE;
static BOOL Simple_parse = FALSE;

/* Token types */

typedef enum {TOK_UNKNOWN,  /* probably a garbage binary char */
	      TOK_ORDINARY, /* see ordinary_char() */
	      TOK_SPECIAL,  /* see special_char() */
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
  char       c;        /* for punctuation and unknown tokens */
  String_buf sb;       /* for other tokens */
  int        buf_pos;  /* position of this token in buffer */
  Token      next;
};

/* A list of terms with some other data. */

typedef struct pterm * Pterm;

struct pterm {
  Term  t;
  BOOL  parenthesized;  /* prevents "a b" being parsed as a(b) */
  Pterm prev, next;
};

/* Token position */

typedef struct tok_pos *Tok_pos;

struct tok_pos {
  Token tok;
  char *error_message;
  int start_error, end_error;
};

/* Private variables */

static BOOL   Translate_neg_equalities = FALSE;
static Plist  Multiple_char_special_syms = NULL;

/*
 * memory management
 */

#define PTRS_TOKEN PTRS(sizeof(struct token))
static unsigned Token_gets, Token_frees;

#define PTRS_PTERM PTRS(sizeof(struct pterm))
static unsigned Pterm_gets, Pterm_frees;

/*************
 *
 *   Token get_token()
 *
 *************/

static
Token get_token(void)
{
  Token p = get_cmem(PTRS_TOKEN);
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
  Pterm p = get_cmem(PTRS_PTERM);
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

  n = sizeof(struct token);
  fprintf(fp, "token (%4d)        %11u%11u%11u%9.1f K\n",
          n, Token_gets, Token_frees,
          Token_gets - Token_frees,
          ((Token_gets - Token_frees) * n) / 1024.);

  n = sizeof(struct pterm);
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
void p_parse_mem(void)
{
  fprint_parse_mem(stdout, TRUE);
}  /* p_parse_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   p_tokens()
 *
 *************/

#if 0
static
void p_tokens(Token t)
{
  Token p;
  for (p = t; p; p = p->next) {
    if (p->sb == NULL)
      printf("%c", p->c);
    else
      fprint_sb(stdout, p->sb);
    printf(" ");
  }
  printf("\n\n");
}  /* p_tokens */
#endif

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
    if (is_symbol(SYMNUM(t), neq_sym(), 2)) {
      Term eq_term = get_rigid_term(eq_sym(), 2);
      Term not_term = get_rigid_term(not_sym(), 1);
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
    if (is_symbol(SYMNUM(t), not_sym(), 1) &&
	is_symbol(SYMNUM(ARG(t,0)), eq_sym(), 2)) {
      Term neq_term = get_rigid_term(neq_sym(), 2);
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
    return (c == COMMENT_CHAR);
}  /* comment_char */

/*************
 *
 *    quote_char()
 *
 *************/

static
BOOL quote_char(char c)
{
    return (c == Quote_char);
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
	    c == Cons_char ||
	    c == '(' || c == ')' ||
	    c == '[' || c == ']' ||
	    c == '{' || c == '}' ||
	    
	    c == '.');
}  /* punctuation_char */

/*************
 *
 *    ordinary_char()
 *
 *************/

static
BOOL ordinary_char(char c)
{
    return ((c >= '0' && c <= '9') ||
	    (c >= 'a' && c <= 'z') ||
	    (c >= 'A' && c <= 'Z') ||
	    c == '_' ||
	    c == '$');
}  /* ordinary_char */

/*************
 *
 *    special_char(c)
 *
 *************/

static
BOOL special_char(char c)
{
  /* This allows us to have special chars in the list below. */
  if (quote_char(c) || end_char(c) || comment_char(c) || punctuation_char(c))
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
	    c == ':' ||
	    c == '!' ||
	    c == '#' ||
	    c == '%' ||
	    c == '\'' ||
	    c == '\"' ||
	    c == '.' ||
	    c == ';'    );
}  /* special_char */

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
 *   all_whitespace()
 *
 *************/

static
BOOL all_whitespace(String_buf sb)
{
  int i = 0;
  BOOL ok = TRUE;
  int n = sb_size(sb);

  while (ok && i < n) {
    char c = sb_char(sb, i);
    if (white_char(c))
      i++;
    else
      ok = FALSE;
  }
  return ok;
}  /* all_whitespace */

/*************
 *
 *   finish_comment()
 *
 *   Return the last character read:
 *
 *      EOF             file ended before comment ended
 *      \n              end of line-comment
 *      COMMENT_CHAR    end of block-comment
 *
 *************/

static
int finish_comment(FILE *fp)
{
  int c;

  if (((c = getc(fp)) == 'B') && 
      ((c = getc(fp)) == 'E') && 
      ((c = getc(fp)) == 'G') && 
      ((c = getc(fp)) == 'I') && 
      ((c = getc(fp)) == 'N')) {

    /* We have a block comment.  Read up to 'END%' */

    while (TRUE) {
      while (c != 'E' && c != EOF)
	c = getc(fp);
      if (c == EOF)
	return EOF;
      else if (((c = getc(fp)) == 'N') && 
	       ((c = getc(fp)) == 'D') && 
	       ((c = getc(fp)) == COMMENT_CHAR))
	return COMMENT_CHAR;
      else if (c == EOF)
	return EOF;
    }
  }
  else {
    /* We have a line comment. */
    while (c != '\n' && c != EOF)
      c = getc(fp);
  }
  return c;
}  /* finish_comment */

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
  int c;                 /* character read */
  BOOL end, eof, eof_q;  /* stop conditions */

  end = eof = eof_q = FALSE;  

  while (!end && !eof && !eof_q) {

    c = getc(fp);

    if (c == EOF)
      eof = TRUE;
    else if (comment_char(c)) {
      c = finish_comment(fp);
      if (c == EOF)
	eof = TRUE;
    }
    else {
      sb_append_char(sb, c);
      if (end_char(c))
	end = TRUE;
      else if (quote_char(c)) {  /* quoted string */
	int qc = c;
	c = getc(fp);
	while (c != qc && c != EOF) {
	  sb_append_char(sb, c);
	  c = getc(fp);
	}
	if (c == EOF)
	  eof_q = TRUE;
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
    /* eof -- make sure that the only things in the buffer are whitespace */
    
    if (all_whitespace(sb)) 
      return READ_BUF_EOF;
    else
      return READ_BUF_ERROR;
  }
}  /* read_buf */

/*************
 *
 *   max_special_symbol()
 *
 *   token is maximal sequence that is a known special symbol
 *
 *************/

static
String_buf max_special_symbol(String_buf sb, int *ip)
{
  String_buf tok_sb;
  int m = longest_string_in_plist(Multiple_char_special_syms);
  char *work = malloc(m+1);
  int n = 0;
  int i = *ip;
  BOOL ok = FALSE;

  /* Get the longest possible token. */

  char c = sb_char(sb,i++);
  while (special_char(c) && n < m) {
    work[n++] = c;
    c = sb_char(sb,i++);
  }
  i--;
  work[n] = '\0';

  /* Keep chopping one from the end until a known special symbol is found.
     If none is known, the token is a single character. */

  while (!ok && n > 1) {
    /* printf("checking symbol :%s:\n", work); fflush(stdout); */
    ok = (position_of_string_in_plist(work,Multiple_char_special_syms) != -1);
    if (!ok) {
      n--;
      work[n] = '\0';
      i--;
    }
  }

  tok_sb = init_string_buf(work);
  free(work);
  *ip = i;
  return tok_sb;
}  /* max_special_symbol */

/*************
 *
 *   tokenize()
 *
 *   Break up a string into a sequence of tokens, including
 *   punctuation tokens such as parentheses and commas.
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
    else if (ordinary_char(c)) {
      tok->type = TOK_ORDINARY;
      tok->sb = get_string_buf();
      while (ordinary_char(c)) {
	sb_append_char(tok->sb, c);
	c = sb_char(sb, ++i);
      }
    }
    else if (special_char(c)) {
      tok->type = TOK_SPECIAL;
      if (Simple_parse) {
	/* token is maximal sequence of special chars */
	tok->sb = get_string_buf();
	while (special_char(c)) {
	  sb_append_char(tok->sb, c);
	  c = sb_char(sb, ++i);
	}
      }
      else {
	/* token is maximal sequence that is a known special symbol */
	tok->sb = max_special_symbol(sb, &i);
	c = sb_char(sb, i);
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
    return comma_terms(ARG(t,0)) + comma_terms(ARG(t,1));
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
    ARG(dest,*p) = copy_term(t);
    (*p)++;
  }
  else {
    transfer_comma_term(ARG(t,0), dest, p);
    transfer_comma_term(ARG(t,1), dest, p);
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
  return (is_symbol(SYMNUM(t), all_sym(),    0) ||
	  is_symbol(SYMNUM(t), exists_sym(), 0));
}  /* quantifier */

/*************
 *
 *   ordinary_constant_string()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL ordinary_constant_string(char *s)
{
  while (ordinary_char(*s))
    s++;
  return *s == '\0';
}  /* ordinary_constant_string */

/*************
 *
 *   ordinary_constant()
 *
 *************/

static
BOOL ordinary_constant(Term t)
{
  if (ARITY(t) != 0)
    return FALSE;
  else
    return ordinary_constant_string(sn_to_str(SYMNUM(t)));
}  /* ordinary_constant */

/*************
 *
 *   quant_prefix()
 *
 *************/

static
BOOL quant_prefix(Pterm start, Pterm end)
{
  return
    quantifier(start->t) &&
    start->next != end &&
    ordinary_constant(start->next->t);
}  /* quant_prefix */

/*************
 *
 *   terms_to_term()
 *
 *   This routine takes a sequence of terms/symbols, and attempts
 *   to construct a term with precedence <= m.
 *   
 *   On success, the resulting term is an entirely new copy.
 *
 *************/

static
Term terms_to_term(Pterm start, Pterm end, int m)
{
  if (start == end) {
    if (is_term(start->t, ",", 0))
      return NULL;  /* don't allow commas as constants */
    else if (ARITY(start->t) == 0 &&
	     special_parse_type(sn_to_str(SYMNUM(start->t))) == 1 &&
	     m < 998)
      /* Don't allow prefix or postfix as constants.  This prevents expressions
	 that would otherwise be ambiguous, like -v(a) and -v&(a->b),
	 where - is prefix and v is infix.

         An exception: allow them as members of comma-list, e.g., f(-), [-],
         f(a,_), [a,-].  This allows lex terms, e.g., lex([-,a,b,f,*]).
       */
      return NULL;
    else
      return copy_term(start->t);
  }
  else {
    int rc, prec;
    Parsetype type;
    char *str;

    /* Try for quantified formula; return if successful. */
    if (Quantifier_precedence <= m && quant_prefix(start, end)) {
      Term t1 = terms_to_term(start->next->next, end, Quantifier_precedence);
      if (t1 != NULL) {
	Term t = get_rigid_term(quant_sym(), 3);
	ARG(t,0) = copy_term(start->t);
	ARG(t,1) = copy_term(start->next->t);
	ARG(t,2) = t1;
	return t;
      }
    }

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
	  ARG(t,0) = t1;
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
	  ARG(t,0) = t1;
	  return t;
	}
      }
    }

    /* Try for an application, e.g., f(a,b); return if successful. */

    if (start->next == end && ARITY(start->t) == 0 &&
	!is_term(start->t, ",", 0) && end->parenthesized) {
	
      int num_args = comma_terms(end->t);  /* number of args for application */
	       
      int argnum = 0;
      Term t = get_rigid_term(sn_to_str(SYMNUM(start->t)), num_args);
      transfer_comma_term(end->t, t, &argnum);
      return t;
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
       * doesn't always work efficiently, for example, with & right
       * and + left, a&...&a -> a+...+a (symmetric shape) is slow both
       * forward and backward.  To speed things up, the user can include
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
	  /* If "v" is infix, prevent "(v)" from being infix. */
	  if (rc && prec <= m && !op->parenthesized) {
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
		ARG(t,0) = t1;
		ARG(t,1) = t2;
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
 *   routine if an error is found; instead, set the error message and
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
  p->error_message = "Set parsing is not available (see end of marked string)";
  p->start_error = 0;
  p->end_error = p->tok->buf_pos;
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
    Term l = make_a_list(ARG(t,1), tail);
    return make_a_list(ARG(t,0), l);
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
  int start_pos = p->tok->buf_pos;
  next_token(p);
  if (p->tok == NULL) {
    p->error_message = "Extra open bracket \'[\'";
    p->start_error = start_pos;
    p->end_error = -1;
    return NULL;
  }
  else if (p->tok->c == ']') {
    return get_nil_term();
  }
  else {
    Term cterm = toks_to_term(p);  /* a comma-term */
    if (cterm == NULL)
      return NULL;
    else if (p->tok == NULL ||
	     p->tok->type != TOK_PUNC ||
	     (p->tok->c != ']' && p->tok->c != Cons_char)) {
      p->error_message = "Character \']\' or \':\' expected in list";
      p->start_error = start_pos;
      p->end_error = p->tok->buf_pos;
      zap_term(cterm);
      return NULL;
    }
    else if (p->tok->type == TOK_PUNC && p->tok->c == Cons_char) {
      Term tail;
      next_token(p);
      tail = toks_to_term(p);
      if (tail == NULL)
	return NULL;
      else if (p->tok == NULL ||
	       p->tok->type != TOK_PUNC || p->tok->c != ']') {
	p->error_message = "Character \']\' expected in list";
	p->start_error = start_pos;
	p->end_error = p->tok->buf_pos;
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
  BOOL done = FALSE;
  BOOL error = FALSE;
  BOOL parenthesized;
  Pterm first = NULL;
  Pterm last = NULL;
  Pterm new;
  int start_pos;

  while (!done && !error) {
    parenthesized = FALSE;
    t = NULL;
    start_pos = p->tok->buf_pos;
    
    if (p->tok->type == TOK_ORDINARY ||
	p->tok->type == TOK_SPECIAL ||
	p->tok->type == TOK_STRING ) {
      char *str = sb_to_malloc_string(p->tok->sb);
      t = get_rigid_term(str, 0);
      free(str);
    }
    else if (p->tok->type == TOK_PUNC && p->tok->c == ',') {
      /* Special case: comma is both punctuation and operator. */
      t = get_rigid_term(",", 0);
    }
    else if (p->tok->type == TOK_PUNC) {
      if (p->tok->c == '(') {
	next_token(p);
	if (p->tok == NULL) {
	  p->error_message = "Extra open parenthesis";
	  p->start_error = start_pos;
	  p->end_error = -1;
	  error = TRUE;
	}
	else {
	  t = toks_to_term(p);
	  if (t == NULL)
	    error = TRUE;
	  else if (p->tok == NULL || p->tok->c != ')') {
	    p->error_message = "Closing parenthesis expected";
	    p->start_error = start_pos;
	    p->end_error = (p->tok ? p->tok->buf_pos : -1);
	    zap_term(t);
	    t = NULL;
	    error = TRUE;
	  }
	  else
	    parenthesized = TRUE;
	}
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
	p->error_message = "Unexpected character (see end of marked string)";
	p->start_error = 0;  /* mark whole string */
	p->end_error = p->tok->buf_pos;
	error = TRUE;
      }
    }
    else if (p->tok->type == TOK_COMMENT) {
      ;  /* do nothing */
    }
    else if (p->tok->type == TOK_UNKNOWN) {
      p->error_message = "Unexpected character (see end of marked string)";
      p->start_error = 0;  /* mark whole string */
      p->end_error = p->tok->buf_pos;
      error = TRUE;
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
      new->parenthesized = parenthesized;
      last = new;
    }

    if (!error) {
      /*  */
      next_token(p);
      if (p->tok == NULL)
	done = TRUE;
      else if(p->tok->type == TOK_PUNC &&
	      (p->tok->c == ')' || p->tok->c == ']' ||
	       p->tok->c == '}' || p->tok->c == Cons_char))
	done = TRUE;
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
  int start_pos = p->tok->buf_pos;
  Pterm terms = toks_to_terms(p);
  if (terms == NULL)
    t = NULL;
  else {
    Pterm end;
    for (end = terms; end->next != NULL; end = end->next);
    t = terms_to_term(terms, end, 1000);
    free_pterm_list(terms);
    if (t == NULL) {
      p->error_message = "A term cannot be constructed from the marked string";
      p->start_error = start_pos;
      p->end_error = (p->tok ? p->tok->buf_pos-1 : -1);
    }
  }
  return t;
}  /* toks_to_term */

/*************
 *
 *   fprint_parse_error()
 *
 *   This routine prints an error message, pointing to a
 *   substring of a String_buf.
 *
 *   (There is a similar routine that points to a whole term.)
 *
 *************/

static
void fprint_parse_error(FILE *fp, char *msg, String_buf sb,
			int start_pos, int end_pos)
{
  int i;
  int n = sb_size(sb);
  if (end_pos == -1)
    end_pos = n-1;
  fprintf(fp, "%%%%ERROR: %s:\n\n", msg);
  for (i = 0; i < start_pos; i++)
    fprintf(fp, "%c", sb_char(sb, i));
  fprintf(fp, "%%%%START ERROR%%%%");
  for (i = start_pos; i <= end_pos; i++)
    fprintf(fp, "%c", sb_char(sb, i));
  fprintf(fp, "%%%%END ERROR%%%%");
  for (i = end_pos+1; i < n; i++)
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
  if (tokens == NULL) {
    fprint_parse_error(fout, "Empty term (too many periods?)",
		       sb, 0, sb_size(sb)-1);
    free_token_list(tokens);
    fatal_error("sread_term, empty term (too many periods?)");
  }
  tp.tok = tokens;
  tp.error_message = "";
  tp.start_error = 0;
  tp.end_error   = 0;  /* index of last char (not like python) */
  t = toks_to_term(&tp);
  if (t == NULL) {
    fprint_parse_error(fout, tp.error_message, sb,
		       tp.start_error, tp.end_error);
    free_token_list(tp.tok);
    fatal_error("sread_term error");
  }
  else if (tp.tok != NULL) {
    fprint_parse_error(fout,
	     "Unexpected character (extra closing parenthesis?)",
		       sb, 0, tp.tok->buf_pos);
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
      msg = "Unexpected end of input (missing period?)";
      break;
    case READ_BUF_QUOTE_ERROR:
      msg = "Quoted string has no end";
      break;
    default:
      msg = "Error reading term";
      break;
    }
    fprint_parse_error(fout, msg, sb, 0, sb_size(sb));
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
 *   parse_term_from_string()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term parse_term_from_string(char *s)
{
  String_buf sb = init_string_buf(s);
  Term t = sread_term(sb, stdout);
  zap_string_buf(sb);
  return t;
}  /* parse_term_from_string */

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
    symbol_for_variable(str, VARNUM(t));
    sb_append(sb, str);
  }  /* variable */

  else if (CONSTANT(t)) {
    if (nil_term(t))
      sb_append(sb, "[]");
    else
      sb_append(sb, sn_to_str(SYMNUM(t)));
  }  /* constant */

  else if (is_term(t, quant_sym(), 3)) {
    /* This may insert more parentheses than necessary, to avoid 
       confusing formulas like "all x p | q".  That formula means
       "(all x p) | q", and we insert those parentheses.  Also,
       we print "(all x all y p)" instead of "(all x (all y p))",
       by handling a sequence of quantifier iteratively instead
       of recursively.
    */
    Term q = t;
    sb_append(sb, "(");
    do {
      arrange_term(sb, ARG(q,0), 1000);  /* quantifier */
      sb_append(sb, " ");
      arrange_term(sb, ARG(q,1), 1000);  /* variable */
      sb_append(sb, " ");
      q = ARG(q,2);
    } while (is_term(q, quant_sym(), 3));
    arrange_term(sb, q, Quantifier_precedence);
    sb_append(sb, ")");
  }  /* quantified */

  else if (cons_term(t)) {
    Term t1 = t;
    sb_append(sb, "[");
    while (cons_term(t1)) {
      arrange_term(sb, ARG(t1,0), 1000);
      t1 = ARG(t1,1);
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
		
      if (op_prec > par_prec || Parenthesize)
	sb_append(sb, "(");
      arrange_term(sb, ARG(t,0), p1);
      sb_append(sb, " ");
      sb_append(sb, sn_to_str(SYMNUM(t)));
      sb_append(sb, " ");
      arrange_term(sb, ARG(t,1), p2);
      if (op_prec > par_prec || Parenthesize)
	sb_append(sb, ")");
    }  /* binary infix */
    else if (ARITY(t) == 1 &&
	     unary_parse_type(sn_to_str(SYMNUM(t)), &op_prec, &type)) {

      int p1 = ((type == PREFIX_PAREN || type == POSTFIX_PAREN) ?
		op_prec-1 : op_prec);

      if (op_prec > par_prec || Parenthesize)
	sb_append(sb, "(");

      if (type == PREFIX_PAREN || type == PREFIX) {
	sb_append(sb, sn_to_str(SYMNUM(t)));
	sb_append(sb, " ");
	arrange_term(sb, ARG(t,0), p1);
      }  /* prefix */
      else {
	arrange_term(sb, ARG(t,0), p1);
	sb_append(sb, " ");
	sb_append(sb, sn_to_str(SYMNUM(t)));
      }  /* postfix */

      if (op_prec > par_prec || Parenthesize)
	sb_append(sb, ")");

    }  /* unary prefix or postfix */
    else {
      /* ordinary application */
      int i;
      sb_append(sb, sn_to_str(SYMNUM(t)));
      sb_append(sb, "(");
      for (i = 0; i < ARITY(t); i++) {
	arrange_term(sb, ARG(t,i), 1000);
	if (i < ARITY(t)-1)
	  sb_append(sb, ",");
      }
      sb_append(sb, ")");
    }  /* ordinary application */
  }
}  /* arrange_term */

/*************
 *
 *   sb_remove_some_space()
 *
 *************/

static
void sb_remove_some_space(String_buf sb, int begin, int end)
{
  int i;
  BOOL in_quote = FALSE;
  for (i = begin; i < end-1; i++) {
    char c = sb_char(sb, i);
    if (quote_char(c))
      in_quote = !in_quote;
    else if (!in_quote && sb_char(sb, i) == ' ') {
      if (sb_char(sb, i-1) == '-') {
	if (!special_char(sb_char(sb, i-2)))
	  sb_replace_char(sb, i, '\0');
      }
      else if (sb_char(sb, i+1) == '\'') {
	if (!special_char(sb_char(sb, i+2)))
	  sb_replace_char(sb, i, '\0');
      }
    }
  }
}  /* sb_remove_some_space */

/*************
 *
 *   sb_write_term()
 *
 *************/

/* DOCUMENTATION
This routine prints a term to a String_buf in readable form,
that is, infix where appropriate and without extra parentheses.
A period and newline are <I>not</I> printed.
<P>
See the documentation on mixfix terms and the routine set_parse_type().
*/

/* PUBLIC */
void sb_write_term(String_buf sb, Term t)
{
  int begin = sb_size(sb) + 1;
  int end;
  if (Translate_neg_equalities) {
    Term temp_term = untranslate_neg_eq(copy_term(t));
    arrange_term(sb, temp_term, 1000);
    zap_term(temp_term);
  }
  else
    arrange_term(sb, t, 1000);
  end = sb_size(sb)+1;
  sb_remove_some_space(sb, begin, end);
}  /* sb_write_term */

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
  sb_write_term(sb, t);
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
 *   process_quoted_symbol()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *process_quoted_symbol(char *str)
{
  if (quote_char(*str)) {
    int sn;
    char *str2 = new_str_copy(str);
    str2[strlen(str2)-1] = '\0';
    str2 = str2+1;
    /* str2 is nwo quote-free. */
    sn = str_to_sn(str2, 0);  /* Inserts a copy into the symbol table. */
    /* Free what was returned from new_str_copy above. */
    free(str2-1);
    /* Get the quote-free string out of the symbol table. */

    str2 = sn_to_str(sn);

    return str2;
  }
  else
    return str;
}  /* process_quoted_symbol */

/*************
 *
 *   remember_multiple_char_special_syms()
 *
 *************/

static
void remember_multiple_char_special_syms(char *str)
{
  /* If string is multiple-char-special, add it to a global list
     (which is used for parsing).
  */
  if (special_char(*str) &&
      strlen(str) > 1 &&
      !string_member_plist(str, Multiple_char_special_syms)) {
    Multiple_char_special_syms =
      plist_prepend(Multiple_char_special_syms, str);
  }
}  /* remember_multiple_char_special_syms */

/*************
 *
 *   forget_multiple_char_special_syms()
 *
 *************/

static
void forget_multiple_char_special_syms(char *str)
{
  /* If string is multiple-char-special, add it to a global list
     (which is used for parsing).
  */
  if (special_char(*str) &&
      strlen(str) > 1 &&
      string_member_plist(str, Multiple_char_special_syms)) {
    Multiple_char_special_syms =
      plist_remove_string(Multiple_char_special_syms, str);
  }
}  /* forget_multiple_char_special_syms */

/*************
 *
 *   look_for_illegal_symbols()
 *
 *************/

static
void look_for_illegal_symbols(char *str)
{
  if (strlen(str) > 1 && string_of_repeated('-', str)) {
    printf("bad string: %s\n", str);
    fatal_error("operations that are strings of repeated \"-\" are not allowed");
  }
  else if (strlen(str) > 1 && string_of_repeated('\'', str))
    fatal_error("operations that are strings of repeated \"\'\" are not allowed");
}  /* look_for_illegal_symbols */

/*************
 *
 *   declare_parse_type()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_parse_type(char *str, int precedence, Parsetype type)
{
  char *str2 = process_quoted_symbol(str);  /* if quoted, remove quotes */

  if (Check_for_illegal_symbols)
    look_for_illegal_symbols(str2);
  set_parse_type(str2, precedence, type);
  if (type == NOTHING_SPECIAL)
    forget_multiple_char_special_syms(str2);
  else
    remember_multiple_char_special_syms(str2);
}  /* declare_parse_type */

/*************
 *
 *   declare_quantifier_precedence()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void declare_quantifier_precedence(int prec)
{
  Quantifier_precedence = prec;
}  /* declare_quantifier_precedence */

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
  declare_parse_type(",",        999, INFIX_RIGHT);  /* essential */
			    
  declare_parse_type(attrib_sym(), 810, INFIX_RIGHT);
			    
  declare_parse_type(iff_sym(),  800, INFIX);
  declare_parse_type(imp_sym(),  800, INFIX);
  declare_parse_type(impby_sym(),800, INFIX);
  declare_parse_type(or_sym(),   790, INFIX_RIGHT);
  declare_parse_type("||",       790, INFIX_RIGHT);  /* special-purpose */
  declare_parse_type(and_sym(),  780, INFIX_RIGHT);
  declare_parse_type("&&",       780, INFIX_RIGHT);  /* special-purpose */
			    
  declare_quantifier_precedence(750);  /* special case for quantifiers */

  declare_parse_type(eq_sym(),   700, INFIX);
  declare_parse_type(neq_sym(),  700, INFIX);
  declare_parse_type("==",       700, INFIX);
  declare_parse_type("!==",      700, INFIX);
  declare_parse_type("<",        700, INFIX);
  declare_parse_type("<=",       700, INFIX);
  declare_parse_type(">",        700, INFIX);
  declare_parse_type(">=",       700, INFIX);
  declare_parse_type("@<",       700, INFIX);
  declare_parse_type("@<=",      700, INFIX);
  declare_parse_type("@>",       700, INFIX);
  declare_parse_type("@>=",      700, INFIX);
			    
  declare_parse_type("+",        500, INFIX);
  declare_parse_type("*",        500, INFIX);
  declare_parse_type("@",        500, INFIX);
  declare_parse_type("/",        500, INFIX);
  declare_parse_type("\\",       500, INFIX);
  declare_parse_type("^",        500, INFIX);
  declare_parse_type("v",        500, INFIX);

  declare_parse_type(not_sym(),    350, PREFIX);
  declare_parse_type("\'",       300, POSTFIX);

  /* Other things */

  set_cons_char(':');  /* List cons, as in [x:y] */

}  /* declare_standard_parse_types */

/*************
 *
 *   redeclare_symbol_and_copy_parsetype()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL redeclare_symbol_and_copy_parsetype(char *operation, char *str,
					 BOOL echo, FILE *fout)
{
  char *new_str = process_quoted_symbol(str);  /* if quoted, remove quotes */
  if (symbol_in_use(new_str))
    return FALSE;
  else {
    Parsetype type;
    int prec;
    char *old_str = get_operation_symbol(operation);
    set_operation_symbol(operation, new_str);
    remember_multiple_char_special_syms(new_str);  /* for subsequent parsing */
    if (unary_parse_type(old_str, &prec, &type) ||
	binary_parse_type(old_str, &prec, &type)) {
      set_parse_type(new_str, prec, type);
      if (echo) {
	fprintf(fout,
		"  %% op(%d, %s, %s).  %% copying parse/print properties"
		" from %s to %s\n",
		prec, parse_type_to_str(type), new_str, old_str, new_str);
      }
    }
    return TRUE;  /* success */
  }
}  /* redeclare_symbol_and_copy_parsetype */

/*************
 *
 *   skip_to_nl()
 *
 *************/

/* DOCUMENTATION
Read characters up to the first newline (or EOF).
*/

/* PUBLIC */
void skip_to_nl(FILE *fp)
{
  int c = fgetc(fp);
  while (c != EOF && c != '\n')
    c = fgetc(fp);
}  /* skip_to_nl */

/*************
 *
 *   split_string()
 *
 *************/

/* DOCUMENTATION
This splits a string of symbols, separated by whitespace,
into a list of strings.  A Plist of (malloced) strings
is returned.
*/

/* PUBLIC */
Plist split_string(char *onlys)
{
  Plist p = NULL;
  char *work = malloc(strlen(onlys)+1);
  int len = strlen(onlys);
  int i = 0;
  while (i < len) {
    while (i < len && white_char(onlys[i])) i++;
    if (i < len) {
      int j = 0;
      while (i < len && !white_char(onlys[i]))
	work[j++] = onlys[i++];
      work[j] = '\0';
      p = plist_append(p, new_str_copy(work));
    }
  }
  free(work);
  return p;
}  /* split_string */

/*************
 *
 *   set_cons_char()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_cons_char(char c)
{
  Cons_char = c;
}  /* set_cons_char */

/*************
 *
 *   get_cons_char()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char get_cons_char(void)
{
  return Cons_char;
}  /* set_cons_char */

/*************
 *
 *   set_quote_char()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void set_quote_char(char c)
{
  Quote_char = c;
}  /* set_quote_char */

/*************
 *
 *   get_quote_char()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char get_quote_char(void)
{
  return Quote_char;
}  /* set_quote_char */

/*************
 *
 *   parenthesize()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void parenthesize(BOOL setting)
{
  Parenthesize = setting;
}  /* parenthesize */

/*************
 *
 *   check_for_illegal_symbols()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void check_for_illegal_symbols(BOOL setting)
{
  Check_for_illegal_symbols = setting;
}  /* check_for_illegal_symbols */

/*************
 *
 *   simple_parse()
 *
 *************/

/* DOCUMENTATION
   Do not try to parse "op command" symbols.
*/

/* PUBLIC */
void simple_parse(BOOL setting)
{
  Simple_parse = setting;
}  /* simple_parse */
