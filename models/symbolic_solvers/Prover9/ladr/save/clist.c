#include "clist.h"

/* Private definitions and types */

/*
 * memory management
 */

static unsigned Clist_pos_gets, Clist_pos_frees;
static unsigned Clist_gets, Clist_frees;

#define BYTES_CLIST_POS sizeof(struct clist_pos)
#define PTRS_CLIST_POS BYTES_CLIST_POS%BPP == 0 ? BYTES_CLIST_POS/BPP : BYTES_CLIST_POS/BPP + 1

#define BYTES_CLIST sizeof(struct clist)
#define PTRS_CLIST BYTES_CLIST%BPP == 0 ? BYTES_CLIST/BPP : BYTES_CLIST/BPP + 1

/*************
 *
 *   Clist_pos get_clist_pos()
 *
 *************/

static
Clist_pos get_clist_pos(void)
{
  Clist_pos p = get_mem(PTRS_CLIST_POS);
  Clist_pos_gets++;
  return(p);
}  /* get_clist_pos */

/*************
 *
 *    free_clist_pos()
 *
 *************/

static
void free_clist_pos(Clist_pos p)
{
  free_mem(p, PTRS_CLIST_POS);
  Clist_pos_frees++;
}  /* free_clist_pos */

/*************
 *
 *   Clist get_clist()
 *
 *************/

static
Clist get_clist(void)
{
  Clist p = get_mem(PTRS_CLIST);
  Clist_gets++;
  return(p);
}  /* get_clist */

/*************
 *
 *    free_clist()
 *
 *************/

static
void free_clist(Clist p)
{
  free_mem(p, PTRS_CLIST);
  Clist_frees++;
}  /* free_clist */

/*************
 *
 *   fprint_clist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the clist package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_clist_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_CLIST_POS;
  fprintf(fp, "clist_pos (%4d)    %11u%11u%11u%9.1f K\n",
          n, Clist_pos_gets, Clist_pos_frees,
          Clist_pos_gets - Clist_pos_frees,
          ((Clist_pos_gets - Clist_pos_frees) * n) / 1024.);

  n = BYTES_CLIST;
  fprintf(fp, "clist (%4d)        %11u%11u%11u%9.1f K\n",
          n, Clist_gets, Clist_frees,
          Clist_gets - Clist_frees,
          ((Clist_gets - Clist_frees) * n) / 1024.);

}  /* fprint_clist_mem */

/*************
 *
 *   p_clist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the clist package.
*/

/* PUBLIC */
void p_clist_mem()
{
  fprint_clist_mem(stdout, TRUE);
}  /* p_clist_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   clist_init()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns an empty Clist, which
is a doubly-linked list of pointers to clauses.
You give it a string (any length, which is copied), representing
the name of the list.  If don't wish to name the list, send NULL.
(You can name the list later with name_clist().)
*/

/* PUBLIC */
Clist clist_init(char *name)
{
  Clist p = get_clist();

  if (name == NULL)
    p->name = NULL;
  else
    p->name = new_str_copy(name);
  return p;
}  /* clist_init */

/*************
 *
 *   name_clist()
 *
 *************/

/* DOCUMENTATION
This routine names or renames a Clist.
The string you supply can be any length and is copied.
*/

/* PUBLIC */
void name_clist(Clist p, char *name)
{
  if (p->name != NULL)
    free(p->name);

  if (name == NULL)
    p->name = NULL;
  else
    p->name = new_str_copy(name);
}  /* name_clist */

/*************
 *
 *   clist_free()
 *
 *************/

/* DOCUMENTATION
This routine frees an empty Clist.  If the Clist is not empty,
nothing happens.
*/

/* PUBLIC */
void clist_free(Clist p)
{
  if (p->first == NULL) {
    if (p->name != NULL) {
      free(p->name);
      p->name = NULL;
    }
    free_clist(p);
  }
}  /* clist_free */

/*************
 *
 *     clist_append()
 *
 *************/

/* DOCUMENTATION
This routine appends a Clause to a Clist.
*/

/* PUBLIC */
void clist_append(Clause c, Clist l)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = l;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->next = NULL;
  p->prev = l->last;
  l->last = p;
  if (p->prev)
    p->prev->next = p;
  else
    l->first = p;
  l->length++;

}  /* clist_append */

/*************
 *
 *     clist_prepend()
 *
 *************/

/* DOCUMENTATION
This routine inserts a Clause as the first member of a Clist.
*/

/* PUBLIC */
void clist_prepend(Clause c, Clist l)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = l;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->prev = NULL;
  p->next = l->first;
  l->first = p;
  if (p->next)
    p->next->prev = p;
  else
    l->last = p;
  l->length++;
    
}  /* clist_prepend */

/*************
 *
 *     clist_insert_before()
 *
 *************/

/* DOCUMENTATION
This routine inserts a Clause before a given position in a Clist.
*/

/* PUBLIC */
void clist_insert_before(Clause c, Clist_pos pos)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = pos->list;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->next = pos;
  p->prev = pos->prev;
  pos->prev = p;
  if (p->prev)
    p->prev->next = p;
  else
    pos->list->first = p;
  pos->list->length++;
    
}  /* clist_insert_before */

