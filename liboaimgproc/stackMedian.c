/*****************************************************************************
 *
 * stackMedian.c -- median stacking method
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#include <stdlib.h>

static int	_cmpUint8 ( const void*, const void* );


int
oaStackMedian8 ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint8_t		values[512]; // FIX ME -- should be dynamically allocated?
	uint8_t**	frames = ( uint8_t** ) frameArray;
	uint8_t*	tgt = target;
  unsigned int i, j;
	unsigned int medianPos;

	medianPos = numFrames >> 1;
	for ( i = 0; i < length; i++ ) {
		for ( j = 0; j < numFrames; j++ ) {
			values[j] = frames[j][i];
		}
		qsort ( values, numFrames, sizeof ( uint8_t ), _cmpUint8 );
		*tgt++ = values[ medianPos ];
	}

  return 0;
}


static int
_cmpUint8 ( const void* pa, const void* pb )
{
	uint8_t	a, b;

	a = *(( uint8_t* ) pa );
	b = *(( uint8_t* ) pb );
	return ( a - b );
}
