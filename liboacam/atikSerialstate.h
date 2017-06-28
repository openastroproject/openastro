/*****************************************************************************
 *
 * atikSerialstate.h -- Atik serial camera state header
 *
 * Copyright 2014,2015,2016 James Fidell (james@openastroproject.org)
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

#ifndef OA_ATIK_SERIAL_STATE_H
#define OA_ATIK_SERIAL_STATE_H

#include <sys/types.h>
#include <pthread.h>

struct atikSerialbuffer {
  void   *start;
  size_t length;
};


struct AtikSerial_STATE;

typedef struct AtikSerial_STATE {
  // camera details
  unsigned long         index;
  int                   cameraType;
  // connection data
  int                   fd;
  libusb_context*       usbContext;
  libusb_device_handle* usbHandle;
#ifdef HAVE_LIBFTDI
  // libftdi connection data
  void*			ftdiContext;
#endif
  // pointers to read/write functions
  int			( *write )( struct AtikSerial_STATE*, const unsigned char*,
                            int );
  int			( *read )( struct AtikSerial_STATE*, unsigned char*, int );
  int			( *readToZero )( struct AtikSerial_STATE*,
                            unsigned char*, unsigned int );
  int			( *readBlock )( struct AtikSerial_STATE*,
                            unsigned char*, int );
  // video mode settings
  // buffering for image transfers
  struct atikSerialbuffer* buffers;
  int                   configuredBuffers;
  unsigned char*        xferBuffer;
  unsigned int          imageBufferLength;
  int			nextBuffer;
  int			buffersFree;
  // camera status
  unsigned int          cameraFlags;
  unsigned int		hardwareType;
  unsigned int		haveFIFO;
  unsigned int		colour;
  unsigned int		droppedFrames;
  uint32_t		ccdReadFlags;
  // camera settings
  unsigned int		binMode;
  unsigned int		horizontalBinMode;
  unsigned int		verticalBinMode;
  unsigned int          xSize;
  unsigned int          ySize;
  // image settings
  unsigned int          maxResolutionX;
  unsigned int          maxResolutionY;
  FRAMESIZES            frameSizes[2];
  unsigned int          pixelSizeX;
  unsigned int          pixelSizeY;
  unsigned int          maxBinningX;
  unsigned int          maxBinningY;
  unsigned int          wellDepth;
  // control values
  unsigned int          currentExposure;
  // thread management
  pthread_t		controllerThread;
  pthread_mutex_t	commandQueueMutex;
  pthread_cond_t	commandComplete;
  pthread_cond_t	commandQueued;
  int			stopControllerThread;

  pthread_t		callbackThread;
  pthread_mutex_t	callbackQueueMutex;
  pthread_cond_t	callbackQueued;
  CALLBACK		frameCallbacks[ OA_CAM_BUFFERS ];
  int			stopCallbackThread;
  // queues for controls and callbacks
  DL_LIST		commandQueue;
  DL_LIST		callbackQueue;
  // streaming
  int			isStreaming;
  CALLBACK		streamingCallback;
} AtikSerial_STATE;

#endif	/* OA_ATIK_SERIAL_STATE_H */
