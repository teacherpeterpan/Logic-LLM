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

#ifndef TP_AVLTREE_H
#define TP_AVLTREE_H

#include "memory.h"
#include "order.h"

/* INTRODUCTION
This is a simple implementation of AVL trees.  These are binary search trees
with the property that for each node, the difference in height of the two
subtrees is at most one.
<p>
The items that are stored in the tree are simply pointers, which
are assumed to contain the keys and data.
<p>
The caller supplies a comparison function, for example
<pre>
    Ordertype compare(void *v1, void *v2)
or
    Ordertype compare(struct my_data *p1, struct my_data *p2)
</pre>
that is assumed to return LESS_THAN, GREATER_THAN, or SAME_AS.
<p>
Aside from the ordinary "insert", "find", and "delete" operations,
there is a "position" operation that tells where an item is
in w.r.t. the inorder traversal of the tree, and an operation
that finds the item at a given position.
*/

/* Public definitions */

typedef struct avl_node * Avl_node;

/* End of public definitions */

/* Public function prototypes from avltree.c */

void fprint_avltree_mem(FILE *fp, BOOL heading);

void p_avltree_mem();

int avl_height(Avl_node p);

int avl_size(Avl_node p);

Avl_node avl_insert(Avl_node p, void *item,
		    Ordertype (*compare) (void *, void *));

Avl_node avl_delete(Avl_node p, void *item,
		    Ordertype (*compare) (void *, void *));

void *avl_find(Avl_node p, void *item,
	       Ordertype (*compare) (void *, void *));

void *avl_smallest(Avl_node p);

void *avl_largest(Avl_node p);

int avl_place(Avl_node p, void *item,
	      Ordertype (*compare) (void *, void *));

double avl_position(Avl_node p, void *item,
		    Ordertype (*compare) (void *, void *));

void *avl_nth_item(Avl_node p, int n);

void *avl_item_at_position(Avl_node p, double pos);

void avl_zap(Avl_node p);

void avl_check(Avl_node p,
	       Ordertype (*compare) (void *, void *));

void p_avl(Avl_node p, int level);

#endif  /* conditional compilation of whole file */
