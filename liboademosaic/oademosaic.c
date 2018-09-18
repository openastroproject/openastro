/*****************************************************************************
 *
 * demosaic.c -- main demosaic library entrypoint
 *
 * Copyright 2013,2014,2018 James Fidell (james@openastroproject.org)
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
#include <openastro/demosaic.h>

#include "nearestNeighbour.h"
#include "bilinear.h"
#include "smoothHue.h"
#include "vng.h"


int
oademosaic ( void* source, void* target, int xSize, int ySize, int bitDepth,
    int format, int method )
{
  if ( OA_DEMOSAIC_CMYG == format || OA_DEMOSAIC_MCGY == format ||
      OA_DEMOSAIC_YGCM == format || OA_DEMOSAIC_GYMC == format ) {
    // This is the only method we have for CMYG etc. at the moment
    method = OA_DEMOSAIC_NEAREST_NEIGHBOUR;
  }

  switch ( method ) {
    case OA_DEMOSAIC_NEAREST_NEIGHBOUR:
      oadNearestNeighbour ( source, target, xSize, ySize, bitDepth, format );
      return 0;
    case OA_DEMOSAIC_BILINEAR:
      oadBilinear ( source, target, xSize, ySize, bitDepth, format );
      return 0;
    case OA_DEMOSAIC_SMOOTH_HUE:
      oadSmoothHue ( source, target, xSize, ySize, bitDepth, format );
      return 0;
    case OA_DEMOSAIC_VNG:
      oadVNG ( source, target, xSize, ySize, bitDepth, format );
      return 0;
    default:
      return -1;
  }
  return 0;
}


const char*
oademosaicMethodName ( int method )
{
  char* names[ OA_DEMOSAIC_LAST_P1 ] = {
      "",
      "Nearest Neighbour",
      "Bilinear",
      "Smooth Hue",
      "Variable Number of Gradients"
  };

  if ( method > 0 && method < OA_DEMOSAIC_LAST_P1 ) {
    return names[ method ];
  }
  return 0;
}
