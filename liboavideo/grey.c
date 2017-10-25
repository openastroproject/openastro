/*****************************************************************************
 *
 * grey.c -- convert any format to GREYxx
 *
 * Copyright 2014 James Fidell (james@openastroproject.org)
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
#include <openastro/video.h>

#include "grey.h"

void
oaGreyscale_8to16LE( const uint8_t* source, uint8_t* target, int len )
{
  int i;
  uint8_t* t = target + (len-1)*2;
  const uint8_t* s = source + (len-1);

  for ( i = len; i > 0; i--, s-=1, t-=2 ) {
    *(t+1) = *s; 
    *t = 0x00;
  }
}

void
oaGreyscale_8to16BE( const uint8_t* source, uint8_t* target, int len )
{
  int i;
  uint8_t v;
  uint8_t* t = target + (len-1)*2;
  const uint8_t* s = source + (len-1);

  for ( i = len; i > 0; i--, s-=1, t-=2 ) {
    *t = *s;
    *(t+1) = 0x00; 
  }
}

/* FIXME awaiting OA_PIX_FMT_XXXX10X
void
oaGreyscale_10to8( const uint8_t* s, uint8_t* t, int len, bool packed )
{
  int i;
  uint16_t p;

  if ( packed ) {
    for ( i = 0; i < len; i += 5, s += 5 ) {
      *t++ = *s;
      *t++ = *(s+1);
      *t++ = *(s+2);
      *t++ = *(s+3);
      // ignore 5th byte
    }
  }
  else {
    for ( i = 0; i < len; i += 2, s+=2 ) {
      p = (*s << 6) + ((*(s+1)&0x03) << 14);
      *t++ = (p & 0xff00) >> 8;
    }
  }
}

void
oaGreyscale_10to16( const uint8_t* source, uint8_t* target, int len, bool packed, bool little_endian )
{
  int i;
  const uint8_t* s;
  uint8_t* t;
  uint16_t p1, p2, p3, p4;

  const int numPixels = len / ( packed ? 1.25 : 2 );

  if ( packed ) {
    t = target + (numPixels-4) * 2;
    s = source + (numPixels-4) * 5/4;

    for ( i=0; i < len; i += 5, s -= 5, t -= 8)
    {
      uint8_t lsb = *(s+4);

      uint16_t p1 = (*(s)   << 8) + ((lsb&0x03) << 6);
      uint16_t p2 = (*(s+1) << 8) + ((lsb&0x0c) << 4);
      uint16_t p3 = (*(s+2) << 8) + ((lsb&0x30) << 2);
      uint16_t p4 = (*(s+3) << 8) +  (lsb&0xc0);

      if (little_endian) {
        *t     =  p1 & 0x00ff;
        *(t+1) = (p1 & 0xff00) >> 8;
        *(t+2) =  p2 & 0x00ff;
        *(t+3) = (p2 & 0xff00) >> 8;
        *(t+4) =  p3 & 0x00ff;
        *(t+5) = (p3 & 0xff00) >> 8;
        *(t+6) =  p4 & 0x00ff;
        *(t+7) = (p4 & 0xff00) >> 8;
      }
      else {
        *t     = (p1 & 0xff00) >> 8;
        *(t+1) =  p1 & 0x00ff;
        *(t+2) = (p2 & 0xff00) >> 8;
        *(t+3) =  p2 & 0x00ff;
        *(t+4) = (p3 & 0xff00) >> 8;
        *(t+5) =  p3 & 0x00ff;
        *(t+6) = (p4 & 0xff00) >> 8;
        *(t+7) =  p4 & 0x00ff;
      }
    }
  }
  else {
    t = target + (numPixels-1)*2;
    s = source + (numPixels-1)*2;

    for ( i=0; i < len; i += 2, s-=2, t-=2)
    {
      p1 = (*s << 6) + ((*(s+1)&0x03) << 14);
      if (little_endian) {
        *t     =  p1 & 0x00ff;
        *(t+1) = (p1 & 0xff00) >> 8;
      }
      else {
        *t     = (p1 & 0xff00) >> 8;
        *(t+1) =  p1 & 0x00ff;
      }
    }
  }
}

void
oaGreyscale_10to16LE( const uint8_t* s, uint8_t* t, int len, bool packed )
{
  oaGreyscale_10to16( s, t, len, packed, true );
}

void
oaGreyscale_10to16BE( const uint8_t* s, uint8_t* t, int len, bool packed )
{
  oaGreyscale_10to16( s, t, len, packed, false );
}
*/

void
oaGreyscale_16LEto8( const uint8_t* s, uint8_t* t, int len )
{
  int i;
  for ( i = 0; i < len; i+=2, s+=2, t++ ) {
    *t = *(s+1);
  }
}

void
oaGreyscale_16BEto8( const uint8_t* s, uint8_t* t, int len )
{
  int i;
  for ( i = 0; i < len; i+=2, s+=2, t++ ) {
    *t = *s;
  }
}

