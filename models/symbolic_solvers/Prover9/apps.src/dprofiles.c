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

/* Take a stream of interpretations and remove the isomorphic ones.
 */

#include "../ladr/top_input.h"
#include "../ladr/interp.h"

#define PROGRAM_NAME    "profiles"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program reads a stream of interpretations (from stdin)\n"
"and shows the profiles."
"Argument \"discrim '<filename>'\" is accepted.\n";

int main(int argc, char **argv)
{
  Term t;
  Plist discriminators = NULL;
  int rc;

  init_standard_ladr();

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  rc = which_string_member("discrim", argv, argc);
  if (rc == -1)
    rc = which_string_member("-discrim", argv, argc);
  if (rc > 0) {
    if (rc+1 >= argc)
      fatal_error("isofilter: missing \"discrim\" argument");
    else {
      FILE *fp = fopen(argv[rc+1], "r");
      if (fp == NULL)
	fatal_error("discrim file cannot be opened for reading");
      discriminators = read_clause_list(fp, stderr, TRUE);
    }
  }

  /* Input is a stream of interpretations. */

  t = read_term(stdin, stderr);

  while (t != NULL) {
    Interp a, c;

    a = compile_interp(t, FALSE);
    c = normal3_interp(a, discriminators);
    p_interp_profile(c, discriminators);
    zap_interp(a);
    zap_interp(c);
    zap_term(t);
    t = read_term(stdin, stderr);
  }

  exit(0);
}  /* main */

