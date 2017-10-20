/*****************************************************************************
 *
 * grey.h -- convert any format to GREYxx header
 *
 * Copyright 2017 ...
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

#ifndef OPENASTRO_VIDEO_GREY_H
#define OPENASTRO_VIDEO_GREY_H

#include <stdint.h>
typedef enum { false, true } bool;

extern void oaGreyscale_8to16LE( uint8_t* s, uint8_t* t, int len );
extern void oaGreyscale_8to16BE( uint8_t* s, uint8_t* t, int len );

extern void oaGreyscale_10to8( uint8_t* s, uint8_t* t, int len, bool packed );
extern void oaGreyscale_10to16LE( uint8_t* s, uint8_t* t, int len, bool packed );
extern void oaGreyscale_10to16BE( uint8_t* s, uint8_t* t, int len, bool packed );

extern void oaGreyscale_16LEto8( uint8_t* s, uint8_t* t, int len );
extern void oaGreyscale_16BEto8( uint8_t* s, uint8_t* t, int len );
extern void oaGreyscale_16swap( uint8_t* s, uint8_t* t, int len );

extern void oaGreyscale_RGB24to8( uint8_t* s, uint8_t* t, int len, bool swapRB );
extern void oaGreyscale_RGB24to16LE( uint8_t* s, uint8_t* t, int len, bool swapRB );
extern void oaGreyscale_RGB24to16BE( uint8_t* s, uint8_t* t, int len, bool swapRB );

extern void oaGreyscale_RGB48LEto8( uint8_t* s, uint8_t* t, int len, bool swapRB );
extern void oaGreyscale_RGB48LEto16LE( uint8_t* s, uint8_t* t, int len, bool swapRB );
extern void oaGreyscale_RGB48LEto16BE( uint8_t* s, uint8_t* t, int len, bool swapRB );

extern void oaGreyscale_RGB48BEto8( uint8_t* s, uint8_t* t, int len, bool swapRB );
extern void oaGreyscale_RGB48BEto16LE( uint8_t* s, uint8_t* t, int len, bool swapRB );
extern void oaGreyscale_RGB48BEto16BE( uint8_t* s, uint8_t* t, int len, bool swapRB );

#endif	/* OPENASTRO_VIDEO_GREY_H */
