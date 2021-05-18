/*****************************************************************************
 *
 * IMG132Econtroller.c -- Main camera controller thread
 *
 * Copyright 2017,2018,2019,2021
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

#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <sys/time.h>

#if HAVE_MATH_H
#include <math.h>
#endif

#include "oacamprivate.h"
#include "unimplemented.h"
#include "QHY.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "IMG132E.h"
#include "QHYusb.h"


static int	_processSetControl ( QHY_STATE*, OA_COMMAND* );
static int	_processGetControl ( QHY_STATE*, OA_COMMAND* );
static int	_processSetResolution ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStop ( QHY_STATE*, OA_COMMAND* );
static int	_doSetGain ( QHY_STATE*, unsigned int, uint8_t );
//static int	_doSetSpeed ( QHY_STATE*, unsigned int );
static int	_doSetExposure ( QHY_STATE*, unsigned long );
static int	_doSetResolution ( QHY_STATE*, int, int );
static int	_doSetColourBalance ( QHY_STATE* );
static void     _processPayload ( oaCamera*, unsigned char*, unsigned int );
static void     _releaseFrame ( QHY_STATE* );


void*
oacamIMG132Econtroller ( void* param )
{
  oaCamera*		camera = param;
  QHY_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode, streaming = 0;

  do {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    exitThread = cameraInfo->stopControllerThread;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      // stop us busy-waiting
      streaming = ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) ? 1 : 0;
      if ( !streaming && oaDLListIsEmpty ( cameraInfo->commandQueue )) {
        pthread_cond_wait ( &cameraInfo->commandQueued,
            &cameraInfo->commandQueueMutex );
      }
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    }
    do {
      command = oaDLListRemoveFromHead ( cameraInfo->commandQueue );
      if ( command ) {
        switch ( command->commandType ) {
          case OA_CMD_CONTROL_SET:
            resultCode = _processSetControl ( cameraInfo, command );
            break;
          case OA_CMD_CONTROL_GET:
            resultCode = _processGetControl ( cameraInfo, command );
            break;
          case OA_CMD_RESOLUTION_SET:
            resultCode = _processSetResolution ( camera, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( camera, command );
            break;
          case OA_CMD_STOP_STREAMING:
            resultCode = _processStreamingStop ( cameraInfo, command );
            break;
          default:
            oaLogError ( OA_LOG_CAMERA, "%s: Invalid command type %d",
								__func__, command->commandType );
            resultCode = -OA_ERR_INVALID_CONTROL;
            break;
        }
        if ( command->callback ) {
					oaLogWarning ( OA_LOG_CAMERA, "%s: command has callback", __func__ );
        } else {
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          command->completed = 1;
          command->resultCode = resultCode;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
          pthread_cond_broadcast ( &cameraInfo->commandComplete );
        }
      }
    } while ( command );
  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*valp = command->commandData;
  int			control = command->controlId;

  int32_t val_s32;
  int64_t val_s64;

	oaLogInfo ( OA_LOG_CAMERA, "%s ( %p, %p ): entered", __func__, cameraInfo,
			command );
	oaLogDebug ( OA_LOG_CAMERA, "%s: control = %d", __func__, control );

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: invalid control type %d where int32 expected", __func__,
            valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      if ( valp->int32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentGain = valp->int32;
      _doSetGain ( cameraInfo, cameraInfo->currentGain,
          cameraInfo->currentDigitalGain );
      break;

    case OA_CAM_CTRL_DIGITAL_GAIN:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: invalid control type %d where int32 expected", __func__,
            valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      if ( valp->int32 < 0 || valp->int32 > 3 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentDigitalGain = valp->int32;
      _doSetGain ( cameraInfo, cameraInfo->currentGain,
          cameraInfo->currentDigitalGain );
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( valp->valueType != OA_CTRL_TYPE_INT64 ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: invalid control type %d where int64 expected", __func__,
            valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s64 = valp->int64;
      if ( val_s64 < 1 ) { val_s64 = 1; }
      cameraInfo->currentExposure = val_s64;
      _doSetExposure ( cameraInfo, val_s64 );
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: invalid control type %d where int32 expected", __func__,
            valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = valp->int32;
      if ( val_s32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentBlueBalance = val_s32;
      _doSetColourBalance ( cameraInfo );
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: invalid control type %d where int32 expected", __func__,
            valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = valp->int32;
      if ( val_s32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentRedBalance = val_s32;
      _doSetColourBalance ( cameraInfo );
      break;

    case OA_CAM_CTRL_GREEN_BALANCE:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: invalid control type %d where int32 expected", __func__,
            valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = valp->int32;
      if ( val_s32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentGreenBalance = val_s32;
      _doSetColourBalance ( cameraInfo );
      break;

    case OA_CAM_CTRL_FRAME_FORMAT:
      // nothing to do here as there's only one format
      break;

    default:
      oaLogError ( OA_LOG_CAMERA, "%s: IMG132E: unrecognised control %d",
					__func__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

	oaLogInfo ( OA_LOG_CAMERA, "%s): exiting", __func__ );

  return OA_ERR_NONE;

}


static int
_doSetGain ( QHY_STATE* cameraInfo, unsigned int gain, uint8_t digitalGain )
{
  uint16_t	gainVal;
  uint8_t	g;

  // For the IMX035 0x1000 is the lowest gain value, 0x200 is the highest
  // There's also a 3-bit digital shift for each colour, but until I'm
  // sure which is which I'll treat them all the same for the time being

  gainVal = 0x1000 - gain;
  if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_AGAIN_LO, gainVal & 0xff )) {
    oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_AGAIN_LO failed",
				__func__ );
  }
  if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_AGAIN_HI, ( gainVal >> 8 ) &
      0xff )) {
    oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_AGAIN_HI failed",
				__func__ );
  }
  g = digitalGain | ( digitalGain << 2 ) | ( digitalGain << 4 );
  _i2cWriteIMX035 ( cameraInfo, IMX035_REG_DGAIN, g );

  return OA_ERR_NONE;
}


static int
_doSetExposure ( QHY_STATE* cameraInfo, unsigned long value )
{
  unsigned int		exposureMS, units, remainder, fraction;
  unsigned int		vunits, hunits;

	oaLogInfo ( OA_LOG_CAMERA, "%s ( %p, %ld ): entered", __func__, cameraInfo,
			value );

  exposureMS = value / 1000;
  if ( cameraInfo->xSize > 640 || cameraInfo->ySize > 480 ) {
    vunits = 40;
    hunits = 1063;
  } else {
    vunits = 11;
    hunits = 511;
  }
  units = exposureMS / vunits;
  remainder = exposureMS % vunits;
  fraction = hunits - remainder * hunits / vunits;

  if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_SVS_LO, units & 0xff )) {
    oaLogError ( OA_LOG_CAMERA, "%s: write reg IMX035_REG_SVS_LO failed",
				__func__ );
  }
  if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_SVS_HI,
      ( units >> 8 ) & 0xff )) {
    oaLogError ( OA_LOG_CAMERA, "%s: write reg IMX035_REG_SVS_HI failed",
				__func__ );
  }
  if ( exposureMS < vunits ) {
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_SSBRK, 1 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write reg IMX035_REG_SSBRK, 1 failed",
          __func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_SSBRK, 0 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write reg IMX035_REG_SSBRK, 0 failed",
          __func__ );
    }
  }
  if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_SHS1_LO, fraction & 0xff )) {
    oaLogError ( OA_LOG_CAMERA, "%s: write reg IMX035_REG_SHS1_LO failed",
				__func__ );
  }
  if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_SHS1_HI,
      ( fraction >> 8 ) & 0xff )) {
    oaLogError ( OA_LOG_CAMERA, "%s: write reg IMX035_REG_SHS1_HI failed",
				__func__ );
  }

	oaLogInfo ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return OA_ERR_NONE;
}


static int
_doSetColourBalance ( QHY_STATE* cameraInfo )
{
  unsigned char	buffer[8];
  unsigned int	xferred;

  buffer[0] = 0x03;
  buffer[1] = cameraInfo->currentRedBalance;
  buffer[2] = cameraInfo->currentGreenBalance;
  buffer[3] = cameraInfo->currentBlueBalance;

  if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, buffer, 5,
      &xferred, 3000 )) {
    oaLogError ( OA_LOG_CAMERA, "%s: bulk xfer 5 failed", __func__ );
  }

  return OA_ERR_NONE;
}


int
oaIMG132EInitialiseRegisters ( QHY_STATE* cameraInfo )
{
  int err;

  /*
   * Think speed is always 0, so I'm not sure this is required
   *
  if (( err = _doSetSpeed ( cameraInfo, IMG132E_DEFAULT_SPEED )) !=
      OA_ERR_NONE ) {
    return err;
  }
   */
  if (( err = _doSetExposure ( cameraInfo, IMG132E_DEFAULT_EXPOSURE )) !=
      OA_ERR_NONE ) {
    return err;
  }
  if (( err = _doSetGain ( cameraInfo, IMG132E_DEFAULT_GAIN,
      IMG132E_DEFAULT_DIGITAL_GAIN )) != OA_ERR_NONE ) {
    return err;
  }
  if (( err = _doSetResolution ( cameraInfo, IMG132E_IMAGE_WIDTH,
      IMG132E_IMAGE_HEIGHT )) != OA_ERR_NONE ) {
    return err;
  }
  if (( err = _doSetColourBalance ( cameraInfo )) != OA_ERR_NONE ) {
    return err;
  }

  return OA_ERR_NONE;
}


