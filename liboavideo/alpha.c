/*****************************************************************************
 *
 * alpha.c -- convert formats with an alpha channel to RGB888
 *
 * Copyright 2021
 *   James Fidell (james@openastroproject.org)
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
#include <openastro/video.h>

#include "alpha.h"


static void
_skipEveryFourth ( uint8_t* source, uint8_t* target, unsigned int len )
{
	unsigned int		i;

	for ( i = 0; i < len; i++ ) {
		if ( i % 4 != 3 ) {
			*target++ = *source++;
		} else {
			source++;
		}
	}
}


static void
_skipEveryFourthReverse ( uint8_t* source, uint8_t* target, unsigned int len )
{
	unsigned int		i;

	for ( i = 0; i < len; i += 4 ) {
		*( target + 2 ) = *source++;
		*( target + 1 ) = *source++;
		*target = *source++;
		target += 4;
	}
}


void
oaRGBAtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
	_skipEveryFourth ( source, target, len );
}


void
oaARGBtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize - 1;
	uint8_t *s = source;
	s++;
	_skipEveryFourth ( s, target, len );
}


void
oaBGRAtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
	_skipEveryFourthReverse ( source, target, len );
}


void
oaABGRtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize - 1;
	uint8_t *s = source;
	s++;
	_skipEveryFourthReverse ( s, target, len );
}
