#ifndef TP_COMMANDS_H
#define TP_COMMANDS_H

#include "options.h"
#include "ioutil.h"

/* INTRODUCTION
This package has some high-level routines for reading
a sequence of commands from an input stream.  The commands
do things like set options and give properties to symbols.
<P>
This is intended to be used to read the prologue to the input.
*/

/* Public definitions */

/* What shall we do if we read an unknown flag or parameter? */

enum {
  IGNORE_UNKNOWN,
  NOTE_UNKNOWN,
  WARN_UNKNOWN,
  KILL_UNKNOWN
};

/* End of public definitions */

/* Public function prototypes from commands.c */

void flag_handler(FILE *fout, Term t, int unknown_action);

void parm_handler(FILE *fout, Term t, int unknown_action);

void preliminary_precedence(Plist p);

Term read_commands_from_file(FILE *fin, FILE *fout, BOOL echo, int unknown_action);

void read_commands(int argc, char **argv, FILE *fout, BOOL echo, int unknown_action);

#endif  /* conditional compilation of whole file */
