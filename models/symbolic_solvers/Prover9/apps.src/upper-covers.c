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

/* Take a stream of interpretations, and for each, print the upper-covers.
 */

#include "../ladr/top_input.h"
#include "../ladr/interp.h"

#define I2(n,i,j)     ((i) * (n) + (j))

#define PROGRAM_NAME    "upper-covers"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program reads a stream of lattices (from stdin) and\n"
"for each one, prints its upper-covers.  Each lattice is\n"
"assumed to be in \"portable format\" as produced by mace4,\n"
"and have a binary operation (meet) named \"^\", \"m\", or \"meet\"\n"
"(it can have other operations as well).\n";

/*************
 *
 *   adjacent()
 *
 *************/

static BOOL adjacent(int n, int i, int j, BOOL *lt)
{
  int k;
  for (k = 0; k < n; k++)
    if (lt[I2(n,i,k)] && lt[I2(n,k,j)])
      return FALSE;
  return TRUE;
}  /* adjacent */

static void upper_covers(Interp p, int lattice_number)
{
  int n = interp_size(p);
  BOOL *lt = malloc(n * n * sizeof(BOOL *));
  BOOL *uc = malloc(n * n * sizeof(BOOL *));
  int i, j;
  int *a;
  a = interp_table(p, "^", 2);
  if (a == NULL) {
    a = interp_table(p, "meet", 2);
    if (a == NULL) {
      a = interp_table(p, "m", 2);
      if (a == NULL)
	fatal_error("upper_covers: meet not found");
    }
  }

  /* Construct less-than relation; initialize upper-covers to less-than. */
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      lt[I2(n,i,j)] = (a[I2(n,i,j)] == i && i != j);
      uc[I2(n,i,j)] = lt[I2(n,i,j)];
    }

  /* Remove entries from uc if something is between the elements. */
    
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      if (lt[I2(n,i,j)] && !adjacent(n, i, j, lt))
	uc[I2(n,i,j)] = FALSE;
    }

  /* Print upper-covers. */

  printf("%% UPPER-COVERS of Lattice %d\n", lattice_number);
  printf("(\n");
  for (i = 0; i < n; i++) {
    BOOL started = FALSE;
    printf("(%d (", i);
    for (j = 0; j < n; j++) {
      if (uc[I2(n,i,j)]) {
	printf("%s%d", started ? " " : "", j);
	started = TRUE;
      }
    }
    printf("))\n");
  }
  printf(")\n");
  printf("%% end of upper-covers\n");

  free(lt);
  free(uc);
}  /* upper_covers */

int main(int argc, char **argv)
{
  Term t;
  int interps_read = 0;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  init_standard_ladr();

  /* Note that we do not read any commands. */
  /* terms(interpretations) and end_of_list are optional. */

  t = read_term(stdin, stderr);
  if (is_term(t, "terms", 1))
    t = read_term(stdin, stderr);

  while (t != NULL && !end_of_list_term(t)) {
    Interp a = compile_interp(t, FALSE);
    interps_read++;

    upper_covers(a, interps_read);

    zap_interp(a);
    zap_term(t);
    t = read_term(stdin, stderr);
  }

  printf("%% upper-covers: translated=%d\n", interps_read);

  exit(0);
}  /* main */

