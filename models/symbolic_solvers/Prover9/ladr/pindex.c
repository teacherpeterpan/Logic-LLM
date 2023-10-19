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

#include "pindex.h"

/* Private definitions and types */

struct pair_index {
    int finished;        /* set if nothing to retrieve */
    int n;               /* number of lists */
    int i, j;            /* working pair */
    int min;             /* smallest wt of inserted clause */
    int new_min;         /* smallest inserted wt since previous retrieval */
    Clist *lists;         /* lists */
    Clist_pos *top;
    Clist_pos *curr;
    struct pair_index *next;  /* for avail list */
    };

#define INT_LARGE INT_MAX / 2  /* It can be doubled without overflow. */
#define IN_RANGE(i, min, max) (i < min ? min : (i > max ? max : i))

/*
 * memory management
 */

#define PTRS_PAIR_INDEX PTRS(sizeof(struct pair_index))
static unsigned Pair_index_gets, Pair_index_frees;

/*************
 *
 *   Pair_index get_pair_index()
 *
 *************/

static
Pair_index get_pair_index(void)
{
  Pair_index p = get_cmem(PTRS_PAIR_INDEX);
  p->min = INT_LARGE;
  p->new_min = INT_LARGE;
  Pair_index_gets++;
  return(p);
}  /* get_pair_index */

/*************
 *
 *    free_pair_index()
 *
 *************/

static
void free_pair_index(Pair_index p)
{
  free_mem(p, PTRS_PAIR_INDEX);
  Pair_index_frees++;
}  /* free_pair_index */

/*************
 *
 *   fprint_pindex_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the pindex package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_pindex_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct pair_index);
  fprintf(fp, "pair_index (%4d)   %11u%11u%11u%9.1f K\n",
          n, Pair_index_gets, Pair_index_frees,
          Pair_index_gets - Pair_index_frees,
          ((Pair_index_gets - Pair_index_frees) * n) / 1024.);

}  /* fprint_pindex_mem */

/*************
 *
 *   p_pindex_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the pindex package.
*/

/* PUBLIC */
void p_pindex_mem()
{
  fprint_pindex_mem(stdout, TRUE);
}  /* p_pindex_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   init_pair_index()
 *
 *
 *************/

/* DOCUMENTATION
This routine allocates and initializes a Pair_index.
Parameter n specifies the range 0 .. n-1 of weights
that will be used.  If a clause is inserted with weight
outside of that range, the effective weight for pair
indexing will be set to 0 or n-1.
*/

/* PUBLIC */
Pair_index init_pair_index(int n)
{
  Pair_index p;
  int i, j;

  p = get_pair_index();

  p->finished = 1;
  p->n = n;
  p->i = 0;
  p->j = 0;
  p->min = INT_LARGE;
  p->new_min = INT_LARGE;

  p->lists = malloc(n * sizeof(Clist));
  p->top   = malloc(n * n * sizeof(Clist_pos));
  p->curr  = malloc(n * n * sizeof(Clist_pos));

  /* top and curr will be indexed as top[i*n+j]. */

  for (i = 0; i < n; i++)
    p->lists[i] = clist_init("");
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      p->top[i*p->n+j] = NULL;
      p->curr[i*p->n+j] = NULL;
    }

  return p;
}  /* init_pair_index */

/*************
 *
 *   zap_pair_index()
 *
 *
 *************/

/* DOCUMENTATION
This routine frees a pair index.  It need not be empty.
*/

/* PUBLIC */
void zap_pair_index(Pair_index p)
{
  int i;
  for (i = 0; i < p->n; i++) {
    clist_zap(p->lists[i]);
  }
  free(p->lists);
  free(p->top);
  free(p->curr);
  free_pair_index(p);
}  /* zap_pair_index */

/*************
 *
 *   pairs_exhausted()
 *
 *************/

/* DOCUMENTATION
<I>
This routine is TRUE if the previous call to retrieve_pair()
returned nothing and no more pairs have been inserted since then.
(Also, TRUE is returned if no pairs were ever inserted.)
<P>
Note that FALSE may be returned when there really no pairs
available.
</I>
*/

/* PUBLIC */
int pairs_exhausted(Pair_index p)
{
  return p->finished;
}  /* pairs_exhausted */

/*************
 *
 *   init_pair()
 *
 *   Initialize top and curr for lists i and j.
 *
 *************/

