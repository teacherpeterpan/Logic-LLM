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

#include "avltree.h"
#include <math.h>

/* Private definitions and types */

struct avl_node {
  void *item;             /* data (including key) */
  int size;               /* number of nodes */
  int height;             /* leaf has height 1 */
  Avl_node left, right;   /* subtrees */
};

static BOOL Debug = FALSE;

/*
 * memory management
 */

#define PTRS_AVL_NODE PTRS(sizeof(struct avl_node))
static unsigned Avl_node_gets, Avl_node_frees;

/*************
 *
 *   Avl_node get_avl_node()
 *
 *************/

static
Avl_node get_avl_node(void)
{
  Avl_node p = get_mem(PTRS_AVL_NODE);
  Avl_node_gets++;
  return(p);
}  /* get_avl_node */

/*************
 *
 *    free_avl_node()
 *
 *************/

static
void free_avl_node(Avl_node p)
{
  free_mem(p, PTRS_AVL_NODE);
  Avl_node_frees++;
}  /* free_avl_node */

/*************
 *
 *   fprint_avltree_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the avltree package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_avltree_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct avl_node);
  fprintf(fp, "avl_node (%4d)     %11u%11u%11u%9.1f K\n",
          n, Avl_node_gets, Avl_node_frees,
          Avl_node_gets - Avl_node_frees,
          ((Avl_node_gets - Avl_node_frees) * n) / 1024.);

}  /* fprint_avltree_mem */

/*************
 *
 *   p_avltree_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the avltree package.
*/

/* PUBLIC */
void p_avltree_mem()
{
  fprint_avltree_mem(stdout, TRUE);
}  /* p_avltree_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   avl_height()
 *
 *************/

/* DOCUMENTATION
Return the height of an AVL tree.  Leaves have height 1.
*/

/* PUBLIC */
int avl_height(Avl_node p)
{
  return (p == NULL ? 0 : p->height);
}  /* avl_height */

/*************
 *
 *   avl_size()
 *
 *************/

/* DOCUMENTATION
Return the number of nodes in an AVL tree.
*/

/* PUBLIC */
int avl_size(Avl_node p)
{
  return (p == NULL ? 0 : p->size);
}  /* avl_size */

/*************
 *
 *   set_height_and_size()
 *
 *************/

static
void set_height_and_size(Avl_node p)
{
  p->height = IMAX(avl_height(p->left), avl_height(p->right)) + 1;
  p->size = (avl_size(p->left) + avl_size(p->right)) + 1;
}  /* set_height_and_size */

/*************
 *
 *   balance_ok()
 *
 *************/

static
BOOL balance_ok(Avl_node p)
{
  return (abs(avl_height(p->left) - avl_height(p->right)) <= 1);
}  /* balance_ok */

/*************
 *
 *   rotate_left()
 *
 *************/

static
Avl_node rotate_left(Avl_node p)
{
  Avl_node b = p;
  Avl_node c = p->right->left;
  Avl_node d = p->right;
  if (Debug)
    printf("rotate left\n");
  b->right = c; set_height_and_size(b);
  d->left  = b; set_height_and_size(d);
  return d;
}  /* rotate_left */

/*************
 *
 *   rotate_right()
 *
 *************/

static
Avl_node rotate_right(Avl_node p)
{
  Avl_node b = p->left;
  Avl_node c = p->left->right;
  Avl_node d = p;
  if (Debug)
    printf("rotate right\n");
  d->left  = c; set_height_and_size(d);
  b->right = d; set_height_and_size(b);
  return b;
}  /* rotate_right */

/*************
 *
 *   avl_fix()
 *
 *************/

static
Avl_node avl_fix(Avl_node p)
{
  if (balance_ok(p)) {
    set_height_and_size(p);
    return p;
  }
  else {
    if (avl_height(p->left) < avl_height(p->right)) {
      if (avl_height(p->right->left) <= avl_height(p->right->right))
	return rotate_left(p);
      else {
	p->right = rotate_right(p->right);
	return rotate_left(p);
      }
    }
    else {
      if (avl_height(p->left->right) <= avl_height(p->left->left))
	return rotate_right(p);
      else {
	p->left = rotate_left(p->left);
	return rotate_right(p);
      }
    }
  }
}  /* avl_fix */

/*************
 *
 *   avl_insert()
 *
 *************/

/* DOCUMENTATION
Insert an item into an AVL tree, and return the updated tree.
The item is an arbitrary pointer, and the caller gives a
function that takes two pointers and compares two items.
The comparison function must return LESS_THAN, SAME_AS, or
GREATER_THAN.
<p>
If the item is already in the tree (that is, if the comparison
function return SAME_AS for any item already in the tree),
a fatal error occurs.
*/

