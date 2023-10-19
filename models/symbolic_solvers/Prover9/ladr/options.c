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

#include "options.h"
#include <stdarg.h>  /* for variable argument lists */

/* Private definitions and types */

/*
  Dependencies.
*/

typedef enum { FLAGT, PARMT, FLOATPARMT, STRINGPARMT } Opttype;
typedef struct optdep * Optdep;

struct optdep {  /* these get attached to flags/parms that have dependents */
  BOOL flag_trigger;   /* whether set or clear triggers action */
  Opttype type;        /* type of dependent option */
  int id;              /* id of dependent option */
  int val;             /* value for dependent flag or parm */
  double dval;         /* value for dependent floatparm */
  char *sval;          /* value for dependent stringparm */
  BOOL multiply;       /* means val is multiplier instead of fixed value */
  Optdep next;         /* next dependent */
};

struct flag {  /* Flags are boolean valued options */
  char *name;
  BOOL val;
  BOOL default_val;
  Optdep dependencies;
};

struct parm {  /* Parms are integer valued options */
  char *name;
  int val;
  int default_val;
  int min, max;  /* minimum and maximum permissible values */
  Optdep dependencies;
};

struct floatparm {  /* FloatParms are double-valued options */
  char *name;
  double val;
  double default_val;
  double min, max;  /* minimum and maximum permissible values */
  Optdep dependencies;
};

struct stringparm {  /* Stringparms are string valued options */
  char *name;
  char *val;      /* current valuse: index into array of strings */
  char *default_val;  /* current valuse: index into array of strings */
  int n;          /* number of possible values */
  char **range;   /* array of possible values */
  /* Optdep dependencies; */
};

static struct flag Flags[MAX_FLAGS];
static struct parm Parms[MAX_PARMS];
static struct floatparm Floatparms[MAX_FLOATPARMS];
static struct stringparm Stringparms[MAX_STRINGPARMS];

static int Next_flag = 0;
static int Next_parm = 0;
static int Next_floatparm = 0;
static int Next_stringparm = 0;

static int Option_updates = 0;  /* Flag, Parm, Stringparm */

static BOOL Ignore_dependencies = FALSE;

/*
 * memory management
 */

#define PTRS_OPTDEP PTRS(sizeof(struct optdep))
static unsigned Optdep_gets, Optdep_frees;

/*************
 *
 *   Optdep get_optdep()
 *
 *************/

static
Optdep get_optdep(void)
{
  Optdep p = get_cmem(PTRS_OPTDEP);
  Optdep_gets++;
  return(p);
}  /* get_optdep */

/*************
 *
 *    free_optdep()
 *
 *************/

#if 0
static
void free_optdep(Optdep p)
{
  free_mem(p, PTRS_OPTDEP);
  Optdep_frees++;
}  /* free_optdep */
#endif

/*************
 *
 *   fprint_options_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the options package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_options_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = sizeof(struct optdep);
  fprintf(fp, "optdep (%4d)       %11u%11u%11u%9.1f K\n",
          n, Optdep_gets, Optdep_frees,
          Optdep_gets - Optdep_frees,
          ((Optdep_gets - Optdep_frees) * n) / 1024.);

}  /* fprint_options_mem */

/*************
 *
 *   p_options_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the options package.
*/

/* PUBLIC */
void p_options_mem()
{
  fprint_options_mem(stdout, TRUE);
}  /* p_options_mem */

/*
 *  end of memory management
 */

/*************
 *
 *   enable_option_dependencies()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void enable_option_dependencies(void)
{
  Ignore_dependencies = FALSE;
}  /* enable_option_dependencies */

/*************
 *
 *   disable_option_dependencies()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void disable_option_dependencies(void)
{
  Ignore_dependencies = TRUE;
}  /* disable_option_dependencies */

/*************
 *
 *   option_dependencies_state()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
BOOL option_dependencies_state(void)
{
  return !Ignore_dependencies;
}  /* option_dependencies_state */

/*************
 *
 *    init_flag()
 *
 *************/

/* DOCUMENTATION
Initialize a flag (boolean-valued option).
You give it a name and a default value, and an integer id is returned.
Flags are typically changed by user commands
which are parsed by read_commands().  The value of a flag is
checked with flag(ID).
*/

