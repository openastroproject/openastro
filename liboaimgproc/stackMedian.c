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

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_MALLOC_H
#include <malloc.h>
#endif

static int	_cmpUint8 ( const void*, const void* );
static int	_cmpUint16 ( const void*, const void* );


int
oaStackMedian8 ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint8_t*	values;
	uint8_t**	frames = ( uint8_t** ) frameArray;
	uint8_t*	tgt = target;
  unsigned int i, j;
	unsigned int medianPos;

	if (!( values = ( uint8_t* ) malloc ( numFrames ))) {
		return -1;
	}
	medianPos = numFrames >> 1;
	for ( i = 0; i < length; i++ ) {
		for ( j = 0; j < numFrames; j++ ) {
			values[j] = frames[j][i];
		}
		qsort ( values, numFrames, sizeof ( uint8_t ), _cmpUint8 );
		*tgt++ = values[ medianPos ];
	}

	free (( void* ) values );
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


int
oaStackMedian16LE ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint16_t*			values;
	uint8_t**			frames = ( uint8_t** ) frameArray;
	uint8_t*			tgt = target;
  unsigned int	i, j;
	unsigned int	medianPos;

	if (!( values = ( uint16_t* ) malloc ( numFrames ))) {
		return -1;
	}
	medianPos = numFrames >> 1;
	for ( i = 0; i < length; i += 2 ) {
		for ( j = 0; j < numFrames; j++ ) {
			values[j] = frames[j][i] + ( frames[j][i+1] << 8 );
		}
		qsort ( values, numFrames, sizeof ( uint16_t ), _cmpUint16 );
		*tgt++ = values[ medianPos ] & 0xff;
		*tgt++ = values[ medianPos ] >> 8;
	}

	free (( void* ) values );
  return 0;
}


int
oaStackMedian16BE ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length )
{
	uint16_t*			values;
	uint8_t**			frames = ( uint8_t** ) frameArray;
	uint8_t*			tgt = target;
  unsigned int	i, j;
	unsigned int	medianPos;

	if (!( values = ( uint16_t* ) malloc ( numFrames ))) {
		return -1;
	}
	medianPos = numFrames >> 1;
	for ( i = 0; i < length; i += 2 ) {
		for ( j = 0; j < numFrames; j++ ) {
			values[j] = frames[j][i+1] + ( frames[j][i] << 8 );
		}
		qsort ( values, numFrames, sizeof ( uint16_t ), _cmpUint16 );
		*tgt++ = values[ medianPos ] >> 8;
		*tgt++ = values[ medianPos ] & 0xff;
	}

	free (( void* ) values );
  return 0;
}


static int
_cmpUint16 ( const void* pa, const void* pb )
{
	uint16_t	a, b;

	a = *(( uint16_t* ) pa );
	b = *(( uint16_t* ) pb );
	return ( a - b );
}
