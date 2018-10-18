/*****************************************************************************
 *
 * ptr.h -- PTR API header
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

#ifndef OPENASTRO_PTR_H
#define OPENASTRO_PTR_H

#include <limits.h>

#include <openastro/openastro.h>
#include <openastro/userConfig.h>
#include <openastro/ptr/controls.h>
#include <openastro/timer/controls.h>
#include <openastro/timer/features.h>


struct oaPTR;
struct oaPTRDevice;

typedef struct oaPTRFuncs {
  struct oaPTR*		( *init )( struct oaPTRDevice* );
  int			( *close )( struct oaPTR* );
  int			( *reset )( struct oaPTR* );
  int			( *start )( struct oaPTR* );
  int			( *stop )( struct oaPTR* );
  int			( *isRunning )( struct oaPTR* );
  int			( *readControl )( struct oaPTR*, int,
				oaControlValue* );
  int			( *setControl )( struct oaPTR*, int,
				oaControlValue* );
  int			( *testControl )( struct oaPTR*, int,
				oaControlValue* );
  int			( *getControlRange )( struct oaPTR*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
  int			( *readTimestamp )( struct oaPTR*, int, oaTimerStamp* );
  int			( *readGPS )( struct oaPTR*, double* );
  int			( *readCachedGPS )( struct oaPTR*, double* );
} oaPTRFuncs;

typedef struct oaPTR {
  char			deviceName[OA_MAX_NAME_LEN+1];
  oaPTRFuncs		funcs;
  uint8_t		controls[OA_TIMER_CTRL_LAST_P1];
  oaTimerFeatures	features;
  void*			_common;
  void*			_private;
} oaPTR;


typedef struct oaPTRDevice {
  char			deviceName[OA_MAX_NAME_LEN+1];
  oaPTR*		( *init )( struct oaPTRDevice* );
  void*			_private;
} oaPTRDevice;


extern int		oaGetPTRDevices ( oaPTRDevice*** );
extern void		oaReleasePTRDevices ( oaPTRDevice** );
extern unsigned		oaGetPTRAPIVersion ( void );
extern const char*	oaGetPTRAPIVersionStr ( void );
extern void		oaSetPTRDebugLevel ( int );
extern unsigned int	oaGetPTRConfigFlags ( void );
extern void		oaClearPTRIDFilters ( void );
extern void		oaAddPTRIDFilter ( userDeviceConfig* );

#endif	/* OPENASTRO_PTR_H */
