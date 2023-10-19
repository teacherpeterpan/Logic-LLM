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

#ifndef MACE4_SYMS_H
#define MACE4_SYMS_H

/* INTRODUCTION
*/

/* Public definitions */

/* Symbol types */

enum {
  FUNCTION,
  RELATION
};

/* Symbol attributes */

enum {
  ORDINARY_SYMBOL,
  EQUALITY_SYMBOL,
  SKOLEM_SYMBOL
};

typedef struct symbol_data * Symbol_data;

struct symbol_data {  /* SORT */
  int sn;             /* ordinary symbol ID */
  int mace_sn;        /* MACE symbol ID */
  int arity;
  int type;           /* FUNCTION or RELATION */
  int base;
  int attribute;      /* ORDINARY_SYMBOL or EQUALITY_SYMBOL */
  Symbol_data next;
};

/* End of public definitions */

/* Public function prototypes from syms.c */

Symbol_data get_symbol_data(void);
Symbol_data find_symbol_data(int sn);
void init_built_in_symbols(void);
int collect_mace4_syms(Plist clauses, BOOL arithmetic);
void p_symbol_data(Symbol_data s);
Symbol_data find_symbol_node(int id);
int id_to_domain_size(int id);
int max_index(int id, Symbol_data s);

#endif  /* conditional compilation of whole file */
