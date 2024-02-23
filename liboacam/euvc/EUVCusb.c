/*****************************************************************************
 *
 * EUVCusb.c -- USB interface for EUVC cameras
 *
 * Copyright 2015,2021,2024
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

#include "EUVC.h"
#include "oacamprivate.h"
#include "EUVCstate.h"
#include "EUVCoacam.h"
#include "EUVCusb.h"


int
euvcUsbReadRegister ( EUVC_STATE* cameraInfo, uint8_t regNo, uint8_t* value )
{
  return euvcUsbControlMsg ( cameraInfo, USB_DIR_IN | USB_CTRL_TYPE_VENDOR |
      USB_RECIP_DEVICE, 0x0, 0x0, regNo, value, 1, USB_CTRL_TIMEOUT );
}


int
euvcUsbWriteRegister ( EUVC_STATE* cameraInfo, uint8_t regNo, uint8_t value )
{
  return euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_VENDOR |
      USB_RECIP_DEVICE, 0x0, 0x0, regNo, &value, 1, USB_CTRL_TIMEOUT );
}


int
euvcUsbControlMsg ( EUVC_STATE* cameraInfo, uint8_t reqType, uint8_t req,
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
    oaLogError ( OA_LOG_CAMERA, "%s: libusb control error: %d (%s)", __func__,
				ret, libusb_error_name ( ret ));
  }

	oaLogDebug ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return ret;
}


int
euvcUsbBulkTransfer ( EUVC_STATE* cameraInfo, unsigned char endpoint,
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
    oaLogError ( OA_LOG_CAMERA, "%s: libusb bulk error: %d (%s)", __func__,
				ret, libusb_error_name ( ret ));
  }

	oaLogDebug ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return ret;
}


void
euvcStatusCallback ( struct libusb_transfer* transfer )
{
  int		resubmit = 1;
	EUVC_STATE*	cameraInfo = ( EUVC_STATE* ) transfer->user_data;

  switch ( transfer->status ) {
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_NO_DEVICE:
      oaLogError ( OA_LOG_CAMERA,
					"%s: not processing/resubmitting status xfer: err = %d", __func__,
					transfer->status );
      return;
      break;

    case LIBUSB_TRANSFER_CANCELLED:
      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      if ( cameraInfo->statusTransfer ) {
        libusb_free_transfer ( transfer );
        cameraInfo->statusTransfer = 0;
      }
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
      resubmit = 0;
      break;

    case LIBUSB_TRANSFER_COMPLETED:
      // This is the good one, but for the moment we'll do nothing here
      // oaLogError ( OA_LOG_CAMERA, "%s: unhandled completed status xfer",
			// __func__ );
      break;

    case LIBUSB_TRANSFER_TIMED_OUT:
    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_OVERFLOW:
      oaLogWarning ( OA_LOG_CAMERA, "%s: retrying xfer, status = %d",
					__func__, transfer->status );
      break;

    default:
      oaLogError ( OA_LOG_CAMERA,
					"%s: unexpected interrupt transfer status = %d",  __func__,
          transfer->status );
      break;
  }

  if ( resubmit ) {
    libusb_submit_transfer ( transfer );
  }
  return;
}
