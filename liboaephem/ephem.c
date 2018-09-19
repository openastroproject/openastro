/*****************************************************************************
 *
 * ephem.c -- common stuff and entrypoints
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

#include <time.h>
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#include <openastro/ephemeris.h>

#include "orbitalElements.h"
#include "mercury.h"
#include "venus.h"
#include "mars.h"
#include "jupiter.h"
#include "saturn.h"
#include "uranus.h"
#include "neptune.h"
#include "moon.h"
#include "sun.h"
#include "trig.h"


int
oaEclipticCartesianPosition ( unsigned int body, struct tm* date,
		cartesian* posn )
{
	switch ( body ) {
		case OA_SSO_MERCURY:
			mercuryEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_VENUS:
			venusEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_MARS:
			marsEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_JUPITER:
			jupiterEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_SATURN:
			saturnEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_URANUS:
			uranusEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_NEPTUNE:
			neptuneEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_MOON:
			moonEclipticCartesianPosition ( date, posn );
			break;
		case OA_SSO_SUN:
			sunEclipticCartesianPosition ( date, posn );
			break;
		default:
			return -1;
	}
	return 0;
}


int
oaEquatorialCartesianPosition ( unsigned int body, struct tm* date,
    cartesian* posn )
{
	int			day;
	double	ecl;

	if ( oaEclipticCartesianPosition ( body, date, posn )) {
		return -1;
	}
	if ( OA_SSO_MOON == body ) {
		return 0;
	}

	day = oaDayNumber ( date );
	ecl = oaEclipticObliquity ( day );
	if ( OA_SSO_SUN == body ) {
    posn->y = posn->y * cosDeg ( ecl );
    posn->z = posn->y * sinDeg ( ecl );
	} else {
		double eccentricity, meanAnomaly, perihelion, eccentricAnomaly, longitude;
		double xv, yv, xs, ys, v, rs;

		eccentricity = orbitalElements[ OA_SSO_SUN ].eccentricityC +
        orbitalElements[ OA_SSO_SUN ].eccentricityM * day;
    meanAnomaly = orbitalElements[ OA_SSO_SUN ].meanAnomalyC +
        orbitalElements[ OA_SSO_SUN ].meanAnomalyM * day;
    perihelion = orbitalElements[ OA_SSO_SUN ].perihelionC +
        orbitalElements[ OA_SSO_SUN ].perihelionM * day;
    eccentricAnomaly = meanAnomaly + eccentricity * ( 180 / M_PI ) *
        sinDeg ( meanAnomaly ) * ( 1.0 + eccentricity *
				cosDeg ( meanAnomaly ));

    xv = cosDeg ( eccentricAnomaly ) - eccentricity;
    yv = sqrt ( 1.0 - eccentricity * eccentricity ) *
				sinDeg ( eccentricAnomaly );
    v = atan2Deg ( yv, xv );
    rs = xv*xv + yv*yv;
    longitude = v + perihelion;
    xs = rs * cosDeg ( longitude );
    ys = rs * sinDeg ( longitude );
		posn->x = posn->x + xs;
		posn->y = ( posn->y + ys ) * cosDeg ( ecl ) - posn->z * sinDeg ( ecl );
		posn->z = ( posn->y + ys ) * sinDeg ( ecl ) - posn->z * cosDeg ( ecl );
	}
  return 0;
}


int
oaRADECPosition ( unsigned int body, struct tm* date,
    radec* posn )
{
	cartesian	coords;

	if ( oaEquatorialCartesianPosition ( body, date, &coords )) {
		return -1;
	}

	posn->RA = atan2Deg ( coords.y, coords.x );
	posn->dec = atan2Deg ( coords.z, sqrt ( coords.x * coords.x +
			coords.y * coords.y ));
  return 0;
}


double
oaApparentEquatorialDiameter ( unsigned int body, struct tm* date )
{
	cartesian coords;
	double		r;

	if ( oaEquatorialCartesianPosition ( body, date, &coords )) {
		return -1;
	}

	r = sqrt ( coords.x * coords.x + coords.y * coords.y + coords.z * coords.z );
  return orbitalElements[ body ].equatorialDia / r;
}
