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

#ifndef TP_TPTP_TRANS_H
#define TP_TPTP_TRANS_H

#include "ioutil.h"
#include "clausify.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from tptp_trans.c */

void declare_tptp_input_types(void);

void declare_tptp_output_types(void);

Formula tptp_input_to_ladr_formula(Term t);

Plist ladr_list_to_tptp_list(Plist in, char *name, char *type);

Ilist syms_in_form(Term t, BOOL clausal);

I2list map_for_bad_tptp_syms(Ilist syms, BOOL quote_bad_syms);

I2list map_for_bad_ladr_syms(Ilist syms, BOOL quote_bad_syms);

Term replace_bad_syms_term(Term t, I2list map);

Term replace_bad_tptp_syms_form(Term t, BOOL clausal, I2list map);

#endif  /* conditional compilation of whole file */
