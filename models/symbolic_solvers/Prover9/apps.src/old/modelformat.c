/* Format an interpretation in various ways
 */

#include "../ladr/top_input.h"
#include "../ladr/interp.h"

#define PROGRAM_NAME    "modelformat"
#include "../VERSION_DATE.h"

static char Help_string[] =
"\nThis program reads a stream containing interpretations in portable format\n"
"(from stdin) and takes a command-line argument saying how to print them:\n\n"
"    portable      : one line per operation\n"
"    portable2     : portable, with binary operations in a square\n"
"    tabular       : as nice tables\n"
"    raw           : similar to portable, but without punctuation\n"
"    cooked        : as terms, e.g., f(0,1)=2\n"
"    tex           : formatted for LaTeX\n"
"    xml           : XML\n\n";

enum {PORTABLE, PORTABLE2, TABULAR, RAW, COOKED, TEX, XML};

/*************
 *
 *   circ()
 *
 *************/

static
BOOL circ(char *b, int n, int j, char *str)
{
  int i;
  for (i = 0; i < n; i++, j++) {
    if (b[j % n] != str[i])
      return FALSE;
  }
  return TRUE;
}  /* circ */

/*************
 *
 *   read_to_string()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int read_to_string(FILE *fp, char *str)
{
  int n = strlen(str);
  char *b = calloc(n, 1);
  BOOL found = FALSE;
  int i = 0;
  int c;

  c = getc(fp);
  while (c != EOF && !found) {
    b[i % n] = c;
    i++;
    found = circ(b, n, i, str);
    if (!found)
      c = getc(fp);
  }
  free(b);
  return c;
}  /* read_to_string */

int main(int argc, char **argv)
{
  int type, rc;

  if (string_member("help", argv, argc) ||
      string_member("-help", argv, argc)) {
    printf("\n%s, version %s, %s\n",PROGRAM_NAME,PROGRAM_VERSION,PROGRAM_DATE);
    printf("%s", Help_string);
    exit(1);
  }
  else if (string_member("portable", argv, argc))
    type = PORTABLE;
  else if (string_member("portable2", argv, argc))
    type = PORTABLE2;
  else if (string_member("tabular", argv, argc))
    type = TABULAR;
  else if (string_member("raw", argv, argc))
    type = RAW;
  else if (string_member("cooked", argv, argc))
    type = COOKED;
  else if (string_member("tex", argv, argc))
    type = TEX;
  else if (string_member("xml", argv, argc))
    type = XML;
  else {
    printf("%s", Help_string);
    exit(1);
  }

  init_standard_ladr();

  if (type == XML) {
    printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    printf("\n<!DOCTYPE interps SYSTEM \"interp.dtd\">\n");
    printf("\n<?xml-stylesheet type=\"text/xsl\" href=\"interp.xsl\"?>\n");
    printf("\n<interps>\n");
  }

  rc = read_to_string(stdin, "interpretation(");

  while (rc != EOF) {

    /* ok, we have an interpretation */
    Term t, interp;
    Interp a;
    int n;

    rc = scanf("%d", &n);             /* get domain size */
    rc = read_to_string(stdin, ",");  /* get past comma */
    
    ungetc('(', stdin);  /* so that we can read a complete term */
    t = read_term(stdin, stderr);
    interp = build_binary_term(str_to_sn("interpretation", 2),
			       nat_to_term(n),
			       t);

    a = compile_interp(interp, TRUE);

    if (type == PORTABLE)
      fprint_interp(stdout, a);
    else if (type == PORTABLE2)
      fprint_interp_2(stdout, a);
    else if (type == TABULAR)
      fprint_interp_tabular(stdout, a);
    else if (type == COOKED)
      fprint_interp_cooked(stdout, a);
    else if (type == RAW)
      fprint_interp_raw(stdout, a);
    else if (type == TEX)
      fprint_interp_tex(stdout, a);
    else if (type == XML)
      fprint_interp_xml(stdout, a);

    zap_interp(a);
    zap_term(interp);

    rc = read_to_string(stdin, "interpretation(");
  }

  if (type == XML) {
    printf("\n</interps>\n");
  }

  exit(0);
}  /* main */

