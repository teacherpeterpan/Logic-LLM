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

#include "fastparse.h"

/* Private definitions and types */

static int Arity[256];      /* table of arities for each symbol */
static int Symnum[256];     /* table of arities for each symbol */

static int Pos;            /* index for parsing */

#define MAX_LINE 1000

/*************
 *
 *   fast_set_symbol()
 *
 *************/

/* DOCUMENTATION
Call this routine to declare a symbol/arity for fast parsing.
Since fast parsing handles single-characters symbols only,
you send character/arity to this routine.
<P>
For fast parsing, variables are always 'r' -- 'z', and
those symbols should not be declared.
<P>
Also, you don't need to call this routine for constants.
*/

/* PUBLIC */
void fast_set_symbol(char c, int arity)
{
  if (c >= 'r' && c <= 'z') {
    fatal_error("fast_set_symbol, r--z are variables and cannot be declared");
  }
  else {
    char str[2];

    str[0] = c;
    str[1] = '\0';
    Arity[(int) c] = arity;
    Symnum[(int) c] = str_to_sn(str, arity);
  }
}  /* fast_set_symbol */

/*************
 *
 *   fast_set_defaults()
 *
 *************/

/* DOCUMENTATION
Call this routine to declare a fixed set of symbols for fast parsing.
<UL>
<LI>binary: [=mjfd*+/]
<LI>unary: [cgi-~']
</UL>
The defaults can be overridden by nsubsequent calls to fast_set_symbol
*/

/* PUBLIC */
void fast_set_defaults(void)
{
  fast_set_symbol('=', 2);
  fast_set_symbol('m', 2);
  fast_set_symbol('j', 2);
  fast_set_symbol('f', 2);
  fast_set_symbol('d', 2);
  fast_set_symbol('*', 2);
  fast_set_symbol('+', 2);
  fast_set_symbol('/', 2);

  fast_set_symbol('c', 1);
  fast_set_symbol('g', 1);
  fast_set_symbol('i', 1);
  fast_set_symbol('-', 1);
  fast_set_symbol('~', 1);
  fast_set_symbol('\'', 1);
}  /* fast_set_defaults */

/*************
 *
 *   fast_parse()
 *
 *************/

static
Term fast_parse(char *s)
{
  char c = s[Pos++];
  Term t;

  if (c >= 'r' && c <= 'z') {
    switch (c) {
    case 'z': t = get_variable_term(0); break;
    case 'y': t = get_variable_term(1); break;
    case 'x': t = get_variable_term(2); break;
    case 'w': t = get_variable_term(3); break;
    case 'v': t = get_variable_term(4); break;
    case 'u': t = get_variable_term(5); break;
    case 't': t = get_variable_term(6); break;
    case 's': t = get_variable_term(7); break;
    case 'r': t = get_variable_term(8); break;
    default:  t = NULL;
    }
    return t;
  }
  else {
    int i;
    if (Symnum[(int) c] == 0) {
      /* Undeclared symbol; make it a constant. */
      char str[2];
      str[0] = c; str[1] = '\0';
      Symnum[(int) c] = str_to_sn(str, 0);
    }
    t = get_rigid_term_dangerously(Symnum[(int) c], Arity[(int) c]);
    for (i = 0; i < Arity[(int) c]; i++) {
      ARG(t,i) = fast_parse(s);
    }
    return t;
  }
}  /* fast_parse */

/*************
 *
 *   fast_read_term()
 *
 *************/

/* DOCUMENTATION
This routine reads a prefix term.
<UL>
<LI> The term must start on a new line and end with a period.
<LI> Without parentheses, commas, or spaces.
<LI> Each symbol is one character.
<LI> Variables are 'r' -- 'z'.
<LI> Symbols with arity > 0 (including '=') must declared first by calling
     fast_set_symbol().
<LI> If the first character is '%', the line is a comment and sent
     directly to the output stream fout.
<LI> Example:
<PRE>
=mxxx.
=jxyjyx.
=jxxmxx.
=jjxyzjxjyz.
</PRE>
</UL>
*/