/* PUBLIC */
int init_flag(char *name,
	      BOOL default_value)
{
  int id = -1;
  if (Next_flag == MAX_FLAGS)
    fatal_error("init_flag, too many flags");
  else if (FALSE && str_to_flag_id(name) != -1) {
    char s[100];
    sprintf(s, "init_flag, flag %s already exists", name);
    fatal_error(s);
  }
  else {
    id = Next_flag++;
    Flags[id].name = name;
    Flags[id].val = default_value;
    Flags[id].default_val = default_value;
    Flags[id].dependencies = NULL;
  }
  return id;
}  /* init_flag */

/*************
 *
 *    init_parm()
 *
 *************/

/* DOCUMENTATION
Initialize a parm (integer-valued option).
You give it a name, a default value, and  min and max values.
An integer id is returned.
Parms are typically
changed by user commands which are parsed by read_commands().
The value of a parm is checked with parm(ID).
*/

/* PUBLIC */
int init_parm(char *name,
	      int default_value,
	      int min_value,
	      int max_value)
{
  int id = -1;
  if (Next_parm == MAX_PARMS)
    fatal_error("init_parm: too many parms");
  else {
    id = Next_parm++;
    Parms[id].name = name;
    Parms[id].val = default_value;
    Parms[id].default_val = default_value;
    Parms[id].min = min_value;
    Parms[id].max = max_value;
  }
  return id;
}  /* init_parm */

/*************
 *
 *    init_floatparm()
 *
 *************/

/* DOCUMENTATION
Initialize a floatparmparm (double-valued option).
You give it a name, a default value, and  min and max values.
An integer id is returned.
Parms are typically
changed by user commands which are parsed by read_commands().
The value of a parm is checked with floatparm(ID).
*/

/* PUBLIC */
int init_floatparm(char *name,
		   double default_value,
		   double min_value,
		   double max_value)
{
  int id = -1;
  if (Next_floatparm == MAX_FLOATPARMS)
    fatal_error("init_floatparm: too many parms");
  else {
    id = Next_floatparm++;
    Floatparms[id].name = name;
    Floatparms[id].val = default_value;
    Floatparms[id].default_val = default_value;
    Floatparms[id].min = min_value;
    Floatparms[id].max = max_value;
  }
  return id;
}  /* init_floatparm */

/*************
 *
 *    init_stringparm()
 *
 *************/

/* DOCUMENTATION
Initialize a stringparm (string-valued option).
You give it a name, a number n of possible values,
and n strings which are the possible values.
An integer id is returned
The first string given is the default value.
<P>
Stringparms are typically changed by user commands which are parsed
by read_commands().
The value of a stringparm is checked with the Boolean routine
stringparm(ID, string).
*/

/* PUBLIC */
int init_stringparm(char *name,
		    int n,
		    ...)
{
  int id = -1;
  if (Next_stringparm == MAX_STRINGPARMS)
    fatal_error("init_stringparm: too many stringparms");
  else {
    int i;
    va_list parameters;
    id = Next_stringparm++;
    va_start(parameters, n);
    Stringparms[id].range = malloc(n * sizeof(char *));
    for (i = 0; i < n; i++)
      Stringparms[id].range[i] = va_arg(parameters, char *);
    va_end(parameters);

    Stringparms[id].name = name;
    Stringparms[id].n = n;       /* number of values */
    Stringparms[id].val = Stringparms[id].range[0];  /* first is default */
    Stringparms[id].default_val = Stringparms[id].range[0];  /* first is default */
  }
  return id;
}  /* init_stringparm */

/*************
 *
 *    fprint_options(fp)
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) the current values of all of the
the options (flags, parms, and stringparms).
*/

