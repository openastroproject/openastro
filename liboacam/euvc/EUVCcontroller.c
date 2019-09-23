/*****************************************************************************
 *
 * EUVCcontroller.c -- Main camera controller thread
 *
 * Copyright 2015,2017,2018,2019
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

#include "oacamprivate.h"
#include "unimplemented.h"
#include "EUVC.h"
#include "EUVCstate.h"
#include "EUVCoacam.h"
#include "EUVCusb.h"


static int	_processSetControl ( EUVC_STATE*, OA_COMMAND* );
static int	_processGetControl ( EUVC_STATE*, OA_COMMAND* );
static int	_processSetResolution ( oaCamera*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStop ( EUVC_STATE*, OA_COMMAND* );
static int      _processSetFrameInterval ( oaCamera*, OA_COMMAND* );
static void	_processPayload ( oaCamera*, unsigned char*, unsigned int );
static void	_releaseFrame ( EUVC_STATE* );
static void	_doSetFrameRate ( EUVC_STATE*, unsigned int, unsigned int );

void*
oacamEUVCcontroller ( void* param )
{
  oaCamera*		camera = param;
  EUVC_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode;
  int			streaming = 0;

  do {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    exitThread = cameraInfo->stopControllerThread;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      // stop us busy-waiting
      streaming = cameraInfo->isStreaming;
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
          case OA_CMD_ROI_SET:
            resultCode = _processSetROI ( camera, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( camera, command );
            break;
          case OA_CMD_STOP_STREAMING:
            resultCode = _processStreamingStop ( cameraInfo, command );
            break;
          case OA_CMD_FRAME_INTERVAL_SET:
            resultCode = _processSetFrameInterval ( camera, command );
            break;
          default:
            fprintf ( stderr, "Invalid command type %d in controller\n",
                command->commandType );
            resultCode = -OA_ERR_INVALID_CONTROL;
            break;
        }
        if ( command->callback ) {
//fprintf ( stderr, "CONT: command has callback\n" );
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
_processSetControl ( EUVC_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->commandData;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "EUVC: control: %s ( %d, ? )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    {
      unsigned int exp100ns;

      if ( val->valueType != OA_CTRL_TYPE_INT64 ) {
        fprintf ( stderr, "%s: invalid control type %d where int64 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentExposure = val->int64;
      exp100ns = val->int64 / 100;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL << 8, EUVC_CAM_TERMINAL << 8,
          ( unsigned char* ) &exp100ns, sizeof ( exp100ns ),
          USB_CTRL_TIMEOUT ) != sizeof ( exp100ns )) {
        fprintf ( stderr, "set exposure failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_BACKLIGHT_COMPENSATION:
    {
      uint16_t val_u16;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u16 = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_PU_BACKLIGHT_COMPENSATION_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "set backlight compensation failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_POWER_LINE_FREQ:
    {
      uint8_t val_u8;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u8 = val->int32;
    
      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_PU_POWER_LINE_FREQUENCY_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set powerline frequency failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE_TEMP ):
    { 
      uint8_t val_u8;
      
      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u8 = val->boolean ? 1 : 0;
      
      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL << 8, 
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set auto white balance temperature failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ):
    {   
      uint8_t val_u8;
      
      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u8 = val->boolean ? 1 : 0;
      
      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_PU_CONTRAST_AUTO_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set auto contrast control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_GAIN:
    {
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentGain = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR, EUVC_PU_GAIN_CONTROL << 8,
          cameraInfo->processingUnitId << 8,
          ( unsigned char* ) &cameraInfo->currentGain,
          sizeof ( cameraInfo->currentGain ), USB_CTRL_TIMEOUT ) !=
          sizeof ( cameraInfo->currentGain )) {
        fprintf ( stderr, "set gain failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_BRIGHTNESS:
    {
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentBrightness = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR, EUVC_PU_BRIGHTNESS_CONTROL << 8,
          cameraInfo->processingUnitId << 8,
          ( unsigned char* ) &cameraInfo->currentBrightness,
          sizeof ( cameraInfo->currentBrightness ), USB_CTRL_TIMEOUT ) !=
          sizeof ( cameraInfo->currentBrightness )) {
        fprintf ( stderr, "set brightness failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_BLUE_BALANCE:
    {
      uint32_t	balance;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentBlueBalance = val->int32 & 0xffff;
      balance = cameraInfo->currentBlueBalance << 16 |
         cameraInfo->currentRedBalance;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_PU_WHITE_BALANCE_COMPONENT_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &balance,
          sizeof ( balance ), USB_CTRL_TIMEOUT ) != sizeof ( balance )) {
        fprintf ( stderr, "set white balance failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_RED_BALANCE:
    {
      uint32_t  balance;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentRedBalance = val->int32 & 0xffff;
      balance = cameraInfo->currentBlueBalance << 16 | 
         cameraInfo->currentRedBalance;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_PU_WHITE_BALANCE_COMPONENT_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &balance,
          sizeof ( balance ), USB_CTRL_TIMEOUT ) != sizeof ( balance )) {
        fprintf ( stderr, "set white balance failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_BINNING:
      if ( val->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->binMode = val->discrete;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
			cameraInfo->autoExposure = val->menu;
			if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
					USB_RECIP_INTERFACE, REQ_SET_CUR, EUVC_CT_AE_MODE_CONTROL << 8,
					EUVC_CAM_TERMINAL << 8, ( unsigned char* )
					&cameraInfo->autoExposure, sizeof ( cameraInfo->autoExposure ),
					USB_CTRL_TIMEOUT ) != sizeof ( cameraInfo->autoExposure )) {
				fprintf ( stderr, "set auto exposure failed\n" );
				return -OA_ERR_SYSTEM_ERROR;
			}
			break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
      cameraInfo->autoWhiteBalance = val->boolean;
      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL << 8,
          cameraInfo->processingUnitId << 8,
          ( unsigned char* ) &cameraInfo->autoWhiteBalance,
          sizeof ( cameraInfo->autoWhiteBalance ), USB_CTRL_TIMEOUT ) !=
          sizeof ( cameraInfo->autoWhiteBalance )) {
        fprintf ( stderr, "set auto exposure failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
      fprintf ( stderr, "auto gain not yet implemented\n" );
      break;

    case OA_CAM_CTRL_INTERLACE_ENABLE:
    {
      uint8_t val_u8;

      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u8 = val->boolean ? 1 : 0;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_SCANNING_MODE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set interlace mode control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_ZOOM_ABSOLUTE:
    {
      uint16_t val_u16;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u16 = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_ZOOM_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "set absolute zoom control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_FOCUS_ABSOLUTE:
    {
      uint16_t val_u16;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u16 = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_FOCUS_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "set absolute focus control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_IRIS_ABSOLUTE:
    {
      uint16_t val_u16;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u16 = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_IRIS_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "set absolute iris control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_PAN_ABSOLUTE:
    case OA_CAM_CTRL_TILT_ABSOLUTE:
    {
      int32_t data[2];

      if ( OA_CAM_CTRL_PAN_ABSOLUTE == control ) {
        cameraInfo->currentPan = val->int32;
      } else {
        cameraInfo->currentTilt = val->int32;
      }

      data[0] = cameraInfo->currentPan;
      data[1] = cameraInfo->currentTilt;
      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_PANTILT_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) data, 8,
          USB_CTRL_TIMEOUT ) != 8 ) {

        fprintf ( stderr, "uvc_set_pantilt_abs ( %d, %d ) failed in %s",
            cameraInfo->currentPan, cameraInfo->currentTilt, __FUNCTION__ );
      }
      break;
    }

    case OA_CAM_CTRL_ROLL_ABSOLUTE:
    {
      uint16_t val_u16;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u16 = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_ROLL_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "set absolute roll control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_PRIVACY_ENABLE:
    {
      uint8_t val_u8;

      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u8 = val->boolean ? 1 : 0;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_PRIVACY_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set privacy control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_FOCUS_SIMPLE:
    {
      uint8_t val_u8;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u8 = val->int32;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_ROLL_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set simple focus control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_RELATIVE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_SIMPLE ):
    {
      uint8_t val_u8;

      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u8 = val->boolean ? 1 : 0;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_SET_CUR,
          EUVC_CT_FOCUS_AUTO_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set privacy control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      break;
    }

    case OA_CAM_CTRL_FRAME_FORMAT:
      // Only the one mode is supported per camera, so silently ignore
      // this
      return OA_ERR_NONE;
      break;

    default:
      fprintf ( stderr, "Unrecognised control %d in %s\n", control,
          __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  return OA_ERR_NONE;
}


static int
_processGetControl ( EUVC_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "EUVC: control: %s ( %d )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT64;
      val->int64 = cameraInfo->currentExposure;
      break;

    case OA_CAM_CTRL_BINNING:
      val->valueType = OA_CTRL_TYPE_DISCRETE;
      val->discrete = cameraInfo->binMode;
      break;

    case OA_CAM_CTRL_DROPPED:
      val->valueType = OA_CTRL_TYPE_READONLY;
      val->readonly = cameraInfo->droppedFrames;
      break;

    case OA_CAM_CTRL_PAN_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentPan;
      break;

    case OA_CAM_CTRL_TILT_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentTilt;
      break;

    case OA_CAM_CTRL_BACKLIGHT_COMPENSATION:
    {
      uint16_t val_u16;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_PU_BACKLIGHT_COMPENSATION_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "get backlight compensation failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u16;
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
		{
      uint8_t	val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR, EUVC_CT_AE_MODE_CONTROL << 8,
					cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u8, 1,
					USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "get auto exposure failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_MENU;
			val->menu = val_u8;
      break;
		}

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE_TEMP ):
    {
      uint8_t val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "get auto white balance temperature failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = val_u8 ? 1 : 0;
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ):
    {
      uint8_t val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_PU_CONTRAST_AUTO_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "get auto contrast control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = val_u8 ? 1 : 0;
      break;
    }

    case OA_CAM_CTRL_POWER_LINE_FREQ:
    {
      uint8_t val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_PU_POWER_LINE_FREQUENCY_CONTROL << 8,
          cameraInfo->processingUnitId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "get powerline frequency failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u8;
      break;
    }

    case OA_CAM_CTRL_INTERLACE_ENABLE:
    {
      uint8_t val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_SCANNING_MODE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "get interlace mode control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u8;
      break;
    }

    case OA_CAM_CTRL_ZOOM_ABSOLUTE:
    {
      uint16_t val_u16;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_ZOOM_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "get absolute zoom control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u16;
      break;
    }

    case OA_CAM_CTRL_FOCUS_ABSOLUTE:
    {
      uint16_t val_u16;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_FOCUS_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "get absolute focus control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u16;
      break;
    }

    case OA_CAM_CTRL_IRIS_ABSOLUTE:
    {
      uint16_t val_u16;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_IRIS_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "get absolute iris control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u16;
      break;
    }

    case OA_CAM_CTRL_ROLL_ABSOLUTE:
    {
      uint16_t val_u16;
      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_ROLL_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u16, 2,
          USB_CTRL_TIMEOUT ) != 2 ) {
        fprintf ( stderr, "get absolute roll control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u16;
      break;
    }

    case OA_CAM_CTRL_PRIVACY_ENABLE:
    {
      uint8_t val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_PRIVACY_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "get privacy control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->int32 = val_u8 ? 1 : 0;
      break;
    }

    case OA_CAM_CTRL_FOCUS_SIMPLE:
    {
      uint8_t val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_ROLL_ABSOLUTE_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set simple focus control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = val_u8;
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_RELATIVE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_SIMPLE ):
    {
      uint8_t val_u8;

      if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
          USB_RECIP_INTERFACE, REQ_GET_CUR,
          EUVC_CT_FOCUS_AUTO_CONTROL << 8,
          cameraInfo->terminalId << 8, ( unsigned char* ) &val_u8, 1,
          USB_CTRL_TIMEOUT ) != 1 ) {
        fprintf ( stderr, "set privacy control failed\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->int32 = val_u8 ? 1 : 0;
      break;
    }

    default:
      fprintf ( stderr,
          "Unrecognised control %d in %s\n", control, __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
_processSetResolution ( oaCamera* camera, OA_COMMAND* command )
{
  EUVC_STATE*		cameraInfo = camera->_private;
  FRAMESIZE*		size = command->commandData;
  FRAMESIZES*		sizeList;
  int			restartStreaming = 0;
  unsigned int		x, y, sizeIndex;
  PROBE_BLOCK		probe;
  uint8_t		binMode;
  uint32_t		posn;

  x = size->x;
  y = size->y;

  if ( cameraInfo->isStreaming ) {
    // FIX ME -- check for errors?
    _processStreamingStop ( cameraInfo, 0 );
    restartStreaming = 1;
  }

  OA_CLEAR ( probe );

  // Have we actually got what has been asked for?

  sizeList = &cameraInfo->frameSizes[ cameraInfo->binMode ];
  for ( sizeIndex = 0; sizeIndex < sizeList->numSizes; sizeIndex++ ) {
    if ( x == sizeList->sizes[sizeIndex].x &&
        y == sizeList->sizes[sizeIndex].y ) {
      probe.bFormatIndex =
          cameraInfo->frameInfo[ cameraInfo->binMode][sizeIndex].formatId;
      probe.bFrameIndex =
          cameraInfo->frameInfo[ cameraInfo->binMode][sizeIndex].frameId;
      break;
    }
  }
  probe.bmHint = 1;
  probe.wFrameInterval = 333333;

  if ( sizeIndex == sizeList->numSizes ) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  if ( euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT | USB_CTRL_TYPE_CLASS |
      USB_RECIP_INTERFACE, REQ_SET_CUR, VS_COMMIT_CONTROL << 8, 1,
      ( unsigned char* ) &probe, sizeof ( probe ), USB_CTRL_TIMEOUT ) !=
      sizeof ( probe )) {
    fprintf ( stderr, "set format failed\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  usleep ( 100000 );

  // First step of setting resolution is to disable binning
  if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING )) {
    binMode = 1;
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_BINNING,
        &binMode, 1, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable disable binning\n" );
      return -OA_ERR_INVALID_CONTROL;
    }
    // usleep ( 5000 );
  }

  // Now reset ROI position
  if ( camera->features.flags & OA_CAM_FEATURE_ROI ) {
    posn = 0;
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_X,
        &posn, 4, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable to reset x posn\n" );
      return -OA_ERR_INVALID_CONTROL;
    }
    // usleep ( 5000 );
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_Y,
        &posn, 4, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable to reset y posn\n" );
      return -OA_ERR_INVALID_CONTROL;
    }
    // usleep ( 5000 );
  }

  // FIX ME
  // The Burst C uses commands 0x38 and 0x39 send with the image width
  // and height at this point, but the camera appears to work without
  // them and I have no idea what they do.

  // Now the frame rate
  _doSetFrameRate ( cameraInfo, x, y );

  // Set the ROI and ROI position
  if ( camera->features.flags & OA_CAM_FEATURE_ROI ) {
    uint8_t* p = ( uint8_t* ) &posn;
    p[0] = x & 0xff;
    p[1] = ( x >> 8 ) & 0xff;
    p[2] = ( x >> 16 ) & 0xff;
    p[3] = ( x >> 24 ) & 0xff;
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_WIDTH,
        &posn, 4, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable to set x size\n" );
      return -OA_ERR_INVALID_CONTROL;
    }
    // usleep ( 5000 );
    p[0] = y & 0xff;
    p[1] = ( y >> 8 ) & 0xff;
    p[2] = ( y >> 16 ) & 0xff;
    p[3] = ( y >> 24 ) & 0xff;
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_HEIGHT,
        &posn, 4, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable to set y size\n" );
      return -OA_ERR_INVALID_CONTROL;
    }
    // usleep ( 5000 );
    posn = 0;
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_X,
        &posn, 4, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable to set x posn\n" );
      return -OA_ERR_INVALID_CONTROL;
    }
    // usleep ( 5000 );
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_Y,
        &posn, 4, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable to set y posn\n" );
      return -OA_ERR_INVALID_CONTROL;
    }
    // usleep ( 5000 );
  }

  // And finally the binning mode
  if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING )) {
    binMode = cameraInfo->binMode;
    if ( setEUVCTermControl ( cameraInfo, EUVC_CT_BINNING,
        &binMode, 1, EUVC_SET_CUR )) {
      fprintf ( stderr, "unable to set %dx binning\n", binMode );
      return -OA_ERR_INVALID_CONTROL;
    }
  }

  cameraInfo->sizeIndex = sizeIndex;
  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  cameraInfo->imageBufferLength = x * y * cameraInfo->bytesPerPixel;

  if ( restartStreaming ) {
    return _processStreamingStart ( camera, 0 );
  }

  return OA_ERR_NONE;
}


static int
_processSetROI ( oaCamera* camera, OA_COMMAND* command )
{
  EUVC_STATE*		cameraInfo = camera->_private;
  FRAMESIZE*		size = command->commandData;
  FRAMESIZES*		sizeList;
  unsigned int		x, y, frameX, frameY;
  uint32_t		posn;

  if (!( camera->features.flags & OA_CAM_FEATURE_ROI )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  x = size->x;
  y = size->y;
  sizeList = &cameraInfo->frameSizes[ cameraInfo->binMode ];
  frameX = sizeList->sizes[ cameraInfo->sizeIndex ].x;
  frameY = sizeList->sizes[ cameraInfo->sizeIndex ].y;

  if (( x % 8 != 0 ) || ( y % 4 != 0 ) || ( x > frameX ) ||
      ( y > frameY )) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  // Set the ROI and ROI position
  uint8_t* p = ( uint8_t* ) &posn;
  p[0] = x & 0xff;
  p[1] = ( x >> 8 ) & 0xff;
  p[2] = ( x >> 16 ) & 0xff;
  p[3] = ( x >> 24 ) & 0xff;
  if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_WIDTH,
      &posn, 4, EUVC_SET_CUR )) {
    fprintf ( stderr, "unable to set x size\n" );
    return -OA_ERR_INVALID_CONTROL;
  }
  // usleep ( 5000 );
  p[0] = y & 0xff;
  p[1] = ( y >> 8 ) & 0xff;
  p[2] = ( y >> 16 ) & 0xff;
  p[3] = ( y >> 24 ) & 0xff;
  if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_HEIGHT,
      &posn, 4, EUVC_SET_CUR )) {
    fprintf ( stderr, "unable to set y size\n" );
    return -OA_ERR_INVALID_CONTROL;
  }
  // usleep ( 5000 );
  posn = ( frameX - x ) / 2;
  if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_X,
      &posn, 4, EUVC_SET_CUR )) {
    fprintf ( stderr, "unable to set x posn\n" );
    return -OA_ERR_INVALID_CONTROL;
  }
  // usleep ( 5000 );
  posn = ( frameY - y ) / 2;
  if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PARTIAL_SCAN_Y,
      &posn, 4, EUVC_SET_CUR )) {
    fprintf ( stderr, "unable to set y posn\n" );
    return -OA_ERR_INVALID_CONTROL;
  }
  // usleep ( 5000 );

  cameraInfo->xSize = x;
  cameraInfo->ySize = y;
  cameraInfo->imageBufferLength = x * y * cameraInfo->bytesPerPixel;

  return OA_ERR_NONE;
}


libusb_transfer_cb_fn
_euvcVideoStreamCallback ( struct libusb_transfer* transfer )
{
  oaCamera*	camera = transfer->user_data;
  EUVC_STATE*	cameraInfo = camera->_private;
  int		resubmit = 1, streaming;

  switch ( transfer->status ) {

    case LIBUSB_TRANSFER_COMPLETED:
      if ( transfer->num_iso_packets == 0 ) { // bulk mode transfer
        _processPayload ( camera, transfer->buffer, transfer->actual_length );
      } else {
        fprintf ( stderr, "Unexpected isochronous transfer\n" );
      }
      break;

    case LIBUSB_TRANSFER_CANCELLED:
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_NO_DEVICE:
    {
      int i;

      pthread_mutex_lock ( &cameraInfo->videoCallbackMutex );

      for ( i = 0; i < EUVC_NUM_TRANSFER_BUFS; i++ ) {
        if ( cameraInfo->transfers[i] == transfer ) {
          free ( transfer->buffer );
          libusb_free_transfer ( transfer );
          cameraInfo->transfers[i] = 0;
          break;
        }
      }

      if ( EUVC_NUM_TRANSFER_BUFS == i ) {
        fprintf ( stderr, "transfer %p not found; not freeing!\n", transfer );
      }

      resubmit = 0;

      pthread_mutex_unlock ( &cameraInfo->videoCallbackMutex );

      break;
    }
    case LIBUSB_TRANSFER_TIMED_OUT:
      break;

    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_OVERFLOW:
      fprintf ( stderr, "retrying transfer, status = %d (%s)\n",
          transfer->status, libusb_error_name ( transfer->status ));
      break;
  }

  if ( resubmit ) {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    streaming = cameraInfo->isStreaming;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( streaming ) {
      libusb_submit_transfer ( transfer );
    } else {
      int i;
      pthread_mutex_lock ( &cameraInfo->videoCallbackMutex );
      // Mark transfer deleted
      for ( i = 0; i < EUVC_NUM_TRANSFER_BUFS; i++ ) {
        if ( cameraInfo->transfers[i] == transfer ) {
          fprintf ( stderr, "Freeing orphan transfer %d (%p)\n", i, transfer );
          free ( transfer->buffer );
          libusb_free_transfer ( transfer );
          cameraInfo->transfers[i] = 0;
        }
      }
      if ( EUVC_NUM_TRANSFER_BUFS == i ) {
        fprintf ( stderr, "orphan transfer %p not found; not freeing!\n",
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
  EUVC_STATE*			cameraInfo = camera->_private;
  CALLBACK*			cb;
  int				txId, ret, txBufferSize, numTxBuffers;
  struct libusb_transfer*	transfer;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }
  if ( command ) {
    cb = command->commandData;
    cameraInfo->streamingCallback.callback = cb->callback;
    cameraInfo->streamingCallback.callbackArg = cb->callbackArg;
  }

  txBufferSize = cameraInfo->frameInfo [ cameraInfo->binMode ][
      cameraInfo->sizeIndex ].maxBufferSize;
#ifdef USB_OVERFLOW_HANGS
  if ( cameraInfo->overflowTransmit ) {
    txBufferSize *= 2.5;
  }
#endif
  // This is a guess based on experimentation
  numTxBuffers = 200 * 1024 * 1024 / txBufferSize;
  if ( numTxBuffers < 8 ) {
    numTxBuffers = 8;
  }
  if ( numTxBuffers > 100 ) {
    numTxBuffers = 100;
  }
  for ( txId = 0; txId < EUVC_NUM_TRANSFER_BUFS; txId++ ) {
    if ( txId < numTxBuffers ) {
      transfer = libusb_alloc_transfer(0);
      cameraInfo->transfers[ txId ] = transfer;
      if (!( cameraInfo->transferBuffers [ txId ] =
          malloc ( txBufferSize ))) {
        fprintf ( stderr, "malloc failed.  Need to free buffer\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      libusb_fill_bulk_transfer ( transfer, cameraInfo->usbHandle,
          USB_BULK_EP_IN, cameraInfo->transferBuffers [ txId ],
          txBufferSize, ( libusb_transfer_cb_fn ) _euvcVideoStreamCallback,
          camera,
          USB_BULK_TIMEOUT );
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
    for ( ; txId < EUVC_NUM_TRANSFER_BUFS; txId++) {
      if ( cameraInfo->transfers[ txId ] ) {
        if ( cameraInfo->transfers[ txId ]->buffer ) {
          free ( cameraInfo->transfers[ txId ]->buffer );
        }
        libusb_free_transfer ( cameraInfo->transfers[ txId ]);
        cameraInfo->transfers[ txId ] = 0;
      }
    }
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_processStreamingStop ( EUVC_STATE* cameraInfo, OA_COMMAND* command )
{
  int		queueEmpty, i, res, allReleased;

  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  pthread_mutex_lock ( &cameraInfo->videoCallbackMutex );
  for ( i = 0; i < EUVC_NUM_TRANSFER_BUFS; i++ ) {
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
    for ( i = 0; i < EUVC_NUM_TRANSFER_BUFS && allReleased; i++ ) {
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
      usleep ( 100 );  // lazy.  should use a condition or something similar
    }
  } while ( !queueEmpty );

  return OA_ERR_NONE;
}


static int
_processSetFrameInterval ( oaCamera* camera, OA_COMMAND* command )
{
  EUVC_STATE*		cameraInfo = camera->_private;
  FRAMERATE*		rate = command->commandData;
  FRAMERATES*		rateList;
  int			matchedInterval;
  unsigned int		i;

  rateList = cameraInfo->frameRates;
  matchedInterval = -1;
  for ( i = 0; i < rateList->numRates && matchedInterval < 0; i++ ) {
    if ( rateList->rates[i].numerator == rate->numerator &&
        rateList->rates[i].denominator == rate->denominator ) {
      matchedInterval = i;
    }
  }

  if ( matchedInterval < 0 ) {
    fprintf ( stderr, "no matching interval found\n" );
    return -OA_ERR_OUT_OF_RANGE;
  }

  cameraInfo->frameRateNumerator = rate->numerator;
  cameraInfo->frameRateDenominator = rate->denominator;
  cameraInfo->currentFrameRate = matchedInterval;
  _doSetFrameRate ( cameraInfo, cameraInfo->xSize, cameraInfo->ySize );

  return OA_ERR_NONE;
}


static void
_processPayload ( oaCamera* camera, unsigned char* buffer, unsigned int len )
{
  EUVC_STATE*		cameraInfo = camera->_private;
  size_t		headerLength, dataLength;
  uint8_t		headerInfo;
  unsigned int		buffersFree;

  if ( 0 == len ) {
    return;
  }

  headerLength = buffer[0];
  if ( headerLength > len ) {
    fprintf ( stderr, "Weird packet: actual len: %d, header len: %zd\n",
        len, headerLength );
    return;
  }
  dataLength = len - headerLength;
  if ( headerLength < 2 ) {
    headerInfo = 0;
  } else {
    headerInfo = buffer[1];
    if ( headerInfo & 0x40 ) {
      fprintf ( stderr, "Bad packet: error bit set\n" );
      return;
    }

    if ( cameraInfo->streamFrameId != ( headerInfo & 1 ) &&
        cameraInfo->receivedBytes > 0 ) {
      // Frame ID changed, but we saw no EOF for some reason
      _releaseFrame ( cameraInfo );
    }

    cameraInfo->streamFrameId = headerInfo & 1;
  }

  if ( dataLength > 0 ) {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    buffersFree = cameraInfo->buffersFree;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    if ( buffersFree && ( cameraInfo->receivedBytes + dataLength ) <=
        cameraInfo->imageBufferLength ) {
      memcpy (( unsigned char* ) cameraInfo->buffers[
          cameraInfo->nextBuffer ].start + cameraInfo->receivedBytes,
          buffer + headerLength, dataLength );
      cameraInfo->receivedBytes += dataLength;
      if ( headerInfo & 0x2 ) { // EOF
        _releaseFrame ( cameraInfo );
      }
    } else {
      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      cameraInfo->droppedFrames++;
      cameraInfo->receivedBytes = 0;
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    }
  }
}


static void
_releaseFrame ( EUVC_STATE* cameraInfo )
{
  int		nextBuffer = cameraInfo->nextBuffer;

  cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
      OA_CALLBACK_NEW_FRAME;
  cameraInfo->frameCallbacks[ nextBuffer ].callback =
      cameraInfo->streamingCallback.callback;
  cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
      cameraInfo->streamingCallback.callbackArg;
  cameraInfo->frameCallbacks[ nextBuffer ].buffer =
      cameraInfo->buffers[ nextBuffer ].start;
  cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
      cameraInfo->imageBufferLength;
  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  oaDLListAddToTail ( cameraInfo->callbackQueue,
      &cameraInfo->frameCallbacks[ nextBuffer ]);
  cameraInfo->buffersFree--;
  cameraInfo->nextBuffer = ( nextBuffer + 1 ) % cameraInfo->configuredBuffers;
  cameraInfo->receivedBytes = 0;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
  pthread_cond_broadcast ( &cameraInfo->callbackQueued );
}


int
getEUVCControl ( EUVC_STATE* cameraInfo, uint8_t ctrl, int len, int req )
{
  int           ret, i;
  uint8_t       data[4] = { 0xde, 0xad, 0xbe, 0xef };
  unsigned int  val;

  if (( ret = euvcUsbControlMsg ( cameraInfo, USB_DIR_IN |
      USB_CTRL_TYPE_CLASS | USB_RECIP_INTERFACE, req, ctrl << 8,
      cameraInfo->processingUnitId << 8, data, len,
      USB_CTRL_TIMEOUT )) != len ) {
    fprintf ( stderr, "%s requested %d for control %d, got %d\n",
        __FUNCTION__, len, ctrl, ret );
    return ret;
  }

  val = 0;
  i = len - 1;
  do {
    val <<= 8;
    val += data[i--];
  } while ( i >= 0 );

  return val;
}


int
getEUVCTermControl ( EUVC_STATE* cameraInfo, uint8_t ctrl, void* data,
    int len, int req )
{
  int           ret;

  if (( ret = euvcUsbControlMsg ( cameraInfo, USB_DIR_IN |
      USB_CTRL_TYPE_CLASS | USB_RECIP_INTERFACE, req, ctrl << 8,
      cameraInfo->terminalId << 8, data, len, USB_CTRL_TIMEOUT )) != len ) {
    fprintf ( stderr, "%s requested %d for control %d, got %d\n",
        __FUNCTION__, len, ctrl, ret );
    return ret;
  }

  return OA_ERR_NONE;
}


int
setEUVCTermControl ( EUVC_STATE* cameraInfo, uint8_t ctrl, void* data,
    int len, int req )
{
  int           ret;

  if (( ret = euvcUsbControlMsg ( cameraInfo, USB_DIR_OUT |
      USB_CTRL_TYPE_CLASS | USB_RECIP_INTERFACE, req, ctrl << 8,
      cameraInfo->terminalId << 8, data, len, USB_CTRL_TIMEOUT )) != len ) {
    fprintf ( stderr, "%s requested %d for control %d, got %d\n",
        __FUNCTION__, len, ctrl, ret );
    return ret;
  }

  return OA_ERR_NONE;
}


static void
_doSetFrameRate ( EUVC_STATE* cameraInfo, unsigned int x, unsigned int y )
{
  uint32_t	totalPixels, newPixelClock, pixelWidth, xBlanking, yBlanking;
  FRAMERATE*	rate;
  uint8_t	data[4];
  
  // FIX ME -- if this is always the same it could be moved to the
  // connect function
  if ( getEUVCTermControl ( cameraInfo, EUVC_CT_BLANKING_INFO,
      &data, 4, EUVC_GET_CUR )) {
    fprintf ( stderr, "unable to get blanking info\n" );
    return;
  }
  pixelWidth = x * cameraInfo->binMode;
  xBlanking = data[0] + ( data[1] << 8 );
  yBlanking = data[2] + ( data[3] << 8 );
  totalPixels = ( pixelWidth + xBlanking ) * ( y + yBlanking );

  rate = &( cameraInfo->frameRates->rates[ cameraInfo->currentFrameRate ]);
  newPixelClock = totalPixels * rate->denominator / rate->numerator;
  if ( newPixelClock < cameraInfo->minPixelClock ) {
    newPixelClock = cameraInfo->minPixelClock;
  }
  if ( newPixelClock > cameraInfo->maxPixelClock ) {
    newPixelClock = cameraInfo->maxPixelClock;
  }

  data[0] = newPixelClock & 0xff;
  data[1] = ( newPixelClock >> 8 ) & 0xff;
  data[2] = ( newPixelClock >> 16 ) & 0xff;
  data[3] = ( newPixelClock >> 24 ) & 0xff;

  if ( setEUVCTermControl ( cameraInfo, EUVC_CT_PIXEL_CLOCK,
      &data, 4, EUVC_SET_CUR )) {
    fprintf ( stderr, "unable to set clock rate\n" );
    return;
  }
}


const char*
oaEUVCCameraGetMenuString ( oaCamera* camera, int control, int index )
{
  if ( control != OA_CAM_CTRL_AUTO_EXPOSURE_PRIORITY ) {
    fprintf ( stderr, "%s: control not implemented\n", __FUNCTION__ );
    return "";
  }

  switch ( index ) {
    case 0:
      return "Constant frame rate";
      break;
    case 1:
      return "Variable frame rate";
      break;
  }

  return "Unknown";
}
