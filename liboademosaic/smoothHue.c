/*****************************************************************************
 *
 * smoothHue.c -- smooth hue demosaic method
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

#include "smoothHue.h"


static void
_smoothHueRGGB8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col, lastX, lastY, tRowSize;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle outer two rows and columns

  // First all the green pixels have to be done

  // odd rows
  // odd pixels, G avg(vert/horiz)
  // even pixels, G direct copy
  
  lastX = xSize - 2;
  lastY = ySize - 1;
  for ( row = 1; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;

      t++;
      *t++ = *s;  // G
      t++;
      s++;
    }
  }

  // even rows
  // odd pixels, G direct copy
  // even pixels, G avg(vert/horiz)

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = *s;  // G
      t++;
      s++;

      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;
    }
  }

  // And now the red and blue pixels

  // odd rows
  // odd pixels, R = Gt/4 * sum ( Rs/Gt ) for diagonals
  // odd pixels, B = direct copy
  // even pixels, R = Gt/2 * sum ( Rs/Gt ) for verticals
  // even pixels, B = Gt/2 * sum ( Bs/Gt ) for horizontals

  tRowSize = xSize * 3;

  float g1, g2, g3, g4, r;

  lastX--;
  lastY--;
  for ( row = 3; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      if (!( g1 = *( t - tRowSize + 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize + 1 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - 4 ))) { g1 = 1; }
      if (!( g2 = *( t + 2 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // B
      *t = r;
      t++;
      s++;

      if (!( g1 = *( t - tRowSize - 2 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 4 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 2 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 4 ))) { g4 = 1; }
      r = *( t + 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      *t++ = *s; // B
      s++;

    }
  }

  // even rows
  // odd pixels, R = Gt/2 * sum ( Rs/Gt ) for horizontals
  // odd pixels, B = Gt/2 * sum ( Bs/Gt ) for verticals
  // even pixels, R = direct copy
  // even pixels, B = Gt/4 * sum ( Bs/Gt ) for diagonals

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      *t = *s; // R
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 4 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 2 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 4 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 2 ))) { g4 = 1; }
      r = *( t - 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;

      if (!( g1 = *( t - 2 ))) { g1 = 1; }
      if (!( g2 = *( t + 4 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize - 1 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;
    }
  }
}


static void
_smoothHueBGGR8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col, lastX, lastY, tRowSize;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle outer two rows and columns

  // Green pixels first

  // odd rows
  // odd pixels, G avg(vert/horiz)
  // even pixels, G direct copy
  
  lastX = xSize - 2;
  lastY = ySize - 1;
  for ( row = 1; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;
      t++;
      *t++ = *s;  // G
      t++;
      s++;
    }
  }

  // even rows
  // odd pixels, G direct copy
  // even pixels, G avg(vert/horiz)

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = *s;  // G
      t++;
      s++;
      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;
    }
  }

  // And now the red and blue pixels

  // odd rows
  // odd pixels, R = direct copy
  // odd pixels, B = Gt/4 * sum ( Bs/Gt ) for diagonals
  // even pixels, R = Gt/2 * sum ( Rs/Gt ) for horizontals
  // even pixels, B = Gt/2 * sum ( Bs/Gt ) for verticals

  tRowSize = xSize * 3;

  float g1, g2, g3, g4, r;

  lastX--;
  lastY--;
  for ( row = 3; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      if (!( g1 = *( t - 2 ))) { g1 = 1; }
      if (!( g2 = *( t + 4 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize - 1 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;

      *t = *s; // R
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 4 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 2 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 4 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 2 ))) { g4 = 1; }
      r = *( t - 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;
    }
  }

  // even rows
  // odd pixels, R = Gt/2 * sum ( Rs/Gt ) for verticals
  // odd pixels, B = Gt/2 * sum ( Bs/Gt ) for horizontals
  // even pixels, R = Gt/4 * sum ( Rs/Gt ) for diagonals
  // even pixels, B = direct copy

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      if (!( g1 = *( t - tRowSize - 2 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 4 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 2 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 4 ))) { g4 = 1; }
      r = *( t + 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      *t++ = *s; // B
      s++;

      if (!( g1 = *( t - tRowSize + 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize + 1 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - 4 ))) { g1 = 1; }
      if (!( g2 = *( t + 2 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // B
      *t = r;
      t++;
      s++;
    }
  }
}


static void
_smoothHueGRBG8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col, lastX, lastY, tRowSize;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle outer two rows and columns

  // first do the green

  // odd rows
  // odd pixels, G direct copy
  // even pixels, G avg(vert/horiz)
  
  lastX = xSize - 2;
  lastY = ySize - 1;
  for ( row = 1; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = *s;  // G
      t++;
      s++;
      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;
    }
  }

  // even rows
  // odd pixels, G avg(vert/horiz)
  // even pixels, G direct copy

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;
      t++;
      *t++ = *s;  // G
      t++;
      s++;
    }
  }

  // now red and blue

  // odd rows
  // odd pixels, R = Gt/2 * sum ( Rs/Gt ) for verticals
  // odd pixels, B = Gt/2 * sum ( Bs/Gt ) for horizontals
  // even pixels, R = Gt/4 * sum ( Rs/Gt ) for diagonals
  // even pixels, B = direct copy

  tRowSize = xSize * 3;

  float g1, g2, g3, g4, r;

  lastX--;
  lastY--;
  for ( row = 3; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      if (!( g1 = *( t - tRowSize - 2 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 4 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 2 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 4 ))) { g4 = 1; }
      r = *( t + 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      *t++ = *s; // B
      s++;

      if (!( g1 = *( t - tRowSize + 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize + 1 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - 4 ))) { g1 = 1; }
      if (!( g2 = *( t + 2 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // B
      *t = r;
      t++;
      s++;
    }
  }

  // even rows
  // odd pixels, R = direct copy
  // odd pixels, B = Gt/4 * sum ( Bs/Gt ) for diagonals
  // even pixels, R = Gt/2 * sum ( Rs/Gt ) for horizontals
  // even pixels, B = Gt/2 * sum ( Bs/Gt ) for verticals

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      if (!( g1 = *( t - 2 ))) { g1 = 1; }
      if (!( g2 = *( t + 4 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize - 1 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;

      *t = *s; // R
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 4 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 2 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 4 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 2 ))) { g4 = 1; }
      r = *( t - 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;

    }
  }
}


static void
_smoothHueGBRG8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col, lastX, lastY, tRowSize;
  unsigned char* s;
  unsigned char* t;

  // FIX ME -- handle outer two rows and columns

  // green data first

  // odd rows
  // odd pixels, G direct copy
  // even pixels, G avg(vert/horiz)
  
  lastX = xSize - 2;
  lastY = ySize - 2;
  for ( row = 1; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = *s;  // G
      t++;
      s++;
      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;
    }
  }

  // even rows
  // odd pixels, G avg(vert/horiz)
  // even pixels, G direct copy

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col < lastX; col += 2 ) {
      t++;
      *t++ = ( *( s - xSize ) + *( s - 1 ) + *( s + 1 ) +
          *( s + xSize )) / 4;  // G
      t++;
      s++;
      t++;
      *t++ = *s;  // G
      t++;
      s++;
    }
  }

  // And now the red and blue pixels

  // odd rows
  // odd pixels, R = Gt/2 * sum ( Rs/Gt ) for horizontals
  // odd pixels, B = Gt/2 * sum ( Bs/Gt ) for verticals
  // even pixels, R = direct copy
  // even pixels, B = Gt/4 * sum ( Bs/Gt ) for diagonals

  tRowSize = xSize * 3;

  float g1, g2, g3, g4, r;

  lastX--;
  lastY--;

  for ( row = 3; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      *t = *s; // R
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 4 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 2 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 4 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 2 ))) { g4 = 1; }
      r = *( t - 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;

      if (!( g1 = *( t - 2 ))) { g1 = 1; }
      if (!( g2 = *( t + 4 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - tRowSize - 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize - 1 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // B
      if ( r > 255 ) { r = 255; }
      *t = r;
      t++;
      s++;

    }
  }

  // even rows
  // odd pixels, R = Gt/4 * sum ( Rs/Gt ) for diagonals
  // odd pixels, B = direct copy
  // even pixels, R = Gt/2 * sum ( Rs/Gt ) for verticals
  // even pixels, B = Gt/2 * sum ( Bs/Gt ) for horizontals

  for ( row = 2; row < lastY; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 2;
    t = ( unsigned char* ) target + ( row * xSize + 2 ) * 3;
    for ( col = 2; col < lastX; col += 2 ) {

      if (!( g1 = *( t - tRowSize + 1 ))) { g1 = 1; }
      if (!( g2 = *( t + tRowSize + 1 ))) { g2 = 1; }
      r = *( t + 1 ) / 2 * (
          ( *( s - xSize ) / g1 ) +
          ( *( s + xSize ) / g2 )); // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      if (!( g1 = *( t - 4 ))) { g1 = 1; }
      if (!( g2 = *( t + 2 ))) { g2 = 1; }
      r = *( t - 1 ) / 2 * (
          ( *( s - 1 ) / g1 ) +
          ( *( s + 1 ) / g2 ));  // B
      *t = r;
      t++;
      s++;

      if (!( g1 = *( t - tRowSize - 2 ))) { g1 = 1; }
      if (!( g2 = *( t - tRowSize + 4 ))) { g2 = 1; }
      if (!( g3 = *( t + tRowSize - 2 ))) { g3 = 1; }
      if (!( g4 = *( t + tRowSize + 4 ))) { g4 = 1; }
      r = *( t + 1 ) / 4 * (
          ( *( s - xSize - 1 ) / g1 ) +
          ( *( s - xSize + 1 ) / g2 ) +
          ( *( s + xSize - 1 ) / g3 ) +
          ( *( s + xSize + 1 ) / g4 ));  // R
      if ( r > 255 ) { r = 255; }
      *t = r;
      t += 2; // skip G
      *t++ = *s; // B
      s++;
    }
  }
}


void
oadSmoothHue ( void* source, void* target, int xSize, int ySize,
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
      _smoothHueRGGB8 ( source, target, xSize, ySize );
      break;
    case OA_DEMOSAIC_BGGR:
      _smoothHueBGGR8 ( source, target, xSize, ySize );
      break;
    case OA_DEMOSAIC_GRBG:
      _smoothHueGRBG8 ( source, target, xSize, ySize );
      break;
    case OA_DEMOSAIC_GBRG:
      _smoothHueGBRG8 ( source, target, xSize, ySize );
      break;
  }
}