void
oaGreyscale_16swap( const uint8_t* s, uint8_t* t, int len )
{
  int i;
  for ( i = 0; i < len; i+=2, s+=2, t+=2 ) {
    uint8_t v = *s;
    *t = *(s+1);
    *(t+1) = v; 
  }
}

void
oaGreyscale_RGB24to8( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint8_t R, G, B;
  for ( i = 0; i < len; i += 3, s += 3, t += 1 ) {
    R = swapRB ? *(s+2) : *s; G = *(s+1); B = swapRB ? *s : *(s+2);
    *t = .2126 * R + .7152 * G + .0722 * B;
  }
}

void
oaGreyscale_RGB24to16LE( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 3, s += 3, t += 2 ) {
    R = swapRB ? *(s+2) : *s; G = *(s+1); B = swapRB ? *s : *(s+2);
    Y = .2126 * (R<<8) + .7152 * (G<<8) + .0722 * (B<<8);
    *t = Y & 0x00ff;
    *(t+1) = (Y & 0xff00) >> 8;
  }
}

void
oaGreyscale_RGB24to16BE( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 3, s += 3, t += 2 ) {
    R = swapRB ? *(s+2) : *s; G = *(s+1); B = swapRB ? *s : *(s+2);
    Y = .2126 * (R<<8) + .7152 * (G<<8) + .0722 * (B<<8);
    *t = (Y & 0xff00) >> 8;
    *(t+1) = Y & 0x00ff;
  }
}

void
oaGreyscale_RGB48LEto8( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 6, s += 6, t += 1 ) {
    R = swapRB ? ((*(s+5)<<8) + *(s+4)) : (((*(s+1))<<8) + *s);
    G = (*(s+3)<<8) + *(s+2);
    B = swapRB ? ((*(s+1)<<8) + *s) : ((*(s+5)<<8) + *(s+4));
    Y = .2126 * R + .7152 * G + .0722 * B;
    *t = (Y & 0xff00) >> 8;
  }
}

void
oaGreyscale_RGB48LEto16LE( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 6, s += 6, t += 2 ) {
    R = swapRB ? ((*(s+5)<<8) + *(s+4)) : ((*(s+1)<<8) + *s);
    G = (*(s+3)<<8) + *(s+2);
    B = swapRB ? ((*(s+1)<<8) + *s) : ((*(s+5)<<8) + *(s+4));
    Y = .2126 * R + .7152 * G + .0722 * B;
    *t = Y & 0x00ff;
    *(t+1) = (Y & 0xff00) >> 8;
  }
}

void
oaGreyscale_RGB48LEto16BE( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 6, s += 6, t += 2 ) {
    R = swapRB ? ((*(s+5)<<8) + *(s+4)) : ((*(s+1)<<8) + *s);
    G = (*(s+3)<<8) + *(s+2);
    B = swapRB ? ((*(s+1)<<8) + *s) : ((*(s+5)<<8) + *(s+4));
    Y = .2126 * R + .7152 * G + .0722 * B;
    *t = (Y & 0xff00) >> 8;
    *(t+1) = Y & 0x00ff;
  }
}

void
oaGreyscale_RGB48BEto8( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 6, s += 6, t += 1 ) {
    R = swapRB ? ((*(s+4)<<8) + *(s+5)) : ((*s<<8) + *(s+1));
    G = (*(s+2)<<8) + *(s+3);
    B = swapRB ? ((*s<<8) + *(s+1)) : ((*(s+4)<<8) + *(s+5));
    Y = .2126 * R + .7152 * G + .0722 * B;
    *t = (Y & 0xff00) >> 8;
  }
}

void
oaGreyscale_RGB48BEto16LE( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 6, s += 6, t += 2 ) {
    R = swapRB ? ((*(s+4)<<8) + *(s+5)) : ((*s<<8) + *(s+1));
    G = (*(s+2)<<8) + *(s+3);
    B = swapRB ? ((*s<<8) + *(s+1)) : ((*(s+4)<<8) + *(s+5));
    Y = .2126 * R + .7152 * G + .0722 * B;
    *t = Y & 0x00ff;
    *(t+1) = (Y & 0xff00) >> 8;
  }
}

void
oaGreyscale_RGB48BEto16BE( const uint8_t* s, uint8_t* t, int len, bool swapRB )
{
  int i;
  uint16_t R, G, B;
  uint16_t Y;
  for ( i = 0; i < len; i += 6, s += 6, t += 2 ) {
    R = swapRB ? ((*(s+4)<<8) + *(s+5)) : ((*s<<8) + *(s+1));
    G = (*(s+2)<<8) + *(s+3);
    B = swapRB ? ((*s<<8) + *(s+1)) : ((*(s+4)<<8) + *(s+5));
    Y = .2126 * R + .7152 * G + .0722 * B;
    *t = (Y & 0xff00) >> 8;
    *(t+1) = Y & 0x00ff;
  }
}
