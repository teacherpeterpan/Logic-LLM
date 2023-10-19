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

#include "di_tree.h"

/* Private definitions and types */

static int Nonunit_fsub_tests;
static int Nonunit_bsub_tests;

static unsigned Sub_calls = 0;
static unsigned Sub_calls_overflows = 0;
#define BUMP_SUB_CALLS {Sub_calls++; if (Sub_calls == 0) Sub_calls_overflows++;}

/*************
 *
 *   nonunit_fsub_tests(void)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int nonunit_fsub_tests(void)
{
  return Nonunit_fsub_tests;
}  /* nonunit_fsub_tests */

/*************
 *
 *   nonunit_bsub_tests(void)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int nonunit_bsub_tests(void)
{
  return Nonunit_bsub_tests;
}  /* nonunit_bsub_tests */

/*
 * memory management
 */

#define PTRS_DI_TREE PTRS(sizeof(struct di_tree))
static unsigned Di_tree_gets, Di_tree_frees;

/*************
 *
 *   Di_tree get_di_tree()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Di_tree get_di_tree(void)
{
  Di_tree p = get_cmem(PTRS_DI_TREE);
  Di_tree_gets++;
  return(p);
}  /* get_di_tree */

/*************
 *
 *    free_di_tree()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_di_tree(Di_tree p)
{
  free_mem(p, PTRS_DI_TREE);
  Di_tree_frees++;
}  /* free_di_tree */

/*************
 *
 *   fprint_di_tree_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the di_tree package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_di_tree_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct di_tree);
  fprintf(fp, "di_tree (%4d)      %11u%11u%11u%9.1f K\n",
          n, Di_tree_gets, Di_tree_frees,
          Di_tree_gets - Di_tree_frees,
          ((Di_tree_gets - Di_tree_frees) * n) / 1024.);

}  /* fprint_di_tree_mem */

/*************
 *
 *   p_di_tree_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the di_tree package.
*/

/* PUBLIC */
void p_di_tree_mem(void)
{
  fprint_di_tree_mem(stdout, TRUE);
}  /* p_di_tree_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   init_di_tree()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns an empty integer-vector
discrimination index.
*/

/* PUBLIC */
Di_tree init_di_tree(void)
{
  return get_di_tree();
}  /* init_di_tree */

/*************
 *
 *   di_tree_insert()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void di_tree_insert(Ilist vec, Di_tree node, void *datum)
{
  if (vec == NULL) {
    Plist p = get_plist();
    p->v = datum;
    p->next = node->u.data;
    node->u.data = p;
  }
  else {
    Di_tree prev = NULL;
    Di_tree curr = node->u.kids;
    /* kids are in increasing order */
    while (curr && vec->i > curr->label) {
      prev = curr;
      curr = curr->next;
    }
    if (curr == NULL || vec->i != curr->label) {
      Di_tree new = get_di_tree();
      new->label = vec->i;
      new->next = curr;
      if (prev)
	prev->next = new;
      else
	node->u.kids = new;
      curr = new;
    }
    di_tree_insert(vec->next, curr, datum);
  }
}  /* di_tree_insert */

/*************
 *
 *   di_tree_delete()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL di_tree_delete(Ilist vec, Di_tree node, void *datum)
{
  if (vec == NULL) {
    node->u.data = plist_remove(node->u.data, datum);
    return node->u.data != NULL;  /* tells parent whether to keep node */
  }
  else {
    BOOL keep;
    Di_tree prev = NULL;
    Di_tree curr = node->u.kids;
    /* kids are in increasing order */
    while (curr && vec->i > curr->label) {
      prev = curr;
      curr = curr->next;
    }
    if (curr == NULL || vec->i != curr->label)
      fatal_error("di_tree_delete, node not found");
    keep = di_tree_delete(vec->next, curr, datum);
    if (keep)
      return TRUE;
    else {
      if (prev)
	prev->next = curr->next;
      else
	node->u.kids = curr->next;
      free_di_tree(curr);
      return node->u.kids != NULL;
    }
  }
}  /* di_tree_delete */

/*************
 *
 *   zap_di_tree()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void zap_di_tree(Di_tree node, int depth)
{
  if (depth == 0)
    zap_plist(node->u.data);
  else {
    Di_tree kid = node->u.kids;
    while (kid) {
      Di_tree tmp = kid;
      kid = kid->next;
      zap_di_tree(tmp, depth-1);
    }
  }
  free_di_tree(node);
}  /* zap_di_tree */

