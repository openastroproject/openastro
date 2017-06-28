/*****************************************************************************
 *
 * QHY5.h -- header for QHY5-specific control
 *
 * Copyright 2013,2014,2015 James Fidell (james@openastroproject.org)
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

#ifndef OA_QHY5_H
#define OA_QHY5_H

extern int		_QHY5InitCamera ( oaCamera* );
extern void*		oacamQHY5controller ( void* );

#define QHY5_SENSOR_WIDTH	1558
#define QHY5_SENSOR_HEIGHT	1048
#define QHY5_IMAGE_WIDTH	1280
#define QHY5_IMAGE_HEIGHT	1024
#define QHY5_VBLANK		26
#define QHY5_DARK_WIDTH_X	36
#define QHY5_PIXEL_RATE		24000000

#define QHY5_BUFFER_SIZE	19

#endif	/* OA_QHY5_H */