static
void init_pair(int i, int j, Pair_index p)
{
  int n = p->n;
  Clist_pos lp_i = p->lists[i]->first;
  Clist_pos lp_j = p->lists[j]->first;

  if (lp_i && lp_j) {
    if (i == j) {
      p->top[i*n+i] = lp_i;
      p->curr[i*n+i] = NULL;
    }
    else {
      p->top[i*n+j] = lp_i;
      p->top[j*n+i] = lp_j;
      /* It doesn't matter which curr gets set to NULL. */
      p->curr[i*n+j] = lp_i;
      p->curr[j*n+i] = NULL;
    }
  }
  else {
    p->top[i*n+j] = NULL;
    p->top[j*n+i] = NULL;
    p->curr[i*n+j] = NULL;
    p->curr[j*n+i] = NULL;
  }
}  /* init_pair */

/*************
 *
 *   insert_pair_index()
 *
 *************/

/* DOCUMENTATION
This routine inserts a clause into a Pair_index.
If the given weight is out of range [0 ... n-1] (where
n is the parameter given to init_pair_index()),
weight 0 or n-1 will be used instead.
*/

/* PUBLIC */
void insert_pair_index(Topform c, int wt, Pair_index p)
{
  /* If the new clause will be the only one in its list, then
   * for each nonempty list, set the top and curr.
     */
  int i, j, n;

  n = p->n;
  j = IN_RANGE(wt, 0, n-1);

  if (p->lists[j]->first == NULL) {
    clist_append(c, p->lists[j]);
    for (i = 0; i < p->n; i++)
      init_pair(i, j, p);
  }
  else
    clist_append(c, p->lists[j]);

  p->finished = 0;
  if (wt < p->new_min)
    p->new_min = wt;
  if (wt < p->min)
    p->min = wt;
}  /* insert_pair_index */

/*************
 *
 *   delete_pair_index()
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from a Pair_index.
The parameter wt must be the same as when the clause was inserted.
A fatal error may occur if the clause was not previously
inserted or if it was inserted with a different weight.
*/

/* PUBLIC */
void delete_pair_index(Topform c, int wt, Pair_index p)
{
  int i, j;
  int n = p->n;
  Clist_pos lp;

  j = IN_RANGE(wt, 0, n-1);

  for (lp = p->lists[j]->first; lp && lp->c != c; lp = lp->next);
  if (!lp) {
    fatal_error("delete_pair_index, clause not found.");
  }

  /* We are deleting a clause from list j.  For each list i, consider the
   * pair [i,j].  Top[i,j] and curr[i,j] (say t1 and c1) point into list i,
   * and top[j,i] and curr[j,i] (say t2 anc c2) point into list j.
   */

  for (i = 0; i < n; i++) {
    Clist_pos t1 = p->top[i*n+j];
    Clist_pos c1 = p->curr[i*n+j];
    Clist_pos t2 = p->top[j*n+i];
    Clist_pos c2 = p->curr[j*n+i];

    if (i == j) {
      if (t2 == lp) {
	/* printf("HERE: i == j\n"); */
	/* This handles t2=c2, c2==NULL, c2 != NULL, singleton list. */
	if (t2->next) {
	  p->top[i*n+i] = t2->next;
	  p->curr[i*n+i] = NULL;
	}
	else {
	  p->top[i*n+i] = t2->prev;
	  p->curr[i*n+i] = t2->prev;
	}
      }
      else if (c2 == lp) {
	p->curr[i*n+i] = c2->prev;
      }
    }
    else {  /* i != j */

      if (lp == t2) {
	/* printf("HERE: i != j (B)\n"); */
	if (t2 == c2) {
	  if (t2->next) {
	    t2 = t2->next;
	    c2 = c2->next;
	    c1 = NULL;
	  }
	  else if (t2->prev) {
	    t2 = t2->prev;
	    c2 = c2->prev;
	    c1 = t1;
	  }
	  else
	    t1 = c1 = t2 = c2 = NULL;
	}
	else if (t2->prev)
	  t2 = t2->prev;
	else if (t2->next) {
	  t2 = t2->next;
	  c2 = NULL;
	  t1 = c1 = p->lists[i]->first;
	}
	else
	  t1 = c1 = t2 = c2 = NULL;
      }
      else if (lp == c2) {
	/* printf("HERE: i != j (D)\n"); */
	c2 = c2->prev;
      }

      p->top[i*n+j] = t1;
      p->curr[i*n+j] = c1;
      p->top[j*n+i] = t2;
      p->curr[j*n+i] = c2;
    }
  }
  clist_remove(c, p->lists[j]);
}  /* delete_pair_index */

/*************
 *
 *   retrieve_pair()
 *
 *************/

/* DOCUMENTATION
This routine retrieves the next pair from a Pair_index.
The caller gives addresses of clauses which are filled
in with the answer.  If no pair is available, NULLs are
filled in.
*/

