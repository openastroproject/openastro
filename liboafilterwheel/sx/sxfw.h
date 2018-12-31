/*****************************************************************************
 *
 * sxfw.h -- header for Starlight Xpress filter wheel API
 *
 * Copyright 2014,2015,2018 James Fidell (james@openastroproject.org)
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

#ifndef OA_SX_FW_H
#define OA_SX_FW_H

extern void*		oafwSXcontroller ( void* );

extern int		oaSXGetFilterWheels ( FILTERWHEEL_LIST* );
extern oaFilterWheel*   oaSXInitFilterWheel ( oaFilterWheelDevice* );
extern int		oaSXWheelClose ( oaFilterWheel* );

extern int              oaSXWheelGetControlRange ( oaFilterWheel*, int,
                                int64_t*, int64_t*, int64_t*, int64_t* );

extern void		oaSXClearIDFilters ( void );
extern void		oaSXAddIDFilter ( userDeviceConfig* );

extern int		_sxWheelWrite ( PRIVATE_INFO*, unsigned char* );
extern int		_sxWheelRead ( PRIVATE_INFO*, unsigned char* );

#define	STARLIGHT_XPRESS_VID			0x1278
#define STARLIGHT_XPRESS_FILTERWHEEL_PID	0x0920

#define FW_UNKNOWN      0
#define SX_UNIVERSAL    1
#define SX_MINI         2

#endif	/* OA_SX_FW_H */
