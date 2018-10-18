/*****************************************************************************
 *
 * ptr.h -- header for PTR API
 *
 * Copyright 2015,2016,2017,2018 James Fidell (james@openastroproject.org)
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

#ifndef OA_PTR_H
#define OA_PTR_H

extern void*		oaptrPTRcontroller ( void* );
extern void*		oaptrPTRcallbackHandler ( void* );

extern int		oaPTREnumerate ( PTR_LIST* );
extern oaPTR*           oaPTRInit ( oaPTRDevice* );
extern int		oaPTRClose ( oaPTR* );
extern int		oaPTRReset ( oaPTR* );
extern int		oaPTRStart ( oaPTR* );
extern int		oaPTRStop ( oaPTR* );
extern int		oaPTRIsRunning ( oaPTR* );

extern int              oaPTRTestControl ( oaPTR*, int,
                                oaControlValue* );
extern int              oaPTRSetControl ( oaPTR*, int,
                                oaControlValue* );
extern int              oaPTRReadControl ( oaPTR*, int,
                                oaControlValue* );
extern int              oaPTRGetTimestamp ( oaPTR*, int, oaTimerStamp* );
extern int              oaPTRReadGPS ( oaPTR*, double* );
extern int              oaPTRReadCachedGPS ( oaPTR*, double* );

extern void		oaPTRClearIDFilters ( void );
extern void		oaPTRAddIDFilter ( userDeviceConfig* );

extern int		_ptrWrite ( int, const char*, int );
extern int		_ptrRead ( int, char*, int );

extern userDeviceConfig* ptrConfig;
extern int		ptrConfigEntries;

#define	PTR_VID		0x0403
#define	PTR_PID		0x6001

#endif	/* OA_PTR_H */
