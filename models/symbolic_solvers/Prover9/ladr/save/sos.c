#include "sos.h"

/****************************************************************************/

/* Private definitions and types */

/* Stree is a simple binary search tree organized by Topform->weight.
   Each node has a Clist of clauses (ordered by ID) with that weight.

   This structure will be used to order clasues first by weight, then by ID.

   In a typical search there aren't very many different weights,
   so it doesn't seem important to balance the tree; that can be
   added later if necessary.

 */

typedef struct stree * Stree;

struct stree {
  int weight;       /* weight of clauses in this node */
  Clist clauses;    /* clauses (ordered by increasing ID) in this node */
  int n;            /* number of clauses in this subtree (node and children) */
  int greatest_id;  /* greatest ID in this subtree */
  Stree left;
  Stree right;
};

/*
 * memory management
 */

#define PTRS_STREE PTRS(sizeof(struct stree))

/* #define PTRS_STREE PTRS(sizeof(struct stree)) */
static unsigned Stree_gets, Stree_frees;

/*************
 *
 *   Stree get_stree()
 *
 *************/

static
Stree get_stree(void)
{
  Stree p = get_cmem(PTRS_STREE);
  Stree_gets++;
  return(p);
}  /* get_stree */

/*************
 *
 *    free_stree()
 *
 *************/

static
void free_stree(Stree p)
{
  free_mem(p, PTRS_STREE);
  Stree_frees++;
}  /* free_stree */

/*************
 *
 *   p_stree()
 *
 *************/

static
void p_stree(Stree s)
{
  if (s == NULL)
    printf("Slist NULL\n");
  else {
    p_stree(s->left);
    printf("\nWeight=%d.\n", s->weight);
    fprint_clause_clist(stdout, s->clauses);
    p_stree(s->right);
  }
}  /* p_stree */

/*************
 *
 *   p_dist()
 *
 *************/

static
void p_dist(Stree s)
{
  if (s != NULL) {
    p_dist(s->left);
    printf("Weight=%d, clauses=%d.\n", s->weight, s->clauses->length);
    p_dist(s->right);
  }
}  /* p_dist */

/*************
 *
 *   stree_insert()
 *
 *************/

static
Stree stree_insert(Stree s, Topform c)
{
  if (s == NULL) {
    s = get_stree();
    s->weight = c->weight;
    s->clauses = clist_init("clauses_by_weight");
    clist_append(c, s->clauses);
  }
  else if (c->weight == s->weight) {
    clist_append(c, s->clauses);
  }
  else if (c->weight < s->weight)
    s->left = stree_insert(s->left, c);
  else
    s->right = stree_insert(s->right, c);
  s->greatest_id = c->id;  /* clauses always inserted with increasing IDs */
  s->n++;
  return s;
}  /* stree_insert */

/*************
 *
 *   stree_remove()
 *
 *************/

static
Stree stree_remove(Stree s, Topform c)
{
  if (s == NULL)
    fatal_error("stree_remove, clause not found");
  else if (c->weight == s->weight) {
    clist_remove(c, s->clauses);
    if (clist_empty(s->clauses) && s->left == NULL && s->right == NULL) {
      clist_free(s->clauses);
      free_stree(s);
      return NULL;
    }
    else if (clist_empty(s->clauses))
      s->greatest_id = 0;
    else
      s->greatest_id = s->clauses->last->c->id;
  }
  else if (c->weight < s->weight)
    s->left = stree_remove(s->left, c);
  else
    s->right = stree_remove(s->right, c);

  {
    int a = s->left ? s->left->greatest_id : 0;
    int b = clist_empty(s->clauses) ? 0 : s->clauses->last->c->id;
    int c = s->right ? s->right->greatest_id : 0;
    int d = IMAX(b,c);
    s->greatest_id = IMAX(a,d);
  }
  s->n--;
  return s;
}  /* stree_remove */

/*************
 *
 *   stree_of_weight()
 *
 *************/

static
Stree stree_of_weight(Stree s, int weight)
{
  if (s == NULL)
    return NULL;
  else if (weight == s->weight)
    return s;
  else if (weight < s->weight)
    return stree_of_weight(s->left, weight);
  else
    return stree_of_weight(s->right, weight);
}  /* stree_of_weight */

/*************
 *
 *   stree_first_lightest()
 *
 *************/

static
Topform stree_first_lightest(Stree s)
{     
  if (s == NULL)
    return NULL;
  else {
    Topform c;
    c = stree_first_lightest(s->left);
    if (c == NULL)
      c = clist_empty(s->clauses) ? NULL : s->clauses->first->c;
    if (c == NULL)
      c = stree_first_lightest(s->right);
    return c;
  }
}  /* stree_first_lightest */

/*************
 *
 *   stree_last_heaviest()
 *
 *************/

