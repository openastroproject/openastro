/*****************************************************************************
 *
 * demosaic.h -- demosaic API header
 *
 * Copyright 2013,2014,2018 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_DEMOSAIC_H
#define OPENASTRO_DEMOSAIC_H

#define OA_DEMOSAIC_RGGB	1
#define OA_DEMOSAIC_BGGR	2
#define OA_DEMOSAIC_GRBG	3
#define OA_DEMOSAIC_GBRG	4
#define OA_DEMOSAIC_CMYG	5
#define OA_DEMOSAIC_MCGY	6
#define OA_DEMOSAIC_YGCM	7
#define OA_DEMOSAIC_GYMC	8
#define OA_DEMOSAIC_AUTO	9

#define	OA_DEMOSAIC_NEAREST_NEIGHBOUR	1
#define	OA_DEMOSAIC_NEAREST_NEIGHBOR	OA_DEMOSAIC_NEAREST_NEIGHBOUR
#define	OA_DEMOSAIC_BILINEAR		2
#define	OA_DEMOSAIC_SMOOTH_HUE		3
#define	OA_DEMOSAIC_VNG			4
#define OA_DEMOSAIC_LAST_P1		( OA_DEMOSAIC_VNG + 1 )

extern int		oademosaic ( void*, void*, int, int, int, int, int );
extern const char*	oademosaicMethodName ( int );

#endif	/* OPENASTRO_DEMOSAIC_H */
