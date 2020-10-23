/*****************************************************************************
 *
 * stackSum.c -- sum stacking method
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
oaStackSum8 ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint8_t		v;
	uint8_t**	frames = ( uint8_t** ) frameArray;
	uint8_t*	tgt = target;
  unsigned int i, j;

	for ( i = 0; i < length; i++ ) {
		v = 0;
		for ( j = 0; j < numFrames; j++ ) {
			v += frames[j][i];
		}
		*tgt++ = v;
	}

  return 0;
}


int
oaStackSum16LE ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint16_t		v;
	uint8_t**		frames = ( uint8_t** ) frameArray;
	uint8_t*		tgt = target;
  unsigned int i, j;

	for ( i = 0; i < length; i += 2 ) {
		v = 0;
		for ( j = 0; j < numFrames; j++ ) {
			v += frames[j][i] + ( frames[j][i+1] << 8 );
		}
		*tgt++ = v & 0xff;
		*tgt++ = v >> 8;
	}

  return 0;
}


int
oaStackSum16BE ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint16_t		v;
	uint8_t**		frames = ( uint8_t** ) frameArray;
	uint8_t*		tgt = target;
  unsigned int i, j;

	for ( i = 0; i < length; i += 2 ) {
		v = 0;
		for ( j = 0; j < numFrames; j++ ) {
			v += frames[j][i+1] + ( frames[j][i] << 8 );
		}
		*tgt++ = v >> 8;
		*tgt++ = v & 0xff;
	}

  return 0;
}
