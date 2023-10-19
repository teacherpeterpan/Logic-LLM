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

#include "../ladr/top_input.h"

#define PROGRAM_NAME    "mirror-flip"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program ... \n";

static BOOL Debug = FALSE;

/*************
 *
 *   mirror_term()
 *
 *************/

static
Term mirror_term(Term t)
{
  if (ARITY(t) == 2) {
    Term alpha = ARG(t,0);
    Term beta  = ARG(t,1);
    ARG(t,0) = mirror_term(beta);
    ARG(t,1) = mirror_term(alpha);
  }
  else if (ARITY(t) == 3) {
    Term alpha = ARG(t,0);
    Term beta  = ARG(t,1);
    Term gamma = ARG(t,2);
    ARG(t,0) = mirror_term(gamma);
    ARG(t,1) = mirror_term(beta);
    ARG(t,2) = mirror_term(alpha);
  }
  return t;
}  /* mirror_term */

/*************
 *
 *   mirror()
 *
 *************/

static
Topform mirror(Topform c)
{
  Topform m = copy_clause(c);
  Term atom = m->literals->atom;
  ARG(atom,0) = mirror_term(ARG(atom, 0));
  ARG(atom,1) = mirror_term(ARG(atom, 1));
  renumber_variables(m, MAX_VARS);
  return m;
}  /* mirror */

/*************
 *
 *   flip()
 *
 *************/

static
Topform flip(Topform c)
{
  Topform f = copy_clause(c);
  Term atom = f->literals->atom;
  Term alpha = ARG(atom,0);
  ARG(atom,0) = ARG(atom,1);
  ARG(atom,1) = alpha;
  renumber_variables(f, MAX_VARS);
  return f;
}  /* flip */

/*************
 *
 *   contains_mirror_flip()
 *
 *************/

static
BOOL contains_mirror_flip(Topform c, Plist kept)
{
  Topform f = flip(c);
  Topform m = mirror(c);
  Topform fm = flip(m);
  Plist p;
  BOOL found;

  if (Debug) {
    printf("\ntesting: "); f_clause(c);
    printf("flip:    "); f_clause(f);
    printf("mirror:  "); f_clause(m);
    printf("fm:      "); f_clause(fm);
  }

  for (p = kept, found = FALSE; p && !found; p = p->next) {
    Topform k = p->v;
    if (clause_ident(k->literals, c->literals) ||
	clause_ident(k->literals, f->literals) ||
	clause_ident(k->literals, m->literals) ||
	clause_ident(k->literals, fm->literals))
      found = TRUE;
  }
  delete_clause(f);
  delete_clause(m);
  delete_clause(fm);
  return found;
}  /* contains_mirror_flip */

int main(int argc, char **argv)
{
  Topform c;
  int i;
  int number_read = 0;
  int number_kept = 0;
  Plist kept = NULL;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  init_standard_ladr();

  i = register_attribute("label",  STRING_ATTRIBUTE);  /* ignore these */
  i = register_attribute("answer", TERM_ATTRIBUTE);  /* ignore these */

  c = read_clause(stdin, stderr);

  while (c != NULL && !end_of_list_clause(c)) {
    number_read++;

    if (!contains_mirror_flip(c, kept)) {
      number_kept++;
      kept = plist_prepend(kept, c);
      fwrite_clause(stdout, c, CL_FORM_BARE);
    }

    c = read_clause(stdin, stderr);
  }

  printf("%% mirror-flip: %d read, %d kept, %.2f seconds.\n",
	 number_read, number_kept, user_seconds());
  
  exit(0);
}  /* main */