/* PUBLIC */
void fprint_options(FILE *fp)
{
  int i, j;

  fprintf(fp, "\n--------------- options ---------------\n");

  j = 0;
  for (i = 0; i < Next_flag; i++) {  /* print flags */
    fprintf(fp, "%s", Flags[i].val ? "set(" : "clear(");
    fprintf(fp, "%s). ", Flags[i].name);
    fprintf(fp, "%s", (++j % 3 == 0) ? "\n" : "");
  }
  fprintf(fp, "\n\n");

  j = 0;
  for (i = 0; i < Next_parm; i++) {  /* print parms */
    fprintf(fp, "assign(");
    fprintf(fp, "%s, %d). ", Parms[i].name, Parms[i].val);
    fprintf(fp, "%s", (++j % 3 == 0) ? "\n" : "");
  }
  fprintf(fp, "\n\n");

  j = 0;
  for (i = 0; i < Next_floatparm; i++) {  /* print parms */
    fprintf(fp, "assign(");
    fprintf(fp, "%s, \"%.3f\"). ", Floatparms[i].name, Floatparms[i].val);
    fprintf(fp, "%s", (++j % 3 == 0) ? "\n" : "");
  }
  fprintf(fp, "\n\n");

  j = 0;
  for (i = 0; i < Next_stringparm; i++) {  /* print stringparms */
    struct stringparm *sp = &(Stringparms[i]);
    fprintf(fp, "assign(");
    fprintf(fp, "%s, %s). ", sp->name, sp->val);
    fprintf(fp, "%s", (++j % 3 == 0) ? "\n" : "");
  }
  fprintf(fp, "\n");
  fflush(fp);
}  /* fprint_options */

/*************
 *
 *    p_options()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) the current values of the
the options (flags, parms, and stringparms).
*/

/* PUBLIC */
void p_options(void)
{
  fprint_options(stdout);
}  /* p_options */

/*************
 *
 *   flag()
 *
 *************/

/* DOCUMENTATION
This Boolean routine returns the value of a flag.  If the Flag
index is out of range, bad things can happen.
*/

/* PUBLIC */

int flag(int flag_id)
{
  return Flags[flag_id].val;
}  /* flag */

/*************
 *
 *   parm()
 *
 *************/

/* DOCUMENTATION
This integer routine returns the value of a parameter.  If the parm
index is out of range, bad things can happen.
*/

/* PUBLIC */
int parm(int parm_id)
{
  return Parms[parm_id].val;
}  /* parm */

/*************
 *
 *   at_parm_limit()
 *
 *************/

/* DOCUMENTATION
This assumes that -1 represents infinity.
*/

/* PUBLIC */
BOOL at_parm_limit(int value, int parm_id)
{
  int limit = Parms[parm_id].val;
  if (limit == -1)
    return FALSE;  /* no limit */
  else
    return value >= limit;
}  /* at_parm_limit */

/*************
 *
 *   over_parm_limit()
 *
 *************/

/* DOCUMENTATION
This assumes that -1 represents infinity.
*/

/* PUBLIC */
BOOL over_parm_limit(int value, int parm_id)
{
  int limit = Parms[parm_id].val;
  if (limit == -1)
    return FALSE;  /* no limit */
  else
    return value > limit;
}  /* over_parm_limit */

/*************
 *
 *   floatparm()
 *
 *************/

/* DOCUMENTATION
This integer routine returns the value of a parameter.  If the floatparm
index is out of range, bad things can happen.
*/

/* PUBLIC */
double floatparm(int floatparm_id)
{
  return Floatparms[floatparm_id].val;
}  /* floatparm */

/*************
 *
 *   stringparm()
 *
 *************/

/* DOCUMENTATION
This routine checks if the current value of a stringparm
matches the given string.  The ID must be valid.
*/

/* PUBLIC */
BOOL stringparm(int id, char *s)
{
  return str_ident(Stringparms[id].val, s);
}  /* stringparm */

/*************
 *
 *   stringparm1()
 *
 *************/

/* DOCUMENTATION
This routine returns the current value of a stringparm.
The ID must be valid.
*/

/* PUBLIC */
char *stringparm1(int id)
{
  return Stringparms[id].val;
}  /* stringparm1 */

/*************
 *
 *   update_flag()
 *
 *************/

/* DOCUMENTATION
This performs the role of set_flag() and clear_flag().
In addition, an output file is given for dependency messages.
*/

