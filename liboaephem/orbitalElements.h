/*****************************************************************************
 *
 * orbitalElements.h -- header for orbital elements data
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

#ifndef	OA_EPHEM_ORBITAL_ELEMENTS
#define	OA_EPHEM_ORBITAL_ELEMENTS

typedef struct {
	double	longitudeM;
	double	longitudeC;
	double	inclinationM;
	double	inclinationC;
	double	perihelionM;
	double	perihelionC;
	double	semiMajorAxisM;
	double	semiMajorAxisC;
	double	eccentricityM;
	double	eccentricityC;
	double	meanAnomalyM;
	double	meanAnomalyC;
	double	equatorialDia;
	double	polarDia;
} orbitalElement;

extern orbitalElement	orbitalElements[OA_EPHEM_NUM_SSO+1];

void eclipticCartesianPosition ( unsigned int, struct tm*, cartesian* posn );

#endif
