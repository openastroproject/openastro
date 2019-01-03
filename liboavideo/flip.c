/*****************************************************************************
 *
 * flip.c -- flip an image in X and/or Y
 *
 * Copyright 2018,2019
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
#include <openastro/errno.h>
#include <openastro/video.h>
#include <openastro/video/formats.h>


static void		_processFlip8Bit ( uint8_t*, unsigned int, unsigned int, int );
static void		_processFlip16Bit ( uint8_t*, unsigned int, unsigned int, int );
static void		_processFlip24BitColour ( uint8_t*, unsigned int, unsigned int,
		int );


int
oaFlipImage ( void* imageData, unsigned int xSize, unsigned int ySize,
		int format, int axis )
{
  uint8_t* data = ( uint8_t* ) imageData;
  int assumedFormat = format;

  // fake up a format for mosaic frames here as properly flipping a
  // mosaicked frame would be quite hairy

  if ( oaFrameFormats[ format ].rawColour ) {
    if ( oaFrameFormats[ format ].bitsPerPixel == 8 ) {
      assumedFormat = OA_PIX_FMT_GREY8;
    } else {
      if ( oaFrameFormats[ format ].bitsPerPixel == 16 ) {
        assumedFormat = OA_PIX_FMT_GREY16BE;
      } else {
        fprintf ( stderr, "No flipping idea how to handle format %d\n",
            format );
				return OA_ERR_UNIMPLEMENTED;
      }
    }
  }

  switch ( assumedFormat ) {
    case OA_PIX_FMT_GREY8:
      _processFlip8Bit ( data, xSize, ySize, axis );
      break;
    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_GREY16LE:
      _processFlip16Bit ( data, xSize, ySize, axis );
      break;
    case OA_PIX_FMT_RGB24:
    case OA_PIX_FMT_BGR24:
      _processFlip24BitColour ( data, xSize, ySize, axis );
      break;
    default:
      fprintf ( stderr, "Unable to flip format %d\n", format );
			return OA_ERR_UNIMPLEMENTED;
      break;
  }

	return OA_ERR_NONE;
}


static void
_processFlip8Bit ( uint8_t* imageData, unsigned int xSize, unsigned int ySize,
		int axis )
{
	unsigned int length = xSize * ySize;

  if (( OA_FLIP_X | OA_FLIP_Y ) == axis ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 1;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2-- = s;
    }
  } else {
    if ( OA_FLIP_X == axis ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      unsigned int y;
      for ( y = 0; y < ySize; y++ ) {
        p1 = imageData + y * xSize;
        p2 = p1 + xSize - 1;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2-- = s;
        }
      }
    }
    if ( OA_FLIP_Y == axis ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      unsigned int x, y;
      p1 = imageData;
      for ( y = ySize - 1; y >= ySize / 2; y-- ) {
        p2 = imageData + y * xSize;
        for ( x = 0; x < xSize; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


static void
_processFlip16Bit ( uint8_t* imageData, unsigned int xSize, unsigned int ySize,
		int axis )
{
	unsigned int length = xSize * ySize * 2;

  if (( OA_FLIP_X | OA_FLIP_Y ) == axis ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 2;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2 = s;
      p2 -= 3;
    }
  } else {
    if ( OA_FLIP_X == axis ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      unsigned int y;
      for ( y = 0; y < ySize; y++ ) {
        p1 = imageData + y * xSize * 2;
        p2 = p1 + ( xSize - 1 ) * 2;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2 = s;
          p2 -= 3;
        }
      }
    }
    if ( OA_FLIP_Y == axis ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      unsigned int x, y;
      p1 = imageData;
      for ( y = ySize - 1; y > ySize / 2; y-- ) {
        p2 = imageData + y * xSize * 2;
        for ( x = 0; x < xSize * 2; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


static void
_processFlip24BitColour ( uint8_t* imageData, unsigned int xSize,
		unsigned int ySize, int axis )
{
	unsigned int length = xSize * ySize * 3;

  if (( OA_FLIP_X | OA_FLIP_Y ) == axis ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 3;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2 = s;
      p2 -= 5;
    }
  } else {
    if ( OA_FLIP_X == axis ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      unsigned int y;
      for ( y = 0; y < ySize; y++ ) {
        p1 = imageData + y * xSize * 3;
        p2 = p1 + ( xSize - 1 ) * 3;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2 = s;
          p2 -= 5;
        }
      }
    }
    if ( OA_FLIP_Y == axis ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      unsigned int x, y;
      p1 = imageData;
      for ( y = ySize - 1; y > ySize / 2; y-- ) {
        p2 = imageData + y * xSize * 3;
        for ( x = 0; x < xSize * 3; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}
