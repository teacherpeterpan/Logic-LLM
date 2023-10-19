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

#ifndef TP_DEMODULATE_H
#define TP_DEMODULATE_H

#include "../ladr/ladr.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from demodulate.c */

void init_demodulator_index(Mindextype mtype, Uniftype utype, int fpa_depth);

void init_back_demod_index(Mindextype mtype, Uniftype utype, int fpa_depth);

void index_demodulator(Topform c, int type, Indexop operation, Clock clock);

void index_back_demod(Topform c, Indexop operation, Clock clock, BOOL enabled);

void destroy_demodulation_index(void);

void destroy_back_demod_index(void);

void demodulate_clause(Topform c, int step_limit, int increase_limit,
		       BOOL print, BOOL lex_order_vars);

Plist back_demodulatable(Topform demod, int type, BOOL lex_order_vars);

void back_demod_idx_report(void);

#endif  /* conditional compilation of whole file */
