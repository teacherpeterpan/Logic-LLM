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

#include "termflag.h"

/* Private definitions and types */

/* Bits field of term. */

#define TERM_BITS (sizeof(FLAGS_TYPE) * CHAR_BIT)

#define SET_BIT(bits, flag)    (bits = (bits) | (flag))
#define CLEAR_BIT(bits, flag)  (bits = (bits) & ~(flag))
/* #define TP_BIT(bits, flag)     (bits & flag) */

static int bits_in_use[TERM_BITS];

/*************
 *
 *   claim_term_flag()
 *
 *************/

/* DOCUMENTATION
This routine returns an available flag number for marking terms.
If the use will be temporary, make sure to call release_term_flag()
when you are finished using the marks.
A fatal error occurs if no more flags are available.
*/

/* PUBLIC */
int claim_term_flag()
{
  int i = 0;
  while (i < TERM_BITS && bits_in_use[i])
    i++;
  if (i < TERM_BITS) {
    bits_in_use[i] = 1;
    return i;
  }
  else {
    fatal_error("claim_term_flag, no more flags are available");
    return -1;  /* to please the compiler */
  }
}  /* claim_term_flag */

/*************
 *
 *   release_term_flag()
 *
 *************/

/* DOCUMENTATION
This routine frees a flag number for future use.
*/

/* PUBLIC */
void release_term_flag(int bit)
{
  bits_in_use[bit] = 0;
}  /* release_term_flag */

/*************
 *
 *   term_flag_set()
 *
 *************/

/* DOCUMENTATION
This routine sets a flag on a term.  The flag argument is
a small integer in the range [0 .. n-1], where n is the
number of bits available for flags.  If n is out of range,
none of the flags will change.
<P>
Term flags are stored as bits in the field private_flags.
(Look at the definition of Term to find out big private_flags
is.)  If you need more flags, you can simply change the
type of private_flags to unsigned short (usually 16 bits)
or unsigned int (usually 32 bits).  It is your responsibility
to make sure that all of the flags you use are in range.
*/

/* PUBLIC */
void term_flag_set(Term t, int flag)
{
  if (flag >= 0 && flag < TERM_BITS)
    SET_BIT(t->private_flags, 1 << flag);
}  /* term_flag_set */

/*************
 *
 *   term_flag_clear()
 *
 *************/

/* DOCUMENTATION
This routine clears a flag on a term.  The flag argument is
a small integer in the range [0 .. n-1], where n is the
number of bits available for flags.  If n is out of range,
none of the flags will change.
*/

/* PUBLIC */
void term_flag_clear(Term t, int flag)
{
  if (flag >= 0 && flag < TERM_BITS)
    CLEAR_BIT(t->private_flags, 1 << flag);
}  /* term_flag_clear */

/*************
 *
 *   term_flag()
 *
 *************/

/* DOCUMENTATION
This function gets the value of a flag on a term.  The flag
argument is a small integer in the range [0 ... n-1], where n is the
number of bits available for flags.  If n is out of range, FALSE is
returned.
*/

/* PUBLIC */
BOOL term_flag(Term t, int flag)
{
  if (flag < 0 || flag >= TERM_BITS)
    return FALSE;
  else if (TP_BIT(t->private_flags, 1 << flag))
    return TRUE;
  else
    return FALSE;
}  /* term_flag */

/*************
 *
 *   term_flags()
 *
 *************/

/* DOCUMENTATION
This routine returns the number of bits available for term flags.
The value should always be at least 8.  If the value is n,
you can use flags numbered [0 ... n-1].
*/

/* PUBLIC */
int term_flags()
{
  return TERM_BITS;
}  /* term_flags */

/*************
 *
 *    Term copy_term_with_flags()
 *
 *************/

/* DOCUMENTATION
This routine copies a term, including <I>all of the flags</I>
of the term and its subterms.  Any other extra fields are not
copied.
*/

/* PUBLIC */
Term copy_term_with_flags(Term t)
{
  Term t2;

  if (VARIABLE(t))
    t2 = get_variable_term(VARNUM(t));
  else {
    int i;
    t2 = get_rigid_term_like(t);
    for (i = 0; i < ARITY(t); i++)
      ARG(t2,i) = copy_term_with_flags(ARG(t,i));
  }
  t2->private_flags = t->private_flags;
  return t2;
}  /* copy_term_with_flags */

/*************
 *
 *    Term copy_term_with_flag()
 *
 *************/

/* DOCUMENTATION
This routine copies a term, including <I>one specified flag</I>
of the term and its subterms.  Any other flags or extra fields are not
copied.
*/

/* PUBLIC */
Term copy_term_with_flag(Term t, int flag)
{
  Term t2;

  if (VARIABLE(t))
    t2 = get_variable_term(VARNUM(t));
  else {
    int i;
    t2 = get_rigid_term_like(t);
    for (i = 0; i < ARITY(t); i++)
      ARG(t2,i) = copy_term_with_flag(ARG(t,i), flag);
  }
  if (term_flag(t, flag))
    term_flag_set(t2, flag);
  return t2;
}  /* copy_term_with_flag */

/*************
 *
 *   term_flag_clear_recursively()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
void term_flag_clear_recursively(Term t, int flag)
{
  int i;
  term_flag_clear(t, flag);
  for (i = 0; i < ARITY(t); i++)
    term_flag_clear_recursively(ARG(t,i), flag);
}  /* clear_term_flag_recursively */
