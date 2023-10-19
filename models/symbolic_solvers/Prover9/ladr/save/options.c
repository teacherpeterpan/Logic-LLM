#include "options.h"
#include <stdarg.h>  /* for variable argument lists */

/* Private definitions and types */

/*
  Dependencies.  For now, only flags can have dependencies.
*/

typedef enum { FLAGT, PARMT, STRINGPARMT } Opttype;  /* type of dependent */
typedef struct optdep * Optdep;

struct optdep {  /* these get attached to flags that have dependents */
  BOOL flag_trigger;   /* whether set or clear triggers action */
  Opttype type;        /* type of dependent option */
  int id;              /* id of dependent option */
  int val;             /* value for dependent flag or parm */
  char *sval;          /* value for dependent stringparm */
  Optdep next;         /* next dependent */
};

struct flag {  /* Flags are boolean valued options */
  char *name;
  BOOL val;
  Optdep dependencies;
};

struct parm {  /* Parms are integer valued options */
  char *name;
  int val;
  int min, max;  /* minimum and maximum permissible values */
  // Optdep dependencies;
};

struct stringparm {  /* Stringparms are string valued options */
  char *name;
  char *val;      /* current valuse: index into array of strings */
  int n;          /* number of values */
  char **range;   /* array of possible vales */
  // Optdep dependencies;
};

static struct flag Flags[MAX_FLAGS];
static struct parm Parms[MAX_PARMS];
static struct stringparm Stringparms[MAX_STRINGPARMS];

/*
 * memory management
 */

static unsigned Optdep_gets, Optdep_frees;

#define BYTES_OPTDEP sizeof(struct optdep)
#define PTRS_OPTDEP BYTES_OPTDEP%BPP == 0 ? BYTES_OPTDEP/BPP : BYTES_OPTDEP/BPP + 1

/*************
 *
 *   Optdep get_optdep()
 *
 *************/

