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

#ifndef TP_STRBUF_H
#define TP_STRBUF_H

#include "string.h"

/* INTRODUCTION
A String_buf is a kind of string that can grow as big as you need.
This is implemented as a list of dynamically allocated character
arrays of fixed size.
The only problem with using String_bufs is that you have to remember
to free a String_buf when you are finished with it.
<P>
This is similar to the StringBuffer class in Java, and the
cstrings of our old theorem prover LMA/ITP.  We didn't have anything
like this in Otter, but there were times when I wish we had,
so here it is.
*/

/* Public definitions */

typedef struct string_buf * String_buf;

/* End of public definitions */

/* Public function prototypes from strbuf.c */

String_buf get_string_buf(void);

void fprint_strbuf_mem(FILE *fp, BOOL heading);

void p_strbuf_mem();

String_buf init_string_buf(char *s);

void fprint_sb(FILE *fp, String_buf sb);

void p_sb(String_buf sb);

void sb_append(String_buf sb, char *s);

void sb_append_char(String_buf sb, char c);

void sb_replace_char(String_buf sb, int i, char c);

char sb_char(String_buf sb, int n);

void sb_cat_copy(String_buf sb1, String_buf sb2);

void sb_cat(String_buf sb1, String_buf sb2);

void zap_string_buf(String_buf sb);

char *sb_to_malloc_string(String_buf sb);

char *sb_to_malloc_char_array(String_buf sb);

int sb_size(String_buf sb);

void sb_append_int(String_buf sb, int i);

#endif  /* conditional compilation of whole file */
