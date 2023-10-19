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

#ifndef TP_IBUFFER_H
#define TP_IBUFFER_H

#include "fatal.h"

/* INTRODUCTION
*/

/* Public definitions */

#define IBUF_INIT_SIZE    40000
#define IBUF_EOF        INT_MIN

struct ibuffer {
  int write_position; /* current number of ints in buf (next pos to write) */
  int read_position;  /* next pos to read */
  int size;           /* size of buf */
  int *buf;           /* the buffer */
};

typedef struct ibuffer * Ibuffer;

/* End of public definitions */

/* Public function prototypes from ibuffer.c */

Ibuffer ibuf_init(void);

void ibuf_free(Ibuffer ibuf);

void ibuf_write(Ibuffer ibuf, int i);

void ibuf_write_block(Ibuffer ibuf, int *a, int n);

void ibuf_rewind(Ibuffer ibuf);

int ibuf_read(Ibuffer ibuf);

int ibuf_xread(Ibuffer ibuf);

int ibuf_length(Ibuffer ibuf);

int *ibuf_buffer(Ibuffer ibuf);

Ibuffer fd_read_to_ibuf(int fd);

void p_ibuf(Ibuffer ibuf);

#endif  /* conditional compilation of whole file */