static int
_processSetResolution ( oaCamera* camera, OA_COMMAND* command )
{
  QHY_STATE*	cameraInfo = camera->_private;
  FRAMESIZE*	size = command->commandData;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;

  _doSetResolution ( cameraInfo, cameraInfo->xSize, cameraInfo->ySize );
  return OA_ERR_NONE;
}


static int
_doSetResolution ( QHY_STATE* cameraInfo, int x, int y )
{
  int		xoffset, yoffset;
  unsigned char	buffer[64];
  uint16_t	v1, v2;
  uint16_t	horizClocks, vertLines;
  uint32_t	xferred;

	oaLogInfo ( OA_LOG_CAMERA, "%s ( %p, %d, %d ): entered", __func__,
			cameraInfo, x, y );

  // make sure these numbers are rounded to 4-pixel boundaries
  x = ( x + 3 ) & ~3;
  y = ( y + 3 ) & ~3;
  xoffset = ( IMG132E_IMAGE_WIDTH - x ) / 2;
  yoffset = ( IMG132E_IMAGE_WIDTH - y ) / 2;

  memset ( buffer, 0, 64 );
  if (( x + xoffset ) <= 640 && ( y + yoffset ) <= 480 ) {
    if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, buffer, 1,
        &xferred, 3000 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: bulk xfer 1 failed", __func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_TESTEN, 2 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_TESTEN failed",
					__func__ );
    }
    v1 = 0x17c; // 380
    v2 = 0x12c; // 300
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_WIN_1, v1 & 0xff )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_WIN_1 failed",
					__func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_WIN_2, (( v1 >> 8 ) & 0x07 ) |
        (( v2 & 0x0f ) << 4 ))) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_WIN_2 failed",
					__func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_WIN_3, v2 >> 4 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_WIN_3 failed",
					__func__ );
    }
    horizClocks = 0x3840; // 900 << 4 ?
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_HMAX_HI, horizClocks >> 8 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_HMAX_HI failed",
					__func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_HMAX_LO,
        horizClocks & 0xff )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_HMAX_LO failed",
					__func__ );
    }
    vertLines = 0x0200;
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_VMAX_HI, vertLines >> 8 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_VMAX_HI failed",
					__func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_VMAX_LO, vertLines & 0xff )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_VMAX_LO failed",
					__func__ );
    }
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0xc9;
    buffer[3] = 0x03;
    buffer[4] = 0x49;
    buffer[5] = 0x00;
    buffer[6] = 0x1a;
    buffer[7] = 0x01;
    buffer[8] = 0xfa;
    if ( _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xb5, 0, 0, buffer,
        64, 3000 ) != 64 ) {
      oaLogError ( OA_LOG_CAMERA, "%s: ctrl xfer 0xb5 failed", __func__ );
    }
    cameraInfo->smallFrame = 1;
    buffer[0] = 0x04;
    buffer[1] = 0x0a;
    if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, buffer, 5,
        &xferred, 3000 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: bulk xfer 5 failed", __func__ );
    }
    buffer[0] = 0x03;
    buffer[1] = 0x40;
    buffer[2] = 0x40;
    buffer[3] = 0x40;
    buffer[4] = 0x40;
    if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, buffer, 5,
        &xferred, 3000 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: bulk xfer 5 #2 failed", __func__ );
    }
  } else {
    if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, buffer, 1,
        &xferred, 3000 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: bulk xfer 1 #2 failed", __func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_TESTEN, 0 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_TESTEN failed",
					__func__ );
    }
    horizClocks = 0x5dc0; // I suspect this may be 1500 << 4
    vertLines = 0x0428;
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_HMAX_HI, horizClocks >> 8 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_HMAX_HI failed",
					__func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_HMAX_LO,
        horizClocks & 0xff )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_HMAX_LO failed",
					__func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_VMAX_HI, vertLines >> 8 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_VMAX_HI failed",
					__func__ );
    }
    if ( _i2cWriteIMX035 ( cameraInfo, IMX035_REG_VMAX_LO, vertLines & 0xff )) {
      oaLogError ( OA_LOG_CAMERA, "%s: write IMX035_REG_VMAX_LO failed",
					__func__ );
    }
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0xc9;
    buffer[3] = 0x05;
    buffer[4] = 0xc9;
    buffer[5] = 0x00;
    buffer[6] = 0x1a;
    buffer[7] = 0x04;
    buffer[8] = 0x1a;
    if ( _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xb5, 0, 0, buffer,
        64, 3000 ) != 64 ) {
      oaLogError ( OA_LOG_CAMERA, "%s: ctrl xfer 0xb5 #2 failed", __func__ );
    }
    cameraInfo->smallFrame = 0;
    buffer[0] = 0x04;
    buffer[1] = 0x0a;
    if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, buffer, 5,
        &xferred, 3000 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: bulk xfer 5 #3 failed", __func__ );
    }
    buffer[0] = 0x03;
    buffer[1] = 0x40;
    buffer[2] = 0x40;
    buffer[3] = 0x40;
    buffer[4] = 0x40;
    if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, buffer, 5,
        &xferred, 3000 )) {
      oaLogError ( OA_LOG_CAMERA, "%s: bulk xfer 5 #4 failed", __func__ );
    }
  }

  cameraInfo->frameSize = x * y;
  cameraInfo->captureLength = cameraInfo->frameSize;

	oaLogInfo ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return OA_ERR_NONE;
}


