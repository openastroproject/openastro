/*****************************************************************************
 *
 * oaptrprivate.h -- shared declarations not exposed to the cruel world
 *
 * Copyright 2015,2016,2017,2018,2019
 *   James Fidell (james@openastroproject.org)
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

#ifndef OA_PTR_PRIVATE_H
#define OA_PTR_PRIVATE_H

#include <openastro/controller.h>
#include <openastro/timer.h>

#define OA_CLEAR(x)	memset ( &(x), 0, sizeof ( x ))

#define OA_TIMESTAMP_BUFFERS	128

typedef struct {
  oaPTRDevice**         ptrList;
  unsigned int          numPTRDevices;
  unsigned int          maxPTRDevices;
} PTR_LIST;

typedef struct {
  int64_t               min[ OA_TIMER_CTRL_LAST_P1 ];
  int64_t               max[ OA_TIMER_CTRL_LAST_P1 ];
  int64_t               step[ OA_TIMER_CTRL_LAST_P1 ];
  int64_t               def[ OA_TIMER_CTRL_LAST_P1 ];
} COMMON_INFO;

typedef struct {
  int                   initialised;
  int                   index;
  char			devicePath[ PATH_MAX+1 ];
  int			fd;
  pthread_mutex_t       ioMutex;
  uint32_t		majorVersion;
  uint32_t		minorVersion;
  uint32_t		version;
  double		longitude;
  double		latitude;
  double		altitude;
	int				validGPS;
  // timer configuration
  int32_t		requestedCount;
  int32_t		requestedInterval;
  int32_t		requestedMode;
	int8_t		externalLEDState;
  // thread management
  pthread_t             controllerThread;
  pthread_mutex_t       commandQueueMutex;
  pthread_cond_t        commandComplete;
  pthread_cond_t        commandQueued;
  int                   stopControllerThread;

  pthread_t             callbackThread;
  pthread_mutex_t       callbackQueueMutex;
  pthread_cond_t        callbackQueued;
  int                   stopCallbackThread;
  // buffers for timestamps
  int			timestampsAvailable;
  int			timestampExpected;
  int			timestampCountdown;
  int			firstTimestamp;
  oaTimerStamp		timestampBuffer[ OA_TIMESTAMP_BUFFERS ];
  // queues for controls and callbacks
  DL_LIST               commandQueue;
  DL_LIST               callbackQueue;
  // running
  int			isRunning;
  CALLBACK		timestampCallback;
} PRIVATE_INFO;

extern void*		oaPTRcontroller ( void* );
extern void*		oaPTRcallbackHandler ( void* );

extern void		oaptrSetDebugLevel ( int );
extern void		oaptrClearDebugLevel ( int );
extern void		oaptrAddDebugLevel ( int );
extern void		oaptrDebugMsg ( int, const char*, ... );
extern int		_oaCheckPTRArraySize ( PTR_LIST* );
extern void		_oaFreePTRDeviceList ( PTR_LIST* );

#define PTR_TIMESTAMP_BUFFER_LEN_V1_0 28
#define PTR_TIMESTAMP_BUFFER_LEN_V1_1 32
#define PTR_TIMESTAMP_BUFFER_LEN_V2   64

#endif /* OA_PTR_PRIVATE_H */
