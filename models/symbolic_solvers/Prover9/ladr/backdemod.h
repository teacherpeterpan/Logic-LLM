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

#ifndef TP_BACKDEMOD_H
#define TP_BACKDEMOD_H

#include "demod.h"
#include "clist.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from backdemod.c */

void index_clause_back_demod(Topform c, Mindex idx, Indexop op);

BOOL rewritable_clause(Topform demod, Topform c);

Plist back_demod_linear(Topform demod, Clist lst, Plist rewritables);

Plist back_demod_indexed(Topform demod, int type, Mindex idx,
			 BOOL lex_order_vars);

#endif  /* conditional compilation of whole file */
