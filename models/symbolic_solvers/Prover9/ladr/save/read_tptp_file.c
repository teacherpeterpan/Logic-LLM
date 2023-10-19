/*************
 *
 *   read_tptp_file()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void read_tptp_file(FILE *fin, FILE *fout, BOOL echo, int unknown_action)
{
  Term t;
  Readlist assumps, goals;

  assumps = readlist_member(Input_lists, "assumptions", FORMULAS);
  if (assumps == NULL)
    assumps = readlist_member_wild(Input_lists, FORMULAS);
  if (assumps == NULL)
    fatal_error("read_tptp_file: assumptions/wild list not found");

  goals = readlist_member(Input_lists, "goals", FORMULAS);
  if (goals == NULL)
    fatal_error("read_tptp_file: goals list not found");

  t = read_term(fin, fout);
  while (t != NULL) {
    if (echo)
      fwrite_term_nl(fout, t);

    if (is_term(t, "include", 1) || is_term(t, "include", 2)) {
      char *fname = new_str_copy(sn_to_str(SYMNUM(ARG(t,0))));
      char *fname2 = fname;
      if (*fname2 == '\'' || *fname2 == '"') {
	fname2[strlen(fname2)-1] = '\0';
	fname2++;
      }
      FILE *fin2 = fopen(fname2, "r");
      if (fin2 == NULL) {
	char s[100];
	sprintf(s, "read_tptp_file, file %s not found", fname2);
	fatal_error(s);
      }
      if (echo)
	printf("\n%% Including file %s\n\n", fname2);
      read_tptp_file(fin2, fout, echo, unknown_action);
      if (echo)
	printf("\n%% Finished including file %s\n\n", fname2);
      free(fname);
    }
    else if (is_term(t, "cnf", 3) || is_term(t, "cnf", 4) ||
	     is_term(t, "fof", 3) || is_term(t, "fof", 4)) {
      Formula form = tptp_formula(t);
      if (is_term(ARG(t,1), "conjecture", 0))
	*(goals->p) = plist_append(*(goals->p), form);
      else
	*(assumps->p) = plist_append(*(assumps->p), form);
    }
    else {
      p_term(t);
      fatal_error("read_tptp_file: unknown term");
    }
    zap_term(t);
    t = read_term(fin, fout);
  }
}  /* read_tptp_file */