/* PUBLIC */
void update_flag(FILE *fout, int id, BOOL val, BOOL echo)
{
  if (id < 0 || id >= Next_flag) {
    fprintf(fout, "update_flag: flag id %d, is out of range.\n", id);
    fprintf(stderr, "update_flag: flag id %d, is out of range.\n", id);
    fatal_error("update_flag, flag out of range");
  }
  else {
    Optdep p;
    Flags[id].val = val;
    Option_updates++;
    /* special case */
    if (str_ident(Flags[id].name, "ignore_option_dependencies"))
      Ignore_dependencies = val;

    if (!Ignore_dependencies) {
      for (p = Flags[id].dependencies; p; p = p->next) {
	if (p->flag_trigger == val) {
	  if (p->type == FLAGT) {
	    if (echo) {
	      fprintf(fout, "    %% %s(%s) -> %s(%s).\n",
		      val ? "set" : "clear",
		      Flags[id].name,
		      p->val ? "set" : "clear",
		      Flags[p->id].name);
	    }
	    update_flag(fout, p->id, p->val, echo);
	  }
	  else if (p->type == PARMT) {
	    if (echo) {
	      fprintf(fout, "    %% %s(%s) -> assign(%s, %d).\n",
		      val ? "set" : "clear",
		      Flags[id].name,
		      Parms[p->id].name,
		      p->val);
	    }
	    assign_parm(p->id, p->val, echo);
	  }
	  else if (p->type == FLOATPARMT) {
	    if (echo) {
	      fprintf(fout, "    %% %s(%s) -> assign(%s, \"%.3f\").\n",
		      val ? "set" : "clear",
		      Flags[id].name,
		      Floatparms[p->id].name,
		      p->dval);
	    }
	    assign_floatparm(p->id, p->dval, echo);
	  }
	  else {
	    /* assume it's a stringparm */
	    if (echo) {
	      fprintf(fout, "    %% %s(%s) -> assign(%s, %s).\n",
		      val ? "set" : "clear",
		      Flags[id].name,
		      Stringparms[p->id].name,
		      p->sval);
	    }
	    assign_stringparm(p->id, p->sval, echo);
	  }
	}
      }
    }
  }
}  /* update_flag */

/*************
 *
 *   set_flag()
 *
 *************/

/* DOCUMENTATION
This routine sets a flag.  The flag is identified by its
integer ID (which is available from str_to_flag_id()).
If the ID is not valid, a fatal error occurs.
*/

/* PUBLIC */
void set_flag(int id, BOOL echo)
{
  update_flag(stdout, id, TRUE, echo);
}  /* set_flag */

/*************
 *
 *   clear_flag()
 *
 *************/

/* DOCUMENTATION
This routine clears a flag.  The flag is identified by its
integer ID (which is available from str_to_flag_id()).
If the ID is not valid, a fatal error occurs.
*/

/* PUBLIC */
void clear_flag(int id, BOOL echo)
{
  update_flag(stdout, id, FALSE, echo);
}  /* clear_flag */

/*************
 *
 *   assign_parm()
 *
 *************/

/* DOCUMENTATION
This routine assigns a value to a parm.  The parm is identified by
its integer ID (which is available from str_to_parm_id()).
If the ID is not valid, or if the value is out of range, a
fatal error occurs.
*/

/* PUBLIC */
void assign_parm(int id, int val, BOOL echo)
{
  if (id < 0 || id >= Next_parm) {
    fprintf(stdout, "assign_parm: parm id %d, is out of range.\n", id);
    fprintf(stderr, "assign_parm: parm id %d, is out of range.\n", id);
    fatal_error("assign_parm");
  }
  else if (val < Parms[id].min || val > Parms[id].max) {
    fprintf(stdout, "assign_parm: parm %s, value %d out of range [%d..%d].\n",
	    Parms[id].name, val, Parms[id].min, Parms[id].max);
    fprintf(stderr, "assign_parm: parm %s, value %d out of range [%d..%d].\n",
	    Parms[id].name, val, Parms[id].min, Parms[id].max);
    fatal_error("assign_parm");
  }
  else {
    Optdep p;
    Parms[id].val = val;
    Option_updates++;
    if (!Ignore_dependencies) {
      for (p = Parms[id].dependencies; p; p = p->next) {
	if (p->type == PARMT) {
	  /* parm -> parm */
	  int dval = p->multiply ? p->val * val : p->val;
	  if (echo) {
	    fprintf(stdout, "    %% assign(%s, %d) -> assign(%s, %d).\n",
		    Parms[id].name,
		    val,
		    Parms[p->id].name,
		    dval);
	  }
	  assign_parm(p->id, dval, echo);
	}
	else {
	  /* parm -> flag */
	  if (echo) {
	    fprintf(stdout, "    %% assign(%s, %d) -> %s(%s).\n",
		    Parms[id].name,
		    val,
		    p->val ? "set" : "clear",
		    Flags[p->id].name);
	  }
	  update_flag(stdout, p->id, p->val, echo);
	}
      }
    }
  }
}  /* assign_parm */

