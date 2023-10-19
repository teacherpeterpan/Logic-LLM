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
#include "../ladr/cnf.h"

#define PROGRAM_NAME    "miniscope"
#include "../VERSION_DATE.h"

/*************
 *
 *   main()
 *
 *************/

int main(int argc, char **argv)
{
  Plist formulas;
  Formula f;

  init_standard_ladr();

  formulas = read_formula_list(stdin, stderr);

  f = formulas_to_conjunction(formulas);

  printf("\n------------------------\n");

  printf("\nbefore (denial): ");
  p_formula(f);

  f = nnf(negate(f));
  printf("\nnnf (positive):    ");
  p_formula(f);
    
  f = miniscope_formula(f, 0);

  if (f->type == AND_FORM) {
    int i;
    printf("result is %d subproblems\n", f->arity);
    for (i = 0; i < f->arity; i++) {
      printf("\nproblem %d:  ", i+1);
      p_formula(negate(f->kids[i]));
    }
  }
  else {
    printf("\nresult indivisible\n\n");
    p_formula(negate(f));
  }

  printf("exiting %s\n", PROGRAM_NAME);
  exit(0);
}  /* main */
