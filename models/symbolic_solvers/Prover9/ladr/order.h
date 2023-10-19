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

#ifndef TP_ORDER_H
#define TP_ORDER_H

#include <stdlib.h>  /* for malloc/free */

/* INTRODUCTION
This package defines Ordertype and has a generic sorting 
routine merge_sort() that can be used to sort any kind of data.
*/

/* Public definitions */

/* basic order relations */

typedef enum { NOT_COMPARABLE,
	       SAME_AS,
	       LESS_THAN,
	       GREATER_THAN,
	       LESS_THAN_OR_SAME_AS,
	       GREATER_THAN_OR_SAME_AS,
	       NOT_LESS_THAN,
	       NOT_GREATER_THAN
             } Ordertype;

/* End of public definitions */

/* Public function prototypes from order.c */

void merge_sort_recurse(void *a[],    /* array to sort */
			void *w[],    /* work array */
			int start,    /* index of first element */
			int end,      /* index of last element */
			Ordertype (*comp_proc) (void *, void *));

void merge_sort(void *a[],   /* array to sort */
		int n,       /* size of array */
		Ordertype (*comp_proc) (void *, void *));

Ordertype compare_vecs(int *a, int *b, int n);

void copy_vec(int *a, int *b, int n);

#endif  /* conditional compilation of whole file */
