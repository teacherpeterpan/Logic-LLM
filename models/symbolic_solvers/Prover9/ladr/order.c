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

#include "order.h"

/*************
 *
 *    merge_sort_recurse
 *
 *************/

/* DOCUMENTATION
This is the recursive part of a general-purpose merge sort.
You won't ordinarily call this (use merge_sort instead).
Use this only if you manage allocation of the work array.
<P>
Here is an example of how to use it.
<PRE>
  {
    Term args[MAX_ACM_ARGS], work[MAX_ACM_ARGS];
    int n;

    < put the n terms you wish to sort into args[] >

    merge_sort_recurse((void **) args, (void **) work, 0, n-1,
               (Ordertype (*)(void*,void*)) term_compare_ncv);

    < args[] is now ordered by term_compare_ncv() >
  }
</PRE>
*/

/* PUBLIC */
void merge_sort_recurse(void *a[],    /* array to sort */
			void *w[],    /* work array */
			int start,    /* index of first element */
			int end,      /* index of last element */
			Ordertype (*comp_proc) (void *, void *))
{
  int mid, i, i1, i2, e1, e2;

  if (start < end) {
    mid = (start+end)/2;
    merge_sort_recurse(a, w, start, mid, comp_proc);
    merge_sort_recurse(a, w, mid+1, end, comp_proc);
    i1 = start; e1 = mid;
    i2 = mid+1; e2 = end;
    i = start;
    while (i1 <= e1 && i2 <= e2) {
      if ((*comp_proc)(a[i1], a[i2]) == GREATER_THAN)
	w[i++] = a[i2++];
      else
	w[i++] = a[i1++];
    }

    if (i2 > e2)
      while (i1 <= e1)
	w[i++] = a[i1++];
    else
      while (i2 <= e2)
	w[i++] = a[i2++];

    for (i = start; i <= end; i++)
      a[i] = w[i];
  }
}  /* merge_sort_recurse */

/*************
 *
 *    merge_sort
 *
 *************/

/* DOCUMENTATION
This is a general-purpose sorting routine.  You give it an
array of pointers to sort, the size of the array, and a
comparison function.
<P>
Here is an example of how to use it.
<PRE>
  {
    Term args[MAX_ACM_ARGS];
    int n;

    < set n and put the n terms you wish to sort into args[] >

    merge_sort((void **) args, n, (Ordertype (*)(void*,void*)) term_compare_ncv);

    < args[] is now ordered by term_compare_ncv() >
  }
</PRE>
*/

/* PUBLIC */
void merge_sort(void *a[],   /* array to sort */
		int n,       /* size of array */
		Ordertype (*comp_proc) (void *, void *))
{
  void **work = malloc(n * sizeof(void *));
  merge_sort_recurse((void **) a, work, 0, n-1, comp_proc);
  free(work);
}  /* merge_sort */

/*************
 *
 *   compare_vecs()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Ordertype compare_vecs(int *a, int *b, int n)
{
  int i;
  for (i = 0; i < n; i++) {
    if (a[i] < b[i])
      return LESS_THAN;
    else if (a[i] > b[i])
      return GREATER_THAN;
  }
  return SAME_AS;
}  /* compare_vecs */

/*************
 *
 *   copy_vec()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void copy_vec(int *a, int *b, int n)
{
  int i;
  for (i = 0; i < n; i++)
    b[i] = a[i];
}  /* copy_vec */

