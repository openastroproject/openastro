/*****************************************************************************
 *
 * atikSerialstate.h -- Atik serial camera state header
 *
 * Copyright 2014,2015,2016,2019 James Fidell (james@openastroproject.org)
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

#include "sharedState.h"


typedef struct AtikSerial_STATE {

#include "sharedDecs.h"

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
  frameBuffer*		buffers;
  // camera status
  unsigned int    cameraFlags;
  unsigned int		hardwareType;
  unsigned int		haveFIFO;
  unsigned int		colour;
  unsigned int		droppedFrames;
  uint32_t		ccdReadFlags;
  // camera settings
  unsigned int		binMode;
  unsigned int		horizontalBinMode;
  unsigned int		verticalBinMode;
  // image settings
  unsigned int          pixelSizeX;
  unsigned int          pixelSizeY;
  unsigned int          maxBinningX;
  unsigned int          maxBinningY;
  unsigned int          wellDepth;
  // control values
  unsigned int          currentExposure;
} AtikSerial_STATE;

#endif	/* OA_ATIK_SERIAL_STATE_H */
