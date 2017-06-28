/*****************************************************************************
 *
 * scharr.h -- Scharr convolution routines
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_IMGPROC_SCHARR_H
#define OPENASTRO_IMGPROC_SCHARR_H

extern unsigned long	scharr8 ( uint8_t*, uint8_t*, int, int );
extern unsigned long	scharr16 ( uint16_t*, uint16_t*, int, int );

#endif	/* OPENASTRO_IMGPROC_SCHARR_H */
