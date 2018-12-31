/*****************************************************************************
 *
 * xagylfw.h -- header for Xagyl filter wheel API
 *
 * Copyright 2014,2015,2017,2018 James Fidell (james@openastroproject.org)
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

#ifndef OA_XAGYL_FW_H
#define OA_XAGYL_FW_H

#ifndef HAVE_LIBUDEV
#ifdef HAVE_FTDI_H
#include <ftdi.h>
#endif
#ifdef HAVE_LIBFTDI1_FTDI_H
#include <libftdi1/ftdi.h>
#endif
#ifdef HAVE_LIBFTDI_FTDI_H
#include <libftdi/ftdi.h>
#endif
#endif

extern void*		oafwXagylcontroller ( void* );

extern int		oaXagylGetFilterWheels ( FILTERWHEEL_LIST* );
extern oaFilterWheel*   oaXagylInitFilterWheel ( oaFilterWheelDevice* );
extern int		oaXagylWheelClose ( oaFilterWheel* );

extern int              oaXagylWheelGetControlRange ( oaFilterWheel*, int,
                                int64_t*, int64_t*, int64_t*, int64_t* );

extern void		oaXagylClearIDFilters ( void );
extern void		oaXagylAddIDFilter ( userDeviceConfig* );

extern int		oaXagylWheelWarmReset ( PRIVATE_INFO*, int );
extern int		oaXagylWheelColdReset ( PRIVATE_INFO*, int );
extern int		oaXagylWheelDoReset ( PRIVATE_INFO*, const char*, int );
extern int		oaXagylMoveTo ( PRIVATE_INFO*, int, int );
extern int		oaXagylSetWheelSpeed ( PRIVATE_INFO*, unsigned int,
				int );

#ifdef HAVE_LIBUDEV
extern int		_xagylWheelWrite ( int, const char*, int );
extern int		_xagylWheelRead ( int, char*, int );
#else
extern int		_xagylWheelWrite ( struct ftdi_context*, const char*, int );
extern int		_xagylWheelRead ( struct ftdi_context*, char*, int );
#endif

extern userDeviceConfig* xagylConfig;
extern int		xagylConfigEntries;

#define	XAGYL_VID		0x0403
#define	XAGYL_FILTERWHEEL_PID1	0x6001
#define	XAGYL_FILTERWHEEL_PID2	0x6015

#define	FW_UNKNOWN		0
#define	XAGYL_5125		1
#define	XAGYL_8125		2
#define	XAGYL_5200		3

#define XAGYL_DEFAULT_SPEED	100

#endif	/* OA_XAGYL_FW_H */

