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

#define PROGRAM_NAME    "tptp_to_ladr"
#include "../VERSION_DATE.h"

/*************
 *
 *   read_tptp_input()
 *
 *************/

static
void read_tptp_input(FILE *fin, FILE *fout, FILE *ferr,
		     Plist *assumps, Plist *goals)
{
  Term t = read_term(fin, fout);

  while (t != NULL) {

    if (is_term(t, "include", 1) || is_term(t, "include", 2)) {
      char *fname = new_str_copy(sn_to_str(SYMNUM(ARG(t,0))));
      char *fname2 = fname;
      FILE *fin2;

      if (*fname2 == '\'' || *fname2 == '"') {
	fname2[strlen(fname2)-1] = '\0';
	fname2++;
      }
      fin2 = fopen(fname2, "r");
      if (fin2 == NULL) {
	char s[100];
	sprintf(s, "read_tptp_input, file %s not found", fname2);
	fatal_error(s);
      }
      fprintf(fout, "\n%% Including file %s\n\n", fname2);
      read_tptp_input(fin2, fout, ferr, assumps, goals);
      free(fname);
    }
    else if (is_term(t, "cnf", 3) || is_term(t, "cnf", 4) ||
	     is_term(t, "fof", 3) || is_term(t, "fof", 4) ||
	     is_term(t, "input_clause", 3)) {
      Formula form = tptp_input_to_ladr_formula(t);
      if (is_term(ARG(t,1), "conjecture", 0))
	*goals = plist_prepend(*goals, form);
      else
	*assumps = plist_prepend(*assumps, form);
    }
    else {
      p_term(t);
      fatal_error("read_tptp_input: unknown term");
    }
    zap_term(t);
    t = read_term(fin, fout);
  }
}  /* read_tptp_input */

int main(int argc, char **argv)
{
  Plist assumps = NULL;
  Plist goals = NULL;
  Plist all = NULL;
  Plist p;

  Ilist fp_syms = NULL;
  I2list map;
  int i;

  BOOL quote_bad_ladr_syms = string_member("-q", argv, argc);

  init_standard_ladr();
  i = register_attribute("label",  STRING_ATTRIBUTE);

  clear_parse_type_for_all_symbols();
  declare_tptp_input_types();
  set_quote_char('\'');  /* TPTP uses single quotes for strings. */

  read_tptp_input(stdin, stdout, stderr, &assumps, &goals);

  assumps = reverse_plist(assumps);
  goals = reverse_plist(goals);

  /* Collect all of the LADR terms into one list. */

  all = NULL;
  all = plist_cat2(all, assumps);
  all = plist_cat2(all, goals);

  /* Collect all of the function and predicate symbols. */

  {
    I2list f = NULL;
    I2list r = NULL;
    gather_symbols_in_formulas(all, &r, &f);
    Ilist rsyms = multiset_to_set(r);
    Ilist fsyms = multiset_to_set(f);
    zap_i2list(f);
    zap_i2list(r);
    fp_syms = ilist_cat(rsyms, fsyms);
  }

  /* Determine which symbols are bad for LADR and create a map to new ones. */

  map = map_for_bad_ladr_syms(fp_syms, quote_bad_ladr_syms);

  /* Replace the bad symbols. */

  if (map != NULL) {
    I2list b;
    printf("\n%% The TPTP formulas contain function or predicate symbols\n"
	   "%% that are not legal LADR symbols, and we have replaced those\n"
	   "%% symbols with new symbols.  Here is the list of the unaccepted\n"
	   "%% symbols and the corresponding replacements.\n%%\n");
    for (b = map; b; b = b->next)
      printf("%%   (arity %d) %8s    %s\n",
	     sn_to_arity(b->j),
	     sn_to_str(b->i),
	     sn_to_str(b->j));
    printf("\n");
    for (p = assumps; p; p = p->next) {
      Term t = formula_to_term(p->v);
      Term t2 = replace_bad_syms_term(t, map);
      p->v = term_to_formula(t2);
    }
    for (p = goals; p; p = p->next) {
      Formula f = p->v;
      Term t = formula_to_term(f);
      zap_formula(f);
      t = replace_bad_syms_term(t, map);
      p->v = term_to_formula(t);
      zap_term(t);
    }
  }

  /* Change settings for LADR output. */  

  clear_parse_type_for_all_symbols();
  declare_standard_parse_types();  /* defaults for LADR */

  /* Output the LADR terms. */

  printf("set(prolog_style_variables).\n");
  fwrite_formula_list(stdout, assumps, "assumptions");
  fwrite_formula_list(stdout, goals, "goals");

  exit(0);
}  /* main */

