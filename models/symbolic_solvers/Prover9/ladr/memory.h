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

#ifndef TP_MEMORY_H
#define TP_MEMORY_H

#include "fatal.h"
#include <sys/time.h>
#include <sys/resource.h>

/* INTRODUCTION
*/

/* Public definitions */

/* The following definitions exist because the memory get/free
   routines measure memory by pointers instead of bytes. */

#define CEILING(n,d)   ((n)%(d) == 0 ? (n)/(d) : (n)/(d) + 1)
#define BYTES_POINTER  sizeof(void *)  /* bytes per pointer */
#define PTRS(n)        CEILING(n, BYTES_POINTER) /* ptrs needed for n bytes */

/* End of public definitions */

/* Public function prototypes from memory.c */

void *get_cmem(unsigned n);

void *get_mem(unsigned n);

void free_mem(void *q, unsigned n);

void memory_report(FILE *fp);

int megs_malloced(void);

void set_max_megs(int megs);

void set_max_megs_proc(void (*proc)(void));

int bytes_palloced(void);

void *tp_alloc(size_t n);

unsigned mega_mem_calls(void);

void disable_max_megs(void);

void enable_max_megs(void);

#endif  /* conditional compilation of whole file */
