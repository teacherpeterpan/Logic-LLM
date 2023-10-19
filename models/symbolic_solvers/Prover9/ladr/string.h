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

#ifndef TP_STRING_H
#define TP_STRING_H

#include "memory.h"

/* INTRODUCTION
This package contains a few utilities for character strings.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from string.c */

BOOL str_ident(char *s, char *t);

char *new_str_copy(char *str);

BOOL string_member(char *string, char **strings, int n);

int which_string_member(char *string, char **strings, int n);

BOOL initial_substring(char *x, char *y);

BOOL substring(char *x, char *y);

void reverse_chars(char *s, int start, int end);

int natural_string(char *str);

int char_occurrences(char *s, char c);

char *escape_char(char *s, char c);

BOOL str_to_int(char *str, int *ip);

char *int_to_str(int n, char *s, int size);

BOOL str_to_double(char *s,
		   double *dp);

char *double_to_str(double d,
		    char *s,
		    int size);

BOOL string_of_repeated(char c, char *s);

#endif  /* conditional compilation of whole file */
