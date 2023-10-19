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
#include "../ladr/complex.h"

#define PROGRAM_NAME    "test_complex"
#include "../VERSION_DATE.h"

int main(int argc, char **argv)
{
  Term t;
  double total1 = 0;
  double total2 = 0;
  double total3 = 0;
  double total4 = 0;
  double total5 = 0;
  int n = 0;

  init_standard_ladr();

  /* Evaluate each clause on stdin. */

  t = read_term(stdin, stderr);

  while (t != NULL) {
    int size = symbol_count(t);
    term_set_variables(t, MAX_VARS);
    n++;
    double x1 = 1 - term_complexity(t, 2, 0);
    double x2 = 1 - term_complexity(t, 3, 1);
    double x3 = 1 - term_complexity(t, 3, 2);
    double x4 = 1 - term_complexity(t, 3, 3);
    double x5 = 1 - complex4(t);
    total1 += x1;
    total2 += x2;
    total3 += x3;
    total4 += x4;
    total5 += x5;
    printf("%2d  %.3f  %.3f  %.3f  %.3f  %.3f     ", size,x1,x2,x3,x4,x5); p_term(t);
    zap_term(t);
    t = read_term(stdin, stderr);
  }
  printf("average complexities %.3f  %.3f  %.3f  %.3f  %.3f\n",
	 total1 / n, total2 / n, total3 / n, total4 / n, total5 / n);

  exit(0);
}  /* main */

