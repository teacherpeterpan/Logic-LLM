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

#include "discrim.h"

/* Private definitions and types */

/*
 * memory management
 */

#define PTRS_DISCRIM PTRS(sizeof(struct discrim))
static unsigned Discrim_gets, Discrim_frees;

#define PTRS_DISCRIM_POS PTRS(sizeof(struct discrim_pos))
static unsigned Discrim_pos_gets, Discrim_pos_frees;

/*************
 *
 *   Discrim get_discrim()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
Discrim get_discrim(void)
{
  Discrim p = get_cmem(PTRS_DISCRIM);
  Discrim_gets++;
  return(p);
}  /* get_discrim */

/*************
 *
 *    free_discrim()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_discrim(Discrim p)
{
  free_mem(p, PTRS_DISCRIM);
  Discrim_frees++;
}  /* free_discrim */

/*************
 *
 *   Discrim_pos get_discrim_pos()
 *
 *************/

/* DOCUMENTATION
The structure is not initialized.
*/

/* PUBLIC */
Discrim_pos get_discrim_pos(void)
{
  Discrim_pos p = get_mem(PTRS_DISCRIM_POS);  /* not initialized */
  Discrim_pos_gets++;
  return(p);
}  /* get_discrim_pos */

/*************
 *
 *    free_discrim_pos()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_discrim_pos(Discrim_pos p)
{
  free_mem(p, PTRS_DISCRIM_POS);
  Discrim_pos_frees++;
}  /* free_discrim_pos */

/*************
 *
 *   fprint_discrim_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the discrim package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_discrim_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct discrim);
  fprintf(fp, "discrim (%4d)      %11u%11u%11u%9.1f K\n",
          n, Discrim_gets, Discrim_frees,
          Discrim_gets - Discrim_frees,
          ((Discrim_gets - Discrim_frees) * n) / 1024.);

  n = sizeof(struct discrim_pos);
  fprintf(fp, "discrim_pos (%4d)  %11u%11u%11u%9.1f K\n",
          n, Discrim_pos_gets, Discrim_pos_frees,
          Discrim_pos_gets - Discrim_pos_frees,
          ((Discrim_pos_gets - Discrim_pos_frees) * n) / 1024.);

}  /* fprint_discrim_mem */

/*************
 *
 *   p_discrim_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the discrim package.
*/

/* PUBLIC */
void p_discrim_mem(void)
{
  fprint_discrim_mem(stdout, TRUE);
}  /* p_discrim_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   discrim_init()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns an empty discrimination index.
It can be used for either wild or tame indexing.
*/

/* PUBLIC */
Discrim discrim_init(void)
{
  return get_discrim();
}  /* discrim_init */

/*************
 *
 *   discrim_dealloc(d)
 *
 *************/

/* DOCUMENTATION
This routine frees an empty discrimination index (wild or tame).
*/

/* PUBLIC */
void discrim_dealloc(Discrim d)
{
  if (d->u.kids) {
    fatal_error("discrim_dealloc, nonempty index.");
  }
  else
    free_discrim(d);
}  /* discrim_dealloc */

/*************
 *
 *   zap_discrim_tree()
 *
 *************/

static
void zap_discrim_tree(Discrim d, int n)
{
  if (n == 0) {
    zap_plist(d->u.data);
  }
  else {
    int arity;
    Discrim k, prev;

    k = d->u.kids;
    while (k != NULL) {
      if (k->type == AC_ARG_TYPE || k->type == AC_NV_ARG_TYPE)
	arity = 0;
      else if (DVAR(k))
	arity = 0;
      else
	arity = sn_to_arity(k->symbol);
      prev = k;
      k = k->next;
      zap_discrim_tree(prev, n+arity-1);
    }
  }
  free_discrim(d);
}  /* zap_discrim_tree */

/*************
 *
 *   destroy_discrim_tree()
 *
 *************/

/* DOCUMENTATION
This routine frees all the memory associated with a discrimination
index.  It can be used with either wild or tame trees.
*/

/* PUBLIC */
void destroy_discrim_tree(Discrim d)
{
  zap_discrim_tree(d, 1);
}  /* destroy_discrim_tree */

/*************
 *
 *   discrim_empty()
 *
 *************/

/* DOCUMENTATION
This Boolean function checks if a discrimination index is empty.
It can be used with either wild or tame trees.
*/

/* PUBLIC */
BOOL discrim_empty(Discrim d)
{
  return (d == NULL ? TRUE : (d->u.kids == NULL ? TRUE : FALSE));
}  /* discrim_empty */

