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

#ifndef TP_DEMOD_H
#define TP_DEMOD_H

#include "parautil.h"
#include "mindex.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* types of demodulator */

enum { NOT_DEMODULATOR, ORIENTED, LEX_DEP_LR, LEX_DEP_RL, LEX_DEP_BOTH };

/* Public function prototypes from demod.c */

int demodulator_type(Topform c, int lex_dep_demod_lim, BOOL sane);

void idx_demodulator(Topform c, int type, Indexop operation, Mindex idx);

int demod_attempts();

int demod_rewrites();

Term demodulate(Term t, Mindex demods, Ilist *just_head, BOOL lex_order_vars);

void demod1(Topform c, Topform demodulator, int direction,
	    Ilist *fpos, Ilist *ipos,
	    BOOL lex_order_vars);

void particular_demod(Topform c, Topform demodulator, int target, int direction,
		      Ilist *fpos, Ilist *ipos);

#endif  /* conditional compilation of whole file */
