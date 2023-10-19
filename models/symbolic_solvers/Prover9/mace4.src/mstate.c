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

#include "msearch.h"

/*
 * memory management
 */

#define PTRS_MSTATE CEILING(sizeof(struct mstate),BYTES_POINTER)
static unsigned Mstate_gets, Mstate_frees;

#define PTRS_JNODE CEILING(sizeof(struct jnode),BYTES_POINTER)
static unsigned Jnode_gets, Jnode_frees;

/*************
 *
 *   Mstate get_mstate()
 *
 *************/

Mstate get_mstate(void)
{
  Mstate p = get_cmem(PTRS_MSTATE);
  p->ok = TRUE;
  Mstate_gets++;
  return(p);
}  /* get_mstate */

/*************
 *
 *    free_mstate()
 *
 *************/

void free_mstate(Mstate p)
{
  free_mem(p, PTRS_MSTATE);
  Mstate_frees++;
}  /* free_mstate */

/*************
 *
 *   Jnode get_jnode()
 *
 *************/

Jnode get_jnode(void)
{
  Jnode p = get_mem(PTRS_JNODE);
  Jnode_gets++;
  return(p);
}  /* get_jnode */

/*************
 *
 *    free_jnode()
 *
 *************/

void free_jnode(Jnode p)
{
  free_mem(p, PTRS_JNODE);
  Jnode_frees++;
}  /* free_jnode */

/*************
 *
 *   fprint_mstate_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the mstate package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_mstate_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct mstate);
  fprintf(fp, "mstate (%4d)       %11u%11u%11u%9.1f K\n",
          n, Mstate_gets, Mstate_frees,
          Mstate_gets - Mstate_frees,
          ((Mstate_gets - Mstate_frees) * n) / 1024.);

  n = sizeof(struct jnode);
  fprintf(fp, "jnode (%4d)        %11u%11u%11u%9.1f K\n",
          n, Jnode_gets, Jnode_frees,
          Jnode_gets - Jnode_frees,
          ((Jnode_gets - Jnode_frees) * n) / 1024.);

}  /* fprint_mstate_mem */

/*************
 *
 *   p_mstate_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the mstate package.
*/

/* PUBLIC */
void p_mstate_mem()
{
  fprint_mstate_mem(stdout, TRUE);
}  /* p_mstate_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   job_append()
 *
 *************/

void job_append(Mstate s, int type, int id, Term alpha, Term beta, int pos)
{
  Jnode j = get_jnode();
  j->type = type;
  j->id = id;
  j->alpha = alpha;
  j->beta = beta;
  j->pos = pos;

  if (s->first_job == NULL) {
    j->prev = NULL;
    j->next = NULL;
    s->first_job = j;
    s->last_job = j;
  }
  else {
    j->prev = s->last_job;
    j->next = NULL;
    s->last_job->next = j;
    s->last_job = j;
  }
}  /* job_append */

/*************
 *
 *   job_prepend()
 *
 *************/

void job_prepend(Mstate s, int type, int id, Term alpha, Term beta, int pos)
{
  Jnode j = get_jnode();
  j->type = type;
  j->id = id;
  j->alpha = alpha;
  j->beta = beta;
  j->pos = pos;

  if (s->first_job == NULL) {
    j->prev = NULL;
    j->next = NULL;
    s->first_job = j;
    s->last_job = j;
  }
  else {
    j->next = s->first_job;
    j->prev = NULL;
    s->first_job->prev = j;
    s->first_job = j;
  }
}  /* job_prepend */

/*************
 *
 *   job_pop()
 *
 *************/

void job_pop(Mstate s)
{
  if (s->first_job == NULL)
    fatal_error("job_pop: empty list");
  else {
    Jnode p = s->first_job;
    s->first_job = p->next;
    if (s->first_job == NULL)
      s->last_job = NULL;
    else
      s->first_job->prev = NULL;
    free_jnode(p);
  }
}  /* job_pop */

/*************
 *
 *   zap_jobs()
 *
 *************/

void zap_jobs(Mstate ms)
{
  Jnode j = ms->first_job;
  while (j) {
    Jnode p = j;
    j = j->next;
    free_jnode(p);
  }
}  /* zap_jobs */
