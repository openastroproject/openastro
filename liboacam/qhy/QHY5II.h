/*****************************************************************************
 *
 * QHY5II.h -- header for QHY5II-specific control
 *
 * Copyright 2014,2015 James Fidell (james@openastroproject.org)
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

#ifndef OA_QHY5II_H
#define OA_QHY5II_H

extern int		_QHY5IIInitCamera ( oaCamera* );
extern void*		oacamQHY5IIcontroller ( void* );
extern void		oaQHY5IISetAllControls ( QHY_STATE* );

#define QHY5II_IMAGE_WIDTH		1280
#define QHY5II_IMAGE_HEIGHT		1024
#define QHY5II_DEFAULT_EXPOSURE		20
#define QHY5II_DEFAULT_SPEED		0
#define QHY5II_DEFAULT_USBTRAFFIC	30
#define QHY5II_EOF_LEN			5

#define QHY5II_MONO_GAIN_MIN		0
#define QHY5II_MONO_GAIN_MAX		100

#define	 MT9M001_CHIP_VERSION			0x00
#define	 MT9M001_ROW_START			0x01
#define	 MT9M001_COLUMN_START			0x02
#define	 MT9M001_ROW_SIZE			0x03
#define	 MT9M001_COLUMN_SIZE			0x04
#define	 MT9M001_HORIZ_BLANKING			0x05
#define	 MT9M001_VERT_BLANKING			0x06
#define	 MT9M001_OUTPUT_CONTROL			0x07
#define	 MT9M001_SHUTTER_WIDTH			0x09
#define	 MT9M001_RESTART			0x0B
#define	 MT9M001_SHUTTER_DELAY			0x0C
#define	 MT9M001_RESET				0x0D
#define	 MT9M001_READ_OPTIONS_1			0x1E
#define	 MT9M001_READ_OPTIONS_2			0x20
#define	 MT9M001_GAIN_EVEN_ROW_EVEN_COL		0x2B
#define	 MT9M001_GAIN_ODD_ROW_EVEN_COL		0x2C
#define	 MT9M001_GAIN_EVEN_ROW_ODD_COL		0x2D
#define	 MT9M001_GAIN_ODD_ROW_ODD_COL		0x2E
#define	 MT9M001_GLOBAL_GAIN			0x35
#define	 MT9M001_CAL_THRESHOLD			0x5F
#define	 MT9M001_BLACK_EVEN_ROW_EVEN_COL	0x60
#define	 MT9M001_BLACK_ODD_ROW_EVEN_COL		0x61
#define	 MT9M001_CAL_CONTROL			0x62
#define	 MT9M001_BLACK_EVEN_ROW_ODD_COL		0x63
#define	 MT9M001_BLACK_ODD_ROW_ODD_COL		0x64
#define	 MT9M001_CHIP_ENABLE			0xF1

#endif	/* OA_QHY5II_H */