static
Topform stree_last_heaviest(Stree s)
{     
  if (s == NULL)
    return NULL;
  else {
    Topform c;
    c = stree_last_heaviest(s->right);
    if (c == NULL)
      c = clist_empty(s->clauses) ? NULL : s->clauses->last->c;
    if (c == NULL)
      c = stree_last_heaviest(s->left);
    return c;
  }
}  /* stree_last_heaviest */

/*************
 *
 *   stree_last_heaviest_greater_than()
 *
 *   Find the last heaviest clause that whose ID is greater than the given id.
 *
 *************/

static
Topform stree_last_heaviest_greater_than(Stree s, int id)
{     
  if (s == NULL)
    return NULL;
  else if (s->greatest_id <= id)
    return NULL;
  else {
    Topform c;
    c = stree_last_heaviest_greater_than(s->right, id);
    if (c == NULL) {
      if (!clist_empty(s->clauses) && s->clauses->last->c->id > id)
	c = s->clauses->last->c;
    }
    if (c == NULL)
      c = stree_last_heaviest_greater_than(s->left, id);
    return c;
  }
}  /* stree_last_heaviest_greater_than */

/*************
 *
 *   greatest_check()
 *
 *************/

/* #define GREATEST_CHECK */
#ifdef GREATEST_CHECK
static
void greatest_check(Stree s)
{
  if (s == NULL)
    return;
  else {
    int a = s->left ? s->left->greatest_id : 0;
    int b = clist_empty(s->clauses) ? 0 : s->clauses->last->c->id;
    int c = s->right ? s->right->greatest_id : 0;
    int d = IMAX(b,c);
    int e = IMAX(a,d);
    if (e != s->greatest_id)
      fatal_error("greatest_check, stree corrupt");
  }
}  /* greatest_check */
#endif

/*************
 *
 *   zap_stree()
 *
 *************/

static
void zap_stree(Stree s)
{
  if (s != NULL) {
    zap_stree(s->left);
    zap_stree(s->right);
    clist_zap(s->clauses);  /* clauses not zapped, because occur elsewhere */
    free_stree(s);
  }
}  /* zap_stree */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/* Sos is ordered by clause ID, and it is divided into
   halves by a clause (the center mark).  Clauses left of
   (older than) the center are protected from being "worst"
   clauses, because they might be useful for the breadth-first
   part of the search.  The center is adjusted lazily.
*/

static Clist_pos Center;
static int Cl_before_center;

/*************
 *
 *   init_sos_center()
 *
 *************/

static
void init_sos_center(Clist sos, int method)
{
  if (method == BY_WEIGHT || method == BY_AGE)
    Center = NULL;
  else {
    int n = sos->length / 2;
    int i;
    Clist_pos p = sos->first;
    for (i = 0; i < n; i++)
      p = p->next;
    Center = p;
    Cl_before_center = n-1;
  }
}  /* init_sos_center */

/*************
 *
 *   adjust_sos_center()
 *
 *************/

static
void adjust_sos_center(int size)
{
  int new_clauses_before_center = (size / 2) -1;
  int n = 0;
  int i;
  
  if (Cl_before_center < new_clauses_before_center) {
    /* move mark right */
    n = new_clauses_before_center - Cl_before_center;
    for (i = 0; i < n; i++) {
      Center = Center->next;
      Cl_before_center++;
    }
  }
  else if (Cl_before_center > new_clauses_before_center) {
    /* move mark left */
    n = Cl_before_center - new_clauses_before_center;
    for (i = 0; i < n; i++) {
      Center = Center->prev;
      Cl_before_center--;
    }
  }
}  /* adjust_sos_center */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

Stree Sos1;
Stree Sos2;

/*************
 *
 *   p_sos_tree()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_sos_tree(void)
{
  p_stree(Sos1);
}  /* p_sos_tree */

/*************
 *
 *   p_sos_dist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void p_sos_dist(void)
{
  p_dist(Sos1);
}  /* p_sos_dist */

/*************
 *
 *   index_sos()
 *
 *************/

/* DOCUMENTATION
This routine updates
the (private) index for extracting sos clauses.
*/

/* PUBLIC */
void index_sos(Topform c, int set)
{
  if (set == SOS1)
    Sos1 = stree_insert(Sos1, c);
  else
    Sos2 = stree_insert(Sos2, c);
}  /* index_sos */

/*************
 *
 *   insert_into_sos()
 *
 *************/

/* DOCUMENTATION
This routine appends a clause to the sos list and updates
the (private) index for extracting sos clauses.
*/

/* PUBLIC */
void insert_into_sos(Topform c, Clist sos, int set)
{
  clist_append(c, sos);
  index_sos(c, set);
}  /* insert_into_sos */

