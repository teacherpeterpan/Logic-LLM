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

#define PROGRAM_NAME    "perm3"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program ... \n";

static BOOL Debug = TRUE;

/*************
 *
 *   perm3_term()
 *
 *************/

static
Term perm3_term(Term t, int *p)
{

  if (ARITY(t) == 3) {
    Term alpha = ARG(t,0);
    Term beta  = ARG(t,1);
    Term gamma = ARG(t,2);
    ARG(t,p[0]) = perm3_term(alpha, p);
    ARG(t,p[1]) = perm3_term(beta, p);
    ARG(t,p[2]) = perm3_term(gamma, p);
  }
  return t;
}  /* perm3_term */

/*************
 *
 *   perm3()
 *
 *************/

static
Topform perm3(Topform c, int *p)
{
  Topform m = copy_clause(c);
  Term atom = m->literals->atom;
  ARG(atom,0) = perm3_term(ARG(atom, 0), p);
  ARG(atom,1) = perm3_term(ARG(atom, 1), p);
  renumber_variables(m, MAX_VARS);
  return m;
}  /* perm3 */

/*************
 *
 *   contains_perm3()
 *
 *************/

static
BOOL contains_perm3(Topform c, Plist kept)
{
  int p[3];
  Topform p1, p2, p3, p4, p5, p6;
  Plist a;
  BOOL found;

  p[0] = 0; p[1] = 1; p[2] = 2; p1 = perm3(c, p);
  p[0] = 0; p[1] = 2; p[2] = 1; p2 = perm3(c, p);
  p[0] = 1; p[1] = 0; p[2] = 2; p3 = perm3(c, p);
  p[0] = 1; p[1] = 2; p[2] = 0; p4 = perm3(c, p);
  p[0] = 2; p[1] = 0; p[2] = 1; p5 = perm3(c, p);
  p[0] = 2; p[1] = 1; p[2] = 0; p6 = perm3(c, p);

  if (Debug) {
    printf("\ntesting: "); f_clause(c);
    printf("p1:    "); f_clause(p1);
    printf("p2:    "); f_clause(p2);
    printf("p3:    "); f_clause(p3);
    printf("p4:    "); f_clause(p4);
    printf("p5:    "); f_clause(p5);
    printf("p6:    "); f_clause(p6);
  }

  for (a = kept, found = FALSE; a && !found; a = a->next) {
    Topform k = a->v;
    if (clause_ident(k->literals, p1->literals) ||
	clause_ident(k->literals, p2->literals) ||
	clause_ident(k->literals, p3->literals) ||
	clause_ident(k->literals, p4->literals) ||
	clause_ident(k->literals, p5->literals) ||
	clause_ident(k->literals, p6->literals))
      found = TRUE;
  }
  delete_clause(p1);
  delete_clause(p2);
  delete_clause(p3);
  delete_clause(p4);
  delete_clause(p5);
  delete_clause(p6);
  return found;
}  /* contains_perm3 */

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

    if (!contains_perm3(c, kept)) {
      number_kept++;
      kept = plist_prepend(kept, c);
      fwrite_clause(stdout, c, CL_FORM_BARE);
    }

    c = read_clause(stdin, stderr);
  }

  printf("%% perm3: %d read, %d kept, %.2f seconds.\n",
	 number_read, number_kept, user_seconds());
  
  exit(0);
}  /* main */