/* PUBLIC */
void retrieve_pair(Pair_index p, Topform *cp1, Topform *cp2)
{
  int i, j, k, max_k, found, n;

  /* First, find i and j, the smallest pair of weights that
   * have clauses available.  p->i and p->j are from the
   * previous retrieval, and if no clauses have been inserted
   * since then, start with them.  Otherwise, use new_min
   * (the smallest weight inserted since the previous retrieval)
   * and min (the smallest weight in the index) to decide
   * where to start looking.
   */ 
     
  if (p->min + p->new_min < p->i + p->j) {
    i = p->min;
    j = p->new_min;
  }
  else {
    i = p->i;
    j = p->j;
  }

  n = p->n;
  k = i+j;
  max_k = (n + n) - 2;
  found = 0;

  while (k <= max_k && !found) {
    i = k / 2; j = k - i;
    while (i >= 0 && j < n && !found) {
      /* This test works if (i==j). */
      found = (p->top[i*n+j] != p->curr[i*n+j] ||
	       p->top[j*n+i] != p->curr[j*n+i] ||
	       (p->top[i*n+j] && p->top[i*n+j]->next) ||
	       (p->top[j*n+i] && p->top[j*n+i]->next));
		     		     
      if (!found) {
	i--; j++;
      }
    }
    if (!found)
      k++;
  }

  if (!found) {
    *cp1 = NULL; *cp2 = NULL;
    p->finished = 1;
  }
  else {
    /* OK, there should be a pair in (i,j). */

    /* Recall that if top[i,j]=curr[i,j] and top[j,i]=top[j,i],
     * then all pairs up to those positions have been returned.
     */

    Clist_pos t1 = p->top[i*n+j];
    Clist_pos c1 = p->curr[i*n+j];
    Clist_pos t2 = p->top[j*n+i];
    Clist_pos c2 = p->curr[j*n+i];

    if (i == j) {
      if (t1 == c1) {
	p->top[i*n+i]  = t1 =  t1->next;
	p->curr[i*n+i] = c1 = NULL;
      }
      *cp1 = t1->c;
      p->curr[i*n+i] = c1 = (c1 ? c1->next : p->lists[i]->first);
      *cp2 = c1->c;
    }
    else {  /* i != j */
      if (t1 == c1 && t2 == c2) {
	/* Both tops equal their currs, so pick a top to increment. */
	if (t1->next && (t1->c->id < t2->c->id || !t2->next)) {
	  p->top[i*n+j]  = t1 = t1->next;
	  p->curr[i*n+j] = c1 = c1->next;
	  p->curr[j*n+i] = c2 = NULL;
	}
	else {
	  p->top[j*n+i]  = t2 = t2->next;
	  p->curr[j*n+i] = c2 = c2->next;
	  p->curr[i*n+j] = c1 = NULL;
	}
      }
      if (t1 == c1) {
	*cp1 = t1->c;
	p->curr[j*n+i] = c2 = (c2 ? c2->next : p->lists[j]->first);
	*cp2 = c2->c;
      }
      else if (t2 == c2) {
	*cp1 = t2->c;
	p->curr[i*n+j] = c1 = (c1 ? c1->next : p->lists[i]->first);
	*cp2 = c1->c;
      }
      else {
	fatal_error("retrieve_pair, bad state.");
      }
    }
    /* Save the "working pair" for next time. */
    p->i = i;
    p->j = j;
    p->new_min = INT_LARGE;
  }
}  /* retrieve_pair */

/*************
 *
 *   p_pair_index()
 *
 *   Print a pair index.  It is printed in detail, so it should be
 *   called only for small test indexes.
 *
 *************/

void p_pair_index(Pair_index p)
{
  int i, j, n;
  Clist_pos x;

  n = p->n;

  for (i = 0; i < n; i++) {
    printf("Clist %d: ", i);
    for (x = p->lists[i]->first; x; x = x->next)
      printf(" %3d", x->c->id);
    printf(".\n");
  }
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      printf(" [");
      if (p->top[i*n+j])
	printf("%2d",p->top[i*n+j]->c->id);
      else
	printf("--");
      printf(",");
      if (p->curr[i*n+j])
	printf("%2d",p->curr[i*n+j]->c->id);
      else
	printf("--");
      printf("] ");
      fflush(stdout);
    }
    printf("\n");
  }
}  /* p_pair_index */

/*************
 *
 *   pair_already_used()
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if a pair of clauses (with corresponding
weights) has already been retrieved.
*/

