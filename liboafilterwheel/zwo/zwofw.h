/*****************************************************************************
 *
 * zwofw.h -- header for ZWO filter wheel API
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_ZWO_FW_H
#define OA_ZWO_FW_H

extern void*		oafwZWOcontroller ( void* );

extern int		oaZWOGetFilterWheels ( FILTERWHEEL_LIST* );
extern oaFilterWheel*   oaZWOInitFilterWheel ( oaFilterWheelDevice* );
extern int		oaZWOWheelClose ( oaFilterWheel* );

extern int              oaZWOWheelGetControlRange ( oaFilterWheel*, int,
                                int64_t*, int64_t*, int64_t*, int64_t* );

extern void		oaZWOClearIDFilters ( void );
extern void		oaZWOAddIDFilter ( userDeviceConfig* );

extern int		_zwoWheelWrite ( PRIVATE_INFO*, unsigned char* );
extern int		_zwoWheelRead ( PRIVATE_INFO*, unsigned char* );

#define FW_UNKNOWN      0
#define ZWO_UNIVERSAL    1
#define ZWO_MINI         2

#endif	/* OA_ZWO_FW_H */
