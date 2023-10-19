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

#ifndef TP_ESTACK_H
#define TP_ESTACK_H

#include "../ladr/header.h"
#include "../ladr/memory.h"

/* INTRODUCTION
Estacks can be used to manage generic pointer assignments
that must be undone at a later time.  This is typically
used in backtracking procedures that work by moving
pointers around.  Say you need to assign the value of
one pointer *a to another pointer *b, and some time later
you'll need to undo that assignment, restoring a to its
old value.  Instead of using a special-purpose data structure
to store a's old value, you can write
<pre>
    estack = update_and_push((void **) &a, b, estack);
    ...
    restore_from_stack(estack);
</pre>
The update_and_push call assigns b to a and records the
assignment in the Estack.  The restore_from_stack call undoes
a whole stack of assignments.

<P>
This mechanism uses void pointers for both a and b, so
it works for any kind of pointer.  If b is not a pointer
(for example an integral type), make sure it is the same
size as a pointer. 
*/

/* Public definitions */

typedef struct estack * Estack;

/* End of public definitions */

/* Public function prototypes from estack.c */

/* from estack.c */

void fprint_estack_mem(FILE *fp, int heading);

void p_estack_mem(void);

void free_estack_memory(void);

int estack_bytes(void);

Estack update_and_push(void **p, void *new, Estack stack);

void restore_from_stack(Estack stack);

void zap_estack(Estack s);

#endif  /* conditional compilation of whole file */
