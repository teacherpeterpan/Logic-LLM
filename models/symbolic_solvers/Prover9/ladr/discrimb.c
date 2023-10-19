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

#include "discrimb.h"

/* Private definitions and types */

/*
 * memory management
 */

typedef struct flat2 * Flat2;

struct flat2 {  /* for building a stack of states for backtracking */
  Term     t;
  Flat2    prev, next, last;
  Discrim  alternatives;
  int      bound;
  int      varnum;
  int      place_holder;
};

#define GO        1
#define BACKTRACK 2
#define SUCCESS   3
#define FAILURE   4

/*
 * memory management
 */

#define PTRS_FLAT2 PTRS(sizeof(struct flat2))
static unsigned Flat2_gets, Flat2_frees;

/*************
 *
 *   Flat2 get_flat2()
 *
 *************/

static
Flat2 get_flat2(void)
{
  Flat2 p = get_cmem(PTRS_FLAT2);
  Flat2_gets++;
  return(p);
}  /* get_flat2 */

/*************
 *
 *    free_flat2()
 *
 *************/

static
void free_flat2(Flat2 p)
{
  free_mem(p, PTRS_FLAT2);
  Flat2_frees++;
}  /* free_flat2 */

/*************
 *
 *   fprint_discrimb_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the discrimb package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_discrimb_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct flat2);
  fprintf(fp, "flat2 (%4d)        %11u%11u%11u%9.1f K\n",
          n, Flat2_gets, Flat2_frees,
          Flat2_gets - Flat2_frees,
          ((Flat2_gets - Flat2_frees) * n) / 1024.);

}  /* fprint_discrimb_mem */

/*************
 *
 *   p_discrimb_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the discrimb package.
*/

/* PUBLIC */
void p_discrimb_mem(void)
{
  fprint_discrimb_mem(stdout, TRUE);
}  /* p_discrimb_mem */

/*
 *  end of memory management
 */

/*************
 *
 *     check_discrim_bind_tree(d, n)
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void check_discrim_bind_tree(Discrim d, int n)
{
  if (n > 0) {
    int arity;
    Discrim d1;
    for (d1 = d->u.kids; d1; d1 = d1->next) {
      if (DVAR(d1))
	arity = 0;
      else
	arity = sn_to_arity(d1->symbol);
      check_discrim_bind_tree(d1, n+arity-1);
    }
  }
}  /* check_discrim_bind_tree */

/*************
 *
 *     print_discrim_bind_tree(fp, d, n, depth)
 *
 *************/

static
void print_discrim_bind_tree(FILE *fp, Discrim d, int n, int depth)
{
  int arity, i;

  for (i = 0; i < depth; i++)
    printf(" -");

  if (depth == 0)
    fprintf(fp, "\nroot");
  else if (DVAR(d))
    fprintf(fp, "v%d", d->symbol);
  else
    fprintf(fp, "%s", sn_to_str(d->symbol));

  fprintf(fp, "(%p)", d);
  if (n == 0) {
    Plist p;
    for (i = 0, p = d->u.data; p; i++, p = p->next);
    fprintf(fp, ": leaf has %d objects.\n", i);
  }
  else {
    Discrim d1;
    fprintf(fp, "\n");
    for (d1 = d->u.kids; d1 != NULL; d1 = d1->next) {
      if (DVAR(d1))
	arity = 0;
      else
	arity = sn_to_arity(d1->symbol);
      print_discrim_bind_tree(fp, d1, n+arity-1, depth+1);
    }
  }
}  /* print_discrim_bind_tree */

/*************
 *
 *   fprint_discrim_bind_index()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) a tame discrimination index.
*/

/* PUBLIC */
void fprint_discrim_bind_index(FILE *fp, Discrim d)
{
  print_discrim_bind_tree(fp, d, 1, 0);
}  /* print_discrim_bind_index */

