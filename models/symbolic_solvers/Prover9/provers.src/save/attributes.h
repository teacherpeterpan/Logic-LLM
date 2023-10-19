#ifndef TP_ATTRIBUTES_H
#define TP_ATTRIBUTES_H

#include "../ladr/attrib.h"

/* INTRODUCTION
*/

/* Public definitions */

/* Attribute Identifiers */

enum {
  LABEL_ATT,
  BSUB_HINT_ADD_WT_ATT,
  BSUB_HINT_WT_ATT,
  ANSWER_ATT,
  ACTION_ATT,
  ACTION2_ATT
};

/* End of public definitions */

/* Public function prototypes from attributes.c */

void init_prover_attributes(void);

#endif  /* conditional compilation of whole file */
