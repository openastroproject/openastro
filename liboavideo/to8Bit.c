/*****************************************************************************
 *
 * to8Bit.c -- conversion to 8-bit frame formats
 *
 * Copyright 2017 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <oa_common.h>

#include "to8Bit.h"

void
copyAlternate ( void* source, int offset, void* target, unsigned int length )
{
  uint8_t*	s = source;
  uint8_t*	t = target;
  
  s += offset;
  do {
    *t++ = *s++;
    s++;
    length -= 2;
  } while ( length );
}

void
oaBigEndian16BitTo8Bit ( void* source, void* target, unsigned int length )
{
  copyAlternate ( source, 0, target, length );
}


void
oaLittleEndian16BitTo8Bit ( void* source, void* target, unsigned int length )
{
  copyAlternate ( source, 1, target, length );
}
