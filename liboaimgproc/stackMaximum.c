/*****************************************************************************
 *
 * stackMaximum.c -- maximum stacking method
 *
 * Copyright 2019,2020 James Fidell (james@openastroproject.org)
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
oaStackMaximum8 ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint8_t**	frames = ( uint8_t** ) frameArray;
	uint8_t*	tgt = target;
  unsigned int i, j;
	uint8_t		max;

	for ( i = 0; i < length; i++ ) {
		max = 0;
		for ( j = 0; j < numFrames; j++ ) {
			if ( frames[j][i] > max ) {
				max = frames[j][i];
			}
		}
		*tgt++ = max;
	}

  return 0;
}


int
oaStackMaximum16LE ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint8_t**		frames = ( uint8_t** ) frameArray;
	uint8_t*			tgt = target;
  unsigned int	i, j;
	uint16_t			max;

// FIX ME -- handle byte order
	for ( i = 0; i < length; i += 2 ) {
		max = 0;
		for ( j = 0; j < numFrames; j++ ) {
			int v = frames[j][i] + ( frames[j][i+1] << 8 );
			if ( v > max ) {
				max = v;
			}
		}
		*tgt++ = max & 0xff;
		*tgt++ = max >> 8;
	}

  return 0;
}



int
oaStackMaximum16BE ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint8_t**		frames = ( uint8_t** ) frameArray;
	uint8_t*			tgt = target;
  unsigned int	i, j;
	uint16_t			max;

// FIX ME -- handle byte order
	for ( i = 0; i < length; i += 2 ) {
		max = 0;
		for ( j = 0; j < numFrames; j++ ) {
			int v = frames[j][i+1] + ( frames[j][i] << 8 );
			if ( v > max ) {
				max = v;
			}
		}
		*tgt++ = max >> 8;
		*tgt++ = max & 0xff;
	}

  return 0;
}

