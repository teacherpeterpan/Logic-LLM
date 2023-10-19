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

#include "hash.h"

/* Private definitions and types */

struct hashtab {
  int      size;
  Hashnode *table;
};

struct hashnode {
  void      *v;
  Hashnode next;
};

/*
 * memory management
 */

#define PTRS_HASHTAB PTRS(sizeof(struct hashtab))
static unsigned Hashtab_gets, Hashtab_frees;

#define PTRS_HASHNODE PTRS(sizeof(struct hashnode))
static unsigned Hashnode_gets, Hashnode_frees;

/*************
 *
 *   Hashtab get_hashtab()
 *
 *************/

static
Hashtab get_hashtab(void)
{
  Hashtab p = get_cmem(PTRS_HASHTAB);
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
  Hashnode p = get_cmem(PTRS_HASHNODE);
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

  n = sizeof(struct hashtab);
  fprintf(fp, "hashtab (%4d)      %11u%11u%11u%9.1f K\n",
          n, Hashtab_gets, Hashtab_frees,
          Hashtab_gets - Hashtab_frees,
          ((Hashtab_gets - Hashtab_frees) * n) / 1024.);

  n = sizeof(struct hashnode);
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

  p->size = size;
  p->table = calloc(size, sizeof(Hashnode));
  return p;
}  /* hash_init */

/*************
 *
 *   hash_insert()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void hash_insert(void *v, unsigned hashval, Hashtab h)
{
  int i = hashval % h->size;
  Hashnode new = get_hashnode();
  new->v = v;
  new->next = h->table[i];
  h->table[i] = new;
}  /* hash_insert */

/*************
 *
 *   hash_lookup()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void *hash_lookup(void *v, unsigned hashval, Hashtab h,
		  BOOL (*id_func) (void *, void *))
{
  Hashnode p = h->table[hashval % h->size];
  
  while (p && !(*id_func)(v, p->v))
    p = p->next;

  return (p ? p->v : NULL);
}  /* hash_lookup */

/*************
 *
 *   hash_delete()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void hash_delete(void *v, unsigned hashval, Hashtab h,
		 BOOL (*id_func) (void *, void *))
{
  int i = hashval % h->size;
  Hashnode p = h->table[i];
  Hashnode prev = NULL;
  
  while (p && !(*id_func)(v, p->v)) {
    prev = p;
    p = p->next;
  }

  if (!p)
    fatal_error("hash_delete, object not found");
  
  if (prev)
    prev->next = p->next;
  else
    h->table[i] = p->next;
  free_hashnode(p);
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

/*************
 *
 *   hash_info()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void hash_info(Hashtab h)
{
  int i;
  printf("\nHash info, size=%d\n", h->size);
  for (i = 0; i < h->size; i++) {
    Hashnode p;
    int n = 0;
    for (p = h->table[i]; p; p = p->next)
      n++;
    if (n > 0)
      printf("    bucket %d has %d objects\n", i, n);
  }
  fflush(stdout);
}  /* hash_info */