/*************
 *
 *   p_discrim_bind_index()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) a tame discrimination index.
*/

/* PUBLIC */
void p_discrim_bind_index(Discrim d)
{
    fprint_discrim_bind_index(stdout, d);
}  /* p_discrim_bind_index */

/*************
 *
 *   Discrim discrim_bind_insert_rec(t, d)
 *
 *   Return node of d corresp. to end of term t.  If it does
 *   not exist, add nodes to t so that it does exist.
 *
 *************/

static
Discrim discrim_bind_insert_rec(Term t, Discrim d)
{
  Discrim d1, d2, prev;
  int symbol, i;

  if (VARIABLE(t)) {
    d1 = d->u.kids;
    prev = NULL;
    symbol = VARNUM(t);
    while (d1 && DVAR(d1) && d1->symbol < symbol) {
      prev = d1;
      d1 = d1->next;
    }
    if (d1 == NULL || !DVAR(d1) || d1->symbol != symbol) {
      d2 = get_discrim();
      d2->type = DVARIABLE;
      d2->symbol = VARNUM(t);
      d2->next = d1;
      if (prev == NULL)
	d->u.kids = d2;
      else
	prev->next = d2;
      return d2;
    }
    else  /* found node */
      return d1;
  }

  else {  /* constant || complex */
    d1 = d->u.kids;
    prev = NULL;
    /* arities fixed: handle both NAME and COMPLEX */
    symbol = SYMNUM(t);
    while (d1 && DVAR(d1)) {  /* skip variables */
      prev = d1;
      d1 = d1->next;
    }
    while (d1 && d1->symbol < symbol) {
      prev = d1;
      d1 = d1->next;
    }
    if (d1 == NULL || d1->symbol != symbol) {
      d2 = get_discrim();
      d2->type = DRIGID;
      d2->symbol = symbol;
      d2->next = d1;
      d1 = d2;
    }
    else
      d2 = NULL;  /* new node not required at this level */

    for (i = 0; i < ARITY(t); i++)
      d1 = discrim_bind_insert_rec(ARG(t,i), d1);

    if (d2 != NULL) {  /* link in new subtree (possibly a leaf) */
      if (prev == NULL)
	d->u.kids = d2;
      else
	prev->next = d2;
    }
	    
    return d1;  /* d1 is leaf corresp. to end of input term */
  }
}  /* discrim_bind_insert_rec */

/*************
 *
 *    discrim_bind_insert(t, root, object)
 *
 *************/

static
void discrim_bind_insert(Term t, Discrim root, void *object)
{
  Discrim d;
  Plist gp1, gp2;

  d = discrim_bind_insert_rec(t, root);
  gp1 = get_plist();
  gp1->v = object;
  gp1->next = NULL;

  /* Install at end of list. */
  if (d->u.data == NULL)
    d->u.data = gp1;
  else {
    for (gp2 = d->u.data; gp2->next != NULL; gp2 = gp2->next);
    gp2->next = gp1;
  }
}  /* discrim_bind_insert */

/*************
 *
 *    Discrim discrim_bind_end(t, d, path_p)
 *
 *    Given a discrimination tree (or a subtree) and a term, return the 
 *    node in the tree that corresponds to the last symbol in t (or NULL
 *    if the node doesn't exist).  *path_p is a list that is extended by
 *    this routine.  It is a list of pointers to the
 *    nodes in path from the parent of the returned node up to imd. 
 *    (It is useful for deletions, because nodes do not have pointers to
 *    parents.) 
 *
 *************/

