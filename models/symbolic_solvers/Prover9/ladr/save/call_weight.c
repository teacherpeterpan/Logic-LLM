/*************
 *
 *   call_weight()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int call_weight(char *program, Term t)
{
  FILE *fp;
  int result;
  char *prog_quote_free = process_quoted_symbol(program);
  String_buf sb = get_string_buf();
  sprint_term(sb, t);
  
  /* printf("calling %s with ", prog_quote_free);  p_sb(sb); */

  fflush(stdout);
  fp = popen(prog_quote_free, "w");
  fprint_sb(fp, sb);
  fprintf(fp, "\n");
  fflush(fp);

  /* fscanf(fp, "%d",  &result); */
  result = pclose(fp);

  printf("returning %d: ", result); p_term(t);

  zap_string_buf(sb);

  return result;
}  /* call_weight */

