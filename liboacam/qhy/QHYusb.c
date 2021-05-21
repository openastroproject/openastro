/*****************************************************************************
 *
 * QHYusb.c -- USB interface for QHY cameras
 *
 * Copyright 2013,2014,2015,2017,2021
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

#include <oa_common.h>
#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/errno.h>

#include "oacamprivate.h"
#include "QHY.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHYusb.h"


int
_usbControlMsg ( QHY_STATE* cameraInfo, uint8_t reqType, uint8_t req,
    uint16_t value, uint16_t index, unsigned char* data, uint16_t length,
    unsigned int timeout )
{
  int		ret;

  oaLogDebug ( OA_LOG_CAMERA,
			"%s ( %p, %d, 0x%x, %d, %d, %0lx, %d, %d ): entered", __func__,
			cameraInfo, reqType, req, value, index, ( unsigned long ) data, length,
			timeout );

  pthread_mutex_lock ( &cameraInfo->usbMutex );

  ret = libusb_control_transfer ( cameraInfo->usbHandle, reqType, req, value,
      index, data, length, timeout );
  pthread_mutex_unlock ( &cameraInfo->usbMutex );
  if ( ret != length ) {
    oaLogError ( OA_LOG_CAMERA, "%s: libusb control error: %d", __func__, ret );
    oaLogError ( OA_LOG_CAMERA, "%s: %s", __func__, libusb_error_name ( ret ));
  }

  oaLogDebug ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return ret;
}


int
_usbBulkTransfer ( QHY_STATE* cameraInfo, unsigned char endpoint,
    unsigned char* data, int length, unsigned int* transferred,
    unsigned int timeout )
{
  int		ret;

  pthread_mutex_lock ( &cameraInfo->usbMutex );

	oaLogDebug ( OA_LOG_CAMERA,
			"%s ( %p, 0x%x, %p, %d, %p, %d ): entered", __func__,
			cameraInfo, endpoint, data, length, transferred, timeout );

  ret = libusb_bulk_transfer ( cameraInfo->usbHandle, endpoint, data, length,
      ( int* ) transferred, timeout );
  pthread_mutex_unlock ( &cameraInfo->usbMutex );
  if ( ret ) {
    oaLogWarning ( OA_LOG_CAMERA, "%s: libusb bulk error: %d", __func__, ret );
    oaLogWarning ( OA_LOG_CAMERA, "%s: %s", __func__,
				libusb_error_name ( ret ));
  }

  oaLogDebug ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return ret;
}


unsigned short
_i2cRead16 ( QHY_STATE* cameraInfo, unsigned short address )
{
  unsigned char data[2];

	oaLogDebug ( OA_LOG_CAMERA, "%s ( %p, %d ): entered", __func__, cameraInfo,
			address );

  _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_IN, 0xb7, 0, address, data,
      2, 0 );

  oaLogDebug ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return data[0] * 256 + data[1];
}


int
_i2cWrite16 ( QHY_STATE* cameraInfo, unsigned short address,
    unsigned short value )
{
  unsigned char data[2];

	oaLogDebug ( OA_LOG_CAMERA, "%s ( %p, %d, %d ): entered", __func__,
			cameraInfo, address, data );

  data[0] = value >> 8;
  data[1] = value & 0xff;

  oaLogDebug ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return ( _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xbb, 0, address,
      data, 2, 0 ) == 2 ? 0 : -1 );
}


int
_i2cWriteIMX035 ( QHY_STATE* cameraInfo, unsigned char address,
    unsigned char value )
{
  unsigned char data[32];

	oaLogDebug ( OA_LOG_CAMERA, "%s ( %p, %d, %d ): entered", __func__,
			cameraInfo, address, data );

  memset ( data, 0, 32 );
  data[1] = address;
  data[2] = value;

  oaLogDebug ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return ( _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xb8, 0, 0,
      data, 0x13, 3000 ) == 0x13 ? 0 : -1 );
}


void
qhyStatusCallback ( struct libusb_transfer* transfer )
{
  int           resubmit = 1;

  switch ( transfer->status ) {
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_NO_DEVICE:
      oaLogWarning ( OA_LOG_CAMERA,
					"%s: not processing/resubmitting status xfer: err = %d", __func__,
					transfer->status );
      return;
      break;

    case LIBUSB_TRANSFER_CANCELLED:
      // FIX ME -- I can't get this to work without causing a crash, but
      // things seem to work ok for the moment without it.  Needs more
      // investigation
/*
      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      if ( cameraInfo->statusTransfer ) {
        free ( cameraInfo->statusBuffer );
        libusb_free_transfer ( transfer );
        cameraInfo->statusTransfer = 0;
      }
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
      resubmit = 0;
*/
      break;

    case LIBUSB_TRANSFER_COMPLETED:
      // This is the good one, but for the moment we'll do nothing here
      // oaLogError ( OA_LOG_CAMERA, "%s: unhandled completed status xfer", __func__ );
      break;

    case LIBUSB_TRANSFER_TIMED_OUT:
    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_OVERFLOW:
      oaLogWarning ( OA_LOG_CAMERA, "%s: retrying xfer, status = %d", __func__,
					transfer->status );
      break;
    default:
      oaLogError ( OA_LOG_CAMERA,
					"%s: unexpected interrupt transfer status = %d", __func__,
          transfer->status );
      break;
  }

  if ( resubmit ) {
    libusb_submit_transfer ( transfer );
  }
  return;
}
