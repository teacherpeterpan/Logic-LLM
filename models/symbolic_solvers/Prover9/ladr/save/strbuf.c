#include "strbuf.h"

/*
 * private definitions and types
 */

struct string_buf {
  int size;
  struct chunk *first;
  struct string_buf *link;
};

#define CHUNK 500

struct chunk {
  char s[CHUNK];
  struct chunk *next;
};

typedef struct chunk *Chunk;

/*
 * memory management
 */

static unsigned Chunk_gets, Chunk_frees;
static unsigned String_buf_gets, String_buf_frees;

#define BYTES_CHUNK sizeof(struct chunk)
#define PTRS_CHUNK BYTES_CHUNK%BPP == 0 ? BYTES_CHUNK/BPP : BYTES_CHUNK/BPP + 1

#define BYTES_STRING_BUF sizeof(struct string_buf)
#define PTRS_STRING_BUF BYTES_STRING_BUF%BPP == 0 ? BYTES_STRING_BUF/BPP : BYTES_STRING_BUF/BPP + 1

/*************
 *
 *   Chunk get_chunk()
 *
 *************/

static
Chunk get_chunk(void)
{
  Chunk p = get_mem(PTRS_CHUNK);
  Chunk_gets++;
  return(p);
}  /* get_chunk */

/*************
 *
 *    free_chunk()
 *
 *************/

static
void free_chunk(Chunk p)
{
  free_mem(p, PTRS_CHUNK);
  Chunk_frees++;
}  /* free_chunk */

/*************
 *
 *   String_buf get_string_buf()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
String_buf get_string_buf(void)
{
  String_buf p = get_mem(PTRS_STRING_BUF);
  String_buf_gets++;
  return(p);
}  /* get_string_buf */

/*************
 *
 *    free_string_buf()
 *
 *************/

static
void free_string_buf(String_buf p)
{
  free_mem(p, PTRS_STRING_BUF);
  String_buf_frees++;
}  /* free_string_buf */

/*************
 *
 *   fprint_strbuf_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to FILE *fp) memory usage statistics for data types
associated with the strbuf package.
The Boolean argument heading tells whether to print a heading on the table.
*/

/* PUBLIC */
void fprint_strbuf_mem(FILE *fp, BOOL heading)
{
  int n;
  if (heading)
    fprintf(fp, "  type (bytes each)        gets      frees     in use      bytes\n");

  n = BYTES_CHUNK;
  fprintf(fp, "chunk (%4d)        %11u%11u%11u%9.1f K\n",
          n, Chunk_gets, Chunk_frees,
          Chunk_gets - Chunk_frees,
          ((Chunk_gets - Chunk_frees) * n) / 1024.);

  n = BYTES_STRING_BUF;
  fprintf(fp, "string_buf (%4d)   %11u%11u%11u%9.1f K\n",
          n, String_buf_gets, String_buf_frees,
          String_buf_gets - String_buf_frees,
          ((String_buf_gets - String_buf_frees) * n) / 1024.);

}  /* fprint_strbuf_mem */

/*************
 *
 *   p_strbuf_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) memory usage statistics for data types
associated with the strbuf package.
*/

/* PUBLIC */
void p_strbuf_mem()
{
  fprint_strbuf_mem(stdout, 1);
}  /* p_strbuf_mem */

/*
 *  end of memory management
 */
/*************
 *
 *    String_buf init_string_buf()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns a String_buf, initialized
to string s.  Don't forget to call zap_string_buf(sb) when
finished with it.
Also see get_string_buf().
*/

/* PUBLIC */
String_buf init_string_buf(char *s)
{
  String_buf p = get_string_buf();
  sb_append(p, s);
  return(p);
}  /* init_string_buf */

/*************
 *
 *   fprint_sb()
 *
 *************/

/* DOCUMENTATION
This routine prints String_buf sb to FILE *fp.
*/

/* PUBLIC */
void fprint_sb(FILE *fp, String_buf sb)
{
  Chunk h = sb->first;
  int i = 0;

  while (i < sb->size) {
    fprintf(fp, "%c", h->s[i % CHUNK]);
    i++;
    if (i % CHUNK == 0)
      h = h->next;
  }
}  /* fprint_sb */

/*************
 *
 *   p_sb()
 *
 *************/

/* DOCUMENTATION
This routine prints String_buf sb, followed by '\n' and fflush, to stdout.
If you don't want the newline, use fprint_sb() instead.
*/

