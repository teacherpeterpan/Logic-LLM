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

// static BOOL Debug = TRUE;

/*************
 *
 *   tptp_include()
 *
 *************/

static
Plist tptp_include(Term t)
{
  return NULL;
}  /* tptp_include */

/*************
 *
 *   read_tptp_file()
 *
 *************/

static
Plist read_tptp_file(FILE *fp)
{
  Plist formulas = NULL;
  Term t = read_term(fp, stderr);

  while (t != NULL) {
    if (is_term(t, "include", 1) || is_term(t, "include", 2))
      formulas = plist_cat(formulas, tptp_include(t));
    else if (is_term(t, "cnf", 3) || is_term(t, "cnf", 4))
      formulas = plist_prepend(formulas, tptp_formula(t));
    else if (is_term(t, "fof", 3) || is_term(t, "fof", 4))
      formulas = plist_prepend(formulas, tptp_formula(t));
    else {
      p_term(t);
      fatal_error("read_tptp_file: unknown term");
    }

    zap_term(t);
    t = read_term(stdin, stderr);
  }
  return reverse_plist(formulas);
}  /* read_tptp_file */

int main(int argc, char **argv)
{
  Plist formulas, p;

  init_standard_ladr();

  clear_parse_type_for_all_symbols();
  declare_tptp_parse_types();
  set_variable_style(PROLOG_STYLE);
  set_quote_char('\'');  /* TPTP uses single quotes for strings. */

  int label_att  = register_attribute("label",        STRING_ATTRIBUTE);

  formulas = read_tptp_file(stdin);

  for (p = formulas; p; p = p->next) {
    p_formula(p->v);
    fwrite_formula(stdout, p->v);
  }

  exit(0);
}  /* main */