/* PUBLIC */
Avl_node avl_insert(Avl_node p, void *item,
		    Ordertype (*compare) (void *, void *))
{
  if (p == NULL) {
    Avl_node new = get_avl_node();
    new->item = item;
    new->size = 1;
    new->height = 1;
    new->left = NULL;
    new->right = NULL;
    return new;
  }
  else {
    Ordertype relation = (*compare)(item, p->item);
    switch (relation) {
    case LESS_THAN:    p->left  = avl_insert(p->left,  item, compare); break;
    case GREATER_THAN: p->right = avl_insert(p->right, item, compare); break;
    case SAME_AS: fatal_error("avl_insert, item already there"); break;
    default:      fatal_error("avl_insert, not comparable"); break;
    }

    p = avl_fix(p);  /* rebalance if necessary; set height, size */
    return p;
  }
}  /* avl_insert */

/*************
 *
 *   remove_and_return_largest()
 *
 *************/

static
Avl_node remove_and_return_largest(Avl_node p, Avl_node *removed)
{
  if (p->right == NULL) {
    *removed = p;
    return p->left;
  }
  else {
    p->right = remove_and_return_largest(p->right, removed);
    p = avl_fix(p);  /* rebalance if necessary; set height, size */
    return p;
  }
}  /* remove_and_return_largest */

/*************
 *
 *   remove_and_return_smallest()
 *
 *************/

static
Avl_node remove_and_return_smallest(Avl_node p, Avl_node *removed)
{
  if (p->left == NULL) {
    *removed = p;
    return p->right;
  }
  else {
    p->left = remove_and_return_smallest(p->left, removed);
    p = avl_fix(p);  /* rebalance if necessary; set height, size */
    return p;
  }
}  /* remove_and_return_smallest */

/*************
 *
 *   avl_delete()
 *
 *************/

/* DOCUMENTATION
Delete an item from an AVL tree and return the updated tree.
If the item is not in the tree (that is, if the comparison
function does not return SAME_AS for any item in the tree),
a fatal error occurs.
*/

/* PUBLIC */
Avl_node avl_delete(Avl_node p, void *item,
		    Ordertype (*compare) (void *, void *))
{
  if (p == NULL)
    fatal_error("avl_delete, item not found");
  else {
    Ordertype relation = (*compare)(item, p->item);

    if (relation == LESS_THAN)
      p->left  = avl_delete(p->left, item, compare);
    else if (relation == GREATER_THAN)
      p->right = avl_delete(p->right, item, compare);
    else if (relation != SAME_AS)
      fatal_error("avl_find: not comparable");
    else {
      Avl_node left  = p->left;
      Avl_node right = p->right;
      free_avl_node(p);
      p = NULL;
      if (left == NULL && right == NULL) {
	return NULL;
      }
      else if (avl_height(left) < avl_height(right))
	right = remove_and_return_smallest(right, &p);
      else
	left = remove_and_return_largest(left, &p);
      p->left = left;
      p->right = right;
    }
    p = avl_fix(p);
  }
  return p;
}  /* avl_delete */

/*************
 *
 *   avl_lookup()
 *
 *************/

static
Avl_node avl_lookup(Avl_node p, void *item,
		    Ordertype (*compare) (void *, void *))
{
  if (p == NULL)
    return NULL;
  else {
    Ordertype relation = (*compare)(item, p->item);

    switch (relation) {
    case SAME_AS:       return p;
    case LESS_THAN:     return avl_lookup(p->left,  item, compare);
    case GREATER_THAN:  return avl_lookup(p->right, item, compare);
    default: fatal_error("avl_lookup: not comparable");
    }
    return NULL;  /* won't happen */
  }
}  /* avl_lookup */

/*************
 *
 *   avl_find()
 *
 *************/

/* DOCUMENTATION
Look for an item in an AVL tree.  That is, look for an item
for which the comparison function returns SAME_AS.  If it
is found, return the item in the tree; otherwise return NULL;
*/

/* PUBLIC */
void *avl_find(Avl_node p, void *item,
	       Ordertype (*compare) (void *, void *))
{
  Avl_node n = avl_lookup(p, item, compare);
  return (n == NULL ? NULL : n->item);
}  /* avl_find */

/*************
 *
 *   avl_smallest()
 *
 *************/

/* DOCUMENTATION
Return the smallest (leftmost) item in an AVL tree.
*/

/* PUBLIC */
void *avl_smallest(Avl_node p)
{
  if (p == NULL)
    return NULL;  /* happens at top only */
  else if (p->left == NULL)
    return p->item;
  else
    return avl_smallest(p->left);
}  /* avl_smallest */

/*************
 *
 *   avl_largest()
 *
 *************/

/* DOCUMENTATION
Return the largest (rightmost) item in an AVL tree.
*/

/* PUBLIC */
void *avl_largest(Avl_node p)
{
  if (p == NULL)
    return NULL;  /* happens at top only */
  else if (p->right == NULL)
    return p->item;
  else
    return avl_largest(p->right);
}  /* avl_largest */

/*************
 *
 *   avl_place()
 *
 *************/

/* DOCUMENTATION
How far (counting from 1) from the beginning of the tree is the item.
If the item is not already in the tree, this function says how
far it would be if it were inserted first.
*/

