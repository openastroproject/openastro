/*****************************************************************************
 *
 * oafwprivate.h -- shared declarations not exposed to the cruel world
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

#ifndef OA_FWHEEL_PRIVATE_H
#define OA_FWHEEL_PRIVATE_H

#ifdef HAVE_LIBFTDI
#if HAVE_FTDI_H
#include <ftdi.h>
#else
#include <libftdi1/ftdi.h>
#endif
#endif
#include <hidapi.h>

#include <openastro/controller.h>

#define OA_CLEAR(x)	memset ( &(x), 0, sizeof ( x ))

typedef struct {
  oaFilterWheelDevice**	wheelList;
  unsigned int          numFilterWheels;
  unsigned int          maxFilterWheels;
} FILTERWHEEL_LIST;

typedef struct {
  int                   initialised;
  int                   index;
  char			devicePath[ PATH_MAX+1 ];
  int			fd;
  hid_device*		hidHandle;
#ifdef HAVE_LIBFTDI
  struct ftdi_context*	ftdiContext;
#endif
  pthread_mutex_t       ioMutex;
  int			wheelType;
  int			currentPosition;
  int			numSlots;
  int			currentSpeed;
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
  // queues for controls and callbacks
  DL_LIST               commandQueue;
  DL_LIST               callbackQueue;

} PRIVATE_INFO;

extern void		oafwSetDebugLevel ( int );
extern void		oafwClearDebugLevel ( int );
extern void		oafwAddDebugLevel ( int );
extern void		oafwDebugMsg ( int, const char*, ... );
extern int		_oaCheckFilterWheelArraySize ( FILTERWHEEL_LIST* );
extern void		_oaFreeFilterWheelDeviceList ( FILTERWHEEL_LIST* );

extern int		oaWheelSetControl ( oaFilterWheel*, int, oaControlValue* );
extern int		oaWheelReadControl ( oaFilterWheel*, int, oaControlValue* );
extern int		oaWheelTestControl ( oaFilterWheel*, int, oaControlValue* );

extern void*	oafwCallbackHandler ( void* );

#endif /* OA_FWHEEL_PRIVATE_H */
