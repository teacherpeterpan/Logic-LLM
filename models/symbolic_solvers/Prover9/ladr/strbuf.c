/*  Copyright (C) 2006, 2007 William McCune

    This file is part of the LADR Deduction Library.

    The LADR Deduction Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License,
    version 2.

    The LADR Deduction Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the LADR Deduction Library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "strbuf.h"

/*
 * private definitions and types
 */

struct string_buf {
  struct chunk *first;
  int size;
};

#define CHUNK_SIZE 100

struct chunk {
  char s[CHUNK_SIZE];
  struct chunk *next;
};

typedef struct chunk *Chunk;

/*
 * memory management
 */

#define PTRS_CHUNK PTRS(sizeof(struct chunk))
static unsigned Chunk_gets, Chunk_frees;

#define PTRS_STRING_BUF PTRS(sizeof(struct string_buf))
static unsigned String_buf_gets, String_buf_frees;

/*************
 *
 *   Chunk get_chunk()
 *
 *************/

static
Chunk get_chunk(void)
{
  Chunk p = get_cmem(PTRS_CHUNK);
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
  String_buf p = get_cmem(PTRS_STRING_BUF);
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

  n = sizeof(struct chunk);
  fprintf(fp, "chunk (%4d)        %11u%11u%11u%9.1f K\n",
          n, Chunk_gets, Chunk_frees,
          Chunk_gets - Chunk_frees,
          ((Chunk_gets - Chunk_frees) * n) / 1024.);

  n = sizeof(struct string_buf);
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
    char c = h->s[i % CHUNK_SIZE];
    if (c != '\0')
      putc(c, fp);
    i++;
    if (i % CHUNK_SIZE == 0)
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
    if (n % CHUNK_SIZE == 0) {
      Chunk new = get_chunk();
      if (last != NULL)
	last->next = new;
      else
	sb->first = new;
      last = new;
    }
    last->s[n % CHUNK_SIZE] = s[i];
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
  /* In the first version of this routine, we simply built a string "c",
     and then called sb_append.  This causes a problem, however, if
     we use this package for sequences of small integers.  In particular,
     if we want to append the char 0, that won't work.
  */

  int n = sb->size;
  Chunk last = sb->first;

  while (last != NULL && last->next != NULL)
    last = last->next;

  if (n % CHUNK_SIZE == 0) {
    Chunk new = get_chunk();
    if (last != NULL)
      last->next = new;
    else
      sb->first = new;
    last = new;
  }
  last->s[n % CHUNK_SIZE] = c;
  n++;
  sb->size = n;
}  /* sb_append_char */

/*************
 *
 *   sb_replace_char()
 *
 *************/

/* DOCUMENTATION
This routine replaces a character in a String_buf.
If the index i is out of range, nothing happens.
*/

/* PUBLIC */
void sb_replace_char(String_buf sb, int i, char c)
{
  if (i < 0 || i >= sb->size)
    return;
  else {
    Chunk h = sb->first;
    int j;
    for (j = 0; j < i / CHUNK_SIZE; j++)
      h = h->next;
    h->s[i % CHUNK_SIZE] = c;
  }
}  /* sb_replace_char */

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
    for (i = 0; i < n / CHUNK_SIZE; i++)
      h = h->next;
    return h->s[n % CHUNK_SIZE];
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
 *   sb_to_malloc_string()
 *
 *************/

/* DOCUMENTATION
This routine returns a new, ordinary C string corresponding to the
String_buf argument sb.  WARNING: the new string, say s, is
dynamically allocated (malloced), so don't forget to call
free(s) when you are finished with the string.  (This routine
is not intended for printing String_bufs; use fprint_sb() instead.)
<P>
String_bufs do not have a NULL character marking the end;
instead, they keep a count of the number of characters.
<p>
If the String_buf contains NULL characters, they do NOT mark the
end of the string.  Instead, they are simply ignored when constructing
the ordinary string.
*/

/* PUBLIC */
char *sb_to_malloc_string(String_buf sb)
{
  char *s = malloc(sb->size + 1);

  if (s == NULL)
    return NULL;
  else {
    int j = 0;  /* index for new string */
    int i;      /* index for Str_buf */
    for (i = 0; i < sb->size; i++) {
      char c = sb_char(sb, i);
      if (c != '\0')
	s[j++] = c;
    }
    s[j] = '\0';
    return s;
  }
}  /* sb_to_malloc_string */

/*************
 *
 *   sb_to_malloc_char_array()
 *
 *************/

/* DOCUMENTATION
This routine is similar to sb_to_malloc_string(), except that
null characters are copied to the new string. 
*/

/* PUBLIC */
char *sb_to_malloc_char_array(String_buf sb)
{
  char *s = malloc(sb->size + 1);

  if (s == NULL)
    return NULL;
  else {
    int i;
    for (i = 0; i < sb->size; i++)
      s[i] = sb_char(sb, i);
    s[i] = '\0';
    return s;
  }
}  /* sb_to_malloc_char_array */

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

/*************
 *
 *   sb_append_int()
 *
 *************/

/* DOCUMENTATION
Convert an integer to a string and append the string to a String_buf.
*/

/* PUBLIC */
void sb_append_int(String_buf sb, int i)
{
  char s[25];
  sb_append(sb, int_to_str(i, s, 25));
}  /* sb_append_int */