/*************
 *
 *     clist_insert_after()
 *
 *************/

/* DOCUMENTATION
This routine inserts a Clause after a given position in a Clist.
*/

/* PUBLIC */
void clist_insert_after(Clause c, Clist_pos pos)
{
  Clist_pos p;

  p = get_clist_pos();
  p->list = pos->list;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->prev = pos;
  p->next = pos->next;
  pos->next = p;
  if (p->next)
    p->next->prev = p;
  else
    pos->list->last = p;
  pos->list->length++;
    
}  /* clist_insert_after */

/*************
 *
 *     clist_remove()
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from a Clist.  If the Clause occurs
more than once in the Clist, the most recently inserted occurrence
is removed.  A fatal error occurs if the Clause is not in the Clist.
*/

/* PUBLIC */
void clist_remove(Clause c, Clist l)
{
  Clist_pos p, prev;

  /* Find position from containment list of clause. */
  for (p = c->containers, prev = NULL;
       p && p->list != l;
       prev = p, p = p->nocc);
    
  if (!p)
    fatal_error("clist_remove: clause not in list");

  /* First update normal links. */
  if (p->prev)
    p->prev->next = p->next;
  else
    p->list->first = p->next;
  if (p->next)
    p->next->prev = p->prev;
  else
    p->list->last = p->prev;

    /* Now update containment links. */
  if (prev)
    prev->nocc = p->nocc;
  else
    c->containers = p->nocc;

  free_clist_pos(p);
  l->length--;
}  /* clist_remove */

/*************
 *
 *   clist_remove_all_clauses()
 *
 *************/

/* DOCUMENTATION
This routine removes all clauses from a clist.
The clauses are NOT deleted, even if they occur nowhere else.
*/

/* PUBLIC */
void clist_remove_all_clauses(Clist l)
{
  while (l->first) {
    clist_remove(l->first->c, l);
  }
}  /* clist_remove_all_clauses */

/*************
 *
 *     clist_remove_all()
 *
 *************/

/* DOCUMENTATION
This routine removes a clause from all lists in which it occurs.
The number of lists from which it was removed is returned.
*/

/* PUBLIC */
int clist_remove_all(Clause c)
{
  int i = 0;
  while (c->containers) {
    clist_remove(c, c->containers->list);
    i++;
  }
  return i;
}  /* clist_remove_all */

/*************
 *
 *     clist_member()
 *
 *************/

/* DOCUMENTATION
This Boolean routine checks if a Clause is a member of a Clist.
*/

/* PUBLIC */
int clist_member(Clause c, Clist l)
{
  Clist_pos p;

  for (p = c->containers; p; p = p->nocc) {
    if (p->list == l)
      return TRUE;
  }
  return FALSE;
}  /* clist_member */

/*************
 *
 *    fprint_clist()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) each clause in a Clist.
If the Clist has a non-empty name, say "usable", the
string "list(usable).\n" is printed first.
The string "end_of_list.\n" is always printed at the end.
*/

/* PUBLIC */
void fprint_clist(FILE *fp, Clist l)
{
  Clist_pos p;

  if (l->name != NULL)
    fprintf(fp, "list(%s).\n", l->name);

  for(p = l->first; p; p = p->next)
    fprint_clause(fp, p->c);
  fprintf(fp, "end_of_list.\n");
  fflush(fp);
}  /* fprint_clist */

/*************
 *
 *    p_clist()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) each clause in a Clist.
See fprint_clist().
*/

/* PUBLIC */
void p_clist(Clist l)
{
  fprint_clist(stdout, l);
}  /* p_clist */

/*************
 *
 *   clist_copy()
 *
 *************/

/* DOCUMENTATION
Copy a clist of clauses.  Justificatons and attributes are
copied, and the clauses get new IDs.
*/

/* PUBLIC */
Clist clist_copy(Clist a, BOOL assign_ids)
{
  Clist b = clist_init(a->name);
  Clist_pos p;
  for(p = a->first; p; p = p->next) {
    Clause c = copy_clause(p->c);
    c->justification = copy_justification(p->c->justification);
    c->attributes = copy_attributes(p->c->attributes);
    if (assign_ids)
      assign_clause_id(c);
    clist_append(c, b);
  }
  return b;
}  /* clist_copy */

/*************
 *
 *   clist_assign_ids()
 *
 *************/

/* DOCUMENTATION
Assign clause IDs to the members of a Clist.
*/

/* PUBLIC */
void clist_assign_ids(Clist a)
{
  Clist_pos p;
  for(p = a->first; p; p = p->next)
    assign_clause_id(p->c);
}  /* clist_assign_ids */

/*************
 *
 *    clist_zap()
 *
 *************/

/* DOCUMENTATION
For each Clause (occurrence) in a Clist, remove it, and if it occurs
in no other Clist, call zap_clause() to delete the Clause.
Then, free the Clist.
*/

