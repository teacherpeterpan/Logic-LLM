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

#ifndef TP_HASH_H
#define TP_HASH_H

#include "memory.h"

typedef struct hashtab * Hashtab;
typedef struct hashnode * Hashnode;

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from hash.c */

void fprint_hash_mem(FILE *fp, BOOL heading);

void p_hash_mem();

Hashtab hash_init(int size);

void hash_insert(void *v, unsigned hashval, Hashtab h);

void *hash_lookup(void *v, unsigned hashval, Hashtab h,
		  BOOL (*id_func) (void *, void *));

void hash_delete(void *v, unsigned hashval, Hashtab h,
		 BOOL (*id_func) (void *, void *));

void hash_destroy(Hashtab h);

void hash_info(Hashtab h);

#endif  /* conditional compilation of whole file */
