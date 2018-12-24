/*****************************************************************************
 *
 * filterwheel.h -- Filter Wheel API header
 *
 * Copyright 2014, 2018 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_FILTERWHEEL_H
#define OPENASTRO_FILTERWHEEL_H

#include <limits.h>

#include <openastro/openastro.h>
#include <openastro/userConfig.h>
#include <openastro/filterwheel/controls.h>
#include <openastro/filterwheel/features.h>

#define MAX_FILTER_SLOTS	10

enum oaFilterWheelInterfaceType {
  OA_FW_IF_XAGYL			= 1,
  OA_FW_IF_SX				  = 2,
  OA_FW_IF_ZWO				= 3,
  OA_FW_IF_BRIGHTSTAR	= 4,
  OA_FW_IF_COUNT			= 5
};

extern oaInterface	oaFilterWheelInterfaces[ OA_FW_IF_COUNT + 1 ];

struct oaFilterWheel;
struct oaFilterWheelDevice;

typedef struct oaFilterWheelFuncs {
  struct oaFilterWheel* ( *initWheel )( struct oaFilterWheelDevice* );
  int			( *closeWheel )( struct oaFilterWheel* );
  int			( *readControl )( struct oaFilterWheel*, int,
				oaControlValue* );
  int			( *setControl )( struct oaFilterWheel*, int,
				oaControlValue* );
  int			( *testControl )( struct oaFilterWheel*, int,
				oaControlValue* );
  int			( *getControlRange )( struct oaFilterWheel*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
} oaFilterWheelFuncs;

typedef struct oaFilterWheel {
  enum oaFilterWheelInterfaceType  interface;
  char			deviceName[OA_MAX_NAME_LEN+1];
  oaFilterWheelFuncs	funcs;
  uint8_t		controls[OA_FW_CTRL_LAST_P1];
  // oaFilterWheelFeatures	features;
  int			numSlots;
  void*			_common;
  void*			_private;
} oaFilterWheel;


typedef struct oaFilterWheelDevice {
  enum oaFilterWheelInterfaceType	interface;
  char				deviceName[OA_MAX_NAME_LEN+1];
  oaFilterWheel*		( *initFilterWheel )( struct oaFilterWheelDevice* );
  void*				_private;
} oaFilterWheelDevice;


extern int		oaGetFilterWheels ( oaFilterWheelDevice*** );
extern void		oaReleaseFilterWheels ( oaFilterWheelDevice** );
extern unsigned		oaGetFilterWheelAPIVersion ( void );
extern const char*	oaGetFilterWheelAPIVersionStr ( void );
extern void		oaSetFilterWheelDebugLevel ( int );
extern unsigned int	oaGetFilterWheelConfigFlags ( int );
extern void		oaClearFilterWheelIDFilters ( int );
extern void		oaAddFilterWheelIDFilter ( int, userDeviceConfig* );

#endif	/* OPENASTRO_FILTERWHEEL_H */
