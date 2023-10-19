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

#ifndef TP_MSTATE_H
#define TP_MSTATE_H

/* Public definitions */

typedef struct jnode * Jnode;
typedef struct mstate * Mstate;

struct mstate {
  Estack stack;
  Jnode first_job;
  Jnode last_job;
  BOOL ok;
};

struct jnode {
  int type;
  int id;
  Term alpha;
  Term beta;
  int pos;
  Jnode prev;
  Jnode next;
};

/* End of public definitions */

/* Public function prototypes from mstate.c */

Mstate get_mstate(void);

void free_mstate(Mstate p);

Jnode get_jnode(void);

void free_jnode(Jnode p);

void fprint_mstate_mem(FILE *fp, BOOL heading);

void p_mstate_mem(void);

void job_append(Mstate s, int type, int id, Term atom, Term beta, int pos);

void job_prepend(Mstate s, int type, int id, Term atom, Term beta, int pos);

void job_pop(Mstate s);

void zap_jobs(Mstate s);

#endif  /* conditional compilation of whole file */
