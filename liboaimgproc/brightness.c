/*****************************************************************************
 *
 * brightness.c -- Apply a brightness transformation to an image
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
#include <openastro/demosaic.h>
#include <openastro/errno.h>
#include <openastro/video/formats.h>


static void		_brightnessTransform8Bit ( void*, void*, unsigned int, int );
static void		_brightnessTransform16BitLE ( void*, void*, unsigned int, int );
static void		_brightnessTransform16BitBE ( void*, void*, unsigned int, int );


// brightness should be from -255 to 255

int
oaBrightnessTransform ( void* source, void* target, int xSize, int ySize,
    int frameFormat, int brightness )
{
  unsigned int numPixels = xSize * ySize;
	unsigned int frameSize;

	if ( brightness < -255 || brightness > 255 ) {
		return -OA_ERR_OUT_OF_RANGE;
	}

	frameSize = numPixels * oaFrameFormats[ frameFormat ].bytesPerPixel;

  if ( oaFrameFormats[ frameFormat ].packed ||
			oaFrameFormats[ frameFormat ].lumChrom ) {
		// Can't do brightness transform on packed colour or
		// luminance/chrominance image
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	if ((( oaFrameFormats[ frameFormat ].monochrome ||
			oaFrameFormats[ frameFormat ].rawColour ) &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 1 ) ||
			( oaFrameFormats[ frameFormat ].fullColour &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 3 )) {
		_brightnessTransform8Bit ( source, target, frameSize, brightness );
		return OA_ERR_NONE;
	}

	if ((( oaFrameFormats[ frameFormat ].monochrome ||
			oaFrameFormats[ frameFormat ].rawColour ) &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 2 ) ||
			( oaFrameFormats[ frameFormat ].fullColour &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 6 )) {
		if ( oaFrameFormats[ frameFormat ].littleEndian ) {
			_brightnessTransform16BitLE ( source, target, frameSize, brightness );
		} else {
			_brightnessTransform16BitBE ( source, target, frameSize, brightness );
		}
		return OA_ERR_NONE;
	}

	fprintf ( stderr, "Unrecognised frame format '%s' in %s\n",
			oaFrameFormats[ frameFormat ].name, __FUNCTION__ );

  // FIX ME -- return more meaningful error
  return -OA_ERR_UNSUPPORTED_FORMAT;
}


static void
_brightnessTransform8Bit ( void* source, void* target, unsigned int frameSize,
    int brightness )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;

	for ( i = 0; i < frameSize; i++ ) {
		*t++ = oaclamp ( 0, 255, brightness + *s++ );
	}
}


// Having little-endian and big-endian versions of these functions saves
// on a lot of "if" statements

static void
_brightnessTransform16BitLE ( void* source, void* target,
		unsigned int frameSize, int brightness )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	uint16_t				pixelValue;
	uint16_t				newValue;

	// convert to 16-bit value
	brightness = brightness | ( brightness << 8 );

	for ( i = 0; i < frameSize; i++ ) {
		pixelValue = *s++;
		pixelValue += ( *s++ ) << 8;
		pixelValue += brightness;
		newValue = ( uint16_t ) oaclamp ( 0, 65535, pixelValue );
		*t++ = newValue & 0xff;
		*t++ = newValue >> 8;
	}
}


static void
_brightnessTransform16BitBE ( void* source, void* target,
		unsigned int frameSize, int brightness )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	uint16_t				pixelValue;
	uint16_t				newValue;

	// convert to 16-bit value
	brightness = brightness | ( brightness << 8 );

	for ( i = 0; i < frameSize; i++ ) {
		pixelValue = ( *s++ ) << 8;
		pixelValue += *s++;
		pixelValue += brightness;
		newValue = ( uint16_t ) oaclamp ( 0, 65535, pixelValue );
		*t++ = newValue >> 8;
		*t++ = newValue & 0xff;
	}
}
