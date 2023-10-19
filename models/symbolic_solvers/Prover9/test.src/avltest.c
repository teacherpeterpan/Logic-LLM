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

#include "../ladr/avltree.h"
#include "../ladr/random.h"

static BOOL Debug = TRUE;

static Ordertype ptr_compare(void *i1, void *i2)
{
  return (i1 < i2 ? LESS_THAN : (i1 > i2 ? GREATER_THAN : SAME_AS));
}  /* ptr_compare */

int main(int argc, char **argv)
{
  int n, i, seed, *a;
  Avl_node p = NULL;

  if (argc != 3) {
    printf("need 2 args: size seed\n");
    exit(0);
  }
  
  n = atoi(argv[1]);
  seed = atoi(argv[2]);

  srand(seed);

  a = malloc(n * sizeof(int));
  random_permutation(a, n);

  printf("starting insert\n"); fflush(stdout);
  for (i = 0; i < n; i++) {
    int x = a[i] + 1;
    if (Debug)
      printf("inserting %4d\n", x);
    p = avl_insert(p, (void *) x, ptr_compare);
    if (Debug)
      avl_check(p, ptr_compare);
  }
  if (Debug)
    p_avl(p, 0);

  printf("finished insert\n"); fflush(stdout);

  printf("smallest item is %d.\n", (int) avl_smallest(p));
  printf("largest item is %d.\n", (int) avl_largest(p));
  printf("position of %d is %.4f.\n",
	 a[0]+1, avl_position(p, (void *) a[0]+1, ptr_compare));

  if (Debug) {
    for (i = 1; i <= n; i++) {
      double pos = avl_position(p, (void *) i, ptr_compare);
      int j = (int) avl_item_at_position(p, pos);
      printf("item %d, position %.12f, item=%d.\n", i, pos, j);
    }
    for (i = 0; i <= 100; i++) {
      double d = i / 100.0;
      int j = (int) avl_item_at_position(p, d);
      double pos = avl_position(p, (void *) j, ptr_compare);
      printf("Position %.2f, item %d, position=%.4f.\n", d, j, pos);
    }
  }

  random_permutation(a, n);

  printf("starting delete\n"); fflush(stdout);

  for (i = 0; i < n; i++) {
    int x = a[i] + 1;
    if (Debug)
      printf("********************************** Removing %d\n", x);
    p = avl_delete(p, (void *) x, ptr_compare);
    if (Debug)
      avl_check(p, ptr_compare);
  }
  p_avl(p, 0);
  exit(0);
}  /* main */
