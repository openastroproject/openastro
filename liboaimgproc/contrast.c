/*****************************************************************************
 *
 * contrast.c -- Apply a contrast transformation to an image
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
#include <openastro/errno.h>
#include <openastro/video/formats.h>


static void		_contrastTransform8Bit ( void*, void*, unsigned int, int );
static void		_contrastTransform16BitLE ( void*, void*, unsigned int, int );
static void		_contrastTransform16BitBE ( void*, void*, unsigned int, int );


// contrast should be from -255 to 255

int
oaContrastTransform ( void* source, void* target, int xSize, int ySize,
    int frameFormat, int contrast )
{
  unsigned int numPixels = xSize * ySize;
	unsigned int frameSize;

	if ( contrast < -255 || contrast > 255 ) {
		return -OA_ERR_OUT_OF_RANGE;
	}

	frameSize = numPixels * oaFrameFormats[ frameFormat ].bytesPerPixel;

  if ( oaFrameFormats[ frameFormat ].rawColour ||
			oaFrameFormats[ frameFormat ].packed ||
			oaFrameFormats[ frameFormat ].lumChrom ) {
		// Can't do contrast transform on raw colour, packed colour or
		// luminance/chrominance image
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	if (( oaFrameFormats[ frameFormat ].monochrome &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 1 ) ||
			( oaFrameFormats[ frameFormat ].fullColour &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 3 )) {
		_contrastTransform8Bit ( source, target, frameSize, contrast );
		return OA_ERR_NONE;
	}

	if (( oaFrameFormats[ frameFormat ].monochrome &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 2 ) ||
			( oaFrameFormats[ frameFormat ].fullColour &&
			oaFrameFormats[ frameFormat ].bytesPerPixel == 6 )) {
		if ( oaFrameFormats[ frameFormat ].littleEndian ) {
			_contrastTransform16BitLE ( source, target, frameSize, contrast );
		} else {
			_contrastTransform16BitBE ( source, target, frameSize, contrast );
		}
		return OA_ERR_NONE;
	}

	fprintf ( stderr, "Unrecognised frame format '%s' in %s\n",
			oaFrameFormats[ frameFormat ].name, __FUNCTION__ );

  // FIX ME -- return more meaningful error
  return -OA_ERR_UNSUPPORTED_FORMAT;
}


static void
_contrastTransform8Bit ( void* source, void* target, unsigned int frameSize,
    int contrast )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	double					transformFactor;
	double					pixelValue;

	transformFactor =  259.0 * ( contrast + 255.0 ) /
			( 255.0 * ( 259.0 - contrast ));

	for ( i = 0; i < frameSize; i++ ) {
		pixelValue = *s++;
		pixelValue *= transformFactor;
		*t = ( uint8_t ) oadclamp ( 0, 255, pixelValue );
	}
}


// Having little-endian and big-endian versions of these functions saves
// on a lot of "if" statements

static void
_contrastTransform16BitLE ( void* source, void* target, unsigned int frameSize,
    int contrast )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	double					transformFactor;
	double					pixelValue;
	uint16_t				newValue;

	// convert to 16-bit value
	contrast = contrast | ( contrast << 8 );

	transformFactor =  65539.0 * ( contrast + 65535.0 ) /
			( 65535.0 * ( 65539.0 - contrast ));

	for ( i = 0; i < frameSize; i += 2 ) {
		pixelValue = *s++;
		pixelValue += ( *s++ ) << 8;
		pixelValue *= transformFactor;
		newValue = ( uint16_t ) oadclamp ( 0, 65535, pixelValue );
		*t++ = newValue & 0xff;
		*t++ = newValue >> 8;
	}
}


static void
_contrastTransform16BitBE ( void* source, void* target, unsigned int frameSize,
    int contrast )
{
	uint8_t*				s = source;
	uint8_t*				t = target;
	unsigned int		i;
	double					transformFactor;
	double					pixelValue;
	uint16_t				newValue;

	// convert to 16-bit value
	contrast = contrast | ( contrast << 8 );

	transformFactor =  65539.0 * ( contrast + 65535.0 ) /
			( 65535.0 * ( 65539.0 - contrast ));

	for ( i = 0; i < frameSize; i += 2 ) {
		pixelValue += ( *s++ ) << 8;
		pixelValue = *s++;
		pixelValue *= transformFactor;
		newValue = ( uint16_t ) oadclamp ( 0, 65535, pixelValue );
		*t++ = newValue >> 8;
		*t++ = newValue & 0xff;
	}
}
