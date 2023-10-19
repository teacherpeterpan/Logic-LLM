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

#ifndef TP_OPTIONS_H
#define TP_OPTIONS_H

#include "string.h"

/* INTRODUCTION
There are 3 types of option:
<UL>
<LI>Flags: Boolean
<LI>Parms: Integer
<LI>Stringparm: String
</UL>
<P>
To introduce a new option, choose an integer ID that is
unique for the type of option, and call the appropriate
initialization routine.  Then you can change the value
of the option and check its value as you like.
<P>
The routine read_commands() (in the "commands" package)
will parse the user's set, clear, and assign commands,
making the appropriate changes to the values of the options.
<P>
In most cases, the applications programmer will be using
the following routines.
<UL>
<LI>Flags: init_flag(), flag().
<LI>Parms: init_parm(), parm().
<LI>Stringparm: init_stringparm(), stringparm().
</UL>
*/

/* Public definitions */

#define MAX_FLAGS                100
#define MAX_PARMS                100
#define MAX_STRINGPARMS          100
#define MAX_FLOATPARMS           100

/* End of public definitions */

/* Public function prototypes from options.c */

void fprint_options_mem(FILE *fp, BOOL heading);

void p_options_mem();

void enable_option_dependencies(void);

void disable_option_dependencies(void);

BOOL option_dependencies_state(void);

int init_flag(char *name,
	      BOOL default_value);

int init_parm(char *name,
	      int default_value,
	      int min_value,
	      int max_value);

int init_floatparm(char *name,
		   double default_value,
		   double min_value,
		   double max_value);

int init_stringparm(char *name,
		    int n,
		    ...);

void fprint_options(FILE *fp);

void p_options(void);


int flag(int flag_id);

int parm(int parm_id);

BOOL at_parm_limit(int value, int parm_id);

BOOL over_parm_limit(int value, int parm_id);

double floatparm(int floatparm_id);

BOOL stringparm(int id, char *s);

char *stringparm1(int id);

void update_flag(FILE *fout, int id, BOOL val, BOOL echo);

void set_flag(int id, BOOL echo);

void clear_flag(int id, BOOL echo);

void assign_parm(int id, int val, BOOL echo);

void assign_floatparm(int id, double val, BOOL echo);

void assign_stringparm(int id, char *val, BOOL echo);

int str_to_flag_id(char *name);

int str_to_parm_id(char *name);

int str_to_floatparm_id(char *name);

int str_to_stringparm_id(char *name);

char *flag_id_to_str(int id);

char *parm_id_to_str(int id);

char *floatparm_id_to_str(int id);

char *stringparm_id_to_str(int id);

void flag_flag_dependency(int id, BOOL val, int dep_id, BOOL dep_val);

void flag_flag_dep_default(int id, BOOL val, int dep_id);

void flag_parm_dependency(int id, BOOL val, int dep_id, int dep_val);

void flag_floatparm_dependency(int id, BOOL val, int dep_id, double dep_val);

void flag_parm_dep_default(int id, BOOL val, int dep_id);

void flag_floatparm_dep_default(int id, BOOL val, int dep_id);

void parm_flag_dependency(int id, int dep_id, int dep_val);

void parm_parm_dependency(int id, int dep_id, int dep_val, BOOL multiply);

void flag_stringparm_dependency(int id, BOOL val, int dep_id, char *dep_val);

void flag_stringparm_dep_default(int id, BOOL val, int dep_id);

int option_updates(void);

int flag_default(int flag_id);

int parm_default(int parm_id);

int floatparm_default(int floatparm_id);

char *stringparm1_default(int id);

#endif  /* conditional compilation of whole file */
