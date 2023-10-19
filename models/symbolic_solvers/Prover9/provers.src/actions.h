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

#ifndef TP_ACTIONS_H
#define TP_ACTIONS_H

#include "search-structures.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from actions.c */

void init_action(Term t);

void init_actions(Plist rules,
		  void (*rebuild_sos_proc)(void),
		  void (*exit_proc)(int),
		  void (*assert_proc)(Topform));

void register_action(char *stat, char *val, char *op, char *arg1, char *arg2);

void statistic_actions(char *stat, int n);

void actions_in_proof(Plist proof,
		      Prover_attributes att);

#endif  /* conditional compilation of whole file */