/* PUBLIC */
int pair_already_used(Topform c1, int weight1,
		      Topform c2, int weight2,
		      Pair_index p)
{
  int i, j, id1, id2;
  int rc = 0;
  int n = p->n;
  Clist_pos top1, curr1, top2, curr2;

  id1 = c1->id;
  id2 = c2->id;

  i = IN_RANGE(weight1, 0, n-1);
  j = IN_RANGE(weight2, 0, n-1);

  top1 = p->top[i*n+j];
  curr1 = p->curr[i*n+j];

  top2 = p->top[j*n+i];
  curr2 = p->curr[j*n+i];

  if (!top1 || !top2) {
    /* One of the lists is empty.  If this happens, something is
       probably wrong: why would we be trying to use c1 and c2? 
       */
    fatal_error("pair_already_used, bad state (1).");
  }
  else if (i == j) {
    /* Let id2 be the greater one. */
    if (id1 > id2) {
      int tmp = id1; id1 = id2; id2 = tmp;
    }
    rc = ((id2 < top1->c->id) ||
	  (id2 == top1->c->id && curr1 && id1 <= curr1->c->id));
  }
  else {  /* i != j */

    if (top1 == curr1) {
      rc = ((id1 <  top1->c->id && id2 <= top2->c->id) ||
	    (id1 == top1->c->id && curr2 && id2 <= curr2->c->id));
    }
    else if (top2 == curr2) {
      rc = ((id2 <  top2->c->id && id1 <= top1->c->id) ||
	    (id2 == top2->c->id && curr1 && id1 <= curr1->c->id));
    }
    else {
      fatal_error("pair_already_used, bad state (2).");
    }
  }	

  return rc;
}  /* pair_already_used */

/*
 * This file has code for indexing clauses that are to be retrieved in
 * pairs.  When a clause is inserted, its weight is given.  Retrieval
 * is by sum of the weights of the pair -- lowest first.  Say we have
 * clauses with weights 0--4.  Then pairs will be returned in this order:
 *
 * (0,0)
 * (0,1)
 * (1,1)  (0,2)
 * (1,2)  (0,3)
 * (2,2)  (1,3)  (0,4)
 * (2,3)  (1,4)
 * (3,3)  (2,4)
 * (3,4)
 * (4,4)
 *
 * Objects can be inserted after retrieval has begun; the smallest
 * available pair will always be returned.  When the index is
 * initialized, the caller supplies a parameter N, and the actual
 * weight range for indexing will be 0..N-1.  If an inserted clause has
 * weight outside of this range, the weight will be changed to 0 or N-1.
 *
 * This is intended to be used for binary inference rules such as
 * paramodulation and resolution.
 *
 * We keep a list of clauses with each weight 0..N-1.  When the index
 * is initialized, we allocate two N x N arrays (top and curr)
 * to keep track of the positions in the pairs of lists.
 * (Since the size of the array is not known at compile time,
 * we access elements as a[i*N+j] instead of a[i][j].)
 *
 * For a pair of weights i and j, we use top[i,j] and curr[i,j] to
 * point to positions in list i w.r.t. list j.  And vice versa.
 * Roughly, it goes like this: Lists grow upward, and clauses above
 * "top" have not yet been considered.  Increment top[i,j], then return
 * top[i,j] with each member of list j up to top[j,i].  (Curr[j,i]
 * marks the position between retrievals.)  Now all pairs up to the two
 * tops have been returned.  Now pick the top with the smallest clause
 * ID, increment it, and continue.  If at any point a new clause C is
 * inserted into the index, and C goes with other clasues to make
 * smaller pairs, those smaller pairs are returned before continuing
 * with i and j.
 *
 * Valid states of an index.
 * Case 1:  i == j.
 *   (a) top and curr are both NULL; this occurs if the list is empty.
 *   (b) top and top=curr; done up through top.
 *   (c) top and !curr; done up through top-1.
 *   (d) top and curr and top > curr; done up through <top,curr>.
 * Case 2: i != j.
 *   (a) !t1 and !t2; one or both lists empty.
 *   (b) t1 and t2 and t1=c1 and t2=c2; done up through tops.
 *   (c) t1 and t2 and t1=c1 and c2=NULL; done up through <t1-1, t2>.
 *   (d) t1 and t2 and t1=c1 and c2<t2; done through <t1-1, t2>;
 *       also through <t1,c2>.
 *   (e) same as c,d, but vice versa.
 * 
 * This is similar to the method in "A Theorem-Proving Language
 * for Experimentation" by Henschen, Overbeek, Wos, CACM 17(6), 1974.
 * 
 */