static int
_processGetControl ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	valp = command->resultData;

	oaLogInfo ( OA_LOG_CAMERA, "%s ( %p, %p ): entered", __func__, cameraInfo,
			command );
	oaLogDebug ( OA_LOG_CAMERA, "%s: control = %d", __func__, control );

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = cameraInfo->currentGain;
      break;

    case OA_CAM_CTRL_DIGITAL_GAIN:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = cameraInfo->currentDigitalGain;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      valp->valueType = OA_CTRL_TYPE_INT64;
      valp->int64 = cameraInfo->currentExposure;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      if ( cameraInfo->isColour ) {
        valp->valueType = OA_CTRL_TYPE_INT32;
        valp->int32 = cameraInfo->currentRedBalance;
      } else {
        return -OA_ERR_INVALID_CONTROL;
      }
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      if ( cameraInfo->isColour ) {
        valp->valueType = OA_CTRL_TYPE_INT32;
        valp->int32 = cameraInfo->currentBlueBalance;
      } else {
        return -OA_ERR_INVALID_CONTROL;
      }
      break;

    case OA_CAM_CTRL_GREEN_BALANCE:
      if ( cameraInfo->isColour ) {
        valp->valueType = OA_CTRL_TYPE_INT32;
        valp->int32 = cameraInfo->currentGreenBalance;
      } else {
        return -OA_ERR_INVALID_CONTROL;
      }
      break;

    default:
      oaLogError ( OA_LOG_CAMERA, "%s: Unimplemented control %d", __func__,
					control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

	oaLogInfo ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return OA_ERR_NONE;
}


