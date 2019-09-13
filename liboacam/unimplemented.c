/*****************************************************************************
 *
 * unimplemented.c -- catch-all for unimplemented camera functions
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

#include <oa_common.h>
#include <openastro/camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"


static oaCamera*
_initCamera ( oaCameraDevice* device )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return 0;
}


static int
_closeCamera ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


/*
static int
_resetCamera ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}
*/


static int
_startStreaming ( oaCamera* camera, void* (*callback)(void*, void*, int,
		void* ), void* callbackArg )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_stopStreaming ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_isStreaming ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static const char*
_getMenuString ( oaCamera* camera, int control, int index )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return "";
}


static int
_getControlRange ( oaCamera* camera, int c, int64_t* p1, int64_t* p2,
    int64_t* p3, int64_t* p4 )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_getControlDiscreteSet ( oaCamera* camera, int c, int32_t* p1, int64_t** p2 )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_readControl ( oaCamera* camera, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_setControl ( oaCamera* camera, int c, oaControlValue* v, int dontWait )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_testControl ( oaCamera* camera, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_setResolution ( oaCamera* camera, int x, int y )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_setROI ( oaCamera* camera, int x, int y )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_setFrameInterval ( oaCamera* camera, int n, int d )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


/*
static int
_setControlMulti ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}

*/

static const FRAMESIZES*
_enumerateFrameSizes ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return 0;
}


static const FRAMERATES*
_enumerateFrameRates ( oaCamera* camera, int x, int y )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return 0;
}


static int
_getFramePixelFormat ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_testROISize ( oaCamera* camera, unsigned int tx, unsigned int ty,
    unsigned int* sxp, unsigned int *syp )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_isAuto ( oaCamera* camera, int c )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_getAutoWBManualSetting ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}

/*
static int
_cameraLoadFirmware ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}
*/


static int
_deviceLoadFirmware ( oaCameraDevice* device )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_startExposure ( oaCamera* camera, time_t when,
		void* (*callback)(void*, void*, int, void* ), void* callbackArg )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_abortExposure ( oaCamera* camera )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      camera->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


void
_oaInitCameraDeviceFunctionPointers ( oaCameraDevice* device )
{
  device->initCamera = _initCamera;
  device->loadFirmware = _deviceLoadFirmware;
}


void
_oaInitCameraFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = _initCamera;
  camera->funcs.closeCamera = _closeCamera;

  camera->funcs.readControl = _readControl;
  camera->funcs.setControl = _setControl;
  camera->funcs.testControl = _testControl;
  camera->funcs.getControlRange = _getControlRange;
  camera->funcs.getControlDiscreteSet = _getControlDiscreteSet;

  camera->funcs.startStreaming = _startStreaming;
  camera->funcs.stopStreaming = _stopStreaming;
  camera->funcs.isStreaming = _isStreaming;

  camera->funcs.setResolution = _setResolution;
  camera->funcs.setROI = _setROI;
  camera->funcs.setFrameInterval = _setFrameInterval;

/*
  camera->funcs.resetCamera = _resetCamera;
  camera->funcs.loadFirmware = _cameraLoadFirmware;
*/
  camera->funcs.hasAuto = oacamHasAuto;
  camera->funcs.isAuto = _isAuto;
/*
  camera->funcs.setControlMulti = _setControlMulti;
*/
  camera->funcs.enumerateFrameSizes = _enumerateFrameSizes;
  camera->funcs.enumerateFrameRates = _enumerateFrameRates;
  camera->funcs.getFramePixelFormat = _getFramePixelFormat;
  camera->funcs.testROISize = _testROISize;

  camera->funcs.getMenuString = _getMenuString;
  camera->funcs.getAutoWBManualSetting = _getAutoWBManualSetting;

  camera->funcs.startExposure = _startExposure;
  camera->funcs.abortExposure = _abortExposure;
}
