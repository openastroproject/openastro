/*****************************************************************************
 *
 * imgproc.h -- image processing functions header
 *
 * Copyright 2015, 2019 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_IMGPROC_H
#define OPENASTRO_IMGPROC_H

extern int	oaFocusScore ( void*, void*, int, int, int );

extern int	oaStackSum8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMean8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMedian8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMaximum8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackKappaSigma8 ( void**, unsigned int, void*, unsigned int,
								double );
extern int	oaStackMedianKappaSigma8 ( void**, unsigned int, void*,
								unsigned int, double );

extern int	oaContrastTransform ( void*, void*, int, int, int, int );

extern int		oaclamp ( int, int, int );
extern double	oadclamp ( double, double, double );

#endif	/* OPENASTRO_IMGPROC_H */