/*************
 *
 *   remove_from_sos()
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from the sos list and updates
the index for extracting the lightest and heaviest clauses.
*/

/* PUBLIC */
void remove_from_sos(Topform c, Clist sos, int set)
{
  if (set == SOS1)
    Sos1 = stree_remove(Sos1, c);
  else
    Sos2 = stree_remove(Sos2, c);

  if (Center) {
    if (c == Center->c)
      Center = Center->next;
    else if (c->id < Center->c->id)
      Cl_before_center--;
  }

  clist_remove(c, sos);
}  /* remove_from_sos */

/*************
 *
 *   first_sos_clause()
 *
 *************/

/* DOCUMENTATION
Given a nonempty Clist, return the first clause.
This does not remove the clause from any lists.
(Call remove_from_sos(Topform) to do that.)
*/

/* PUBLIC */
Topform first_sos_clause(Clist lst)
{
  return lst->first ? lst->first->c : NULL;
}  /* first_sos_clause */

/*************
 *
 *   lightest_sos_clause()
 *
 *************/

/* DOCUMENTATION
Return the first (oldest) of the lightest sos clauses.
This does not remove the clause from any lists.
(Call remove_from_sos(Topform, Clist) to do that.)
*/

/* PUBLIC */
Topform lightest_sos_clause(int set)
{
  Topform c;
  if (set == SOS1) {
    c = stree_first_lightest(Sos1);
    if (c == NULL)
      c = stree_first_lightest(Sos2);
  }
  else {
    c = stree_first_lightest(Sos2);
    if (c == NULL)
      c = stree_first_lightest(Sos1);
  }
  return c;
}  /* lightest_sos_clause */

/*************
 *
 *   worst_sos_clause()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Topform worst_sos_clause(Clist sos, int method)
{
  /* get larger of the two sets */
  int n1 = Sos1 ? Sos1->n : 0;
  int n2 = Sos2 ? Sos2->n : 0;
  Stree s = n1 > n2 ? Sos1 : Sos2;

  if (method == BY_WEIGHT)
    return stree_last_heaviest(s);
  else if (method == BY_AGE)
    return (sos->last ? sos->last->c : NULL);
  else {
    if (Center == NULL)
      init_sos_center(sos, method);
    else
      adjust_sos_center(sos->length);

    /* from larger of 2 sets, get the heaviest clause with ID > Center */
    {
      Topform c;

      c = stree_last_heaviest_greater_than(s, Center ? Center->c->id : 0);
					     
      if (c == NULL)
	c = stree_last_heaviest(s);

      if (c == NULL) {
	printf("worst_sos_clause NULL, n1=%d, n2=%d\n",n1,n2);
      }

      return c;
    }
  }
}  /* worst_sos_clause */

/*************
 *
 *   wt_of_nth_clause()
 *
 *   Consider sos, ordered by weight.
 *   Return the weight of the n-th clause.
 *
 *************/

static
int wt_of_nth_clause(Stree s, int n)
{
  int n1, n2;

  if (n < 1)
    n = 1;
  else if (n > s->n)
    n = s->n;

  n1 = s->left ? s->left->n : 0;
  n2 = s->clauses ? s->clauses->length : 0;
  if (n <= n1)
    return wt_of_nth_clause(s->left, n);
  else if (n - n1 <= n2)
    return s->weight;
  else
    return wt_of_nth_clause(s->right, n - (n1 + n2));
}  /* wt_of_nth_clause */

/*************
 *
 *   wt_of_clause_at()
 *
 *************/

/* DOCUMENTATION
Consider sos, ordered by weight.
Assume 0 <= part <= 1; if not, we make it so.
Return the weight of the clause at the part.
If sos is empty, return INT_MAX.
*/

/* PUBLIC */
int wt_of_clause_at(int set, double part)
{
  Stree st = (set == SOS1 ? Sos1 : Sos2);
  if (st == NULL || st->n == 0)
    return INT_MAX;
  else {
    int n;
    if (part < 0)
      part = 0;
    else if (part > 1)
      part = 1;
    n = st->n * part;
    return wt_of_nth_clause(st, n);
  }
}  /* wt_of_clause_at */

/*************
 *
 *   clauses_of_weight()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int clauses_of_weight(int wt, int set)
{
  Stree st = (set == SOS1 ? Sos1 : Sos2);
  st = stree_of_weight(st, wt);
  if (st == NULL)
    return 0;
  else
    return st->clauses->length;
}  /* clauses_of_weight */

/*************
 *
 *   zap_sos_index()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void zap_sos_index(void)
{
  zap_stree(Sos1);
  zap_stree(Sos2);
  Sos1 = NULL;
  Sos2 = NULL;
  Center = NULL;
  Cl_before_center = 0;
}  /* zap_sos_index */

