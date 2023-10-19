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

#ifndef MACE4_GROUND_H
#define MACE4_GROUND_H

/* INTRODUCTION
*/

/* Public definitions */

typedef struct mclause * Mclause;

struct mclause {
  Term *lits;
  Mclause next;
  BOOL subsumed;
  int numlits;
  /* The following union exists in case long int is smaller than pointer;
     This is because "active" is sometimes handled as a pointer.
  */
  union {
    long int active;
    void *for_padding_only;
  } u;
};

#define MAX_MACE_VARS      50

#define TERM_FLAG(t,flag)  (TP_BIT(t->private_flags, 1 << flag))
#define NEGATED(t)         (TERM_FLAG(t,Negation_flag))
#define LITERAL(t)         (TERM_FLAG(t,Relation_flag))
#define LIT(c,i)           ((c)->lits[i])

#define OR_TERM(t)        (SYMNUM((Term) t) == Or_sn)   /* ok if variable */
#define NOT_TERM(t)       (SYMNUM((Term) t) == Not_sn)  /* ok if variable */
#define EQ_TERM(t)        (SYMNUM((Term) t) == Eq_sn)   /* ok if variable */
#define FALSE_TERM(t)     ((t) == Domain[0])  /* ok if not variable */
#define TRUE_TERM(t)      ((t) == Domain[1])  /* ok if not variable */

/* End of public definitions */

/* Public function prototypes from syms.c */

void fprint_mclause_mem(FILE *fp, int heading);
void p_mclause_mem(void);
void zap_mterm(Term t);
void zap_mclause(Mclause c);
int lit_position(Mclause parent, Term child);
Mclause containing_mclause(Term t);
Term containing_mliteral(Term t);
void generate_ground_clauses(Topform c, Mstate state);

#endif  /* conditional compilation of whole file */