static
Discrim discrim_bind_end(Term t, Discrim d, Plist *path_p)
{
  Discrim d1;
  Plist dp;
  int symbol, sym;

  /* add current node to the front of the path list. */

  dp = get_plist();
  dp->v = d;
  dp->next = *path_p;
  *path_p = dp;

  if (VARIABLE(t)) {
    d1 = d->u.kids;
    symbol = VARNUM(t);
    while (d1 && DVAR(d1) && d1->symbol < symbol) 
      d1 = d1->next;

    if (d1 == NULL || !DVAR(d1) || d1->symbol != symbol)
      return NULL;
    else   /* found node */
      return d1;
  }

  else {  /* constant || complex */
    d1 = d->u.kids;
    sym = SYMNUM(t);  /* arities fixed: handle both NAME and COMPLEX */
    while (d1 && DVAR(d1))  /* skip variables */
      d1 = d1->next;
    while (d1 && d1->symbol < sym)
      d1 = d1->next;

    if (d1 == NULL || d1->symbol != sym)
      return NULL;
    else {
      int i;
      for (i = 0; d1 && i < ARITY(t); i++)
	d1 = discrim_bind_end(ARG(t,i), d1, path_p);
      return d1;
    }
  }
}  /* discrim_bind_end */

/*************
 *
 *    discrim_bind_delete(t, root, object)
 *
 *************/

static
void discrim_bind_delete(Term t, Discrim root, void *object)
{
  Discrim end, d2, d3, parent;
  Plist tp1, tp2;
  Plist dp1, path;

    /* First find the correct leaf.  path is used to help with  */
    /* freeing nodes, because nodes don't have parent pointers. */

  path = NULL;
  end = discrim_bind_end(t, root, &path);
  if (end == NULL) {
    fatal_error("discrim_bind_delete, cannot find end.");
  }

    /* Free the pointer in the leaf-list */

  tp1 = end->u.data;
  tp2 = NULL;
  while(tp1 && tp1->v != object) {
    tp2 = tp1;
    tp1 = tp1->next;
  }
  if (tp1 == NULL) {
    fatal_error("discrim_bind_delete, cannot find term.");
  }

  if (tp2 == NULL)
    end->u.data = tp1->next;
  else
    tp2->next = tp1->next;
  free_plist(tp1);

  if (end->u.data == NULL) {
    /* free tree nodes from bottom up, using path to get parents */
    end->u.kids = NULL;  /* probably not necessary */
    dp1 = path;
    while (end->u.kids == NULL && end != root) {
      parent = (Discrim) dp1->v;
      dp1 = dp1->next;
      d2 = parent->u.kids;
      d3 = NULL;
      while (d2 != end) {
	d3 = d2;
	d2 = d2->next;
      }
      if (d3 == NULL)
	parent->u.kids = d2->next;
      else
	d3->next = d2->next;
      free_discrim(d2);
      end = parent;
    }
  }

  /* free path list */

  while (path) {
    dp1 = path;
    path = path->next;
    free_plist(dp1);
  }

}  /* discrim_bind_delete */

/*************
 *
 *   discrim_bind_update()
 *
 *************/

/* DOCUMENTATION
This routine inserts (op==INSERT) or deletes (op==DELETE)
an object into/from a tame discrimination index.
Term t is the key, root is the root of the discrimination tree,
and *object is a pointer (in many cases, *object will be t).
See discrim_tame_retrieve_first().
<P>
A fatal error occurs if yout ry to delete an object that was not
previouly inserted.
*/

/* PUBLIC */
void discrim_bind_update(Term t, Discrim root, void *object, Indexop op)
{
  if (op == INSERT)
    discrim_bind_insert(t, root, object);
  else
    discrim_bind_delete(t, root, object);
}  /* discrim_bind_update */

/*************
 *
 *  check_flat2
 *
 *************/

static
Flat2 check_flat2(Flat2 f)
{
  Flat2 last;
  int i, arity;

  if (f->next != NULL && f->next->prev != f)
    fprintf(stderr, "check_flat2: next-prev error\n");

  if (f->place_holder)
    arity = 0;
  else
    arity = ARITY(f->t);
  
  last = f;
  for (i = 0; i < arity; i++) 
    last = check_flat2(last->next);
  if (f->last != last)
    fprintf(stderr, "check_flat2: last is wrong\n");
  return last;
}  /* check_flat2 */

