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

#ifndef TP_PROPAGATE_H
#define TP_PROPAGATE_H

/* types of items in the Todo list */

#define ASSIGNMENT        0  /* f(1,2)=3      P(1,2)     ~P(1,2) */
#define NEAR_ASSIGNMENT   1  /* f(1,g(2))=3   P(1,g(2))   ~P(1,g(2)) */
#define ELIMINATION       2  /* f(1,2) != 3 */
#define NEAR_ELIMINATION  3  /* f(1,g(2)) != 3 */

/* Public function prototypes from propagate.c */

BOOL eterm(Term t, int *id);

Term decode_eterm_id(int id);

void process_initial_clause(Mclause c, Mstate state);

Estack assign_and_propagate(int id, Term value);

Estack reset_estack(void);

void new_elimination(int id, Term beta, Mstate state);

#endif  /* conditional compilation of whole file */
