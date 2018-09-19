/*****************************************************************************
 *
 * trig.c -- Trig functions for degrees
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

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "trig.h"


double
sinDeg ( double theta )
{
	double v = sin ( theta / 180 * M_PI );
	return v;
}


double
cosDeg ( double theta )
{
	double v = cos ( theta / 180 * M_PI );
	return v;
}


double
atan2Deg ( double x, double y )
{
	double v = 180 / M_PI * atan2 ( x, y );
	return v;
}