/*************
 *
 *  p_flat2
 *
 *************/

void p_flat2(Flat2 f)
{
  while (f != NULL) {
    if (VARIABLE(f->t))
      printf("v%d", VARNUM(f->t));
    else
      printf("%s", sn_to_str(SYMNUM(f->t)));
    if (f->place_holder)
      printf("[]");
    f = f->next;
    if (f != NULL)
      printf("-");
  }
  printf("\n");
}  /* p_flat2 */

/*************
 *
 *    discrim_bind_retrieve_leaf(t_in, root, subst, ppos)
 *
 *************/

static
Plist discrim_bind_retrieve_leaf(Term t_in, Discrim root,
			    Context subst, Flat2 *ppos)
{
  Flat2 f, f1, f2, f_save;
  Term t = NULL;
  Discrim d = NULL;
  int symbol = 0;
  int match = 0;
  int bound = 0;
  int status = 0;

  f = *ppos;  /* Don't forget to reset before return. */
  t = t_in;
  f_save = NULL;

  if (t != NULL) {  /* if first call */
    d = root->u.kids;
    if (d != NULL) {
      f = get_flat2();
      f->t = t;
      f->last = f;
      f->prev = NULL;
      f->place_holder = (COMPLEX(t));
      status = GO;
    }
    else
      status = FAILURE;
  }
  else
    status = BACKTRACK;

  while (status == GO || status == BACKTRACK) {
    if (status == BACKTRACK) {
      while (f && !f->alternatives) {  /* clean up HERE??? */
	if (f->bound) {
	  subst->terms[f->varnum] = NULL;
	  f->bound = 0;
	}
	f_save = f;
	f = f->prev;
      }
      if (f != NULL) {
	if (f->bound) {
	  subst->terms[f->varnum] = NULL;
	  f->bound = 0;
	}
	d = f->alternatives;
	f->alternatives = NULL;
	status = GO;
      }
      else
	status = FAILURE;
    }

    if (status == GO) {
      match = 0;
      while (!match && d && DVAR(d)) {
	symbol = d->symbol;
	if (subst->terms[symbol]) { /* if already bound */
	  match = term_ident(subst->terms[symbol], f->t);
	  bound = 0;
	}
	else { /* bind variable in discrimb tree */
	  match = 1;
	  subst->terms[symbol] = f->t;
	  bound = 1;
	}
	if (!match)
	  d = d->next;
      }
      if (match) {
	/* push alternatives */
	f->alternatives = d->next;
	f->bound = bound;
	f->varnum = symbol;
	f = f->last;
      }
      else if (VARIABLE(f->t))
	status = BACKTRACK;
      else {
	symbol = SYMNUM(f->t);
	while (d && d->symbol < symbol)
	  d = d->next;
	if (!d || d->symbol != symbol)
	  status = BACKTRACK;
	else if (f->place_holder) {
	  int i;
	  /* insert skeleton in place_holder */
	  f1 = get_flat2();
	  f1->t = f->t;
	  f1->prev = f->prev;
	  f1->last = f;
	  f_save = f1;
	  if (f1->prev)
	    f1->prev->next = f1;
		    
	  t = f->t;
	  for (i = 0; i < ARITY(t); i++) {
	    if (i < ARITY(t)-1)
	      f2 = get_flat2();
	    else
	      f2 = f;
	    f2->place_holder = COMPLEX(ARG(t,i));
	    f2->t = ARG(t,i);
	    f2->last = f2;
	    f2->prev = f1;
	    f1->next = f2;
	    f1 = f2;
	  }
	  f = f_save;
	}  /* if f->place_holder */
      }
      if (status == GO) {
	if (f->next) {
	  f = f->next;
	  d = d->u.kids;
	}
	else
	  status = SUCCESS;
      }
    }
  }  /* while */
  if (status == SUCCESS) {
    *ppos = f;
    return d->u.data;
  }
  else {
    /* Free flat2s. */
#ifdef SPEED
    Flat2count = 0;
#else
    while (f_save) {
      f1 = f_save;
      f_save = f_save->next;
      free_flat2(f1);
    }
#endif
    return NULL;
  }
	   
}  /* discrim_bind_retrieve_leaf */

