/*****************************************************************************
 *
 * ephemeris.h -- liboaephem header
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

#ifndef OA_EPHEMERIS
#define OA_EPHEMERIS

#define	OA_SSO_MERCURY	1
#define	OA_SSO_VENUS		2
#define	OA_SSO_EARTH		3
#define	OA_SSO_MARS			4
#define OA_SSO_JUPITER	5
#define	OA_SSO_SATURN		6
#define	OA_SSO_URANUS		7
#define	OA_SSO_NEPTUNE	8
#define OA_SSO_PLUTO		9
#define OA_SSO_MOON			10
#define OA_SSO_SUN			11

#define OA_EPHEM_NUM_SSO	11

typedef struct {
	double		x;
	double		y;
	double		z;
} cartesian;

typedef struct {
	double		RA;
	double		dec;
} radec;

extern int		oaDayNumber ( struct tm* );
extern int		oaJulianDayNumber ( struct tm* );
extern double	oaEclipticObliquity ( int );

extern int		oaEclipticCartesianPosition ( unsigned int, struct tm*,
									cartesian* );
extern int		oaEquatorialCartesianPosition ( unsigned int, struct tm*,
									cartesian* );
extern int		oaRADECPosition ( unsigned int, struct tm*, radec* );
extern double	oaApparentEquatorialDiameter ( unsigned int, struct tm* );

#endif /* OA_EPHEMERIS */
