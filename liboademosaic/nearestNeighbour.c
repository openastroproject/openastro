/*****************************************************************************
 *
 * nearestNeighbour.c -- nearest neighbour demosaic method
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

#include <stdint.h>
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


static void
_nnCMYG8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is (Y (left) + M (up)) / 2
  // G is direct copy
  // B is (M (up) + C (up left)) / 2
  // odd rows, even pixels
  // R is (Y (current) + M (up left)) / 2
  // G is G (left)
  // B is (M (up left) + C (up)) / 2
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      c = *( s - xSize - 1 );
      m = *( s - xSize );
      y = *( s - 1 );
      g = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      c = *( s - xSize );
      y = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  // even rows, odd pixels
  // R is (Y (up left) + M (current)) / 2
  // G is G (up)
  // B is (M (current) + C (left)) / 2
  // odd rows, even pixels
  // R is (Y (up) + M (left)) / 2
  // G is G (up left)
  // B is (M (left) + C (current)) / 2

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      c = *( s - 1 );
      m = *s;
      y = *( s - xSize - 1 );
      g = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      c = *s;
      y = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


static void
_nnMCGY8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      m = *( s - xSize - 1 );
      c = *( s - xSize );
      g = *( s - 1 );
      y = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      m = *( s - xSize );
      g = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      m = *( s - 1 );
      c = *s;
      g = *( s - xSize - 1 );
      y = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      m = *s;
      g = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


static void
_nnYGCM8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      y = *( s - xSize - 1 );
      g = *( s - xSize );
      c = *( s - 1 );
      m = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      y = *( s - xSize );
      c = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      y = *( s - 1 );
      g = *s;
      c = *( s - xSize - 1 );
      m = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      y = *s;
      c = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


static void
_nnGYMC8 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  unsigned char* s;
  unsigned char* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      g = *( s - xSize - 1 );
      y = *( s - xSize );
      m = *( s - 1 );
      c = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      g = *( s - xSize );
      m = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( unsigned char* ) source + row * xSize + 1;
    t = ( unsigned char* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      g = *( s - 1 );
      y = *s;
      m = *( s - xSize - 1 );
      c = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      g = *s;
      m = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


static void
_nnRGGB16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R pixel to left & "up", G is pixel to left, B is direct copy
  // odd rows, even pixels
  // R is pixel "up", G is direct copy, B is pixel to left
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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
_nnBGGR16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is a direct copy, G is pixel to left, B pixel to left & "up"
  // odd rows, even pixels
  // R is pixel to left, G is direct copy, B is pixel "up"
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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
_nnGRBG16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is "up", G is direct copy, B pixel to left
  // odd rows, even pixels
  // R is pixel to left and "up", G is pixel to left, B is direct copy
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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
_nnGBRG16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is pixel to left, G is direct copy, B is "up"
  // odd rows, even pixels
  // R is direct copy, G is pixel to left, B is left and "up"
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
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


static void
_nnCMYG16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  // odd rows, odd pixels
  // R is (Y (left) + M (up)) / 2
  // G is direct copy
  // B is (M (up) + C (up left)) / 2
  // odd rows, even pixels
  // R is (Y (current) + M (up left)) / 2
  // G is G (left)
  // B is (M (up left) + C (up)) / 2
  
  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      c = *( s - xSize - 1 );
      m = *( s - xSize );
      y = *( s - 1 );
      g = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      c = *( s - xSize );
      y = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  // even rows, odd pixels
  // R is (Y (up left) + M (current)) / 2
  // G is G (up)
  // B is (M (current) + C (left)) / 2
  // odd rows, even pixels
  // R is (Y (up) + M (left)) / 2
  // G is G (up left)
  // B is (M (left) + C (current)) / 2

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      c = *( s - 1 );
      m = *s;
      y = *( s - xSize - 1 );
      g = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      c = *s;
      y = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


static void
_nnMCGY16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      m = *( s - xSize - 1 );
      c = *( s - xSize );
      g = *( s - 1 );
      y = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      m = *( s - xSize );
      g = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      m = *( s - 1 );
      c = *s;
      g = *( s - xSize - 1 );
      y = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      m = *s;
      g = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


static void
_nnYGCM16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      y = *( s - xSize - 1 );
      g = *( s - xSize );
      c = *( s - 1 );
      m = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      y = *( s - xSize );
      c = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      y = *( s - 1 );
      g = *s;
      c = *( s - xSize - 1 );
      m = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      y = *s;
      c = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


static void
_nnGYMC16 ( void* source, void* target, int xSize, int ySize )
{
  int row, col;
  uint16_t* s;
  uint16_t* t;
  uint16_t c, m, y, g;

  // FIX ME -- handle first row/column

  ySize--;
  for ( row = 1; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      g = *( s - xSize - 1 );
      y = *( s - xSize );
      m = *( s - 1 );
      c = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      g = *( s - xSize );
      m = *s;
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }

  for ( row = 2; row < ySize; row += 2 ) {
    s = ( uint16_t* ) source + row * xSize + 1;
    t = ( uint16_t* ) target + ( row * xSize + 1 ) * 3;
    for ( col = 1; col <= xSize; col += 2 ) {
      g = *( s - 1 );
      y = *s;
      m = *( s - xSize - 1 );
      c = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
      g = *s;
      m = *( s - xSize );
      *t++ = ( y + m ) / 2; // R
      *t++ = g;  // G
      *t++ = ( m + c ) / 2; // B
      s++;
    }
  }
}


void
oadNearestNeighbour ( void* source, void* target, int xSize, int ySize,
    int bitDepth, int format )
{
	int	done = 0;

	if ( bitDepth == 8 ) {
		switch ( format ) {
			case OA_DEMOSAIC_RGGB:
				_nnRGGB8 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_BGGR:
				_nnBGGR8 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_GRBG:
				_nnGRBG8 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_GBRG:
				_nnGBRG8 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_CMYG:
				_nnCMYG8 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_MCGY:
				_nnMCGY8 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_YGCM:
				_nnYGCM8 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_GYMC:
				_nnGYMC8 ( source, target, xSize, ySize );
				done = 1;
				break;
		}
	}

	if ( bitDepth == 16 ) {
		switch ( format ) {
			case OA_DEMOSAIC_RGGB:
				_nnRGGB16 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_BGGR:
				_nnBGGR16 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_GRBG:
				_nnGRBG16 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_GBRG:
				_nnGBRG16 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_CMYG:
				_nnCMYG16 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_MCGY:
				_nnMCGY16 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_YGCM:
				_nnYGCM16 ( source, target, xSize, ySize );
				done = 1;
				break;
			case OA_DEMOSAIC_GYMC:
				_nnGYMC16 ( source, target, xSize, ySize );
				done = 1;
				break;
		}
	}

	if ( !done ) {
		fprintf ( stderr, "demosaic: %s cannot handle %d-bit data for format %d\n",
				__FUNCTION__, bitDepth, format );
	}

	return;
}