/*************
 *
 *   assign_floatparm()
 *
 *************/

/* DOCUMENTATION
This routine assigns a value to a floatparm.  The floatparm is identified by
its integer ID (which is available from str_to_floatparm_id()).
If the ID is not valid, or if the value is out of range, a
fatal error occurs.
*/

/* PUBLIC */
void assign_floatparm(int id, double val, BOOL echo)
{
  if (id < 0 || id >= Next_floatparm) {
    fprintf(stdout, "assign_floatparm: id %d, is out of range.\n", id);
    fprintf(stderr, "assign_floatparm: id %d, is out of range.\n", id);
    fatal_error("assign_floatparm");
  }
  else if (val < Floatparms[id].min || val > Floatparms[id].max) {
    fprintf(stdout, "assign_floatparm: parm %s, value %.3f out of range [%.3f..%.3f].\n",
	    Floatparms[id].name, val, Floatparms[id].min, Floatparms[id].max);
    fprintf(stderr, "assign_floatparm: parm %s, value %.3f out of range [%.3f..%.3f].\n",
	    Floatparms[id].name, val, Floatparms[id].min, Floatparms[id].max);
    fatal_error("assign_floatparm");
  }
  else {
    Floatparms[id].val = val;
    Option_updates++;
    /* Currently, nothing depends on floatparms. */
  }
}  /* assign_floatparm */

/*************
 *
 *   assign_stringparm()
 *
 *************/

/* DOCUMENTATION
This routine assigns a value to a stringparm.  The stringparm is identified by
its integer ID (which is available from str_to_stringparm_id()).
If the ID is not valid, or if the value is out of range, a
fatal error occurs.
*/

/* PUBLIC */
void assign_stringparm(int id, char *val, BOOL echo)
{
  if (id < 0 || id >= Next_stringparm) {
    fprintf(stdout, "assign_stringparm: id %d, is out of range.\n", id);
    fprintf(stderr, "assign_stringparm: id %d, is out of range.\n", id);
    fatal_error("assign_stringparm");
  }
  else {
    struct stringparm *sp = &(Stringparms[id]);
    int i = 0;;

    while (i < sp->n && !str_ident(sp->range[i], val))
      i++;

    if (i < sp->n) {
      sp->val = sp->range[i];
      Option_updates++;
    }
    else {
      printf("range of values for stringparm %s:\n", sp->name);
      for (i = 0; i < sp->n; i++)
	printf("    %s\n", sp->range[i]);
      fatal_error("assign_stringparm, value out of range");
    }
  }
}  /* assign_stringparm */

/*************
 *
 *   str_to_flag_id()
 *
 *************/

/* DOCUMENTATION
This routine converts the string name of a flag to its integer ID.
If the string name is not valid, -1 is returned.
*/

/* PUBLIC */
int str_to_flag_id(char *name)
{
  int i;
  for (i = 0; i < Next_flag; i++)
    if (str_ident(name, Flags[i].name))
      return i;
  return -1;
}  /* str_to_flag_id */

/*************
 *
 *   str_to_parm_id()
 *
 *************/

/* DOCUMENTATION
This routine converts the string name of a parm to its integer ID.
If the string name is not valid, -1 is returned.
*/

/* PUBLIC */
int str_to_parm_id(char *name)
{
  int i;
  for (i = 0; i < Next_parm; i++)
    if (str_ident(name, Parms[i].name))
      return i;
  return -1;
}  /* str_to_parm_id */

/*************
 *
 *   str_to_floatparm_id()
 *
 *************/

/* DOCUMENTATION
This routine converts the string name of a parm to its integer ID.
If the string name is not valid, -1 is returned.
*/

/* PUBLIC */
int str_to_floatparm_id(char *name)
{
  int i;
  for (i = 0; i < Next_floatparm; i++)
    if (str_ident(name, Floatparms[i].name))
      return i;
  return -1;
}  /* str_to_floatparm_id */

/*************
 *
 *   str_to_stringparm_id()
 *
 *************/

/* DOCUMENTATION
This routine converts the string name of a stringparm to its integer ID.
If the string name is not valid, -1 is returned.
*/

