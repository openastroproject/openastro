/*****************************************************************************
 *
 * imgstack.h -- stacking functions
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_IMGPROC_STACK_H
#define OPENASTRO_IMGPROC_STACK_H

extern int	oaStackSum8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackSum16LE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackSum16BE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMean8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMean16LE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMean16BE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMedian8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMedian16LE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMedian16BE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMaximum8 ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMaximum16LE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackMaximum16BE ( void**, unsigned int, void*, unsigned int );
extern int	oaStackKappaSigma8 ( void**, unsigned int, void*, unsigned int,
								double );
extern int	oaStackKappaSigma16LE ( void**, unsigned int, void*, unsigned int,
								double );
extern int	oaStackKappaSigma16BE ( void**, unsigned int, void*, unsigned int,
								double );
extern int	oaStackMedianKappaSigma8 ( void**, unsigned int, void*,
								unsigned int, double );
extern int	oaStackMedianKappaSigma16LE ( void**, unsigned int, void*,
								unsigned int, double );
extern int	oaStackMedianKappaSigma16BE ( void**, unsigned int, void*,
								unsigned int, double );

#endif	/* OPENASTRO_IMGPROC_STACK_H */
