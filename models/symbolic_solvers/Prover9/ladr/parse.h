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

#ifndef TP_PARSE_H
#define TP_PARSE_H

#include "listterm.h"

/* INTRODUCTION
This package has routines for reading and writing terms in
human-readable form.  Binary function symbols can be declared
to be infix with a precedence and left or right association,
so that many parentheses can be omitted.  Unary symbols
can be declared to be prefix or postfix.
In addition, prolog-style list-notation and quantified formulas are
supported.
<P>
The symbol declarations and parse rules are similar to the method
used by many Prolog systems (although we use mnemonic names 
instead of xfy, yfx, yf, etc.).  The symbol declarations
are made with set_parse_type().
<P>
This package is based on code taked form Otter and EQP, but there
are some important differences.  Detailed documentation should
be available elsewhere.
<P>
The intension is to use this package for reading
and writing all types of data (terms, clauses, formulas, control
information, etc.), with outside routines to translate to/from
the appropriate internal data structure when necessary.
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from parse.c */

void fprint_parse_mem(FILE *fp, BOOL heading);

void p_parse_mem(void);

void translate_neg_equalities(BOOL flag);

BOOL ordinary_constant_string(char *s);

Term sread_term(String_buf sb, FILE *fout);

Term read_term(FILE *fin, FILE *fout);

Term parse_term_from_string(char *s);

void sb_write_term(String_buf sb, Term t);

void fwrite_term(FILE *fp, Term t);

void fwrite_term_nl(FILE *fp, Term t);

char *process_quoted_symbol(char *str);

void declare_parse_type(char *str, int precedence, Parsetype type);

void declare_quantifier_precedence(int prec);

void declare_standard_parse_types(void);

BOOL redeclare_symbol_and_copy_parsetype(char *operation, char *str,
					 BOOL echo, FILE *fout);

void skip_to_nl(FILE *fp);

Plist split_string(char *onlys);

void set_cons_char(char c);

char get_cons_char(void);

void set_quote_char(char c);

char get_quote_char(void);

void parenthesize(BOOL setting);

void check_for_illegal_symbols(BOOL setting);

void simple_parse(BOOL setting);

#endif  /* conditional compilation of whole file */