libusb_transfer_cb_fn
_img132eVideoStreamCallback ( struct libusb_transfer* transfer )
{ 
  oaCamera*     camera = transfer->user_data;
  QHY_STATE*    cameraInfo = camera->_private;
  int           resubmit = 1, streaming;
      
  switch ( transfer->status ) {
      
    case LIBUSB_TRANSFER_COMPLETED:
      if ( transfer->num_iso_packets == 0 ) { // bulk mode transfer
        _processPayload ( camera, transfer->buffer, transfer->actual_length );
      } else {
        oaLogError ( OA_LOG_CAMERA, "%s: Unexpected isochronous transfer",
						__func__ );
      }
      break;
      
    case LIBUSB_TRANSFER_CANCELLED:
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_NO_DEVICE:
    {
      int i;
      
      pthread_mutex_lock ( &cameraInfo->videoCallbackMutex );
  
      for ( i = 0; i < QHY_NUM_TRANSFER_BUFS; i++ ) {
        if ( cameraInfo->transfers[i] == transfer ) {
          free ( transfer->buffer );
          libusb_free_transfer ( transfer );
          cameraInfo->transfers[i] = 0;
          break;
        }
      }

      if ( QHY_NUM_TRANSFER_BUFS == i ) {
        oaLogError ( OA_LOG_CAMERA, "%s: transfer %p not found; not freeing!",
						__func__, transfer );
      }

      resubmit = 0;

      pthread_mutex_unlock ( &cameraInfo->videoCallbackMutex );
      break;
    }
    case LIBUSB_TRANSFER_TIMED_OUT:
      break;

    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_OVERFLOW:
      oaLogError ( OA_LOG_CAMERA, "%s: retrying transfer, status = %d (%s)",
					__func__, transfer->status, libusb_error_name ( transfer->status ));
      break;
  }

  if ( resubmit ) {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    streaming = ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) ? 1 : 0;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( streaming ) {
      libusb_submit_transfer ( transfer );
    } else {
      int i;
      pthread_mutex_lock ( &cameraInfo->videoCallbackMutex );
      // Mark transfer deleted
      for ( i = 0; i < QHY_NUM_TRANSFER_BUFS; i++ ) {
        if ( cameraInfo->transfers[i] == transfer ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Freeing orphan transfer %d (%p)",
							__func__, i, transfer );
          free ( transfer->buffer );
          libusb_free_transfer ( transfer );
          cameraInfo->transfers[i] = 0;
        }
      }
      if ( QHY_NUM_TRANSFER_BUFS == i ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: orphan transfer %p not found; not freeing!", __func__,
            transfer );
      }
      pthread_mutex_unlock ( &cameraInfo->videoCallbackMutex );
    }
  }

  return 0;
}


