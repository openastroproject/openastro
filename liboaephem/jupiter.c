/*****************************************************************************
 *
 * jupiter.c -- Calculations for Jupiter
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

#include "jupiter.h"
#include "orbitalElements.h"
#include "eccentricity.h"
#include "trig.h"


void
jupiterEclipticCartesianPosition ( struct tm* date, cartesian* posn )
{
	int			day;
	double	eclipticLat, eclipticLong, Mj, Ms, r;

	day = oaDayNumber ( date );

	eclipticCartesianPosition ( OA_SSO_JUPITER, date, posn );

	eclipticLat = atan2Deg ( posn->z, sqrt ( posn->x * posn->x +
			posn->y * posn->y ));
	eclipticLong = atan2Deg ( posn->y, posn->x );

	Mj = orbitalElements[ OA_SSO_JUPITER ].meanAnomalyC +
      orbitalElements[ OA_SSO_JUPITER ].meanAnomalyM * day;
	Ms = orbitalElements[ OA_SSO_SATURN ].meanAnomalyC +
      orbitalElements[ OA_SSO_SATURN ].meanAnomalyM * day;

	eclipticLong += -0.332 * sinDeg ( 2 * Mj - 5 * Ms - 67.6 );
	eclipticLong += -0.056 * sinDeg ( 2 * Mj - 2 * Ms + 21 );
	eclipticLong += 0.042 * sinDeg ( 3 * Mj - 5 * Ms + 21 );
	eclipticLong += -0.036 * sinDeg ( Mj - 2 * Ms );
	eclipticLong += 0.022 * cosDeg( Mj - Ms );
	eclipticLong += 0.023 * sinDeg ( 2 * Mj - 3 * Ms + 52 );
	eclipticLong += -0.016 * sinDeg ( Mj - 5 * Ms - 69 );

	r = sqrt ( posn->x * posn->x + posn->y * posn->y + posn->z * posn->z );
	posn->x = r * cosDeg ( eclipticLong ) * cosDeg ( eclipticLat );
	posn->y = r * sinDeg ( eclipticLong ) * cosDeg ( eclipticLat );
	posn->z = r * sinDeg ( eclipticLat );
}
