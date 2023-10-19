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

#define PROGRAM_NAME    "renamer"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program takes a stream of clauses (from stdin) and\n"
"renumbers the variables.  Optional arguments:\n"
"    fast        clauses are read and written in fastparse form.\n"
"    commands    commands can be given and clauses are in a list.\n"
"                (not compatable with \"fast\")\n\n";


int main(int argc, char **argv)
{
  BOOL fast_parse, commands;
  Topform c;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }

  fast_parse = string_member("fast",     argv, argc);
  commands   = string_member("commands", argv, argc);

  if (fast_parse && commands)
    fatal_error("renamer, fast_parse incompatable with commands");

  init_standard_ladr();
  if (fast_parse)
    fast_set_defaults(); /* Declare the symbols for fastparse. */
  else {
    int i;
    i = register_attribute("label",  STRING_ATTRIBUTE);  /* ignore these */
    i = register_attribute("answer", TERM_ATTRIBUTE);  /* ignore these */
  }
  
  if (commands) {
    Term t;
    t = read_commands(stdin, stdout, FALSE, IGNORE_UNKNOWN);
    if (!t || (!is_term(t, "clauses", 1) && !is_term(t, "formulas", 1)))
      fatal_error("renamer, bad command or list identifier");
  }

  c = clause_reader(fast_parse);
  while (c != NULL) {
    renumber_variables(c, 100);  /* fatal if too many variables */
    clause_writer(c, fast_parse);
    fflush(stdout);
    zap_topform(c);
    c = clause_reader(fast_parse);
  }
  exit(0);
}  /* main */