static
Optdep get_optdep(void)
{
  Optdep p = get_mem(PTRS_OPTDEP);
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

  n = BYTES_OPTDEP;
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
 *    next_available_flag_id()
 *
 *************/

/* DOCUMENTATION
If you're not sure what flag IDs have already been claimed,
for example if you're adding options to an existing program,
you can use this function to find an available one.
Return -1 if none is available.
*/

/* PUBLIC */
int next_available_flag_id(void)
{
  int i = 0;
  while (i < MAX_FLAGS && Flags[i].name != NULL)
    i++;
  return (i < MAX_FLAGS ? i : -1);
}  /* next_available_flag_id */

/*************
 *
 *    next_available_parm_id()
 *
 *************/

/* DOCUMENTATION
If you're not sure what parm IDs have already been claimed,
for example if you're adding options to an existing program,
you can use this function to find an available one.
Return -1 if none is available.
*/

/* PUBLIC */
int next_available_parm_id(void)
{
  int i = 0;
  while (i < MAX_PARMS && Parms[i].name != NULL)
    i++;
  return (i < MAX_PARMS ? i : -1);
}  /* next_available_parm_id */

/*************
 *
 *    next_available_stringparm_id()
 *
 *************/

/* DOCUMENTATION
If you're not sure what stringparm IDs have already been claimed,
for example if you're adding options to an existing program,
you can use this function to find an available one.
Return -1 if none is available.
*/

/* PUBLIC */
int next_available_stringparm_id(void)
{
  int i = 0;
  while (i < MAX_STRINGPARMS && Stringparms[i].name == NULL)
    i++;
  return (i < MAX_STRINGPARMS ? i : -1);
}  /* next_available_stringparm_id */

/*************
 *
 *    init_flag()
 *
 *************/

/* DOCUMENTATION
Initialize a flag (boolean-valued option).   You give it an
integer identifier (unique among all flags), a name, and
a default value.  Flags are typically changed by user commands
which are parsed by read_commands().  The value of a flag is
checked with flag(ID).
*/

/* PUBLIC */
void init_flag(int flag_id,
	       char *flag_name,
	       BOOL default_value)
{
  if (flag_id < 0 || flag_id >= MAX_FLAGS)
    fatal_error("init_flag: flag ID out of range");
  else if (Flags[flag_id].name != NULL)
    fatal_error("init_flag: flag ID already in use");
  else if (str_to_flag_id(flag_name) != -1)
    fatal_error("init_flag: flag name already in use");
  else {
    Flags[flag_id].name = flag_name;
    Flags[flag_id].val = default_value;
    Flags[flag_id].dependencies = NULL;
  }
}  /* init_flag */

/*************
 *
 *    init_parm()
 *
 *************/

/* DOCUMENTATION
Initialize a parm (integer-valued option).   You give it an
integer identifier (unique among all parms), a name,
a default value, and min and max values.  Parms are typically
changed by user commands which are parsed by read_commands().
The value of a parm is checked with parm(ID).
*/

/* PUBLIC */
void init_parm(int parm_id,
	       char *parm_name,
	       int default_value,
	       int min_value,
	       int max_value)
{
  if (parm_id < 0 || parm_id >= MAX_PARMS)
    fatal_error("init_parm: parm ID out of range");
  else if (Parms[parm_id].name != NULL)
    fatal_error("init_parm: parm ID already in use");
  else if (str_to_parm_id(parm_name) != -1)
    fatal_error("init_parm: parm name already in use");
  else if (default_value < min_value || default_value > max_value)
    fatal_error("init_parm: default parm value out of range");
  else {
    Parms[parm_id].name = parm_name;
    Parms[parm_id].val = default_value;
    Parms[parm_id].min = min_value;
    Parms[parm_id].max = max_value;
  }
}  /* init_parm */

/*************
 *
 *    init_stringparm()
 *
 *************/

/* DOCUMENTATION
Initialize a stringparm (string-valued option).   You give it an
integer identifier (unique among all stringparms), a name,
a number n of possible values, and n strings which are the possible
values.  The first string given is the default value.
<P>
Stringparms are typically changed by user commands which are parsed
by read_commands().
The value of a stringparm is checked with the Boolean routine
stringparm(ID, string).
*/

/* PUBLIC */
void init_stringparm(int id,
		     char *name,
		     int n,
		     ...)
{
  if (id < 0 || id >= MAX_STRINGPARMS)
    fatal_error("init_stringparm: ID out of range");
  else if (Stringparms[id].name != NULL)
    fatal_error("init_stringparm: ID already in use");
  else if (str_to_stringparm_id(name) != -1)
    fatal_error("init_stringparm: name already in use");
  else {
    int i;
    va_list parameters;
    va_start(parameters, n);
    Stringparms[id].range = malloc(n * sizeof(char *));
    for (i = 0; i < n; i++)
      Stringparms[id].range[i] = va_arg(parameters, char *);
    va_end(parameters);

    Stringparms[id].name = name;
    Stringparms[id].n = n;       /* number of values */
    Stringparms[id].val = Stringparms[id].range[0];  /* first is default */
  }
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
  for (i = 0; i < MAX_FLAGS; i++)  /* print flags */
    if (Flags[i].name[0] != '\0') {
      fprintf(fp, "%s", Flags[i].val ? "set(" : "clear(");
      fprintf(fp, "%s). ", Flags[i].name);
      fprintf(fp, "%s", (++j % 3 == 0) ? "\n" : "");
    }
  fprintf(fp, "\n\n");

  j = 0;
  for (i = 0; i < MAX_PARMS; i++)  /* print parms */
    if (Parms[i].name[0] != '\0') {
      fprintf(fp, "assign(");
      fprintf(fp, "%s, %d). ", Parms[i].name, Parms[i].val);
      fprintf(fp, "%s", (++j % 3 == 0) ? "\n" : "");
    }
  fprintf(fp, "\n\n");

  j = 0;
  for (i = 0; i < MAX_STRINGPARMS; i++) {  /* print stringparms */
    struct stringparm *sp = &(Stringparms[i]);
    if (sp->name[0] != '\0') {
      fprintf(fp, "assign(");
      fprintf(fp, "%s, %s). ", sp->name, sp->val);
      fprintf(fp, "%s", (++j % 3 == 0) ? "\n" : "");
    }
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
void update_flag(FILE *fout, int id, BOOL val)
{
  if (id < 0 || id >= MAX_FLAGS) {
    fprintf(fout, "update_flag: flag id %d, is out of range.\n", id);
    fprintf(stderr, "update_flag: flag id %d, is out of range.\n", id);
    fatal_error("update_flag, flag out of range");
  }
  else {
    Optdep p;
    Flags[id].val = val;
    for (p = Flags[id].dependencies; p; p = p->next) {
      if (p->flag_trigger == val) {
	if (p->type == FLAGT) {
	  fprintf(fout, "    %% %s(%s) -> %s(%s).\n",
		 val ? "set" : "clear",
		 Flags[id].name,
		 p->val ? "set" : "clear",
		 Flags[p->id].name);
	  update_flag(fout, p->id, p->val);
	}
	else if (p->type == PARMT) {
	  fprintf(fout, "    %% %s(%s) -> assign(%s, %d).\n",
		 val ? "set" : "clear",
		 Flags[id].name,
		 Parms[p->id].name,
		 p->val);
	  assign_parm(p->id, p->val);
	}
	else {
	  /* assume it's a stringparm */
	  fprintf(fout, "    %% %s(%s) -> assign(%s, %s).\n",
		 val ? "set" : "clear",
		 Flags[id].name,
		 Stringparms[p->id].name,
		 p->sval);
	  assign_stringparm(p->id, p->sval);
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
void set_flag(int id)
{
  update_flag(stdout, id, TRUE);
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
void clear_flag(int id)
{
  update_flag(stdout, id, FALSE);
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
void assign_parm(int id, int val)
{
  if (id < 0 || id >= MAX_PARMS) {
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
  else
    Parms[id].val = val;
}  /* assign_parm */

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
void assign_stringparm(int id, char *val)
{
  if (id < 0 || id >= MAX_STRINGPARMS) {
    fprintf(stdout, "assign_stringparm: id %d, is out of range.\n", id);
    fprintf(stderr, "assign_stringparm: id %d, is out of range.\n", id);
    fatal_error("assign_stringparm");
  }
  else {
    struct stringparm *sp = &(Stringparms[id]);
    int i = 0;;

    while (i < sp->n && !str_ident(sp->range[i], val))
      i++;

    if (i < sp->n)
      sp->val = sp->range[i];
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
  for (i = 0; i < MAX_FLAGS; i++)
    if (Flags[i].name && str_ident(name, Flags[i].name))
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
  for (i = 0; i < MAX_PARMS; i++)
    if (Parms[i].name && str_ident(name, Parms[i].name))
      return i;
  return -1;
}  /* str_to_parm_id */

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
  for (i = 0; i < MAX_STRINGPARMS; i++)
    if (Stringparms[i].name && str_ident(name, Stringparms[i].name))
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
  if (id < 0 || id > MAX_FLAGS)
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
  if (id < 0 || id > MAX_PARMS)
    fatal_error("parm_id_to_str, bad id");
  return Parms[id].name;
}  /* parm_id_to_str */

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
  if (id < 0 || id > MAX_STRINGPARMS)
    fatal_error("stringparm_id_to_str, bad id");
  return Stringparms[id].name;
}  /* stringparm_id_to_str */

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
  dep->next = Flags[id].dependencies;
  Flags[id].dependencies = dep;
}  /* flag_flag_dependency */

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
  dep->next = Flags[id].dependencies;
  Flags[id].dependencies = dep;
}  /* flag_parm_dependency */

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
  dep->next = Flags[id].dependencies;
  Flags[id].dependencies = dep;
}  /* flag_stringparm_dependency */

