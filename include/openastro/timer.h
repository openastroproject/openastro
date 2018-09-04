/*****************************************************************************
 *
 * timer.h -- Timer API header
 *
 * Copyright 2015,2016,2018 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_TIMER_H
#define OPENASTRO_TIMER_H

typedef struct {
  char		timestamp[64];
  uint32_t	index;
} oaTimerStamp;

#include <openastro/openastro.h>
#include <openastro/userConfig.h>
#include <openastro/ptr.h>

// For the moment this just creates aliases for the PTR device
// types

typedef oaPTRFuncs oaTimerFuncs;
typedef oaPTR oaTimer;
typedef oaPTRDevice oaTimerDevice;

extern int		oaGetTimerDevices ( oaTimerDevice*** );
extern int		oaReleaseTimerDevices ( oaTimerDevice** );
extern unsigned		oaGetTimerAPIVersion ( void );
extern const char*	oaGetTimerAPIVersionStr ( void );
extern void		oaSetTimerDebugLevel ( int );
extern unsigned int	oaGetTimerConfigFlags ( void );
extern void		oaClearTimerIDFilters ( void );
extern void		oaAddTimerIDFilter ( userDeviceConfig* );

#define	OA_TIMER_IF_COUNT	1

#define OA_TIMER_MODE_UNSET	0
#define OA_TIMER_MODE_TRIGGER	1
#define OA_TIMER_MODE_STROBE	2

#endif	/* OPENASTRO_TIMER_H */