/* PUBLIC */
void p_sb(String_buf sb)
{
  fprint_sb(stdout, sb);
  printf("\n");
  fflush(stdout);
}  /* p_sb */

/*************
 *
 *   sb_append()
 *
 *************/

/* DOCUMENTATION
This routine appends string s to String_buf sb.
The NULL character that marks the end of s does not go into
the String_buf.
*/

/* PUBLIC */
void sb_append(String_buf sb, char *s)
{
  int i;
  int n = sb->size;
  Chunk last = sb->first;

  while (last != NULL && last->next != NULL)
    last = last->next;

  for (i = 0; s[i] != '\0'; i++) {
    if (n % CHUNK == 0) {
      Chunk new = get_chunk();
      if (last != NULL)
	last->next = new;
      else
	sb->first = new;
      last = new;
    }
    last->s[n % CHUNK] = s[i];
    n++;
  }
  sb->size = n;
}  /* sb_append */

/*************
 *
 *   sb_append_char()
 *
 *************/

/* DOCUMENTATION
This routine appends character c to String_buf sb.
*/

/* PUBLIC */
void sb_append_char(String_buf sb, char c)
{
  char s[2];
  s[0] = c;
  s[1] = '\0';
  sb_append(sb, s);
}  /* sb_append_char */

/*************
 *
 *   sb_char()
 *
 *************/

/* DOCUMENTATION
This routine returns the n-th character (counting from 0) of String_buf sb.
If index n is out of range, the NULL character '\0' is returned.
*/

/* PUBLIC */
char sb_char(String_buf sb, int n)
{
  if (n < 0 || n >= sb->size)
    return '\0';
  else {
    Chunk h = sb->first;
    int i;
    for (i = 0; i < n / CHUNK; i++)
      h = h->next;
    return h->s[n % CHUNK];
  }
}  /* sb_char */

/*************
 *
 *   sb_cat_copy()
 *
 *************/

/* DOCUMENTATION
This routine appends a copy of sb2 to sb1.
String_buf sb2 is not changed.
You can use sb_cat() instead if you won't be needing sb2.
*/

/* PUBLIC */
void sb_cat_copy(String_buf sb1, String_buf sb2)
{
  /* Note that this is inefficient if there are many chunks in either sb. */
  int i;
  char c;

  for (i = 0; (c = sb_char(sb2, i)); i++)
    sb_append_char(sb1, c);
}  /* sb_cat_copy */

/*************
 *
 *   sb_cat()
 *
 *************/

/* DOCUMENTATION
This routine appends a copy of sb2 to sb1, then deallocates sb2.
Do not refer to sb2 after calling this rouine because it won't exist.
You can use sb_cat_copy() instead if you need to save sb2.
*/

/* PUBLIC */
void sb_cat(String_buf sb1, String_buf sb2)
{
  sb_cat_copy(sb1, sb2);
  zap_string_buf(sb2);
}  /* sb_cat */

/*************
 *
 *   zap_string_buf()
 *
 *************/

/* DOCUMENTATION
This routine deallocates a String_buf and frees all memory
associated with it.
*/

/* PUBLIC */
void zap_string_buf(String_buf sb)
{
  Chunk curr, prev;
  curr = sb->first;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    free_chunk(prev);
  }
  free_string_buf(sb);
}  /* zap_string_buf */

/*************
 *
 *   sb_to_new_string()
 *
 *************/

/* DOCUMENTATION
This routine returns a new, ordinary C string corresponding to the
String_buf argument sb.  WARNING: the new string, say s, is
dynamically allocated (malloced), so don't forget to call the system
routine free(s) when you are finished with the string.  (This routine
is not intended for printing String_bufs; use fprint_sb() instead.)
<P>
String_bufs do not have a NULL character marking the end;
instead, they keep a count of the number of characters.
*/

/* PUBLIC */
char *sb_to_new_string(String_buf sb)
{
  char *s = malloc(sb->size + 1);

  if (s == NULL)
    return NULL;
  else {
    int i;
    for (i = 0; i < sb->size; i++)
      s[i] = sb_char(sb, i);
    s[sb->size] = '\0';
    return s;
  }
}  /* sb_to_new_string */

/*************
 *
 *   sb_size()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int sb_size(String_buf sb)
{
  return sb->size;
}  /* sb_size */
