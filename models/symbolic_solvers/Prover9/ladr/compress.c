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

#include "compress.h"

/* Private definitions and types */

/*************
 *
 *   uncompress_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Term uncompress_term(char *s, int *ip)
{
  char c = s[(*ip)++];
  if (c <= 0)
    return get_variable_term(-c);
  else {
    int arity = sn_to_arity(c);
    Term t = get_rigid_term_dangerously(c, arity);
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = uncompress_term(s, ip);
    return t;
  }
}  /* uncompress_term */

/*************
 *
 *   compress_term_recurse()
 *
 *************/

static
void compress_term_recurse(String_buf sb, Term t)
{
  if (VARIABLE(t))
    sb_append_char(sb, -VARNUM(t));
  else {
    int i;
    sb_append_char(sb, SYMNUM(t));
    for (i = 0; i < ARITY(t); i++) {
      compress_term_recurse(sb, ARG(t,i));
    }
  }
}  /* compress_term_recurse */

/*************
 *
 *   compress_term()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *compress_term(Term t)
{
  String_buf sb = get_string_buf();
  compress_term_recurse(sb, t);
  {
    char *s;
    s = sb_to_malloc_char_array(sb);
    zap_string_buf(sb);
    return s;
  }
}  /* compress_term */

/*************
 *
 *   compress_clause()
 *
 *************/

/* DOCUMENTATION
When clasues are compressed, they lose any orientation marks
on atoms.  Marks are restored by a call to orient_equalities
during uncompression.
*/

/* PUBLIC */
void compress_clause(Topform c)
{
  if (c->compressed != NULL)
    fatal_error("compress_clause, clause already compressed");
  else if (-MAX_VARS < CHAR_MIN || greatest_symnum() > CHAR_MAX)
    return;  /* unable to compress, because symbols don't fit in char */
  else if (c->literals == NULL)
    return;
  else {
    Term t = lits_to_term(c->literals);
    /* printf("compressing clause %d\n", c->id); */
    c->compressed = compress_term(t);
    free_lits_to_term(t);
    c->neg_compressed = negative_clause(c->literals);
    zap_literals(c->literals);
    c->literals = NULL;
  }
}  /* compress_clause */

/*************
 *
 *   uncompress_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void uncompress_clause(Topform c)
{
  if (c->compressed) {
    int i = 0;
    Term t = uncompress_term(c->compressed, &i);
    c->literals = term_to_literals(t, NULL);
    upward_clause_links(c);
    zap_term(t);
    /* printf("UNcompressed clause %d\n", c->id); */
    free(c->compressed);
    c->compressed = NULL;
    c->neg_compressed = FALSE;
    if (!c->used) {
      printf("\n%% Uncompressing unused clause: ");
      fprint_clause(stdout, c);
    }
  }
}  /* uncompress_clause */

/*************
 *
 *   uncompress_clauses()
 *
 *************/

/* DOCUMENTATION
Given a Plist of clauses, uncompress the compressed ones.
*/

/* PUBLIC */
void uncompress_clauses(Plist p)
{
  Plist a;
  for (a = p; a; a = a->next) {
    Topform c = a->v;
    if (c->compressed) {
      uncompress_clause(c);
      orient_equalities(c, FALSE);  /* mark, but don't flip */
    }
  }
}  /* uncompress_clauses */
