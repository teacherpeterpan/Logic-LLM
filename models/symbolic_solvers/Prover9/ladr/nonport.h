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

#ifndef TP_NONPORT_H
#define TP_NONPORT_H

/* #define PRIMITIVE_ENVIRONMENT */

/* INTRODUCTION
This package has some utilities that might not be portable
to other environments.  If you get errors trying to compile
this package, you can either modify nonport.c, or
<PRE>
#define PRIMITIVE_ENVIRONMENT
</PRE>
in nonport.h, which disables everything.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from nonport.c */

char *username(void);

char *hostname(void);

int my_process_id(void);

int get_bits(void);

#endif  /* conditional compilation of whole file */
