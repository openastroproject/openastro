/*****************************************************************************
 *
 * vng.c -- variable number of gradients demosaic method
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

#include <float.h>

#include "vng.h"


const float k1 = 1.5;
const float k2 = 0.5;


static void _fetchPixels ( unsigned char*, int, unsigned char* );


static void
_vng8 ( void* source, void* target, int xSize, int ySize, int format )
{
  int i, row, col, lastX, lastY, numGradients;
  int doingRed = 0, doingBlue = 0, redRow = 0;
  unsigned int r32, g32, b32;
  unsigned char r8, g8, b8;
  unsigned char* s;
  unsigned char* t;
  float gradient[8], minGradient, maxGradient,  threshold;
  float RorBsum, Gsum, BorRsum;
  unsigned char pixel[26];

  // FIX ME -- handle the outside two rows/columns

  lastX = xSize - 4;
  lastY = ySize - 2;

  for ( row = 2; row < lastY; row++ ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;

    // GRBG has red or blue photosites if row is even and col is odd or
    // row is odd and col is even.  That is, ( row + col ) is odd.  Red is
    // on even rows, blue is on odd.  We don't have to calculate this for
    // each pixel though -- it can be done progressively for each pixel
    // in the row based on what we've just done.  The others work
    // similarly

    switch ( format ) {
      case OA_DEMOSAIC_RGGB:
        doingRed = !( row % 2 ); // we start on red on even rows
        doingBlue = 0; // we never start on blue
        redRow = !( row % 2 ); // red is even rows for RGGB
        break;

      case OA_DEMOSAIC_BGGR:
        doingRed = 0; // we never start on red
        doingBlue = !( row % 2 ); // start on blue on even rows
        redRow = ( row % 2 ); // red is odd rows for BGGR
        break;

      case OA_DEMOSAIC_GRBG:
        doingRed = 0; // we never start on red
        doingBlue = ( row % 2 ); // start on blue on odd rows
        redRow = !( row % 2 ); // red is even rows for GRBG
        break;

      case OA_DEMOSAIC_GBRG:
        doingRed = ( row % 2 ); // start on red on odd rows
        doingBlue = 0; // never start on blue
        redRow = ( row % 2 ); // red is odd rows for GBRG
        break;
    }

    for ( col = 2; col < lastX; col++ ) {

      // grab the pixel values for a 5x5 square centred on the pixel
      // in question.  Saves lots of messing dereferencing in the code

      _fetchPixels ( s, xSize, pixel );

      // calculations of the interpolated values at a red or blue site are
      // the same, but the blue and red results will be swapped depending on
      // the colour.

      // N, S, E, W gradients are the same for whatever photosite

      gradient[0] =  // N
          abs ( pixel[8] - pixel[18] ) + abs ( pixel[3] - pixel[13] ) +
          abs ( pixel[7] - pixel[17] )/2 + abs ( pixel[9] - pixel[19] )/2 +
          abs ( pixel[2] - pixel[12] )/2 + abs ( pixel[4] - pixel[14] )/2;

      gradient[2] =  // E
          abs ( pixel[14] - pixel[12] ) + abs ( pixel[15] - pixel[13] ) +
          abs ( pixel[9] - pixel[7] )/2 + abs ( pixel[19] - pixel[17] )/2 +
          abs ( pixel[10] - pixel[8] )/2 + abs ( pixel[20] - pixel[18] )/2;

      gradient[4] =  // S
          abs ( pixel[18] - pixel[8] ) + abs ( pixel[23] - pixel[13] ) +
          abs ( pixel[19] - pixel[9] )/2 + abs ( pixel[17] - pixel[7] )/2 +
          abs ( pixel[24] - pixel[14] )/2 + abs ( pixel[22] - pixel[12] )/2;

      gradient[6] =  // W
          abs ( pixel[12] - pixel[14] ) + abs ( pixel[11] - pixel[13] ) +
          abs ( pixel[17] - pixel[19] )/2 + abs ( pixel[7] - pixel[9] )/2 +
          abs ( pixel[16] - pixel[18] )/2 + abs ( pixel[6] - pixel[8] )/2;

      if ( doingRed || doingBlue ) {

        gradient[1] =  // NE
            abs ( pixel[9] - pixel[17] ) + abs ( pixel[5] - pixel[13] ) +
            abs ( pixel[8] - pixel[12] )/2 + abs ( pixel[14] - pixel[18] )/2 +
            abs ( pixel[4] - pixel[8] )/2 + abs ( pixel[10] - pixel[14] )/2;

        gradient[3] =  // SE
            abs ( pixel[19] - pixel[7] ) + abs ( pixel[25] - pixel[13] ) +
            abs ( pixel[14] - pixel[8] )/2 + abs ( pixel[18] - pixel[12] )/2 +
            abs ( pixel[20] - pixel[14] )/2 + abs ( pixel[24] - pixel[18] )/2;

        gradient[5] =  // SW
            abs ( pixel[17] - pixel[9] ) + abs ( pixel[21] - pixel[13] ) +
            abs ( pixel[18] - pixel[14] )/2 + abs ( pixel[12] - pixel[8] )/2 +
            abs ( pixel[22] - pixel[18] )/2 + abs ( pixel[16] - pixel[12] )/2;

        gradient[7] =  // NW
            abs ( pixel[7] - pixel[19] ) + abs ( pixel[1] - pixel[13] ) +
            abs ( pixel[12] - pixel[18] )/2 + abs ( pixel[8] - pixel[14] )/2 +
            abs ( pixel[6] - pixel[12] )/2 + abs ( pixel[2] - pixel[8] )/2;

        minGradient = FLT_MAX;
        maxGradient = FLT_MIN;
        for ( i = 0; i < 8; i++ ) {
          if ( gradient[i] < minGradient ) {
            minGradient = gradient[i];
          }
          if ( gradient[i] > maxGradient ) {
            maxGradient = gradient[i];
          }
        }

        threshold = k1 * minGradient + k2 * ( maxGradient - minGradient );

        RorBsum = Gsum = BorRsum = 0;
        numGradients = 0;
        for ( i = 0; i < 8; i++ ) {
          if ( gradient[i] < threshold ) {
            numGradients++;
            switch ( i ) {
              case 0: // N
                RorBsum += ( pixel[13] + pixel[3] ) / 2;
                Gsum += pixel[8];
                BorRsum += ( pixel[7] + pixel[9] ) / 2;
                break;
              case 1: // NE
                RorBsum += ( pixel[5] + pixel[13] ) / 2;
                Gsum += ( pixel[4] + pixel[8] + pixel[10] + pixel[14] ) / 4;
                BorRsum += pixel[9];
                break;
              case 2: // E
                RorBsum += ( pixel[13] + pixel[15] ) / 2;
                Gsum += pixel[14];
                BorRsum += ( pixel[9] + pixel[19] ) / 2;
                break;
              case 3: // SE
                RorBsum += ( pixel[13] + pixel[25] ) / 2;
                Gsum += ( pixel[14] + pixel[18] + pixel[20] + pixel[24] ) / 4;
                BorRsum += pixel[19];
                break;
              case 4: // S
                RorBsum += ( pixel[13] + pixel[23] ) / 2;
                Gsum += pixel[18];
                BorRsum += ( pixel[17] + pixel[19] ) / 2;
                break;
              case 5: // SW
                RorBsum += ( pixel[13] + pixel[21] ) / 2;
                Gsum += ( pixel[12] + pixel[16] + pixel[18] + pixel[22] ) / 4;
                BorRsum += pixel[17];
                break;
              case 6: // W
                RorBsum += ( pixel[13] + pixel[11] ) / 2;
                Gsum += pixel[12];
                BorRsum += ( pixel[7] + pixel[17] ) / 2;
                break;
              case 7: // NW
                RorBsum += ( pixel[13] + pixel[1] ) / 2;
                Gsum += ( pixel[2] + pixel[6] + pixel[8] + pixel[12] ) / 4;
                BorRsum += pixel[7];
                break;
            }
          }
        }

        if ( doingRed ) {
          // In this case, RorBsum is actually for R and BorRsum is for B
          r32 = pixel[13];
          g32 = pixel[13] + ( Gsum - RorBsum ) / numGradients;
          b32 = pixel[13] + ( BorRsum - RorBsum ) / numGradients;
          doingRed = 0; // set these for the next photosite
        } else {
          // RorBsum is actually B and BorRsum is R
          r32 = pixel[13] + ( BorRsum - RorBsum ) / numGradients;
          g32 = pixel[13] + ( Gsum - RorBsum ) / numGradients;
          b32 = pixel[13];
          doingBlue = 0; // set these for the next photosite
        }
      } else {

        gradient[1] =  // NE
            abs ( pixel[9] - pixel[17] ) + abs ( pixel[5] - pixel[13] ) +
            abs ( pixel[4] - pixel[12] ) + abs ( pixel[10] - pixel[18] );

        gradient[3] =  // SE
            abs ( pixel[19] - pixel[17] ) + abs ( pixel[5] - pixel[13] ) +
            abs ( pixel[20] - pixel[8] ) + abs ( pixel[24] - pixel[12] );

        gradient[5] =  // SW
            abs ( pixel[17] - pixel[9] ) + abs ( pixel[21] - pixel[13] ) +
            abs ( pixel[22] - pixel[14] ) + abs ( pixel[16] - pixel[8] );

        gradient[7] =  // NW
            abs ( pixel[7] - pixel[19] ) + abs ( pixel[1] - pixel[13] ) +
            abs ( pixel[6] - pixel[18] ) + abs ( pixel[2] - pixel[14] );

        minGradient = FLT_MAX;
        maxGradient = FLT_MIN;
        for ( i = 0; i < 8; i++ ) {
          if ( gradient[i] < minGradient ) {
            minGradient = gradient[i];
          }
          if ( gradient[i] > maxGradient ) {
            maxGradient = gradient[i];
          }
        }

        threshold = k1 * minGradient + k2 * ( maxGradient - minGradient );

        RorBsum = Gsum = BorRsum = 0;
        numGradients = 0;
        for ( i = 0; i < 8; i++ ) {
          if ( gradient[i] < threshold ) {
            numGradients++;
            switch ( i ) {
              case 0: // N
                RorBsum += ( pixel[2] + pixel[4] + pixel[12] + pixel[14] ) / 4;
                Gsum += ( pixel[3] + pixel[13] ) / 2;
                BorRsum += pixel[8];
                break;
              case 1: // NE
                RorBsum += ( pixel[4] + pixel[14] ) / 2;
                Gsum += pixel[9];
                BorRsum += ( pixel[8] + pixel[10] ) / 2;
                break;
              case 2: // E
                RorBsum += pixel[14];
                Gsum += ( pixel[13] + pixel[15] ) / 2;
                BorRsum += ( pixel[8] + pixel[10] + pixel[18] + pixel[20] ) / 4;
                break;
              case 3: // SE
                RorBsum += ( pixel[14] + pixel[24] ) / 2;
                Gsum += pixel[19];
                BorRsum += ( pixel[18] + pixel[20] ) / 2;
                break;
              case 4: // S
                RorBsum += ( pixel[12] + pixel[14] + pixel[22] + pixel[24] )/4;
                Gsum += ( pixel[13] + pixel[23] ) / 2;
                BorRsum += pixel[18];
                break;
              case 5: // SW
                RorBsum += ( pixel[12] + pixel[22] ) / 2;
                Gsum += pixel[17];
                BorRsum += ( pixel[16] + pixel[18] ) / 2;
                break;
              case 6: // W
                RorBsum += pixel[12];
                Gsum += ( pixel[11] + pixel[13] ) / 2;
                BorRsum += ( pixel[6] + pixel[8] + pixel[16] + pixel[18] ) / 4;
                break;
              case 7: // NW
                RorBsum += ( pixel[2] + pixel[12] ) / 2;
                Gsum += pixel[7];
                BorRsum += ( pixel[6] + pixel[8] ) / 2;
                break;
            }
          }
        }

        g32 = pixel[13];
        if ( redRow ) {
          // In this case, RorBsum is actually for R and BorRsum is for B
          r32 = g32 + ( RorBsum - Gsum ) / numGradients;
          b32 = g32 + ( BorRsum - Gsum ) / numGradients;
          doingRed = 1; // set these for the next photosite
        } else {
          // RorBsum is actually B and BorRsum is R
          r32 = g32 + ( BorRsum - Gsum ) / numGradients;
          b32 = g32 + ( RorBsum - Gsum ) / numGradients;
          doingBlue = 1; // set these for the next photosite
        }
      }

      if ( r32 > 255 ) { r32 = 255; };
      if ( g32 > 255 ) { g32 = 255; };
      if ( b32 > 255 ) { b32 = 255; };

      r8 = r32;
      g8 = g32;
      b8 = b32;

      *t++ = r8;
      *t++ = g8;
      *t++ = b8;
      s++;
    }
  }
}


void
oadVNG ( void* source, void* target, int xSize, int ySize,
    int bitDepth, int format )
{
  // FIX ME
  if ( bitDepth != 8 ) {
    fprintf ( stderr, "demosaic: %s can only handle 8-bit data\n",
        __FUNCTION__ );
    return;
  }

  _vng8 ( source, target, xSize, ySize, format );
}


static void
_fetchPixels ( unsigned char* s, int xSize, unsigned char* pixel )
{
  pixel[1] = *( s - xSize * 2 - 2 );
  pixel[2] = *( s - xSize * 2 - 1 );
  pixel[3] = *( s - xSize * 2 );
  pixel[4] = *( s - xSize * 2 + 1 );
  pixel[5] = *( s - xSize * 2 + 2 );

  pixel[6] = *( s - xSize - 2 );
  pixel[7] = *( s - xSize - 1 );
  pixel[8] = *( s - xSize );
  pixel[9] = *( s - xSize + 1 );
  pixel[10] = *( s - xSize + 2 );

  pixel[11] = *( s - 2 );
  pixel[12] = *( s - 1 );
  pixel[13] = *s;
  pixel[14] = *( s + 1 );
  pixel[15] = *( s + 2 );

  pixel[16] = *( s + xSize - 2 );
  pixel[17] = *( s + xSize - 1 );
  pixel[18] = *( s + xSize );
  pixel[19] = *( s + xSize + 1 );
  pixel[20] = *( s + xSize + 2 );

  pixel[21] = *( s + xSize * 2 - 2 );
  pixel[22] = *( s + xSize * 2 - 1 );
  pixel[23] = *( s + xSize * 2 );
  pixel[24] = *( s + xSize * 2 + 1 );
  pixel[25] = *( s + xSize * 2 + 2 );
}
