/*****************************************************************************
 *
 * eccentricity.c -- Calculations of eccentricity for a given body
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

#include "eccentricity.h"
#include "orbitalElements.h"
#include "trig.h"


double
eccentrictyAnomaly ( unsigned int body, struct tm* date )
{
	int				day;
	double		eccentricity, meanAnomaly, anomaly, previous;
	double		delta, prevDelta;

	day = oaDayNumber ( date );
	eccentricity = orbitalElements[ body ].eccentricityC +
		  orbitalElements[ body ].eccentricityM * day;
	meanAnomaly = orbitalElements[ OA_SSO_SUN ].meanAnomalyC +
      orbitalElements[ body ].meanAnomalyM * day;
  anomaly = meanAnomaly + eccentricity * ( 180 / M_PI ) *
		  sinDeg ( meanAnomaly ) * ( 1.0 + eccentricity * cosDeg ( meanAnomaly ));

	if ( anomaly < 0.05 ) {
		return anomaly;
	}

	previous = 0;
	delta = anomaly - previous;
	while ( delta > 0.001 ) {
		previous = anomaly;
		prevDelta = delta;
		anomaly = previous - ( previous -
				eccentricity * 180 / M_PI * sinDeg ( previous ) - meanAnomaly ) /
				( 1 - eccentricity * cosDeg ( previous ));
		delta = anomaly - previous;
		if ( delta > prevDelta ) {
			fprintf ( stderr, "Doesn't look like eccentricity is converging\n" );
			break;
		}
	}

	return anomaly;
}
