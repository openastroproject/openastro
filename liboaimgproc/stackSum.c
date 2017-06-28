/*****************************************************************************
 *
 * stackSum.c -- sum stacking method
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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
#include <openastro/imgproc.h>


int
oaStackSum8 ( void* source1, void* source2, void* target, unsigned int length )
{
  unsigned int i;
  uint8_t*	s1 = source1;
  uint8_t*	s2 = source2;
  uint8_t*	t = target;
  unsigned int	v;

  for ( i = 0; i < length; i++ ) {
    v = *s1++ + *s2++;
    *t++ = ( v > 0xff ) ? 0xff : v;
  }

  return 0;
}