/*************
 *
 *   p_di_tree()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_di_tree(Ilist vec, Di_tree node, int depth)
{
  int i;
  for (i = 0; i < depth; i++)
    printf(" ");
  if (vec == NULL) {
    Plist p = node->u.data;
    printf("IDs:");
    while (p) {
      Topform c = p->v;
      printf(" %d", c->id);
      p = p->next;
    }
    printf("\n");
  }
  else {
    Di_tree kid;
    printf("%d\n", node->label);
    for (kid = node->u.kids; kid; kid = kid->next)
      p_di_tree(vec->next, kid, depth+1);
  }
}  /* p_di_tree */

/*************
 *
 *   subsume_di_literals()
 *
 *************/

static
BOOL subsume_di_literals(Literals clit, Context subst, Literals d, Trail *trp)
{
  BOOL subsumed = FALSE;
  Literals dlit;
  BUMP_SUB_CALLS;
  if (clit == NULL)
    return TRUE;
  else {
    for (dlit = d; !subsumed && dlit != NULL; dlit = dlit->next) {
      if (clit->sign == dlit->sign) {
	Trail mark = *trp;
	if (match(clit->atom, subst, dlit->atom, trp)) {
	  if (subsume_di_literals(clit->next, subst, d, trp))
	    subsumed = TRUE;
	  else {
	    undo_subst_2(*trp, mark);
	    *trp = mark;
	  }
	}
      }
    }
    return subsumed;
  }
}  /* subsume_di_literals */

/*************
 *
 *   subsumes_di()
 *
 *************/

static
BOOL subsumes_di(Literals c, Literals d, Context subst)
{
  Trail tr = NULL;
  BOOL subsumed = subsume_di_literals(c, subst, d, &tr);
  if (subsumed)
    undo_subst(tr);
  return subsumed;
}  /* subsumes_di */

/*************
 *
 *   di_tree_forward()
 *
 *************/

static
Topform di_tree_forward(Ilist vec, Di_tree node, Literals dlits, Context subst)
{
  BUMP_SUB_CALLS;
  if (vec == NULL) {
    Plist p = node->u.data;
    while (p) {
      Topform c = p->v;
      Nonunit_fsub_tests++;
      if (subsumes_di(c->literals, dlits, subst))
	return p->v;
      p = p->next;
    }
    return NULL;
  }
  else {
    void *datum = NULL;
    Di_tree kid = node->u.kids;
    /* kids are in increasing order; look at those <= vec->i */
    while (!datum && kid && kid->label <= vec->i) {
      datum = di_tree_forward(vec->next, kid, dlits, subst);
      kid = kid->next;
    }
    return datum;
  }
}  /* di_tree_forward */

/*************
 *
 *   forward_feature_subsume()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform forward_feature_subsume(Topform d, Di_tree root)
{
  Ilist f = features(d->literals);
  Context subst = get_context();
  Topform c = di_tree_forward(f, root, d->literals, subst);
  free_context(subst);
  zap_ilist(f);
  return c;
}  /* forward_feature_subsume */

/*************
 *
 *   di_tree_back()
 *
 *************/

static
void di_tree_back(Ilist vec, Di_tree node, Literals clits, Context subst,
		  Plist *subsumees)
{
  BUMP_SUB_CALLS;
  if (vec == NULL) {
    Plist p = node->u.data;
    while (p) {
      Topform d = p->v;
      Nonunit_bsub_tests++;
      if (clits != d->literals && subsumes_di(clits, d->literals, subst))
	*subsumees = plist_prepend(*subsumees, d);
      p = p->next;
    }
  }
  else {
    Di_tree kid = node->u.kids;
    /* kids are in increasing order; look at those >= vec->i */
    while (kid && kid->label < vec->i)
      kid = kid->next;
    while (kid) {
      di_tree_back(vec->next, kid, clits, subst, subsumees);
      kid = kid->next;
    }
  }
}  /* di_tree_back */

/*************
 *
 *   back_feature_subsume()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist back_feature_subsume(Topform c, Di_tree root)
{
  Ilist f = features(c->literals);
  Context subst = get_context();
  Plist subsumees = NULL;
  di_tree_back(f, root, c->literals, subst, &subsumees);
  free_context(subst);
  zap_ilist(f);
  return subsumees;
}  /* back_feature_subsume */

/*************
 *
 *   mega_sub_calls()
 *
 *************/

/* DOCUMENTATION
 */

/* PUBLIC */
unsigned mega_sub_calls(void)
{
  return
    (Sub_calls / 1000000) +
    ((UINT_MAX / 1000000) * Sub_calls_overflows);
}  /* mega_sub_calls */
