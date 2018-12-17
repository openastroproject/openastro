/*****************************************************************************
 *
 * targets.h -- names of targets
 *
 * Copyright 2013,2014 James Fidell (james@openastroproject.org)
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

#pragma once

#define TGT_UNKNOWN	0
#define	TGT_MERCURY	1
#define	TGT_VENUS	2
#define	TGT_EARTH	3
#define	TGT_MARS	4
#define TGT_JUPITER	5
#define	TGT_SATURN	6
#define	TGT_URANUS	7
#define	TGT_NEPTUME	8
#define TGT_PLUTO	9
#define TGT_MOON	10
#define TGT_SUN		11
#define NUM_TARGETS	(TGT_SUN+1)

extern const QString targetName ( unsigned int );
