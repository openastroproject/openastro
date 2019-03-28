/*****************************************************************************
 *
 * gamma.c -- Apply a gamma transformation to an image
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

#include <math.h>

#include <openastro/imgproc.h>
#include <openastro/demosaic.h>
#include <openastro/errno.h>
#include <openastro/video/formats.h>


static void		_gammaTransform8Bit ( void*, void*, unsigned int, int );
static void		_gammaTransform16BitLE ( void*, void*, unsigned int, int );
static void		_gammaTransform16BitBE ( void*, void*, unsigned int, int );


// gamma should be from 1 to 255

int
oaGammaTransform ( void* source, void* target, int xSize, int ySize,
    int frameFormat, int gamma )
{
  unsigned int numPixels = xSize * ySize;
	unsigned int frameSize;

	if ( gamma < 1 || gamma > 255 ) {
		return -OA_ERR_OUT_OF_RANGE;
	}

	frameSize = numPixels * oaFrameFormats[ frameFormat ].bytesPerPixel;

  if ( oaFrameFormats[ frameFormat ].rawColour ||
			oaFrameFormats[ frameFormat ].packed ||
			oaFrameFormats[ frameFormat ].lumChrom ) {
		// Can't do gamma transform on raw colour, packed colour or
		// luminance/chrominance image
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	if ((( oaFrameFormats[ frameFormat ].monochrome ||
			oaFrameFormats[ frameFormat ].rawColour ) &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 1 ) ||
			( oaFrameFormats[ frameFormat ].fullColour &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 3 )) {
		_gammaTransform8Bit ( source, target, frameSize, gamma );
		return OA_ERR_NONE;
	}

	if ((( oaFrameFormats[ frameFormat ].monochrome ||
			oaFrameFormats[ frameFormat ].rawColour ) &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 2 ) ||
			( oaFrameFormats[ frameFormat ].fullColour &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 6 )) {
		if ( oaFrameFormats[ frameFormat ].littleEndian ) {
			_gammaTransform16BitLE ( source, target, frameSize, gamma );
		} else {
			_gammaTransform16BitBE ( source, target, frameSize, gamma );
		}
		return OA_ERR_NONE;
	}

	fprintf ( stderr, "Unrecognised frame format '%s' in %s\n",
			oaFrameFormats[ frameFormat ].name, __FUNCTION__ );

  // FIX ME -- return more meaningful error
  return -OA_ERR_UNSUPPORTED_FORMAT;
}


static void
_gammaTransform8Bit ( void* source, void* target, unsigned int frameSize,
    int gamma )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	double					gammaExponent;
	double					pixelValue;

	gammaExponent = 100.0 / gamma;
	for ( i = 0; i < frameSize; i++ ) {
		pixelValue = *s++;
		*t++ = oaclamp ( 0, 255, pow ( pixelValue, gammaExponent ));
	}
}


// Having little-endian and big-endian versions of these functions saves
// on a lot of "if" statements

static void
_gammaTransform16BitLE ( void* source, void* target, unsigned int frameSize,
    int gamma )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	uint16_t				pixelValue;
	uint16_t				newValue;
	double					gammaExponent;

	gammaExponent = 100.0 / gamma;

	for ( i = 0; i < frameSize; i++ ) {
		pixelValue = *s++;
		pixelValue += ( *s++ ) << 8;
		pixelValue += gamma;
		newValue = ( uint16_t ) oaclamp ( 0, 255, pow ( pixelValue,
				gammaExponent ));
		*t++ = newValue & 0xff;
		*t++ = newValue >> 8;
	}
}


static void
_gammaTransform16BitBE ( void* source, void* target, unsigned int frameSize,
    int gamma )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	uint16_t				pixelValue;
	uint16_t				newValue;
	double					gammaExponent;

	gammaExponent = 100.0 / gamma;

	for ( i = 0; i < frameSize; i++ ) {
		pixelValue = ( *s++ ) << 8;
		pixelValue += *s++;
		pixelValue += gamma;
		newValue = ( uint16_t ) oaclamp ( 0, 255, pow ( pixelValue,
				gammaExponent ));
		*t++ = newValue >> 8;
		*t++ = newValue & 0xff;
	}
}