/* PUBLIC */
int avl_place(Avl_node p, void *item,
	      Ordertype (*compare) (void *, void *))
{
  if (p == NULL)
    return 1;
  else {
    Ordertype relation = (*compare)(item, p->item);
    switch (relation) {
    case LESS_THAN:
      return avl_place(p->left, item, compare);
    case SAME_AS:
      return avl_size(p->left) + 1;
    case GREATER_THAN:
      return avl_size(p->left) + 1 + avl_place(p->right, item, compare);
    default:      fatal_error("avl_place, not comparable");
    }
    return INT_MAX;  /* won't happen */
  }
}  /* avl_place */

/*************
 *
 *   avl_position()
 *
 *************/

/* DOCUMENTATION
Return x, 0 < x <= 1, telling the position of the item in the tree.
The last item always has position 1.0.  The first item in a tree of
size 10 has position 0.1.
<p>
*/

/* PUBLIC */
double avl_position(Avl_node p, void *item,
		    Ordertype (*compare) (void *, void *))
{
  int place = avl_place(p, item, compare);
  return place / (double) avl_size(p);
}  /* avl_position */

/*************
 *
 *   avl_nth_item()
 *
 *************/

/* DOCUMENTATION
Return the n-th item (counting from 1) in an AVL tree.
If n is out of range, NULL is returned.
*/

/* PUBLIC */
void *avl_nth_item(Avl_node p, int n)
{
  if (p == NULL || n < 1 || n > p->size)
    return NULL;
  else if (n <= avl_size(p->left))
    return avl_nth_item(p->left, n);
  else if (n == avl_size(p->left)+1)
    return p->item;
  else
    return avl_nth_item(p->right, n - (avl_size(p->left)+1));
}  /* avl_nth_item */

/*************
 *
 *   avl_item_at_position()
 *
 *************/

/* DOCUMENTATION
Given an AVL tree and a position pos, 0.0 < pos <= 1.0,
return the item at that position.  If the tree is empty,
or if pos is out of range, NULL is returned.  The index
(counting from 1) of the returned item is
ceiling(pos * p->size).
*/

/* PUBLIC */
void *avl_item_at_position(Avl_node p, double pos)
{
  if (p == NULL || pos <= 0.0 || pos > 1.0)
    return NULL;
  else {
    int n = (int) ceil(pos * p->size);
    /* It should be, but make sure that 1 <= n <= p->size. */
    n = (n < 1 ? 1 : (n > p->size ? p->size : n));
    return avl_nth_item(p, n);
  }
}  /* avl_item_at_position */

/*************
 *
 *   avl_zap()
 *
 *************/

/* DOCUMENTATION
Free an entire AVL tree.
This does not affect any of the data to which the tree refers.
*/

/* PUBLIC */
void avl_zap(Avl_node p)
{
  if (p != NULL) {
    avl_zap(p->left);
    avl_zap(p->right);
    free_avl_node(p);
  }
}  /* avl_zap */

/*************
 *
 *   avl_check()
 *
 *************/

/* DOCUMENTATION
Check that each node of an AVL tree has the following properties:
(1) "size" and "height" fields are correct; 
(2) heights of the two children differ by at most 1.
In addition, check that the inorder traversal of the whole
tree really is in order.
*/

/* PUBLIC */
void avl_check(Avl_node p,
	       Ordertype (*compare) (void *, void *))
{
  if (p != NULL) {
    avl_check(p->left, compare);
    avl_check(p->right, compare);

    if (p->left && (*compare)(p->left->item, p->item) != LESS_THAN) {
      printf("error: left not smaller: %p\n", p->item);
      fprintf(stderr, "error: left not smaller: %p\n", p->item);
    }
    if (p->right && (*compare)(p->right->item, p->item) != GREATER_THAN) {
      printf("error: right not greater: %p\n", p->item);
      fprintf(stderr, "error: right not greater: %p\n", p->item);
    }
    if (p->height != IMAX(avl_height(p->left), avl_height(p->right)) + 1) {
      printf("error: height wrong: %p\n", p->item);
      fprintf(stderr, "error: height wrong: %p\n", p->item);
    }
    if (p->size != (avl_size(p->left) + avl_size(p->right)) + 1) {
      printf("error: size wrong: %p\n", p->item);
      fprintf(stderr, "error: size wrong: %p\n", p->item);
    }
    if (!balance_ok(p)) {
      printf("error: unbalanced: %p\n", p->item);
      fprintf(stderr, "error: unbalanced: %p\n", p->item);
    }
  }
}  /* avl_check */

/*************
 *
 *   p_avl()
 *
 *************/

/* DOCUMENTATION
Print an AVL tree to stdout.  The pointers to the items
are printed as integers.
*/

/* PUBLIC */
void p_avl(Avl_node p, int level)
{
  int i;
  if (p == NULL) {
    for (i = 0; i < level; i++)
      printf("    ");
    printf("----\n");
  }
  else {
    p_avl(p->right, level+1);
    for (i = 0; i < level; i++)
      printf("    ");
    printf("%4d\n", (int) p->item);
    p_avl(p->left, level+1);
  }
}  /* p_avl */

