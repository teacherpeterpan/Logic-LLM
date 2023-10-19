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

#ifndef TP_FATAL_H
#define TP_FATAL_H

#include "header.h"

/* INTRODUCTION
This package is just a few utilities for handling fatal errors.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from fatal.c */

void bell(FILE *fp);

int get_fatal_exit_code();

void set_fatal_exit_code(int exit_code);

void fatal_error(char *message);

#endif  /* conditional compilation of whole file */
