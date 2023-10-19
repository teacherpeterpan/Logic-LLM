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

#ifndef TP_IOUTIL_H
#define TP_IOUTIL_H

#include "parse.h"
#include "fastparse.h"
#include "ivy.h"
#include "clausify.h"

/* INTRODUCTION
*/

/* Public definitions */

enum { CL_FORM_STD,
       CL_FORM_BARE,
       CL_FORM_PARENTS,
       CL_FORM_XML,
       CL_FORM_TAGGED,
       CL_FORM_IVY};  /* clause print format */

/* End of public definitions */

/* Public function prototypes from ioutil.c */

void fwrite_formula(FILE *fp, Formula f);

Topform read_clause(FILE *fin, FILE *fout);

Topform parse_clause_from_string(char *s);

BOOL end_of_list_clause(Topform c);

Clist read_clause_clist(FILE *fin, FILE *fout, char *name, BOOL assign_id);

Plist read_clause_list(FILE *fin, FILE *fout, BOOL assign_id);

void sb_write_clause_jmap(String_buf sb, Topform c,
			  int format,
			  I3list map);

void sb_write_clause(String_buf sb, Topform c, int format);

void sb_xml_write_clause_jmap(String_buf sb, Topform c, I3list map);

void sb_tagged_write_clause_jmap(String_buf sb, Topform c, I3list map);

void fwrite_clause_jmap(FILE *fp, Topform c, int format, I3list map);
			

void fwrite_clause(FILE *fp, Topform c, int format);

void f_clause(Topform c);

void fwrite_clause_clist(FILE *fp, Clist lst, int format);

void fwrite_demod_clist(FILE *fp, Clist lst, int format);

void fwrite_clause_list(FILE *fp, Plist lst, char *name, int format);

void f_clauses(Plist p);

Formula read_formula(FILE *fin, FILE *fout);

BOOL end_of_list_formula(Formula f);

Plist read_formula_list(FILE *fin, FILE *fout);

void fwrite_formula_list(FILE *fp, Plist lst, char *name);

void zap_formula_list(Plist lst);

BOOL end_of_list_term(Term t);

BOOL end_of_commands_term(Term t);

Plist read_term_list(FILE *fin, FILE *fout);

void fwrite_term_list(FILE *fp, Plist lst, char *name);

Term term_reader(BOOL fast);

void term_writer(Term t, BOOL fast);

Topform clause_reader(BOOL fast);

void clause_writer(Topform c, BOOL fast);

Topform term_to_topform2(Term t);

Topform read_clause_or_formula(FILE *fin, FILE *fout);

Plist read_clause_or_formula_list(FILE *fin, FILE *fout);

#endif  /* conditional compilation of whole file */
