#define PROVER_NAME     "Prover"
#define PROVER_VERSION  "9"
#include "../PROGRAM_DATE.h"

#include "../ladr/nonport.h"
#include "../ladr/top_input.h"

#include "utilities.h"
#include "search.h"

/*************
 *
 *    void print_banner(argc, argv)
 *
 *************/

static
void print_banner(int argc, char **argv)
{
  int i;
  printf("----- %s %s, %s -----\n", PROVER_NAME, PROVER_VERSION, PROGRAM_DATE);
  printf("Process %d was started by %s on %s,\n%s",
	 my_process_id(), username(), hostname(), get_date());
	 
  printf("The command was \"");
  for(i = 0; i < argc; i++)
    printf("%s%s", argv[i], (i < argc-1 ? " " : ""));
  printf("\".\n");
}  // print_banner

/*************
 *
 *    main -- basic prover
 *
 *************/

int main(int argc, char **argv)
{
  Prover_input input;
  Prover_results results;

  print_banner(argc, argv);

  /***************** Initialize and read the input ***************************/

  input = std_prover_init_and_input(argc, argv,
			    TRUE,           // echo input to stdout
			    KILL_UNKNOWN);  // unknown flags/parms are fatal

  /************************ Search for a proof *******************************/

  std_clausify(input, TRUE);

  results = search(input);
  // results = forking_search(input);  // results should be the same here

  exit_with_message(stdout, results->return_code);
  
  exit(1);  // to satisfy the compiler (won't be called)

}  // main
