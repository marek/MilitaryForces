/*
===========================================================================
Copyright (C) 2011 Thilo Schulz <thilo@tjps.eu>

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qasm-inline.h"
#include "../game/q_shared.h"

#if defined (__i386__) || defined(__x86_64__)
static const unsigned short fpucw = 0x0C7F;

/*
 * GNU inline asm ftol conversion functions using SSE or FPU
 */

long qftolsse(float f)
{
  long retval;
  
  __asm__ volatile
  (
    "cvttss2si %1, %0\n"
    : "=r" (retval)
    : "x" (f)
  );
  
  return retval;
}

long qftolx87(float f)
{
  long retval;
  unsigned short oldcw = 0;

  __asm__ volatile
  (
    "fnstcw %2\n"
    "fldcw %3\n"
    "flds %1\n"
    "fistpl %1\n"
    "fldcw %2\n"
    "mov %1, %0\n"
    : "=r" (retval)
    : "m" (f), "m" (oldcw), "m" (fpucw)
  );
  
  return retval;
}

#endif
