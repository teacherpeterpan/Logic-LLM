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

#include "../ladr/header.h"

/*************
 *
 *   random_permutation()
 *
 *************/

void random_permutation(int *a, int n)
{
  int i, t, k;
  for (i = 0; i < n; i++)
    a[i] = i;
  for (i = n-1; i >= 0; i--) {
    /* Get a random number k in [0,...,i], then swap a[i] and a[k]. */
    k = rand() % (i+1);
    t = a[i];
    a[i] = a[k];
    a[k] = t;
  }
}  /* random_permutation */

#define MAX_PRIME 1000

/*************
 *
 *   prime()
 *
 *************/

BOOL prime(int n)
{
  /* Is n a prime number?  If n is too big, a fatal error occurs.         */
  /* On the first call, use the sieve of Eratosthenes to set up an array. */

  static BOOL a[MAX_PRIME+1];
  static BOOL initialized = FALSE;

  if (!initialized) {
    int i, p;
    for (i = 2; i <= MAX_PRIME; i++)
      a[i] = TRUE;
    p = 2;
    while (p*p < MAX_PRIME) {
      int j = p*p;
      while (j  < MAX_PRIME) {
	a[j] = FALSE;
	j = j + p;
      }
      do p++; while (a[p] == FALSE);
    }
    initialized = TRUE;
  }
  if (n > MAX_PRIME)
    exit(1); /* fatal_error("prime: n out of range"); */
  else if (n < 2)
    return FALSE;
  else
    return a[n];
}  /* prime */
