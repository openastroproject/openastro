/*****************************************************************************
 *
 * QHY5LIIcontroller.c -- Main camera controller thread
 *
 * Copyright 2015,2017,2018,2019 James Fidell (james@openastroproject.org)
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
#include "QHY5LII.h"
#include "QHYusb.h"


static uint16_t mt9m034_seq_data[] = {
  0x0025, 0x5050, 0x2D26, 0x0828, 0x0D17, 0x0926, 0x0028, 0x0526,
  0xA728, 0x0725, 0x8080, 0x2925, 0x0040, 0x2702, 0x1616, 0x2706,
  0x1F17, 0x3626, 0xA617, 0x0326, 0xA417, 0x1F28, 0x0526, 0x2028,
  0x0425, 0x2020, 0x2700, 0x171D, 0x2500, 0x2017, 0x1028, 0x0519,
  0x1703, 0x2706, 0x1703, 0x1741, 0x2660, 0x175A, 0x2317, 0x1122,
  0x1741, 0x2500, 0x9027, 0x0026, 0x1828, 0x002E, 0x2A28, 0x081C,
  0x1470, 0x7003, 0x1470, 0x7004, 0x1470, 0x7005, 0x1470, 0x7009,
  0x170C, 0x0014, 0x0020, 0x0014, 0x0050, 0x0314, 0x0020, 0x0314,
  0x0050, 0x0414, 0x0020, 0x0414, 0x0050, 0x0514, 0x0020, 0x2405,
  0x1400, 0x5001, 0x2550, 0x502D, 0x2608, 0x280D, 0x1709, 0x2600,
  0x2805, 0x26A7, 0x2807, 0x2580, 0x8029, 0x2500, 0x4027, 0x0216,
  0x1627, 0x0620, 0x1736, 0x26A6, 0x1703, 0x26A4, 0x171F, 0x2805,
  0x2620, 0x2804, 0x2520, 0x2027, 0x0017, 0x1D25, 0x0020, 0x1710,
  0x2805, 0x1A17, 0x0327, 0x0617, 0x0317, 0x4126, 0x6017, 0xAE25,
  0x0090, 0x2700, 0x2618, 0x2800, 0x2E2A, 0x2808, 0x1D05, 0x1470,
  0x7009, 0x1720, 0x1400, 0x2024, 0x1400, 0x5002, 0x2550, 0x502D,
  0x2608, 0x280D, 0x1709, 0x2600, 0x2805, 0x26A7, 0x2807, 0x2580,
  0x8029, 0x2500, 0x4027, 0x0216, 0x1627, 0x0617, 0x3626, 0xA617,
  0x0326, 0xA417, 0x1F28, 0x0526, 0x2028, 0x0425, 0x2020, 0x2700,
  0x171D, 0x2500, 0x2021, 0x1710, 0x2805, 0x1B17, 0x0327, 0x0617,
  0x0317, 0x4126, 0x6017, 0xAE25, 0x0090, 0x2700, 0x2618, 0x2800,
  0x2E2A, 0x2808, 0x1E17, 0x0A05, 0x1470, 0x7009, 0x1616, 0x1616,
  0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616,
  0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1616, 0x1400, 0x2024,
  0x1400, 0x502B, 0x302C, 0x2C2C, 0x2C00, 0x0225, 0x5050, 0x2D26,
  0x0828, 0x0D17, 0x0926, 0x0028, 0x0526, 0xA728, 0x0725, 0x8080,
  0x2917, 0x0525, 0x0040, 0x2702, 0x1616, 0x2706, 0x1736, 0x26A6,
  0x1703, 0x26A4, 0x171F, 0x2805, 0x2620, 0x2804, 0x2520, 0x2027,
  0x0017, 0x1E25, 0x0020, 0x2117, 0x1028, 0x051B, 0x1703, 0x2706,
  0x1703, 0x1747, 0x2660, 0x17AE, 0x2500, 0x9027, 0x0026, 0x1828,
  0x002E, 0x2A28, 0x081E, 0x0831, 0x1440, 0x4014, 0x2020, 0x1410,
  0x1034, 0x1400, 0x1014, 0x0020, 0x1400, 0x4013, 0x1802, 0x1470,
  0x7004, 0x1470, 0x7003, 0x1470, 0x7017, 0x2002, 0x1400, 0x2002,
  0x1400, 0x5004, 0x1400, 0x2004, 0x1400, 0x5022, 0x0314, 0x0020,
  0x0314, 0x0050, 0x2C2C, 0x2C2C
};


static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( QHY_STATE*, OA_COMMAND* );
static int	_processSetResolution ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStop ( QHY_STATE*, OA_COMMAND* );
static int	_doSetGain ( QHY_STATE*, unsigned int );
static int	_doSetHighSpeed ( QHY_STATE*, unsigned int );
static int	_doSetUSBTraffic ( QHY_STATE*, unsigned int );
static int	_doSetBitDepth ( QHY_STATE*, unsigned int );
static int	_doSetExposure ( QHY_STATE*, unsigned int, int );
static int	_doSetHDR ( QHY_STATE*, unsigned int );
static int	_doSetResolution ( QHY_STATE*, int, int );
static void	_initialiseRegisters ( QHY_STATE* );
static void	_setPLLRegister ( QHY_STATE*, unsigned int );
static int	_abortFrame ( QHY_STATE* );
static int	_doReadTemperature ( QHY_STATE* );
static void     _processPayload ( oaCamera*, unsigned char*, unsigned int );
static void     _releaseFrame ( QHY_STATE* );


void*
oacamQHY5LIIcontroller ( void* param )
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
            resultCode = _processSetControl ( camera, command );
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
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
  QHY_STATE*    cameraInfo = camera->_private;

  oaControlValue	*valp = command->commandData;
  int			control = command->controlId;

  int32_t val_s32;
  int64_t val_s64;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d, ? )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      if ( valp->int32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentGain = valp->int32;
      _doSetGain ( cameraInfo, cameraInfo->currentGain );
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( valp->valueType != OA_CTRL_TYPE_INT64 ) {
        fprintf ( stderr, "%s: invalid control type %d where int64 expected\n",
            __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s64 = valp->int64;
      if ( val_s64 < 1 ) { val_s64 = 1; }
      cameraInfo->currentExposure = val_s64;
      _doSetExposure ( cameraInfo, val_s64 / 1000, 0 );
      break;

    case OA_CAM_CTRL_FRAME_FORMAT:
    {
      int format;

      if ( valp->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      format = valp->discrete;
      
      if ( cameraInfo->isColour && ( format == OA_PIX_FMT_GRBG8 ||
          format == OA_PIX_FMT_GRBG16BE )) {
        cameraInfo->currentFrameFormat = format;
        cameraInfo->currentBitDepth = oaFrameFormats[ format ].bitsPerPixel;
      } else {
        if ( !cameraInfo->isColour && ( format == OA_PIX_FMT_GREY8 ||
            format == OA_PIX_FMT_GREY16BE )) {
          cameraInfo->currentFrameFormat = format;
          cameraInfo->currentBitDepth = oaFrameFormats[ format ].bitsPerPixel;
        }
      }
      oaQHY5LIISetAllControls ( camera );
      break;
    }

    case OA_CAM_CTRL_HIGHSPEED:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentHighSpeed = valp->int32;
      _doSetHighSpeed ( cameraInfo, cameraInfo->currentHighSpeed );
      break;

    case OA_CAM_CTRL_USBTRAFFIC:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = valp->int32;
      if ( val_s32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentUSBTraffic = val_s32;
      _doSetUSBTraffic ( cameraInfo, cameraInfo->currentUSBTraffic );
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = valp->int32;
      if ( val_s32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentBlueBalance = val_s32;
      _doSetGain ( cameraInfo, cameraInfo->currentGain );
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      if ( valp->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = valp->int32;
      if ( val_s32 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentRedBalance = val_s32;
      _doSetGain ( cameraInfo, cameraInfo->currentGain );
      break;

    case OA_CAM_CTRL_DROPPED_RESET:
      // droppedFrames could be mutexed, but it's not the end of the world
      cameraInfo->droppedFrames = 0;
      break;

    case OA_CAM_CTRL_GREEN_BALANCE:
      fprintf ( stderr, "QHY5L-II: No idea how to set green balance\n" );
      break;

    default:
      fprintf ( stderr, "QHY5L-II: unrecognised control %d in %s\n", control,
          __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;

}


static int
_doSetGain ( QHY_STATE* cameraInfo, unsigned int gain )
{
  static double		C[8] = {10, 8, 5, 4, 2.5, 2, 1.25, 1};
  double		S[8];
  int			A[8], B[8];
  double		error[8];
  double		minValue = 0, rebasedGain;
  unsigned short	REG30B0, baseDGain, offset, data;
  int			i, minValueIndex = 0, limit;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d )\n",
      __FUNCTION__, gain );

  if ( cameraInfo->isColour ) {
    // This re-bases the gain from 0 to [ COLOUR_GAIN_MIN/10,
    // COLOUR_GAIN_MAX/10 ]
    if ( gain < 26 ) { gain = 26; }
    rebasedGain = (( QHY5LII_COLOUR_GAIN_MAX - QHY5LII_COLOUR_GAIN_MIN ) *
        gain / 1000.0 + QHY5LII_COLOUR_GAIN_MIN  ) / 10.0;
  } else {
    // This re-bases the gain from 0 to [ MONO_GAIN_MIN/10, MONO_GAIN_MAX/10 ]
    rebasedGain = (( QHY5LII_MONO_GAIN_MAX - QHY5LII_MONO_GAIN_MIN ) *
        gain / 1000.0 + QHY5LII_MONO_GAIN_MIN  ) / 10.0;
  }

  REG30B0 = ( cameraInfo->longExposureMode ? 0x5330 : 0x1330 ) & ~0x0030;
  limit = cameraInfo->isColour ? 3 : 7;

  for ( i = 0; i < 8; i++ ) {
    S[i] = rebasedGain / C[i];
    A[i] = S[i];
    B[i] = ( S[i] - A[i] ) / 0.03125;
    if ( !A[i] || A[i] > limit ) {
      A[i] = 10000;
    }
    error[i] = fabs (( A[i] + B[i] * 0.03125 ) * C[i] - rebasedGain );
    if (( 0 == i ) ||  minValue > error[i] ) {
      minValue = error[i];
      minValueIndex = i;
    }
  }

  offset = 0x30 - 0x10 * ( minValueIndex >> 1 );
  data = 0xd308 - 0x100 * ( minValueIndex & 1 );

  if ( cameraInfo->longExposureMode ) {
    _doSetExposure ( cameraInfo, 1, 1 );
  }

  _i2cWrite16 ( cameraInfo, MT9M034_DIGITAL_TEST, REG30B0 + offset );
  _i2cWrite16 ( cameraInfo, MT9M034_DAC_LD_24_25, data );

  baseDGain = B[ minValueIndex ] + A[ minValueIndex ] * 32;

  if ( cameraInfo->isColour ) {
    _i2cWrite16 ( cameraInfo, MT9M034_BLUE_GAIN, baseDGain *
        cameraInfo->currentBlueBalance / 100 );
    _i2cWrite16 ( cameraInfo, MT9M034_RED_GAIN, baseDGain *
        cameraInfo->currentRedBalance / 100 );
    _i2cWrite16 ( cameraInfo, MT9M034_GREEN2_GAIN, baseDGain );
    _i2cWrite16 ( cameraInfo, MT9M034_GREEN1_GAIN, baseDGain );
  } else {
    _i2cWrite16 ( cameraInfo, MT9M034_GLOBAL_GAIN, baseDGain );
  }

  if ( cameraInfo->longExposureMode ) {
    usleep ( 500000 );
    _doSetExposure ( cameraInfo, cameraInfo->currentExposure, 1 );
  }

  return OA_ERR_NONE;
}


static int
_doSetHighSpeed ( QHY_STATE* cameraInfo, unsigned int value )
{
  unsigned char	buf;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d )\n",
      __FUNCTION__, value );

  if ( value ) {
    if ( 8 == cameraInfo->currentBitDepth ) {
      cameraInfo->CMOSClock = 48;
      buf = 2;
    } else {
      cameraInfo->CMOSClock = 24;
      buf = 1;
    }
  } else {
    if ( 8 == cameraInfo->currentBitDepth ) {
      cameraInfo->CMOSClock = 24;
      buf = 1;
    } else {
      cameraInfo->CMOSClock = 12;
      buf = 0;
    }
  }
  _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xc8, 0, 0, &buf, 1, 0 );
  return OA_ERR_NONE;
}


static int
_doSetBitDepth ( QHY_STATE* cameraInfo, unsigned int depth )
{
  unsigned char buf;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d )\n",
      __FUNCTION__, depth );

  buf = ( depth <= 8 ) ? 0 : 1;
  _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xcd, 0, 0, &buf, 1, 0 );
  cameraInfo->frameSize = cameraInfo->xSize * cameraInfo->ySize *
      (( depth <= 8 ) ? 1 : 2 );
  cameraInfo->captureLength = cameraInfo->frameSize + QHY5LII_EOF_LEN;
  return OA_ERR_NONE;
}


static int
_doSetUSBTraffic ( QHY_STATE* cameraInfo, unsigned int value )
{
  unsigned int	base;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d )\n",
      __FUNCTION__, value );

  base = ( QHY5LII_IMAGE_WIDTH == cameraInfo->xSize ) ? 1650 : 1388;
  _i2cWrite16 ( cameraInfo, MT9M034_LINE_LENGTH_PCK, base + value * 50 );

  return OA_ERR_NONE;
}


static int
_doSetHDR ( QHY_STATE* cameraInfo, unsigned int state )
{
  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d, %d )\n",
      __FUNCTION__, state );

  if ( state ) {
    fprintf ( stderr, "QHY5L-II: %s: function unimplemented\n", __FUNCTION__ );
  }

  return OA_ERR_NONE;
}


static int
_doSetExposure ( QHY_STATE* cameraInfo, unsigned int value, int doAbortFrame )
{
  double		pixelTime, rowTime, maxShortExposureTime;
  unsigned short	rowLength, shortExposureTime, coarseIntegrationTime;
  unsigned long		newTimeMillisec, newTimeMicrosec;
  unsigned char		buf[4];

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d )\n",
      __FUNCTION__, value );

  pixelTime = 1.0 / cameraInfo->CMOSClock / cameraInfo->PLLRatio;

  rowLength = _i2cRead16 ( cameraInfo, MT9M034_LINE_LENGTH_PCK );
  rowTime = rowLength * pixelTime;
  maxShortExposureTime = 65000 * rowTime;
  newTimeMicrosec = value * 1000;

  if ( newTimeMicrosec > maxShortExposureTime ) {
    fprintf ( stderr, "%s: long exposure mode may not work\n", __FUNCTION__ );
    cameraInfo->longExposureMode = 1;
    shortExposureTime = 65000;
    _i2cWrite16 ( cameraInfo, MT9M034_COARSE_INTEGRATION_TIME,
        shortExposureTime );
    newTimeMillisec = ( newTimeMicrosec - maxShortExposureTime ) / 1000;
    buf[0] = 0;
    buf[1] = ( newTimeMillisec >> 16 ) & 0xff;
    buf[2] = ( newTimeMillisec >> 8 ) & 0xff;
    buf[3] = newTimeMillisec & 0xff;
    _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xc1, 0, 0, buf, 4, 0 );
  } else {
    // Aborting the frame here causes an overflow on the libusb bulk transfer.
    // On Linux that appears to be handled ok, but on OSX it looks like the
    // USB subsystem just gives up on the device.
    // So, for the time being we don't abort the frame if exposure setting
    // is changed when the camera is running.
    if ( doAbortFrame ) {
      _abortFrame ( cameraInfo );
    }
    cameraInfo->longExposureMode = 0;
    if (( coarseIntegrationTime = newTimeMicrosec / rowTime ) < 1 ) {
      coarseIntegrationTime = 1;
    }
    _i2cWrite16 ( cameraInfo, MT9M034_COARSE_INTEGRATION_TIME,
        coarseIntegrationTime );
  }

  return OA_ERR_NONE;
}


static int
_abortFrame ( QHY_STATE* cameraInfo )
{
  unsigned char buf[4] = { 0, 0, 0, 0 };

  oacamDebugMsg ( DEBUG_CAM_CMD, "QHY5L-II: command: %s()\n",
      __FUNCTION__ );

  _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xc1, 0, 0, buf, 4, 0 );
  usleep ( 100 );
  return OA_ERR_NONE;
}


void
oaQHY5LIISetAllControls ( oaCamera* camera )
{
  QHY_STATE*		cameraInfo = camera->_private;
  int restart = 0;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s()\n",
      __FUNCTION__ );

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    ( void ) _processStreamingStop ( cameraInfo, 0 );
  }
  _doSetBitDepth ( cameraInfo, cameraInfo->currentBitDepth );
  _doSetHDR ( cameraInfo, cameraInfo->currentHDR );
  _doSetUSBTraffic ( cameraInfo, cameraInfo->currentUSBTraffic );
  _doSetHighSpeed ( cameraInfo, cameraInfo->currentHighSpeed );
  _doSetResolution ( cameraInfo, cameraInfo->xSize, cameraInfo->ySize );
  _doSetExposure ( cameraInfo, cameraInfo->currentExposure, 0 );
  _doSetGain ( cameraInfo, cameraInfo->currentGain );
  _doSetUSBTraffic ( cameraInfo, cameraInfo->currentUSBTraffic );
  _doSetExposure ( cameraInfo, cameraInfo->currentExposure, 0 );
  if ( restart ) {
    ( void ) _processStreamingStart ( camera, 0 );
  }
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
  unsigned int	clock, xStart, yStart, xEnd, yEnd, frameLength, rowLength;

  oacamDebugMsg ( DEBUG_CAM_CMD, "QHY5L-II: command: %s ( %d, %d )\n",
      __FUNCTION__, x, y );

  frameLength = y + 26;
  rowLength = 1388;

  if ( x < 320 && y < 240 ) {
    clock = 1;
  } else if ( x < 640 && y < 480 ) {
    clock = 1;
  } else if ( x < 800 && y < 600 ) {
    clock = 2;
  } else if ( x < 1024 && y < 768 ) {
    clock = 0;
  } else {
    clock = 0;
    frameLength = 990;
    rowLength = 1650;
  }
  rowLength += cameraInfo->currentUSBTraffic * 50;

  _initialiseRegisters ( cameraInfo );
  _setPLLRegister ( cameraInfo, clock );

  xStart = 4 + ( QHY5LII_IMAGE_WIDTH - x ) / 2;
  yStart = 4 + ( QHY5LII_IMAGE_HEIGHT - y ) / 2;
  xEnd = xStart + x - 1;
  yEnd = yStart + y - 1;

  _i2cWrite16 ( cameraInfo, MT9M034_Y_ADDR_START, yStart );
  _i2cWrite16 ( cameraInfo, MT9M034_X_ADDR_START, xStart );
  _i2cWrite16 ( cameraInfo, MT9M034_Y_ADDR_END, yEnd );
  _i2cWrite16 ( cameraInfo, MT9M034_X_ADDR_END, xEnd );
  _i2cWrite16 ( cameraInfo, MT9M034_FRAME_LENGTH_LINES, frameLength );
  _i2cWrite16 ( cameraInfo, MT9M034_LINE_LENGTH_PCK, rowLength );
  _i2cWrite16 ( cameraInfo, MT9M034_RESET_REGISTER, 0x10DC );

  cameraInfo->frameSize = x * y *
      (( cameraInfo->currentBitDepth <= 8 ) ? 1 : 2 );
  cameraInfo->captureLength = cameraInfo->frameSize + QHY5LII_EOF_LEN;

  return OA_ERR_NONE;
}


static void
_initialiseRegisters ( QHY_STATE* cameraInfo )
{
  int i;

  _i2cWrite16 ( cameraInfo, MT9M034_RESET_REGISTER, 0x0001);
  _i2cWrite16 ( cameraInfo, MT9M034_RESET_REGISTER, 0x10D8);
  usleep(100000);

  // Linear sequencer

  _i2cWrite16 ( cameraInfo, MT9M034_SEQ_CTRL_PORT, 0x8000 );

  for ( i = 0; i < 276; i++ ) {
    _i2cWrite16 ( cameraInfo, MT9M034_SEQ_DATA_PORT, mt9m034_seq_data[i] );
  }

  // RESERVED_MFR_309E
  _i2cWrite16 ( cameraInfo, MT9M034_ERS_PROG_START_ADDR, 0x018A );


  _i2cWrite16 ( cameraInfo, MT9M034_RESET_REGISTER, 0x10D8 );
  _i2cWrite16 ( cameraInfo, MT9M034_MODE_CTRL, 0x0029 );
  _i2cWrite16 ( cameraInfo, MT9M034_DATA_PEDESTAL, 0x00C8 );
  _i2cWrite16 ( cameraInfo, 0x3EDA, 0x0F03); // RESERVED_MFR_3EDA
  _i2cWrite16 ( cameraInfo, 0x3EDE, 0xC007); // RESERVED_MFR_3EDE
  _i2cWrite16 ( cameraInfo, 0x3ED8, 0x01EF); // RESERVED_MFR_3ED8
  _i2cWrite16 ( cameraInfo, 0x3EE2, 0xA46B); // RESERVED_MFR_3EE2
  _i2cWrite16 ( cameraInfo, 0x3EE0, 0x067D); // RESERVED_MFR_3EE0
  _i2cWrite16 ( cameraInfo, 0x3EDC, 0x0070); // RESERVED_MFR_3EDC
  _i2cWrite16 ( cameraInfo, MT9M034_DARK_CONTROL, 0x0404 );
  _i2cWrite16 ( cameraInfo, 0x3EE6, 0x4303); // RESERVED_MFR_3EE6
  _i2cWrite16 ( cameraInfo, MT9M034_DAC_LD_24_25, 0xD208 );
  _i2cWrite16 ( cameraInfo, 0x3ED6, 0x00BD); // RESERVED_MFR_3ED6
  _i2cWrite16 ( cameraInfo, 0x3EE6, 0x8303); // RESERVED_MFR_3EE6
  _i2cWrite16 ( cameraInfo, 0x30E4, 0x6372); // RESERVED_MFR_30E4
  _i2cWrite16 ( cameraInfo, 0x30E2, 0x7253); // RESERVED_MFR_30E2
  _i2cWrite16 ( cameraInfo, 0x30E0, 0x5470); // RESERVED_MFR_30E0
  _i2cWrite16 ( cameraInfo, 0x30E6, 0xC4CC); // RESERVED_MFR_30E6
  _i2cWrite16 ( cameraInfo, 0x30E8, 0x8050); // RESERVED_MFR_30E8
  usleep(200);
  _i2cWrite16 ( cameraInfo, MT9M034_VT_SYS_CLK_DIV, 14 );
  _i2cWrite16 ( cameraInfo, MT9M034_VT_PIX_CLK_DIV, 1 );
  _i2cWrite16 ( cameraInfo, MT9M034_PRE_PLL_CLK_DIV, 3 );
  _i2cWrite16 ( cameraInfo, MT9M034_PLL_MULTIPLIER, 65 );
  _i2cWrite16 ( cameraInfo, MT9M034_MODE_CTRL, 0x0029 );
  _i2cWrite16 ( cameraInfo, MT9M034_DIGITAL_TEST, 0x5330 );
  _i2cWrite16 ( cameraInfo, MT9M034_GLOBAL_GAIN, 0x00ff );
  _i2cWrite16 ( cameraInfo, MT9M034_COARSE_INTEGRATION_TIME, 0x0020 );
  _i2cWrite16 ( cameraInfo, MT9M034_EMBEDDED_DATA_CTRL, 0x1802 );
}


static void
_setPLLRegister ( QHY_STATE* cameraInfo, unsigned int clock )
{
  static unsigned short	multiplier[] = { 42, 65, 57 };
  double		pllRatio = 1.0;

   if ( clock && !cameraInfo->longExposureMode ) {
     pllRatio = multiplier[ clock ] / 42.0;
   }
   cameraInfo->PLLRatio = pllRatio;

   _i2cWrite16 ( cameraInfo, MT9M034_VT_SYS_CLK_DIV, 14 );
   _i2cWrite16 ( cameraInfo, MT9M034_VT_PIX_CLK_DIV, 1 );
   _i2cWrite16 ( cameraInfo, MT9M034_PRE_PLL_CLK_DIV, 3 );

   _i2cWrite16 ( cameraInfo, MT9M034_PLL_MULTIPLIER, multiplier[ clock ]);
   _i2cWrite16 ( cameraInfo, MT9M034_MODE_CTRL, 0x0029 );

   _i2cWrite16 ( cameraInfo, MT9M034_DIGITAL_TEST,
        cameraInfo->longExposureMode ?  0x5330 : 0x1330 ); // 5370: PLL BYPASS   1370  USE PLL

   _i2cWrite16 ( cameraInfo, MT9M034_GLOBAL_GAIN, 0x00ff );
   _i2cWrite16 ( cameraInfo, MT9M034_COARSE_INTEGRATION_TIME, 0x0020 );

   _i2cWrite16 ( cameraInfo, MT9M034_EMBEDDED_DATA_CTRL, 0x1802);
}


static int
_processGetControl ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	valp = command->resultData;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = cameraInfo->currentGain;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      valp->valueType = OA_CTRL_TYPE_INT64;
      valp->int64 = cameraInfo->currentExposure;
      break;

    case OA_CAM_CTRL_HIGHSPEED:
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      valp->boolean = cameraInfo->currentHighSpeed;
      break;

    case OA_CAM_CTRL_USBTRAFFIC:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = cameraInfo->currentUSBTraffic;
      break;

    case OA_CAM_CTRL_HDR:
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      valp->boolean = cameraInfo->currentHDR;
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

    case OA_CAM_CTRL_TEMPERATURE:
      valp->valueType = OA_CTRL_TYPE_READONLY;
      valp->readonly = _doReadTemperature ( cameraInfo );
      break;

    case OA_CAM_CTRL_DROPPED:
      valp->valueType = OA_CTRL_TYPE_READONLY;
      valp->readonly = cameraInfo->droppedFrames;
      break;

    default:
      fprintf ( stderr, "Unimplemented control %d in QHY5L-II:%s\n", control,
          __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


libusb_transfer_cb_fn
_qhy5liiVideoStreamCallback ( struct libusb_transfer* transfer )
{ 
  oaCamera*     camera = transfer->user_data;
  QHY_STATE*    cameraInfo = camera->_private;
  int           resubmit = 1, streaming;

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

      for ( i = 0; i < QHY_NUM_TRANSFER_BUFS; i++ ) {
        if ( cameraInfo->transfers[i] == transfer ) {
          free ( transfer->buffer );
          libusb_free_transfer ( transfer );
          cameraInfo->transfers[i] = 0;
          break;
        }
      }

      if ( QHY_NUM_TRANSFER_BUFS == i ) {
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
      for ( i = 0; i < QHY_NUM_TRANSFER_BUFS; i++ ) {
        if ( cameraInfo->transfers[i] == transfer ) {
          fprintf ( stderr, "Freeing orphan transfer %d (%p)\n", i, transfer );
          free ( transfer->buffer );
          libusb_free_transfer ( transfer );
          cameraInfo->transfers[i] = 0;
        }
      }
      if ( QHY_NUM_TRANSFER_BUFS == i ) {
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
  QHY_STATE*			cameraInfo = camera->_private;
  CALLBACK*			cb;
  int				txId, ret, txBufferSize, numTxBuffers;
  struct libusb_transfer*	transfer;
  unsigned char	buf[1] = { 100 };

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  if ( command ) {
    cb = command->commandData;
    cameraInfo->streamingCallback.callback = cb->callback;
    cameraInfo->streamingCallback.callbackArg = cb->callbackArg;
  }

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
        fprintf ( stderr, "malloc failed.  Need to free buffer\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      libusb_fill_bulk_transfer ( transfer, cameraInfo->usbHandle,
          QHY_BULK_ENDP_IN, cameraInfo->transferBuffers [ txId ],
          txBufferSize, ( libusb_transfer_cb_fn ) _qhy5liiVideoStreamCallback,
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

  _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, QHY_REQ_BEGIN_VIDEO,
      0, 0, buf, 1, 0 );

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_processStreamingStop ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  int		queueEmpty, i, res, allReleased;
  unsigned char	buf[4] = { 0, 0, 0, 0 };

  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xc1, 0, 0, buf, 4, 0 );

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
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
  unsigned int          buffersFree, dropFrame;
  unsigned char*	p;

  if ( 0 == len ) {
    return;
  }
 
  dropFrame = 0;

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
  if ( buffersFree && ( cameraInfo->receivedBytes + len ) <=
      cameraInfo->captureLength ) {
    memcpy (( unsigned char* ) cameraInfo->buffers[
        cameraInfo->nextBuffer ].start + cameraInfo->receivedBytes,
        buffer, len );
    cameraInfo->receivedBytes += len;
    // It seems that the last five bytes of the frame should be
    // 0xaa, 0x11, 0xcc, 0xee, 0xXX
    p = ( unsigned char* ) cameraInfo->buffers[
      cameraInfo->nextBuffer ].start + cameraInfo->receivedBytes -
          QHY5LII_EOF_LEN;
    if ( p[0] == 0xaa && p[1] == 0x11 && p[2] == 0xcc && p[3] == 0xee ) {
      if ( cameraInfo->receivedBytes == cameraInfo->captureLength ) {
        _releaseFrame ( cameraInfo );
      } else {
        if ( cameraInfo->receivedBytes == QHY5LII_EOF_LEN ) {
          cameraInfo->receivedBytes = 0;
        } else {
          dropFrame = 1;
        }
      }
    }
  } else {
    dropFrame = 1;
  }

  if ( dropFrame ) {
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


static int
_doReadTemperature ( QHY_STATE* cameraInfo )
{
  double gradient, baseline;
  uint16_t value, calibrationVal1, calibrationVal2;

  // start measuring
  _i2cWrite16 ( cameraInfo, MT9M034_TEMPERATURE_CONTROL, 0x0011 );

  calibrationVal1 = _i2cRead16 ( cameraInfo, MT9M034_TEMPERATURE_CALIB_1 );
  calibrationVal2 = _i2cRead16 ( cameraInfo, MT9M034_TEMPERATURE_CALIB_2 );

  // stop measuring
  _i2cWrite16 ( cameraInfo, MT9M034_TEMPERATURE_CONTROL, 0x0000 );
  value = _i2cRead16 ( cameraInfo, MT9M034_TEMPERATURE );

  gradient = ( 70.0 - 55.0 ) / ( calibrationVal1 - calibrationVal2 );
  baseline = gradient * calibrationVal1 - 70.0;
  return ( int )(( gradient * value - baseline ) * 10 );
}
