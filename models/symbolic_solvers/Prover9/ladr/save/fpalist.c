#include "fpalist.h"

/* Private definitions and types */

/*
  This if the new code (April 2004) for inserting/deleting/traversing
   FPA lists.  It should function exactly the same as the old code,
   except that deletions should be much faster, and memory usage will
   be somewhat different (not much).

   Instead of a (singly-linked) list of pointers to terms, we have a
   (singly-linked) list of *arrays* of pointers to terms.  As before,
   the terms are kept in decreasing order.  Recall that in practice,
   terms being inserted will usually be greater than anything already
   in the list.
*/

/*
  The design is determined by the following properties of the
  application:  (1) items will nearly always be inserted in
  increasing order, (2) the lists will be traversed, and the
  items must be kept in decreasing order, and (3) deletions
  will be arbitrary and occasionally extensive.
*/

/*
 * memory management
 */

static unsigned Fpa_chunk_gets, Fpa_chunk_frees;

#define BYTES_FPA_CHUNK sizeof(struct fpa_chunk)
#define PTRS_FPA_CHUNK BYTES_FPA_CHUNK%BPP == 0 ? BYTES_FPA_CHUNK/BPP : BYTES_FPA_CHUNK/BPP + 1

/*************
 *
 *   Fpa_chunk get_fpa_chunk()
 *
 *************/

static
Fpa_chunk get_fpa_chunk(void)
{
  Fpa_chunk p = get_mem(PTRS_FPA_CHUNK);
  Fpa_chunk_gets++;
  return(p);
}  /* get_fpa_chunk */

/*************
 *
 *    free_fpa_chunk()
 *
 *************/

static
void free_fpa_chunk(Fpa_chunk p)
{
  free_mem(p, PTRS_FPA_CHUNK);
  Fpa_chunk_frees++;
}  /* free_fpa_chunk */

/*************
 *
 *   fprint_fpalist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the fpalist package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_fpalist_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_FPA_CHUNK;
  fprintf(fp, "fpa_chunk (%4d)    %11u%11u%11u%9.1f K\n",
          n, Fpa_chunk_gets, Fpa_chunk_frees,
          Fpa_chunk_gets - Fpa_chunk_frees,
          ((Fpa_chunk_gets - Fpa_chunk_frees) * n) / 1024.);

}  /* fprint_fpalist_mem */

/*************
 *
 *   p_fpalist_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the fpalist package.
*/

/* PUBLIC */
void p_fpalist_mem()
{
  fprint_fpalist_mem(stdout, TRUE);
}  /* p_fpalist_mem */

/*
 *  end of memory management
 */

/* First and last items in chunk.  Items are right-justified. */

#define FLAST(f) (f)->d[FMAX-1]
#define FFIRST(f) (f)->d[FMAX-((f)->n)]

/*************
 *
 *   flist_insert()
 *
 *   If the item is greater than any in the list, insertion should
 *   be constant time.
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Fpa_chunk flist_insert(Fpa_chunk f, Term x)
{
  if (f == NULL) {
    f = get_fpa_chunk();
    FLAST(f) = x;
    f->n = 1;
  }
  else if (f->n == FMAX) {
    if (FLT(x,FLAST(f)))
      f->next = flist_insert(f->next, x);
    else if (x == FLAST(f))
      fprintf(stderr, "WARNING: flist_insert, item %p already here (1)!\n", x);
    else if (FGT(x,FFIRST(f))) {
      /* 
	 This special case isn't necessary.  It is to improve performance.
	 The application for which I'm writing this inserts items in
	 increasing order (most of the time), and this prevents a lot of
	 half-empty chunks in that case.
      */
      Fpa_chunk f2 = flist_insert(NULL, x);
      f2->next = f;
      f = f2;
    }
    else {
      /* split this chunk in half */
      Fpa_chunk f2 = get_fpa_chunk();
      int move = FMAX / 2;
      int i, j;
      for (i = 0, j = FMAX-move; i < move; i++, j++) {
	f2->d[j] = f->d[i];
	f->d[i] = NULL;
      }
      f2->n = move;
      f->n = FMAX - move;
      f2->next = f;
      f = flist_insert(f2, x);
    }
  }
  else {
    if (f->next && FLE(x,FFIRST(f->next)))
      f->next = flist_insert(f->next, x);
    else {
      /* insert into this pa_chunk */
      int n = f->n;
      int i = FMAX - n;
      while (i < FMAX && FLT(x,f->d[i]))
	i++;
      if (i < FMAX && x == f->d[i])
	fprintf(stderr, "WARNING: flist_insert, item %p already here (2)!\n", x);
      else if (i == FMAX - n) {
	f->d[i-1] = x;
	f->n = n+1;
      }
      else {
	/* insert at i-1, shifting the rest */
	int j;
	for (j = FMAX-n; j < i; j++)
	  f->d[j-1] = f->d[j];
	f->d[i-1] = x;
	f->n = n+1;
      }
    }
  }
  return f;
}  /* flist_insert */

/*************
 *
 *   consolidate() - try to join f and f->next; not recursive
 *
 *************/

static
Fpa_chunk consolidate(Fpa_chunk f)
{
  if (f->next && f->n + f->next->n <= FMAX) {
    Fpa_chunk f2 = f->next;
    int i;
    for (i = 0; i < f->n; i++)
      f2->d[FMAX - (f2->n + i + 1)] = f->d[FMAX - (i+1)];
    f2->n = f->n + f2->n;
    free_fpa_chunk(f);
    return f2;
  }
  else
    return f;
}  /* consolidate */

/*************
 *
 *   flist_delete()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Fpa_chunk flist_delete(Fpa_chunk f, Term x)
{
  if (f == NULL)
    fprintf(stderr, "WARNING: flist_delete, item %p not found (1)!\n", x);
  else if (FLT(x,FLAST(f)))
    f->next = flist_delete(f->next, x);
  else {
    int n = f->n;
    int i = FMAX - n;
    while (i < FMAX && FLT(x,f->d[i]))
      i++;
    if (x != f->d[i])
      fprintf(stderr, "WARNING: flist_delete, item %p not found (2)!\n", x);
    else {
      /* delete and close the hole */
      int j;
      for (j = i; j > FMAX-n; j--)
	f->d[j] = f->d[j-1];
      f->d[j] = NULL;
      f->n = n-1;
      if (f->n == 0) {
	/* delete this chunk */
	Fpa_chunk next = f->next;
	free_fpa_chunk(f);
	f = next;
      }
      else {
	/* try to join this chunk with the next */
	f = consolidate(f);
      }
    }
  }
  return f;
}  /* flist_delete */

/*************
 *
 *   first_fpos()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
struct fposition first_fpos(Fpa_chunk f)
{
  if (f == NULL)
    return (struct fposition) {NULL, 0};
  else
    return (struct fposition) {f, FMAX - f->n};
}  /* first_fpos */

/*************
 *
 *   next_fpos()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
struct fposition next_fpos(struct fposition p)
{
  int i = p.i+1;
  if (i < FMAX)
    return (struct fposition) {p.f, i};
  else
    return first_fpos((p.f)->next);
}  /* next_fpos */

/*************
 *
 *   zap_fpa_chunks()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void zap_fpa_chunks(Fpa_chunk p)
{
  if (p != NULL) {
    zap_fpa_chunks(p->next);
    free_fpa_chunk(p);
  }
}  /* zap_fpa_chunks */