static int
_processStreamingStart ( oaCamera* camera, OA_COMMAND* command )
{
  QHY_STATE*			cameraInfo = camera->_private;
  CALLBACK*			cb = command->commandData;
  int				txId, ret, txBufferSize, numTxBuffers;
  struct libusb_transfer*	transfer;
  unsigned char	buf[1] = { 100 };

  if ( cameraInfo->runMode != CAM_RUN_MODE_STOPPED ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  txBufferSize = cameraInfo->captureLength;
  // This is a guess
  numTxBuffers = 8;
  if ( numTxBuffers < 8 ) {
    numTxBuffers = 8;
  }
  if ( numTxBuffers > 100 ) {
    numTxBuffers = 100;
  }
  for ( txId = 0; txId < QHY_NUM_TRANSFER_BUFS; txId++ ) {
    if ( txId < numTxBuffers ) {
      transfer = libusb_alloc_transfer(0);
      cameraInfo->transfers[ txId ] = transfer;
      if (!( cameraInfo->transferBuffers [ txId ] =
          malloc ( txBufferSize ))) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: malloc of buffer failed.  Need to free buffer", __func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
      libusb_fill_bulk_transfer ( transfer, cameraInfo->usbHandle,
          QHY_SDRAM_BULK_ENDP_IN, cameraInfo->transferBuffers [ txId ],
          txBufferSize, ( libusb_transfer_cb_fn ) _img132eVideoStreamCallback,
          camera, USB2_TIMEOUT );
    } else {
      cameraInfo->transfers[ txId ] = 0;
    }
  }

  for ( txId = 0; txId < numTxBuffers; txId++ ) {
    if (( ret = libusb_submit_transfer ( cameraInfo->transfers [ txId ]))) {
      break;
    }
  }

  // free up any transfer buffers that we're not using
  if ( ret && txId > 0 ) {
    for ( ; txId < QHY_NUM_TRANSFER_BUFS; txId++) {
      if ( cameraInfo->transfers[ txId ] ) {
        if ( cameraInfo->transfers[ txId ]->buffer ) {
          free ( cameraInfo->transfers[ txId ]->buffer );
        }
        libusb_free_transfer ( cameraInfo->transfers[ txId ]);
        cameraInfo->transfers[ txId ] = 0;
      }
    }
  }

  if ( _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, QHY_REQ_BEGIN_VIDEO,
      0, 0, buf, 1, 3000 ) != 1 ) {
    oaLogError ( OA_LOG_CAMERA, "%s: ctrl xfer begin video failed", __func__ );
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->runMode = CAM_RUN_MODE_STREAMING;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_processStreamingStop ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  int		queueEmpty, i, res, allReleased;

  if ( cameraInfo->runMode != CAM_RUN_MODE_STREAMING ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->runMode = CAM_RUN_MODE_STOPPED;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  pthread_mutex_lock ( &cameraInfo->videoCallbackMutex );
  for ( i = 0; i < QHY_NUM_TRANSFER_BUFS; i++ ) {
    if ( cameraInfo->transfers[i] ) {
      res = libusb_cancel_transfer ( cameraInfo->transfers[i] );
      if ( res < 0 && res != LIBUSB_ERROR_NOT_FOUND ) {
        free ( cameraInfo->transfers[i]->buffer );
        libusb_free_transfer ( cameraInfo->transfers[i] );
        cameraInfo->transfers[i] = 0;
      }
    }
  }
  pthread_mutex_unlock ( &cameraInfo->videoCallbackMutex );

  do {
    allReleased = 1;
    for ( i = 0; i < QHY_NUM_TRANSFER_BUFS && allReleased; i++ ) {
      pthread_mutex_lock ( &cameraInfo->videoCallbackMutex );
      if ( cameraInfo->transfers[i] ) {
        allReleased = 0;
      }
      pthread_mutex_unlock ( &cameraInfo->videoCallbackMutex );
    }
    if ( !allReleased ) {
      usleep ( 100 ); // FIX ME -- lazy.  should use a pthread condition?
    }
  } while ( !allReleased );

  // We wait here until the callback queue has drained otherwise a future
  // close of the camera could rip the image frame out from underneath the
  // callback

  queueEmpty = 0;
  do {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    queueEmpty = ( OA_CAM_BUFFERS == cameraInfo->buffersFree ) ? 1 : 0;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    if ( !queueEmpty ) {
      usleep ( 10000 );
    }
  } while ( !queueEmpty );

  return OA_ERR_NONE;
}


static void
_processPayload ( oaCamera* camera, unsigned char* buffer, unsigned int len )
{
  QHY_STATE*            cameraInfo = camera->_private;
  unsigned int          buffersFree;

  if ( 0 == len ) {
    return;
  }

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
  if ( buffersFree && ( cameraInfo->receivedBytes + len ) <=
      cameraInfo->captureLength ) {
    memcpy (( unsigned char* ) cameraInfo->buffers[
        cameraInfo->nextBuffer ].start + cameraInfo->receivedBytes,
        buffer, len );
    cameraInfo->receivedBytes += len;
    if ( cameraInfo->receivedBytes == cameraInfo->captureLength ) {
      _releaseFrame ( cameraInfo );
    }
  } else {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    cameraInfo->droppedFrames++;
    cameraInfo->receivedBytes = 0;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
  } 
} 

  
static void
_releaseFrame ( QHY_STATE* cameraInfo )
{ 
  int           nextBuffer = cameraInfo->nextBuffer;
  
  cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
      OA_CALLBACK_NEW_FRAME; 
  cameraInfo->frameCallbacks[ nextBuffer ].callback =
      cameraInfo->streamingCallback.callback;
  cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
      cameraInfo->streamingCallback.callbackArg;
  cameraInfo->frameCallbacks[ nextBuffer ].buffer =
      cameraInfo->buffers[ nextBuffer ].start;
  cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
      cameraInfo->frameSize;
  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  oaDLListAddToTail ( cameraInfo->callbackQueue,
      &cameraInfo->frameCallbacks[ nextBuffer ]);
  cameraInfo->buffersFree--;
  cameraInfo->nextBuffer = ( nextBuffer + 1 ) % cameraInfo->configuredBuffers;
  cameraInfo->receivedBytes = 0;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
  pthread_cond_broadcast ( &cameraInfo->callbackQueued );
}