/* PUBLIC */
int str_to_stringparm_id(char *name)
{
  int i;
  for (i = 0; i < Next_stringparm; i++)
    if (str_ident(name, Stringparms[i].name))
      return i;
  return -1;
}  /* str_to_stringparm_id */

/*************
 *
 *   flag_id_to_str()
 *
 *************/

/* DOCUMENTATION
Given a flag ID, return the corresponding name of the flag.
*/

/* PUBLIC */
char *flag_id_to_str(int id)
{
  if (id < 0 || id >= Next_flag)
    fatal_error("flag_id_to_str, bad id");
  return Flags[id].name;
}  /* flag_id_to_str */

/*************
 *
 *   parm_id_to_str()
 *
 *************/

/* DOCUMENTATION
Given a parm ID, return the corresponding name of the parm.
*/

/* PUBLIC */
char *parm_id_to_str(int id)
{
  if (id < 0 || id >= Next_parm)
    fatal_error("parm_id_to_str, bad id");
  return Parms[id].name;
}  /* parm_id_to_str */

/*************
 *
 *   floatparm_id_to_str()
 *
 *************/

/* DOCUMENTATION
Given a floatparm ID, return the corresponding name of the floatparm.
*/

/* PUBLIC */
char *floatparm_id_to_str(int id)
{
  if (id < 0 || id >= Next_floatparm)
    fatal_error("floatparm_id_to_str, bad id");
  return Floatparms[id].name;
}  /* floatparm_id_to_str */

/*************
 *
 *   stringparm_id_to_str()
 *
 *************/

/* DOCUMENTATION
Given a stringparm ID, return the corresponding name of the stringparm.
*/

/* PUBLIC */
char *stringparm_id_to_str(int id)
{
  if (id < 0 || id >= Next_stringparm)
    fatal_error("stringparm_id_to_str, bad id");
  return Stringparms[id].name;
}  /* stringparm_id_to_str */

/*************
 *
 *   append_dep()
 *
 *************/

static
Optdep append_dep(Optdep d1, Optdep d2)
{
  if (d1 == NULL)
    return d2;
  else {
    d1->next = append_dep(d1->next, d2);
    return d1;
  }
}  /* append_dep */

/*************
 *
 *   flag_flag_dependency()
 *
 *************/

/* DOCUMENTATION
This routine declares that a flag depends on another flag.
If flag "id" gets value "val", then flag "dep_id" is automatically
given value "dep_val".
*/

