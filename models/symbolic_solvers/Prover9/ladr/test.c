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

#include <stdio.h>
#include <stdlib.h>

#define N 5

void compute_args(int *a, int arity, int n, int i)
{
  int x = i;
  int r;
  for (r = arity-1; r >= 0; r--) {
    a[r] = x % n;
    x = x - a[r];
    x = x / n;
  }
}  /* lkj */

main()
{
  int a[4];
  int i;

  for (i = 0; i < 81; i++) {
    int j;
    compute_args(a, 4, 3, i);
    for (j = 0; j < 4; j++)
      printf(" %d", a[j]);
    printf("\n");
  }
}  /* main */

