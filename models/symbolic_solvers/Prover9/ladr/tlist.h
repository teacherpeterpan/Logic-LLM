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

#ifndef TP_TLIST_H
#define TP_TLIST_H

#include "term.h"

/* INTRODUCTION
This little package handles singly-linked lists of terms.
The data type Plist is used to build the lists.
<P>
Note that there is another package, listterm, that does similar
things, but constructing the lists from ordinary terms, as in Prolog.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from tlist.c */

void zap_tlist(Plist p);

Plist tlist_remove(Term t, Plist p);

Plist tlist_union(Plist a, Plist b);

Ilist constants_in_term(Term t, Ilist p);

Plist tlist_copy(Plist p);

#endif  /* conditional compilation of whole file */
