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

#include "nonport.h"
#include <string.h>

/* Private definitions and types */

#ifdef PRIMITIVE_ENVIRONMENT
/* This means that we don't have some UNIXy things */
#else
#  include <pwd.h>
#  include <unistd.h>
#endif

/*************
 *
 *   username()
 *
 *************/

/* DOCUMENTATION
Return the name of the user who started the current job.
*/

/* PUBLIC */
char *username(void)
{
#ifdef PRIMITIVE_ENVIRONMENT
  return("an unknown user");
#else
  struct passwd *p;
  p = getpwuid(getuid());
  return(p ? p->pw_name : "???");
#endif
}  /* username */

/*************
 *
 *   hostname()
 *
 *************/

/* DOCUMENTATION
Return the hostname of the computer on which the current job is running.
*/

/* PUBLIC */
char *hostname(void)
{
#ifdef PRIMITIVE_ENVIRONMENT
  return("an unknown computer");
#else
  static char host[64];
  if (gethostname(host, 64) != 0)
    strcpy(host, "???");
  return(host);
#endif
}  /* hostname */

/*************
 *
 *   my_process_id()
 *
 *************/

/* DOCUMENTATION
Return the process ID of the current process.
*/

/* PUBLIC */
int my_process_id(void)
{
#ifdef PRIMITIVE_ENVIRONMENT
  return 0;
#else
  return getpid();
#endif
}  /* my_process_id */

/*************
 *
 *   get_bits()
 *
 *************/

/* DOCUMENTATION
If (64-bit long and pointer) return 64, else return 32.
*/

/* PUBLIC */
int get_bits(void)
{
  return sizeof(long) == 8 && sizeof(void *) == 8 ? 64 : 32;
}  /* get_bits */
