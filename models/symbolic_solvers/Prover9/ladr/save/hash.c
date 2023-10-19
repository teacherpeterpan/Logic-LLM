#include "hash.h"

/* Private definitions and types */

struct hashtab {
  int      size;
  Hashnode *table;
  Hashtab  next;  /* for avail list only */
};

struct hashnode {
  int      id;
  void     *v;
  Hashnode next;
};

static unsigned Hashtab_gets, Hashtab_frees;
static unsigned Hashnode_gets, Hashnode_frees;

#define BYTES_HASHTAB sizeof(struct hashtab)
#define PTRS_HASHTAB BYTES_HASHTAB%BPP == 0 ? BYTES_HASHTAB/BPP : BYTES_HASHTAB/BPP + 1

#define BYTES_HASHNODE sizeof(struct hashnode)
#define PTRS_HASHNODE BYTES_HASHNODE%BPP == 0 ? BYTES_HASHNODE/BPP : BYTES_HASHNODE/BPP + 1

/*************
 *
 *   Hashtab get_hashtab()
 *
 *************/

static
Hashtab get_hashtab(void)
{
  Hashtab p = get_mem(PTRS_HASHTAB);
  Hashtab_gets++;
  return(p);
}  /* get_hashtab */

/*************
 *
 *    free_hashtab()
 *
 *************/

static
void free_hashtab(Hashtab p)
{
  free_mem(p, PTRS_HASHTAB);
  Hashtab_frees++;
}  /* free_hashtab */

/*************
 *
 *   Hashnode get_hashnode()
 *
 *************/

static
Hashnode get_hashnode(void)
{
  Hashnode p = get_mem(PTRS_HASHNODE);
  Hashnode_gets++;
  return(p);
}  /* get_hashnode */

/*************
 *
 *    free_hashnode()
 *
 *************/

static
void free_hashnode(Hashnode p)
{
  free_mem(p, PTRS_HASHNODE);
  Hashnode_frees++;
}  /* free_hashnode */

/*************
 *
 *   fprint_hash_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the hash package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_hash_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_HASHTAB;
  fprintf(fp, "hashtab (%4d)      %11u%11u%11u%9.1f K\n",
          n, Hashtab_gets, Hashtab_frees,
          Hashtab_gets - Hashtab_frees,
          ((Hashtab_gets - Hashtab_frees) * n) / 1024.);

  n = BYTES_HASHNODE;
  fprintf(fp, "hashnode (%4d)     %11u%11u%11u%9.1f K\n",
          n, Hashnode_gets, Hashnode_frees,
          Hashnode_gets - Hashnode_frees,
          ((Hashnode_gets - Hashnode_frees) * n) / 1024.);

}  /* fprint_hash_mem */

/*************
 *
 *   p_hash_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the hash package.
*/

/* PUBLIC */
void p_hash_mem()
{
  fprint_hash_mem(stdout, TRUE);
}  /* p_hash_mem */

/*
 *  end of memory management
 */
/*************
 *
 *   hash_init()
 *
 *************/

/* DOCUMENTATION
Allocate and initialize a hash table of the given size.
*/

/* PUBLIC */
Hashtab hash_init(int size)
{
  Hashtab p = get_hashtab();
  int i;

  p->size = size;
  p->table = malloc(size * sizeof(Hashnode));
  for (i = 0; i < size; i++) {
    p->table[i] = NULL;
  }
  return p;
}  /* hash_init */

/*************
 *
 *   hash_insert()
 *
 *************/

/* DOCUMENTATION
Insert a pair (integer, void *) into a hash table.
If the integer is already there, a fatal error occurs.
*/

/* PUBLIC */
void hash_insert(int id, void *v, Hashtab h)
{
  Hashnode p, prev, new;
  int i = id % h->size;

  p = h->table[i];
  prev = NULL;
  while (p != NULL && p->id < id) {
    prev = p;
    p = p->next;
  }
  if (p != NULL && p->id == id)
    fatal_error("hash_insert, id already there.");
  else {
    new = get_hashnode();
    new->id = id;
    new->v = v;
    new->next = p;
    if (prev != NULL)
      prev->next = new;
    else
      h->table[i] = new;
  }
}  /* hash_insert */

/*************
 *
 *   hash_lookup()
 *
 *************/

/* DOCUMENTATION
Find and return the pointer associated with the given integer ID.
If the ID is not in the table, NULL is returned.
*/

/* PUBLIC */
void * hash_lookup(int id, Hashtab h)
{
  Hashnode p = h->table[id % h->size];
  while (p != NULL && p->id < id)
    p = p->next;
  if (p != NULL && p->id == id)
    return p->v;
  else
    return NULL;
}  /* hash_lookup */

/*************
 *
 *   hash_delete()
 *
 *************/

/* DOCUMENTATION
Delete the pair (ID, void *) for the given integer ID.
If the ID is not in the table, nothing happens.
*/

/* PUBLIC */
void hash_delete(int id, Hashtab h)
{
  Hashnode p, prev;
  int i = id % h->size;

  p = h->table[i];
  prev = NULL;
  while (p != NULL && p->id < id) {
    prev = p;
    p = p->next;
  }

  if (p != NULL && p->id == id) {
    if (prev == NULL)
      h->table[i] = p->next;
    else
      prev->next = p->next;
    free_hashnode(p);
  }
}  /* hash_delete */

/*************
 *
 *   hash_destroy()
 *
 *************/

/* DOCUMENTATION
Free all of the memory used by the given hash table.  Do not
refer to the table after calling this routine.  The hash table
need not be empty;
*/

/* PUBLIC */
void hash_destroy(Hashtab h)
{
  int i;
  for (i = 0; i < h->size; i++) {
    Hashnode p = h->table[i];
    while (p != NULL) {
      Hashnode p2 = p;
      p = p->next;
      free_hashnode(p2);
    }
  }
  free(h->table);
  free_hashtab(h);
}  /* hash_destroy */