/* PUBLIC */
Term fast_read_term(FILE *fin, FILE *fout)
{
  char line[MAX_LINE];
  char *s;
  Term t;
  s = fgets(line, MAX_LINE, fin);
  while (s != NULL && s[0] == '%')  { /* send comment lines to stdout */
    fprintf(stdout, "%s", s);
    fflush(stdout);
    s = fgets(line, MAX_LINE, fin);
  }
  if (s == NULL)
    return NULL;
  else {
    Pos = 0;
    t = fast_parse(s);
    if (s[Pos] != '.') {
      fprintf(stderr, s);
      fprintf(stdout, s);
      fatal_error("fast_read_term, term ends before period.");
    }
    return t;
  }
}  /* fast_read_term */

/*************
 *
 *   fast_fwrite_term()
 *
 *************/

static
void fast_fwrite_term(FILE *fp, Term t)
{
  char c;
  if (VARIABLE(t)) {
    switch (VARNUM(t)) {
    case 0: c = 'z'; break;
    case 1: c = 'y'; break;
    case 2: c = 'x'; break;
    case 3: c = 'w'; break;
    case 4: c = 'v'; break;
    case 5: c = 'u'; break;
    case 6: c = 't'; break;
    case 7: c = 's'; break;
    case 8: c = 'r'; break;
    default: c = '?'; break;
    }
    fprintf(fp, "%c", c);
  }
  else {
    int i;
    fprintf(fp, "%s", sn_to_str(SYMNUM(t)));
    for (i = 0; i < ARITY(t); i++)
      fast_fwrite_term(fp, ARG(t,i));
  }
}  /* fast_fwrite_term */

/*************
 *
 *   fast_fwrite_term_nl()
 *
 *************/

/* DOCUMENTATION
This routine writes a term in prefix form, without parentheses, commas,
or spaces, followed by ".\n".  If a variable number is >= 9, then '?'
is printed for that variable.
<P>
If each symbol is one character, then terms written by this routine
should be readable by fast_read_term().
<P>
There is nothing particularly "fast" about this routine.
*/

/* PUBLIC */
void fast_fwrite_term_nl(FILE *fp, Term t)
{
  fast_fwrite_term(fp, t);
  fprintf(fp, ".\n");
}  /* fast_fwrite_term_nl */

/*************
 *
 *   fast_read_clause()
 *
 *************/

/* DOCUMENTATION
This routine reads a clause in fast-parse form.
<P>
If the clause has more than one literal, then '|' must first be
declared as binary with fast_set_symbol(), and if the clause has
any negative literals, then '~' must first be declared as unary.
<P>
For example, the fast-parse form of <I>p(a,b) | ~q | ~(f(x)=x)</I> is
<PRE>
|pab|~q~=fxx.
</PRE>
*/

/* PUBLIC */
Topform fast_read_clause(FILE *fin, FILE *fout)
{
  /* This is different from read_clause() in the following way.

     Read_clause() first calls read_term(), which does NOT set
     variables.  Variables are set after term_to_clause().

     Fast_read_clause() first calls fast_read_term(), which DOES set
     variables, so a call to clause_set_variables() is not needed.
  */
  Term t;

  t = fast_read_term(fin, fout);
  if (t == NULL)
    return NULL;
  else {
    Topform c = term_to_clause(t);
    zap_term(t);
    upward_clause_links(c);
    return c;
  }
}  /* fast_read_clause */

/*************
 *
 *   fast_fwrite_clause()
 *
 *************/

/* DOCUMENTATION
This routine writes a clause in fastparse form.
*/

/* PUBLIC */
void fast_fwrite_clause(FILE *fp, Topform c)
{
  Term t = topform_to_term(c);

  if (t == NULL)
    fatal_error("fwrite_clause, clause_to_term returns NULL.");
  
  fast_fwrite_term_nl(fp, t);
  fflush(fp);
  zap_term(t);
}  /* fast_fwrite_clause */

