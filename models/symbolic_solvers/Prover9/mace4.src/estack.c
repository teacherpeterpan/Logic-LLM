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

#include "estack.h"

/* Estack */

#define ESTACK_SIZE 400  /* size of each chunk */

struct estack {
  void **p[ESTACK_SIZE];
  void *v[ESTACK_SIZE];
  int n;
  Estack next;
};

static Estack Estack_avail;
static unsigned Estack_gets, Estack_frees, Estack_avails;

/*************
 *
 *    Estack get_estack()
 *
 *************/

static
Estack get_estack(void)
{
  Estack p;

  Estack_gets++;
  if (Estack_avail == NULL)
    p = malloc(sizeof(struct estack));
  else {
    Estack_avails--;
    p = Estack_avail;
    Estack_avail = Estack_avail->next;
  }

  /* no initialization */	

  return(p);
}  /* get_estack */

/*************
 *
 *    free_estack()
 *
 *************/

static
void free_estack(Estack p)
{
  Estack_frees++;
  Estack_avails++;
  p->next = Estack_avail;
  Estack_avail = p;
}  /* free_estack */

/*************
 *
 *   fprint_estack_mem()
 *
 *************/

void fprint_estack_mem(FILE *fp, int heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct estack);

  fprintf(fp, "estack (%4d)       %11u%11u%11u%9.1f K (avail=%u, %.1f K)\n",
          n, Estack_gets, Estack_frees,
          Estack_gets - Estack_frees,
          ((Estack_gets - Estack_frees) * n) / 1024.,
	  Estack_avails,
	  (Estack_avails * n) / 1024.);

  /* end of printing for each type */
  
}  /* fprint_estack_mem */

/*************
 *
 *   p_estack_mem()
 *
 *************/

void p_estack_mem()
{
  fprint_estack_mem(stdout, 1);
}  /* p_estack_mem */

/*************
 *
 *   free_estack_memory()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void free_estack_memory(void)
{
  Estack a = Estack_avail;
  while (a) {
    Estack b = a;
    a = a->next;
    free(b);
    Estack_avails--;
  }
  Estack_avail = NULL;
}  /* free_estack_memory */

/*************
 *
 *   estack_bytes()
 *
 *************/

/* DOCUMENTATION
Return the number of bytes allocated for Estacks.
*/

/* PUBLIC */
int estack_bytes(void)
{
  return ((Estack_gets - Estack_frees) + Estack_avails) * sizeof(struct estack); 
}  /* estack_bytes */

/*************
 *
 *   update_and_push()
 *
 *************/

Estack update_and_push(void **p, void *new, Estack stack)
{
  if (stack == NULL || stack->n == ESTACK_SIZE) {
    Estack s = get_estack();  /* allocate a stack entry */
    s->n = 0;
    s->next = stack;
    stack = s;
  }

  /* printf("ZZ update: %p, %p\n", p, *p); */

  stack->p[stack->n] = p;              /* record the location */
  stack->v[stack->n] = *p;             /* record the old value */
  *p = new;                            /* make the assignment */
  stack->n++;
  return stack;                  /* return the updated stack */
}  /* update_and_push */

/*************
 *
 *   restore_from_stack()
 *
 *************/

void restore_from_stack(Estack stack)
{
  while (stack != NULL) {
    int i;
    Estack s = stack;
    stack = stack->next;
    for (i = s->n-1; i >= 0; i--) {
      /* printf("ZZ restore: %p, %p\n", s->p[i], *(s->p[i])); */
      *(s->p[i]) = s->v[i];  /* restore */
    }
    free_estack(s);
  }
}  /* restore_from_stack */

/*************
 *
 *   zap_estack()
 *
 *************/

void zap_estack(Estack stack)
{
  while (stack != NULL) {
    Estack s = stack;
    stack = stack->next;
    free_estack(s);
  }
}  /* zap_estack */

