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

#include "ibuffer.h"
#include <unistd.h>

/* Private definitions and types */

/*
 * memory management (nonstandard)
 */

/*************
 *
 *   ibuf_init()
 *
 *************/

/* DOCUMENTATION
Allocate and initialize an Ibuffer.
*/

/* PUBLIC */
Ibuffer ibuf_init(void)
{
  Ibuffer ibuf = malloc(sizeof(struct ibuffer));
  ibuf->buf = malloc(IBUF_INIT_SIZE * sizeof(int));
  ibuf->size = IBUF_INIT_SIZE;
  ibuf->write_position = 0;
  ibuf->read_position = 0;
  return ibuf;
}  /* ibuf_init */

/*************
 *
 *   ibuf_free()
 *
 *************/

/* DOCUMENTATION
Free an Ibuffer.
*/

/* PUBLIC */
void ibuf_free(Ibuffer ibuf)
{
  free(ibuf->buf);
  free(ibuf);
}  /* ibuf_free */

/*************
 *
 *   ibuf_write()
 *
 *************/

/* DOCUMENTATION
Write an integer to an Ibuffer.
*/

/* PUBLIC */
void ibuf_write(Ibuffer ibuf, int i)
{
  if (ibuf->write_position >= ibuf->size) {
    int size2 = ibuf->size * 2;
    int *buf2, i;
    printf("ibuf_write, increasing buf size from %d to %d\n",
	   ibuf->size, size2);
    buf2 = malloc(size2 * sizeof(int));
    for (i = 0; i < ibuf->size; i++)
      buf2[i] = ibuf->buf[i];
    free(ibuf->buf);
    ibuf->buf = buf2;
    ibuf->size = size2;
  }
  ibuf->buf[ibuf->write_position] = i;
  ibuf->write_position++;
}  /* ibuf_write */

/*************
 *
 *   ibuf_write_block()
 *
 *************/

/* DOCUMENTATION
Write an array of integers to an Ibuffer.
*/

/* PUBLIC */
void ibuf_write_block(Ibuffer ibuf, int *a, int n)
{
  int i;
  for (i = 0; i < n; i++)
    ibuf_write(ibuf, a[i]);
}  /* ibuf_write_block */

/*************
 *
 *   ibuf_rewind()
 *
 *************/

/* DOCUMENTATION
Reset an Ibuffer for reading.
*/

/* PUBLIC */
void ibuf_rewind(Ibuffer ibuf)
{
  ibuf->read_position = 0;
}  /* ibuf_rewind */

/*************
 *
 *   ibuf_read()
 *
 *************/

/* DOCUMENTATION
Get the next integer from an Ibuffer.
This version returns IBUF_EOF (INT_MIN) if there is nothing to read.
*/

/* PUBLIC */
int ibuf_read(Ibuffer ibuf)
{
  if (ibuf->read_position >= ibuf->write_position)
    return IBUF_EOF;
  else {
    int i = ibuf->buf[ibuf->read_position];
    ibuf->read_position++;
    return i;
  }
}  /* ibuf_read */

/*************
 *
 *   ibuf_xread()
 *
 *************/

/* DOCUMENTATION
Get the next integer from an Ibuffer.
This version assumes there is an integer to read;
if it is at the end IBUF_EOF, a fatal error occurs.
*/

/* PUBLIC */
int ibuf_xread(Ibuffer ibuf)
{
  int i;
  if (ibuf->read_position >= ibuf->write_position)
    fatal_error("ibuf_xread: end of buffer");
  i = ibuf->buf[ibuf->read_position];
  ibuf->read_position++;
  return i;
}  /* ibuf_xread */

/*************
 *
 *   ibuf_length()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int ibuf_length(Ibuffer ibuf)
{
  return ibuf->write_position;
}  /* ibuf_length */

/*************
 *
 *   ibuf_buffer()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int *ibuf_buffer(Ibuffer ibuf)
{
  return ibuf->buf;
}  /* ibuf_buffer */

/*************
 *
 *   fd_read_to_ibuf()
 *
 *************/

#define ISIZE 100

/* DOCUMENTATION
*/

/* PUBLIC */
Ibuffer fd_read_to_ibuf(int fd)
{
  Ibuffer ibuf = ibuf_init();
  int csize = ISIZE * sizeof(int);
  int tibuf[ISIZE];

  int rc;

  do {
    rc = read(fd, tibuf, csize);
    if (rc == -1) {
      perror("");
      fatal_error("fd_read_to_ibuf, read error");
    }
    else if (rc == 0)
      ;  /* we're done */
    else if (rc % sizeof(int) != 0)
      fatal_error("fd_read_to_ibuf, bad number of chars read");
    else {
      ibuf_write_block(ibuf, tibuf, rc / sizeof(int));
    }
  } while (rc > 0);
  
  return ibuf;
}  /* fd_read_to_ibuf */

/*************
 *
 *   p_ibuf()
 *
 *************/

/* DOCUMENTATION
Print a an Ibuffer to a stdout.  This is mainly for debugging.
*/

/* PUBLIC */
void p_ibuf(Ibuffer ibuf)
{
  int i = ibuf_read(ibuf);
  while (i != IBUF_EOF) {
    printf("%d ", i);
    i = ibuf_read(ibuf);
  }
  printf("\n");
}  /* p_ibuf */

