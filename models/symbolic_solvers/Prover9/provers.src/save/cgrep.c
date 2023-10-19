#include "../ladr/nonport.h"
#include "../ladr/top_input.h"
#include "../ladr/subsume.h"

#include "utilities.h"
#include "search.h"

/*************
 *
 *   cgrep_plist()
 *
 *************/

static
void cgrep_plist(Topform subject, Plist clauses)
{
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = p->v;
    if (subsumes(c, subject) && subsumes(c, subject))
      fwrite_clause(stderr, c, CL_FORM_BARE);
  }
}  /* cgrep_plist */

/*************
 *
 *    main
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;

  Plist wild_clauses, wild_formulas, wild_terms;
  char *s = argv[1];
  Topform c;

  // following says to accept any nonstandard lists there might be

  accept_list("",   CLAUSES,  &wild_clauses);
  accept_list("",  FORMULAS, &wild_formulas);
  accept_list("",     TERMS,    &wild_terms);

  /***************** Initialize and read the input ***************************/

  input = std_prover_init_and_input(argc, argv,
			    FALSE,             // don't echo input to stdout
			    IGNORE_UNKNOWN);   // ignore unknown flags/parms

  c = parse_clause_from_string(s);

  // printf("string: %s\n", s);
  // printf("clause: "); f_clause(c);

  cgrep_plist(c, input->usable);
  cgrep_plist(c, input->sos);
  cgrep_plist(c, input->demods);
  cgrep_plist(c, input->goals);
  cgrep_plist(c, input->hints);
  cgrep_plist(c, wild_clauses);

  exit(0);

}  // main
