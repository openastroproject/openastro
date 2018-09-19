/*****************************************************************************
 *
 * saturn.c -- Calculations for Saturn
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

#include "saturn.h"
#include "orbitalElements.h"
#include "eccentricity.h"
#include "trig.h"


void
saturnEclipticCartesianPosition ( struct tm* date, cartesian* posn )
{
	int			day;
  double	eclipticLat, eclipticLong, Mj, Ms, r;

	day = oaDayNumber ( date );

	eclipticCartesianPosition ( OA_SSO_SATURN, date, posn );

  eclipticLat = atan2Deg ( posn->z, sqrt ( posn->x * posn->x +
      posn->y * posn->y ));
  eclipticLong = atan2Deg ( posn->y, posn->x );

  Mj = orbitalElements[ OA_SSO_JUPITER ].meanAnomalyC +
      orbitalElements[ OA_SSO_JUPITER ].meanAnomalyM * day;
  Ms = orbitalElements[ OA_SSO_SATURN ].meanAnomalyC +
      orbitalElements[ OA_SSO_SATURN ].meanAnomalyM * day;

  eclipticLong += 0.812 * sinDeg ( 2 * Mj - 5 * Ms - 67.6 );
  eclipticLong += -0.229 * cosDeg ( 2 * Mj - 4 * Ms - 2 );
  eclipticLong += +0.119 * sinDeg ( Mj - 2 * Ms - 3 );
  eclipticLong += 0.046 * sinDeg ( 2 * Mj - 6 * Ms - 69 );
  eclipticLong += 0.014 * sinDeg ( Mj - 3 * Ms + 32 );

  eclipticLat += -0.020 * cosDeg ( 2 * Mj - 4 * Ms - 2 );
  eclipticLat += +0.018 * sinDeg ( 2 * Mj - 6 * Ms - 49 );

	r = sqrt ( posn->x * posn->x + posn->y * posn->y + posn->z * posn->z );
	posn->x = r * cosDeg ( eclipticLong ) * cosDeg ( eclipticLat );
	posn->y = r * sinDeg ( eclipticLong ) * cosDeg ( eclipticLat );
	posn->z = r * sinDeg ( eclipticLat );
}