/*************
 *
 *    discrim_bind_retrieve_first(t, root, subst, ppos)
 *
 *    Get the first object associated with a term more general than t.
 *
 *    Remember to call discrim_bind_cancel(*ppos) if you don't want the
 *    whole sequence.
 *
 *************/

/* DOCUMENTATION
This routine, along with discrim_bind_retrieve_next(), gets answers from
a tame discrimination index.
This routine retrieves the first object associated with a term, say ft,
more general than Term t.  (NULL is returned if there is none.)
The substitution for variables of ft is placed into Context subst.
<P>
If an object is returned, Discrim_pos *ppos is set to the retrieval
state and is used for subsequent discrim_tame_retrieve_next() calls.
<P>
If you to get some, but not all answers, you must call
discrim_tame_cancel() to clear the substitution and free memory
associated with the Discrim_pos.
*/

/* PUBLIC */
void *discrim_bind_retrieve_first(Term t, Discrim root,
				  Context subst, Discrim_pos *ppos)
{
  Plist tp;
  Flat2 f;
  Discrim_pos gp;

  tp = discrim_bind_retrieve_leaf(t, root, subst, &f);
  if (tp == NULL)
    return NULL;
  else {
    gp = get_discrim_pos();
    gp->subst = subst;
    gp->backtrack = f;
    gp->data = tp;
    *ppos = gp;
    return tp->v;
  }
}  /* discrim_bind_retrieve_first */

/*************
 *
 *    discrim_bind_retrieve_next(ppos)
 *
 *    Get the next object associated with a term more general than t.
 *
 *    Remember to call discrim_bind_cancel(*ppos) if you don't want the
 *    whole sequence.
 *
 *************/

/* DOCUMENTATION
This routine retrieves the next object in the sequence of answers to
a query of a tame discrimbination tree.
You must <I>not</I> explicitly clear the Context you gave to
discrim_bind_retrieve_first()---that is handled internally.
See discrim_bind_retrieve_first().
*/

/* PUBLIC */
void *discrim_bind_retrieve_next(Discrim_pos pos)
{
  Plist tp;
    
  tp = pos->data->next;
  if (tp != NULL) {  /* if any more terms in current leaf */
    pos->data = tp;
    return tp->v;
  }
  else {  /* try for another leaf */
    tp = discrim_bind_retrieve_leaf(NULL, NULL,
				    pos->subst, (Flat2 *) &(pos->backtrack));
    if (tp != NULL) {
      pos->data = tp;
      return tp->v;
    }
    else {
      free_discrim_pos(pos);
      return NULL;
    }
  }
}  /* discrim_bind_retrieve_next */

/*************
 *
 *    discrim_bind_cancel(pos)
 *
 *************/

/* DOCUMENTATION
This routine <I>must</I> be called if you get some, but not all
answers to a tame discrimbintaion query.
The Context (which was given to the discrim_bind_retrieve_first() call) is
cleared, and the memory associated the retrieval state is freed.
*/

/* PUBLIC */
void discrim_bind_cancel(Discrim_pos pos)
{
  Flat2 f1, f2;

  f1 = pos->backtrack;
  while (f1) {
    if (f1->bound)
      pos->subst->terms[f1->varnum] = NULL;
    f2 = f1;
    f1 = f1->prev;
#ifndef SPEED
    free_flat2(f2);
#endif
  }
#ifdef SPEED
  Flat2count = 0;
#endif
  free_discrim_pos(pos);
}  /* discrim_bind_cancel */

