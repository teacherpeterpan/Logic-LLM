#ifndef TP_DEFINITIONS_H
#define TP_DEFINITIONS_H

#include "../ladr/ioutil.h"
#include "../ladr/resolve.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from definitions.c */

void expand_definitions(Clist clauses, Clist defs, Clist disabled);

BOOL check_definition(Formula f);

void check_definitions(Plist formulas);

#endif  /* conditional compilation of whole file */
