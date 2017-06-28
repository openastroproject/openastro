/*****************************************************************************
 *
 * nearestNeighbour.c -- nearest neighbour demosaic method
 *
 * Copyright 2013,2014 James Fidell (james@openastroproject.org)
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


static void
_nnRGGB8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R pixel to left & "up", G is pixel to left, B is direct copy
  // odd rows, even pixels
  // R is pixel "up", G is direct copy, B is pixel to left
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < xSize; col += 2 ) {
      *t++ = *( s - xSize - 1 );  // R
      *t++ = *( s - 1 );  // G
      *t++ = *s;  // B
      s++;
      *t++ = *( s - xSize );  // R
      *t++ = *s;  // G
      *t++ = *( s - 1 );  // B
      s++;
    }
  }

  // even rows, odd pixels
  // R pixel to left, G is direct copy, B is pixel "up"
  // even rows, even pixels
  // R is direct copy, G is pixel to left, B is pixel to left & "up"

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < xSize; col += 2 ) {
      *t++ = *( s - 1 );  // R
      *t++ = *s;  // G
      *t++ = *( s - xSize );  // B
      s++;
      *t++ = *s;  // R
      *t++ = *( s - 1 );  // G
      *t++ = *( s - 1 - xSize );  // B
      s++;
    }
  }
}


static void
_nnBGGR8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is a direct copy, G is pixel to left, B pixel to left & "up"
  // odd rows, even pixels
  // R is pixel to left, G is direct copy, B is pixel "up"
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      *t++ = *s;  // R
      *t++ = *( s - 1 );  // G
      *t++ = *( s - xSize - 1 );  // B
      s++;
      *t++ = *( s - 1 );  // R
      *t++ = *s;  // G
      *t++ = *( s - xSize );  // B
      s++;
    }
  }

  // even rows, odd pixels
  // R pixel "up", G is direct copy, B is pixel to left
  // even rows, even pixels
  // R is pixel to left and "up", G is pixel to left, B is direct copy

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      *t++ = *( s - xSize );  // R
      *t++ = *s;  // G
      *t++ = *( s - 1 );  // B
      s++;
      *t++ = *( s - 1 - xSize );  // R
      *t++ = *( s - 1 );  // G
      *t++ = *s;  // B
      s++;
    }
  }
}


static void
_nnGRBG8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is "up", G is direct copy, B pixel to left
  // odd rows, even pixels
  // R is pixel to left and "up", G is pixel to left, B is direct copy
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      *t++ = *( s - xSize );  // R
      *t++ = *s;  // G
      *t++ = *( s - 1 );  // B
      s++;
      *t++ = *( s - xSize - 1 );  // R
      *t++ = *( s - 1 );  // G
      *t++ = *s;  // B
      s++;
    }
  }

  // even rows, odd pixels
  // R is direct copy, G is pixel to left, B is pixel to left and "up"
  // even rows, even pixels
  // R is pixel to left, G is direct copy, B is "up"

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      *t++ = *s;  // R
      *t++ = *( s - 1 );  // G
      *t++ = *( s - xSize - 1 );  // B
      s++;
      *t++ = *( s - 1 );  // R
      *t++ = *s;  // G
      *t++ = *( s - xSize );  // B
      s++;
    }
  }
}


static void
_nnGBRG8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is pixel to left, G is direct copy, B is "up"
  // odd rows, even pixels
  // R is direct copy, G is pixel to left, B is left and "up"
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      *t++ = *( s - 1 );  // R
      *t++ = *s;  // G
      *t++ = *( s - xSize );  // B
      s++;
      *t++ = *s;  // R
      *t++ = *( s - 1 );  // G
      *t++ = *( s - xSize - 1 );  // B
      s++;
    }
  }

  // even rows, odd pixels
  // R is left and "up", G is pixel to left, B is direct copy
  // even rows, even pixels
  // R is "up", G is direct copy, B is pixel to left

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      *t++ = *( s - xSize - 1 );  // R
      *t++ = *( s - 1 );  // G
      *t++ = *s;  // B
      s++;
      *t++ = *( s - xSize );  // R
      *t++ = *s;  // G
      *t++ = *( s - 1 );  // B
      s++;
    }
  }
}


void
oadNearestNeighbour ( void* source, void* target, int xSize, int ySize,
    int bitDepth, int format )
{
  // FIX ME
  if ( bitDepth != 8 ) {
    fprintf ( stderr, "demosaic: %s can only handle 8-bit data\n",
        __FUNCTION__ );
    return;
  }

  switch ( format ) {
    case OA_DEMOSAIC_RGGB:
      _nnRGGB8 ( source, target, xSize, ySize );
      break;
    case OA_DEMOSAIC_BGGR:
      _nnBGGR8 ( source, target, xSize, ySize );
      break;
    case OA_DEMOSAIC_GRBG:
      _nnGRBG8 ( source, target, xSize, ySize );
      break;
    case OA_DEMOSAIC_GBRG:
      _nnGBRG8 ( source, target, xSize, ySize );
      break;
  }
}