/* PUBLIC */
void flag_flag_dependency(int id, BOOL val, int dep_id, BOOL dep_val)
{
  Optdep dep = get_optdep();
  dep->type = FLAGT;
  dep->id = dep_id;
  dep->val = dep_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_flag_dependency */

/*************
 *
 *   flag_flag_dep_default()
 *
 *************/

/* DOCUMENTATION
This routine declares that a flag depends on another flag.
If flag "id" gets value "val", then flag "dep_id" is automatically
given its default value.
*/

/* PUBLIC */
void flag_flag_dep_default(int id, BOOL val, int dep_id)
{
  Optdep dep = get_optdep();
  dep->type = FLAGT;
  dep->id = dep_id;
  dep->val = Flags[dep_id].default_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_flag_dep_default */

/*************
 *
 *   flag_parm_dependency()
 *
 *************/

/* DOCUMENTATION
This routine declares that a parm depends on a flag.
If flag "id" gets value "val", then parm "dep_id" is automatically
given value "dep_val".
*/

/* PUBLIC */
void flag_parm_dependency(int id, BOOL val, int dep_id, int dep_val)
{
  Optdep dep = get_optdep();
  dep->type = PARMT;
  dep->id = dep_id;
  dep->val = dep_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_parm_dependency */

/*************
 *
 *   flag_floatparm_dependency()
 *
 *************/

/* DOCUMENTATION
This routine declares that a floatparm depends on a flag.
If flag "id" gets value "val", then parm "dep_id" is automatically
given value "dep_val".
*/

/* PUBLIC */
void flag_floatparm_dependency(int id, BOOL val, int dep_id, double dep_val)
{
  Optdep dep = get_optdep();
  dep->type = FLOATPARMT;
  dep->id = dep_id;
  dep->dval = dep_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_floatparm_dependency */

/*************
 *
 *   flag_parm_dep_default()
 *
 *************/

/* DOCUMENTATION
This routine declares that a parm depends on a flag.
If flag "id" gets value "val", then parm "dep_id" is automatically
given its default value.
*/

/* PUBLIC */
void flag_parm_dep_default(int id, BOOL val, int dep_id)
{
  Optdep dep = get_optdep();
  dep->type = PARMT;
  dep->id = dep_id;
  dep->val = Parms[dep_id].default_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_parm_dep_default */

/*************
 *
 *   flag_floatparm_dep_default()
 *
 *************/

/* DOCUMENTATION
This routine declares that a parm depends on a flag.
If flag "id" gets value "val", then parm "dep_id" is automatically
given its default value.
*/

/* PUBLIC */
void flag_floatparm_dep_default(int id, BOOL val, int dep_id)
{
  Optdep dep = get_optdep();
  dep->type = FLOATPARMT;
  dep->id = dep_id;
  dep->dval = Floatparms[dep_id].default_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_floatparm_dep_default */

/*************
 *
 *   parm_flag_dependency()
 *
 *************/

/* DOCUMENTATION
This routine declares that a flag depends on a parm.
If parm "id" gets changed, then flag "dep_id" is automatically
given value "dep_val".
*/

/* PUBLIC */
void parm_flag_dependency(int id, int dep_id, int dep_val)
{
  Optdep dep = get_optdep();
  dep->type = FLAGT;
  dep->id = dep_id;
  dep->val = dep_val;
  Parms[id].dependencies = append_dep(Parms[id].dependencies, dep);
}  /* parm_flag_dependency */

/*************
 *
 *   parm_parm_dependency()
 *
 *************/

/* DOCUMENTATION
This routine declares that a parm depends on a parm.
If (multiply == TRUE), then dep_val is a multiplier instead of a value.
*/

/* PUBLIC */
void parm_parm_dependency(int id, int dep_id, int dep_val, BOOL multiply)
{
  Optdep dep = get_optdep();
  dep->type = PARMT;
  dep->id = dep_id;
  dep->val = dep_val;
  dep->multiply = multiply;
  Parms[id].dependencies = append_dep(Parms[id].dependencies, dep);
}  /* parm_parm_dependency */

/*************
 *
 *   flag_stringparm_dependency()
 *
 *************/

/* DOCUMENTATION
This routine declares that a stringparm depends on a flag.
If flag "id" gets value "val", then stringparm "dep_id" is automatically
given value "dep_val".
*/

/* PUBLIC */
void flag_stringparm_dependency(int id, BOOL val, int dep_id, char *dep_val)
{
  Optdep dep = get_optdep();
  dep->type = STRINGPARMT;
  dep->id = dep_id;
  dep->sval = dep_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_stringparm_dependency */

/*************
 *
 *   flag_stringparm_dep_default()
 *
 *************/

/* DOCUMENTATION
This routine declares that a stringparm depends on a flag.
If flag "id" gets value "val", then stringparm "dep_id" is automatically
given its default value.
*/

/* PUBLIC */
void flag_stringparm_dep_default(int id, BOOL val, int dep_id)
{
  Optdep dep = get_optdep();
  dep->type = STRINGPARMT;
  dep->id = dep_id;
  dep->sval = Stringparms[dep_id].default_val;
  dep->flag_trigger = val;
  Flags[id].dependencies = append_dep(Flags[id].dependencies, dep);
}  /* flag_stringparm_dep_default */

/*************
 *
 *   option_updates()
 *
 *************/

/* DOCUMENTATION
How many times have Flags, Parms, or Stringparms been updated?
*/

/* PUBLIC */
int option_updates(void)
{
  return Option_updates;
}  /* option_updates */

/*************
 *
 *   flag_default()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int flag_default(int flag_id)
{
  return Flags[flag_id].default_val;
}  /* flag_default */

/*************
 *
 *   parm_default()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int parm_default(int parm_id)
{
  return Parms[parm_id].default_val;
}  /* parm_default */

/*************
 *
 *   floatparm_default()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int floatparm_default(int floatparm_id)
{
  return Floatparms[floatparm_id].default_val;
}  /* floatparm_default */

/*************
 *
 *   stringparm1_default()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
char *stringparm1_default(int id)
{
  return Stringparms[id].default_val;
}  /* stringparm1_default */