/* PUBLIC */
void clist_zap(Clist l)
{
  Clist_pos p;
  Clause c;

  p = l->first;
  while (p) {
    c = p->c;
    p = p->next;
    clist_remove(c, l);
    if (c->containers == NULL)
      zap_clause(c);
  }
  clist_free(l);
}  /* clist_zap */

/*************
 *
 *     clist_check()
 *
 *************/

/* DOCUMENTATION
This routine checks the integrity of a Clist.  If any errors are
found, a message is sent to stdout.  This is used for debugging.
*/

/* PUBLIC */
void clist_check(Clist l)
{
  Clist_pos p;
  int n = 0;

  for (p = l->first; p; p = p->next) {
    n++;
    if (p->list != l)
      printf("clist_check error0\n");
    if (p->next) {
      if (p->next->prev != p)
	printf("clist_check error1\n");
    }
    else if (p != l->last)
      printf("clist_check error2\n");
    if (p->prev) {
      if (p->prev->next != p)
	printf("clist_check error3\n");
    }
    else if (p != l->first)
      printf("clist_check error4\n");
  }
  if (l->length != n)
    printf("clist_check error5\n");
}  /* clist_check */

/*************
 *
 *   clist_append_all()
 *
 *************/

/* DOCUMENTATION
Append each member of l2 to l1, then zap l2.  Do not refer to l2
after the call.
*/

/* PUBLIC */
void clist_append_all(Clist l1, Clist l2)
{
  Clist_pos p;
  for (p = l2->first; p != NULL; p = p->next)
    clist_append(p->c, l1);
  clist_zap(l2); /* This doesn't zap clauses, because they now occur in l1. */
}  /* clist_append_all */

/*************
 *
 *   clist_empty()
 *
 *************/

/* DOCUMENTATION
This function checks if a (non-NULL) Clist is empty.
*/

/* PUBLIC */
BOOL clist_empty(Clist lst)
{
  return lst->first == NULL;
}  /* clist_empty */

/*************
 *
 *   clist_length()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int clist_length(Clist l)
{
  return l->length;
}  /* clist_length */

/*************
 *
 *   max_wt_in_clist()
 *
 *************/

/* DOCUMENTATION
Scan a Clist, and return the maximum clause weight seen.
*/

/* PUBLIC */
int max_wt_in_clist(Clist l)
{
  int max = INT_MIN;
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (p->c->weight > max)
      max = p->c->weight;
  return max;
}  /* max_wt_in_clist */

/*************
 *
 *   horn_clist()
 *
 *************/

/* DOCUMENTATION
Is every clause in the list a Horn clause?
*/

/* PUBLIC */
BOOL horn_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (!horn_clause(p->c))
      return FALSE;
  return TRUE;
}  /* horn_clist */

/*************
 *
 *   unit_clist()
 *
 *************/

/* DOCUMENTATION
Is every clause in the list a unit clause?
*/

/* PUBLIC */
BOOL unit_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (!unit_clause(p->c))
      return FALSE;
  return TRUE;
}  /* unit_clist */

/*************
 *
 *   equality_in_clist()
 *
 *************/

/* DOCUMENTATION
Does the list contain a clause with a positive equality literal?
*/

/* PUBLIC */
BOOL equality_in_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (contains_pos_eq(p->c))
      return TRUE;
  return FALSE;
}  /* equality_in_clist */

/*************
 *
 *   neg_nonunit_in_clist()
 *
 *************/

/* DOCUMENTATION
Does the list contain a negative nonunit clause ?
*/

/* PUBLIC */
BOOL neg_nonunit_in_clist(Clist l)
{
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (negative_clause(p->c) && number_of_literals(p->c) > 1)
      return TRUE;
  return FALSE;
}  /* neg_nonunit_in_clist */

/*************
 *
 *   clauses_in_clist()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Plist clauses_in_clist(Plist p, Clist l)
{
  Plist q;
  Plist result = NULL;
  for (q = p; q; q = q->next) {
    Clause c = q->v;
    if (clist_member(c, l))
      result = plist_append(result, c);
  }
  return result;
}  /* clauses_in_clist */

/*************
 *
 *   clist_swap()
 *
 *************/

/* DOCUMENTATION
Every clause occurs in a set (maybe empty) of Clists.  Given clauses A and B,
this routine puts A into the lists in which B occurs and vice versa.
*/

/* PUBLIC */
void clist_swap(Clause a, Clause b)
{
  Clist_pos p;
  /* Replace refs to a with refs to b. */
  for (p = a->containers; p; p = p->nocc)
    p->c = b;
  /* Replace refs to b with refs to a. */
  for (p = b->containers; p; p = p->nocc)
    p->c = a;
  /* Swap the containment lists. */
  p = a->containers;
  a->containers = b->containers;
  b->containers = p;
}  /* clist_swap */

/*************
 *
 *   clist_move_clauses()
 *
 *************/

/* DOCUMENTATION
Move all clauses from a to the end of b.
*/

/* PUBLIC */
void clist_move_clauses(Clist a, Clist b)
{
  while (a->first) {
    Clause c = a->first->c;
    clist_remove(c, a);
    clist_append(c, b);
  }
}  /* clist_move_clauses */

