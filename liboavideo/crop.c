/*****************************************************************************
 *
 * crop.c -- crop an image frame
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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

int
oaInplaceCrop ( void* data, unsigned int xSize, unsigned int ySize,
		unsigned int cropX, unsigned int cropY, int bpp )
{
  uint8_t*			source;
  uint8_t*			startOfRow;
  uint8_t*			target = ( uint8_t* ) data;
  unsigned int	origRowLength, cropRowLength, i;

  origRowLength = xSize * bpp;
  cropRowLength = cropX * bpp;
  startOfRow = target + ( ySize - cropY ) / 2 * origRowLength +
      ( xSize - cropX ) / 2 * bpp;
  while ( cropY-- ) {
    source = startOfRow;
    for ( i = 0; i < cropRowLength; i++ ) {
      *target++ = *source++;
    }
    startOfRow += origRowLength;
  }

	return OA_ERR_NONE;
}
