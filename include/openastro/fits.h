/*****************************************************************************
 *
 * fits.h -- FITS file data header
 *
 * Copyright 2017,2020 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_FITS_H
#define OPENASTRO_FITS_H

#define OA_FITS_STRING		1
#define OA_FITS_INT		2
#define OA_FITS_FLOAT		3
#define OA_FITS_TIMESTAMP	4
#define OA_FITS_HOUR_ANGLE	5
#define OA_FITS_DEGREE_ANGLE	6
#define OA_FITS_BOOLEAN		7

#define OA_FITS_ORG_STANDARD	1
#define OA_FITS_ORG_SBIG	2
#define OA_FITS_ORG_MAXIMDL	3

#define OA_FITS_FLAG_MANDATORY	1
#define OA_FITS_FLAG_REPEATABLE	2

typedef struct {
  char      keyword[10];
  char      shortDesc[32];
  uint8_t   keywordType;
  uint8_t   keywordOrigin;
  uint8_t   flags;
} fitsKeyword;

extern fitsKeyword fitsKeywords[86];

#define OA_FITS_KEY_SIMPLE	0
#define OA_FITS_KEY_BITPIX	1
#define OA_FITS_KEY_NAXIS	2
#define OA_FITS_KEY_NAXIS1	3
#define OA_FITS_KEY_NAXIS2	4
#define OA_FITS_KEY_OBJECT	5
#define OA_FITS_KEY_TELESCOP	6
#define OA_FITS_KEY_INSTRUME	7
#define OA_FITS_KEY_OBSERVER	8
#define OA_FITS_KEY_DATE_OBS	9
#define OA_FITS_KEY_BSCALE	10
#define OA_FITS_KEY_BZERO	11
#define OA_FITS_KEY_HISTORY	12

#endif	/* OPENASTRO_FITS_H */
