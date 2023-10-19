#ifndef TP_AUTO_H
#define TP_AUTO_H

#include "search.h"

/* INTRODUCTION
*/

/* Public definitions */

/* End of public definitions */

/* Public function prototypes from auto.c */

void process_auto_options(Plist sos, Plist usable,
			  Plist goals, Plist definitions,
			  struct prover_options opt);

#endif  /* conditional compilation of whole file */
