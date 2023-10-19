#include "attributes.h"

/* Private definitions and types */

/*************
 *
 *   init_prover_attributes()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void init_prover_attributes(void)
{
  register_attribute(LABEL_ATT,            "label",         STRING_ATTRIBUTE);
  register_attribute(BSUB_HINT_ADD_WT_ATT, "bsub_hint_add_wt", INT_ATTRIBUTE);
  register_attribute(BSUB_HINT_WT_ATT,     "bsub_hint_wt",     INT_ATTRIBUTE);
  register_attribute(ANSWER_ATT,           "answer",          TERM_ATTRIBUTE);

  register_attribute(ACTION_ATT,           "action",          TERM_ATTRIBUTE);
  register_attribute(ACTION2_ATT,          "action2",         TERM_ATTRIBUTE);

  declare_term_attribute_inheritable(ANSWER_ATT);
  declare_term_attribute_inheritable(ACTION2_ATT);
}  // Init_prover_attributes

