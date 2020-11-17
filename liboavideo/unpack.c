/*****************************************************************************
 *
 * unpack.c -- conversion to 16-bit frame formats
 *
 * Copyright 2020 James Fidell (james@openastroproject.org)
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

#include "unpack.h"


void
oaBigEndianPackedGrey12ToGrey16 ( void* source, void* target,
		unsigned int length )
{
  uint8_t*	s = source;
  uint8_t*	t = target;
  uint16_t	val16;

	// I'm unconvinced all 12-bit packed formats are like this one (which is
	// actually the Point Grey/FLIR Blackfly format) and appears to be:
	//
	// byte 0 high 8 bits
	// byte 0 low 4 bits
	// byte 1 low 4 bits
	// byte 1 high 8 bits
	//
	// The Basler documentaton for Pylon suggests theirs is more like:
	//
	// byte 0 low 8 bits
	// byte 0 high 4 bits
	// byte 1 high 4 bits
	// byte 1 low 8 bits

	do {
		val16 = *s++ << 4;
		val16 |= ( *s && 0x0f );
		*t++ = ( val16 >> 8 );
		*t++ = ( val16 & 0xff );

		val16 = *s++ >> 4;
		val16 |= *s++ << 4;
		*t++ = ( val16 >> 8 );
		*t++ = ( val16 & 0xff );
		length -= 3;
	} while ( length );
}


void
oaLittleEndianPackedGrey12ToGrey16 ( void* source, void* target,
		unsigned int length )
{
  uint8_t*	s = source;
  uint8_t*	t = target;
  uint16_t	val16;

	// I'm unconvinced all 12-bit packed formats are like this one (which is
	// actually the Point Grey/FLIR Blackfly format) and appears to be:
	//
	// byte 0 high 8 bits
	// byte 0 low 4 bits
	// byte 1 low 4 bits
	// byte 1 high 8 bits
	//
	// The Basler documentaton for Pylon suggests theirs is more like:
	//
	// byte 0 low 8 bits
	// byte 0 high 4 bits
	// byte 1 high 4 bits
	// byte 1 low 8 bits

	do {
		val16 = *s++ << 4;
		val16 |= ( *s && 0x0f );
		*t++ = ( val16 & 0xff );
		*t++ = ( val16 >> 8 );

		val16 = *s++ >> 4;
		val16 |= *s++ << 4;
		*t++ = ( val16 & 0xff );
		*t++ = ( val16 >> 8 );
		length -= 3;
	} while ( length );
}
