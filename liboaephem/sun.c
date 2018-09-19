/*****************************************************************************
 *
 * sun.c -- Calculations for the Sun
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

#include "sun.h"
#include "orbitalElements.h"
#include "trig.h"


void
sunEclipticCartesianPosition ( struct tm* date, cartesian* posn )
{
	int				day;
	double		eccentricity, meanAnomaly, eccentricAnomaly, longitude;
	double		perihelion, xv, yv, v, r;

	day = oaDayNumber ( date );
	eccentricity = orbitalElements[ OA_SSO_SUN ].eccentricityC +
		  orbitalElements[ OA_SSO_SUN ].eccentricityM * day;
	meanAnomaly = orbitalElements[ OA_SSO_SUN ].meanAnomalyC +
      orbitalElements[ OA_SSO_SUN ].meanAnomalyM * day;
	perihelion = orbitalElements[ OA_SSO_SUN ].perihelionC +
      orbitalElements[ OA_SSO_SUN ].perihelionM * day;
  eccentricAnomaly = meanAnomaly + eccentricity * ( 180 / M_PI ) *
		  sinDeg ( meanAnomaly ) * ( 1.0 + eccentricity * cosDeg ( meanAnomaly ));

  xv = cosDeg ( eccentricAnomaly ) - eccentricity;
  yv = sqrt ( 1.0 - eccentricity * eccentricity ) * sinDeg ( eccentricAnomaly );
	v = atan2Deg ( yv, xv );
  r = sqrt( xv*xv + yv*yv );
  longitude = v + perihelion;
	posn->x = r * cosDeg ( longitude );
	posn->y = r * sinDeg ( longitude );
	posn->z = 0;
}
