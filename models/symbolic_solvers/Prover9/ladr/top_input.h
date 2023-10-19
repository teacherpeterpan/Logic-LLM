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

#ifndef TP_TOP_INPUT_H
#define TP_TOP_INPUT_H

#include "ioutil.h"
#include "std_options.h"
#include "tptp_trans.h"

/* INTRODUCTION
*/

/* Public definitions */

/* Types of object that can be input */

enum { TERMS, FORMULAS };

/* What shall we do if we read an unknown flag or parameter? */

enum {
  IGNORE_UNKNOWN,
  NOTE_UNKNOWN,
  WARN_UNKNOWN,
  KILL_UNKNOWN
};

/* End of public definitions */

/* Public function prototypes from top_input.c */

void fprint_top_input_mem(FILE *fp, BOOL heading);

void p_top_input_mem();

void init_standard_ladr(void);

void set_program_name(char *name);

void process_op(Term t, BOOL echo, FILE *fout);

void process_redeclare(Term t, BOOL echo, FILE *fout);

void flag_handler(FILE *fout, Term t, BOOL echo, int unknown_action);

void parm_handler(FILE *fout, Term t, BOOL echo, int unknown_action);

void accept_list(char *name, int type, BOOL aux, Plist *l);

void read_from_file(FILE *fin, FILE *fout, BOOL echo, int unknown_action);

void read_all_input(int argc, char **argv, FILE *fout,
		    BOOL echo, int unknown_action);

Plist process_input_formulas(Plist formulas, BOOL echo);

Plist process_demod_formulas(Plist formulas, BOOL echo);

Plist process_goal_formulas(Plist formulas, BOOL echo);

Term read_commands(FILE *fin, FILE *fout, BOOL echo, int unknown_action);

Plist embed_formulas_in_topforms(Plist formulas, BOOL assumption);

#endif  /* conditional compilation of whole file */
