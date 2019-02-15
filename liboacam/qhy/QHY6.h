/*****************************************************************************
 *
 * QHY6.h -- header for QHY6-specific control
 *
 * Copyright 2014 James Fidell (james@openastroproject.org)
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

#ifndef OA_QHY6_H
#define OA_QHY6_H

extern int		_QHY6InitCamera ( oaCamera* );
extern void*		oacamQHY6controller ( void* );
extern void		oaQHY6RecalculateSizes ( QHY_STATE* );

#define QHY6_SENSOR_WIDTH	800
#define QHY6_SENSOR_HEIGHT	596
#define QHY6_OFFSET_X		120

#define	QHY6_DEFAULT_SPEED	1
#define	QHY6_DEFAULT_GAIN	0
#define QHY6_DEFAULT_EXPOSURE	500

#define QHY6_AMP_MODE_OFF	0
#define QHY6_AMP_MODE_ON	1
#define QHY6_AMP_MODE_AUTO	2

#endif	/* OA_QHY6_H */
