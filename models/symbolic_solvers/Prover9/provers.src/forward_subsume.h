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

#ifndef TP_FORWARD_SUBSUME_H
#define TP_FORWARD_SUBSUME_H

#include "../ladr/subsume.h"
#include "../ladr/clock.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from forward_subsume.c */

void init_fsub_index(Mindextype mtype,
		     Uniftype utype,
		     int fpa_depth);

void fsub_destroy_index(void);

void index_fsub(Topform c, Indexop op, Clock clock);

Topform forward_subsumption_old(Topform d);

void unit_deletion_old(Topform c);

#endif  /* conditional compilation of whole file */
