/*****************************************************************************
 *
 * Spinconnect.c -- Initialise Point Grey Spinnaker-based cameras
 *
 * Copyright 2018,2019,2021
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

#include <spinc/SpinnakerC.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/demosaic.h>
#include <openastro/video.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "Spinoacam.h"
#include "Spin.h"
#include "Spinstate.h"
#include "Spinstrings.h"


static void	_spinInitFunctionPointers ( oaCamera* );
static int	_processCameraEntry ( spinCamera, oaCamera* );
static void	_showIntegerNode ( spinNodeHandle, bool8_t );
static void	_showBooleanNode ( spinNodeHandle );
static void	_showFloatNode ( spinNodeHandle, bool8_t );
//static void	_showStringNode ( spinNodeHandle );
static void	_showEnumerationNode ( spinNodeHandle, int );
static int	_checkGainControls ( spinNodeMapHandle, oaCamera* );
static int	_checkGammaControls ( spinNodeMapHandle, oaCamera* );
static int	_checkHueControls ( spinNodeMapHandle, oaCamera* );
static int	_checkSaturationControls ( spinNodeMapHandle, oaCamera* );
static int	_checkSharpnessControls ( spinNodeMapHandle, oaCamera* );
static int	_checkBlackLevelControls ( spinNodeMapHandle, oaCamera* );
static int	_checkWhiteBalanceControls ( spinNodeMapHandle, oaCamera* );
static int	_checkResetControls ( spinNodeMapHandle, oaCamera* );
static int	_checkTemperatureControls ( spinNodeMapHandle, oaCamera* );
static int	_checkExposureControls ( spinNodeMapHandle, oaCamera* );
static int	_checkAcquisitionControls ( spinNodeMapHandle, oaCamera* );
static int	_checkTriggerControls ( spinNodeMapHandle, oaCamera* );
static int	_checkBinningControls ( spinNodeMapHandle, oaCamera* );
static int	_checkFrameSizeControls ( spinNodeMapHandle, oaCamera* );
static int	_checkFrameFormatControls ( spinNodeMapHandle, oaCamera* );
static int	_checkFlipControls ( spinNodeMapHandle, oaCamera* );
static int	_checkUnknownControls ( spinNodeMapHandle, oaCamera* );
static int	_getNodeData ( spinNodeMapHandle, const char*, spinNodeHandle*,
								bool8_t*, bool8_t*, bool8_t*, bool8_t*, spinNodeType* );


oaCamera*
oaSpinInitCamera ( oaCameraDevice* device )
{
  oaCamera*					camera;
  SPINNAKER_STATE*	cameraInfo;
  COMMON_INFO*			commonInfo;
  DEVICE_INFO*			devInfo;
  spinSystem				systemHandle;
  spinInterfaceList	ifaceListHandle = 0;
  size_t						numInterfaces = 0;
  spinCameraList		cameraListHandle = 0;
  size_t						numCameras = 0;
  spinInterface			ifaceHandle = 0;
  spinCamera				cameraHandle;
  spinNodeMapHandle	cameraNodeMapHandle = 0;
  spinNodeHandle		deviceIdHandle, deviceTypeHandle, valueHandle;
  bool8_t						deviceIdAvailable = False;
  bool8_t						deviceIdReadable = False;
  char							deviceId[ SPINNAKER_MAX_BUFF_LEN ];
  size_t						deviceIdLen = SPINNAKER_MAX_BUFF_LEN;
  unsigned int			i, j, found;
	void*							tmpPtr;
	spinError					err;
	int64_t						enumValue;

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( SPINNAKER_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  _spinInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;
	camera->interface = device->interface;

  camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;

  if (( *p_spinSystemGetInstance )( &systemHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get system instance", __func__ );
		FREE_DATA_STRUCTS;
    return 0;
  }

  if (( *p_spinInterfaceListCreateEmpty )( &ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't create empty interface list", __func__ );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  if (( *p_spinSystemGetInterfaces )( systemHandle, ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get interfaces", __func__ );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  if (( *p_spinInterfaceListGetSize )( ifaceListHandle, &numInterfaces ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get size of interface list",
				__func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  if ( !numInterfaces ) {
    oaLogError ( OA_LOG_CAMERA, "%s: No interfaces found", __func__ );
    ( void ) ( *p_spinInterfaceListClear  )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy  )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance  )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't create empty camera list",
				__func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  if (( *p_spinSystemGetCameras )( systemHandle, cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera list", __func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get size of camera list", __func__ );
    ( void ) ( *p_spinCameraListClear )( cameraListHandle );
    ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  ( void ) ( *p_spinCameraListClear )( cameraListHandle );
  ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
  cameraListHandle = 0;
  if ( !numCameras ) {
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

  found = 0;
  for ( i = 0; i < numInterfaces && !found; i++ ) {
    if (( *p_spinInterfaceListGet )( ifaceListHandle, i, &ifaceHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface from list",
					__func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
			FREE_DATA_STRUCTS;
      return 0;
    }

    if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't create empty camera list",
					__func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
			FREE_DATA_STRUCTS;
      return 0;
    }

    if (( *p_spinInterfaceGetCameras )( ifaceHandle, cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera list",
					__func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
			FREE_DATA_STRUCTS;
      return 0;
    }

    if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera count",
					__func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
			FREE_DATA_STRUCTS;
      return 0;
    }

    if ( numCameras ) {
      for ( j = 0; j < numCameras && !found; j++ ) {
        if (( *p_spinCameraListGet )( cameraListHandle, j, &cameraHandle ) !=
            SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera",
							__func__ );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
					FREE_DATA_STRUCTS;
          return 0;
        }

        if (( *p_spinCameraGetTLDeviceNodeMap )( cameraHandle,
            &cameraNodeMapHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera node map",
							__func__ );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
					FREE_DATA_STRUCTS;
          return 0;
        }

        if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "DeviceType",
            &deviceTypeHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera type node",
							__func__ );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
					FREE_DATA_STRUCTS;
          return 0;
        }
        if (( *p_spinEnumerationGetCurrentEntry )( deviceTypeHandle,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera type value node",
							__func__ );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
					FREE_DATA_STRUCTS;
          return 0;
        }

				if (( err = ( *p_spinEnumerationEntryGetIntValue )( valueHandle,
						&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current device type enum value, error %d",
							__func__, err );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
					FREE_DATA_STRUCTS;
          return 0;
        }

				if ( enumValue != DeviceType_GigEVision &&
						enumValue != DeviceType_USB3Vision ) {
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
					continue;
				}

        if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "DeviceID",
            &deviceIdHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id node",
							__func__ );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
					FREE_DATA_STRUCTS;
          return 0;
        }

        *deviceId = 0; 
        if (( *p_spinNodeIsAvailable )( deviceIdHandle,
            &deviceIdAvailable ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id availability",
							__func__ );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
					FREE_DATA_STRUCTS;
          return 0;
        }

        if ( deviceIdAvailable ) {
          if (( *p_spinNodeIsReadable )( deviceIdHandle,
              &deviceIdReadable ) != SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id readability",
								__func__ );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
						FREE_DATA_STRUCTS;
            return 0;
          }
          if ( deviceIdReadable ) {
            if (( *p_spinStringGetValue )( deviceIdHandle, deviceId,
                &deviceIdLen ) != SPINNAKER_ERR_SUCCESS ) {
              oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id string",
									__func__ );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
							FREE_DATA_STRUCTS;
              return 0;
            }

            if ( !strcmp ( deviceId, devInfo->deviceId )) {
              found = 1;
            }
          }
        }

        if ( !found ) {
          if (( *p_spinCameraRelease )( cameraHandle ) !=
              SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA, "%s: Can't release camera", __func__ );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
						FREE_DATA_STRUCTS;
            return 0;
          }
        }
      }
      if (( *p_spinCameraListClear )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't release camera list", __func__ );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
				FREE_DATA_STRUCTS;
        return 0;
      }

      if (( *p_spinCameraListDestroy )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't destroy camera list", __func__ );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
				FREE_DATA_STRUCTS;
        return 0;
      }
    } else {
      ( void ) ( *p_spinCameraListClear )( cameraListHandle );
      ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
      oaLogInfo ( OA_LOG_CAMERA, "%s: Interface %d has no cameras", __func__,
					i );
    }

    if (( *p_spinInterfaceRelease )( ifaceHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't release interface", __func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
			FREE_DATA_STRUCTS;
      return 0;
    }
  }

  ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
  ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );

  if ( !found ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't find camera", __func__ );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

	if ( _processCameraEntry ( cameraHandle, camera ) < 0 ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Failed to process camera nodemap",
				__func__ );
		( void ) ( *p_spinCameraRelease )( cameraHandle );
		( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
		FREE_DATA_STRUCTS;
    return 0;
  }

	cameraInfo->runMode = CAM_RUN_MODE_STOPPED;

	for ( i = cameraInfo->minBinning; i <= cameraInfo->maxBinning;
			i += cameraInfo->binningStep ) {
		cameraInfo->frameSizes[i].numSizes = 1;
		if (!( tmpPtr = realloc ( cameraInfo->frameSizes[i].sizes,
				sizeof ( FRAMESIZE )))) {
			oaLogError ( OA_LOG_CAMERA, "%s: realloc for frame sizes failed",
					__func__ );
			for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
				if ( cameraInfo->frameSizes[ j ].numSizes ) {
					free (( void* ) cameraInfo->frameSizes[ j ].sizes );
				}
			}
			( void ) ( *p_spinCameraRelease )( cameraHandle );
			( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
			FREE_DATA_STRUCTS;
			return 0;
		}
		cameraInfo->frameSizes[i].sizes = tmpPtr;
		cameraInfo->frameSizes[i].sizes[0].x = cameraInfo->maxResolutionX / i;
		cameraInfo->frameSizes[i].sizes[0].y = cameraInfo->maxResolutionY / i;
	}

	// FIX ME -- force camera into 8-bit mode if it has it?

  // The largest buffer size we should need

  cameraInfo->buffers = 0;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * cameraInfo->maxBytesPerPixel;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( frameBuffer ));
  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: calloc for buffers failed", __func__ );
      if ( i ) {
        for ( j = 0; j < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
          cameraInfo->buffers[j].start = 0;
        }
      }
			( void ) ( *p_spinCameraRelease )( cameraHandle );
			( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
			for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
				if ( cameraInfo->frameSizes[ j ].numSizes ) {
					free (( void* ) cameraInfo->frameSizes[ j ].sizes );
				}
			}
			FREE_DATA_STRUCTS;
      return 0;
    }
  }

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamSpinController, ( void* ) camera )) {
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
      cameraInfo->buffers[j].start = 0;
    }
		for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
			if ( cameraInfo->frameSizes[ j ].numSizes ) {
				free (( void* ) cameraInfo->frameSizes[ j ].sizes );
			}
		}
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
		( void ) ( *p_spinCameraRelease )( cameraHandle );
		( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamSpinCallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
      cameraInfo->buffers[j].start = 0;
    }
		for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
			if ( cameraInfo->frameSizes[ j ].numSizes ) {
				free (( void* ) cameraInfo->frameSizes[ j ].sizes );
			}
		}
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
		( void ) ( *p_spinCameraRelease )( cameraHandle );
		( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    FREE_DATA_STRUCTS;
    return 0;
  }

	cameraInfo->nodeMapHandle = cameraNodeMapHandle;
  cameraInfo->systemHandle = systemHandle;
  cameraInfo->cameraHandle = cameraHandle;
  cameraInfo->initialised = 1;

  return camera;
}


static int
_processCameraEntry ( spinCamera cameraHandle, oaCamera* camera )
{
  spinNodeMapHandle	cameraNodeMapHandle = 0;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
  int								err;

  if (( err = ( *p_spinCameraInit )( cameraHandle )) !=
			SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't initialise camera, error %d",
			__func__, err );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( err = ( *p_spinCameraGetNodeMap )( cameraHandle,
      &cameraNodeMapHandle )) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera nodemap: err %d",
				__func__, err );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if ( _checkGainControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkGammaControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkSharpnessControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkBlackLevelControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkWhiteBalanceControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkResetControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkTemperatureControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkExposureControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkAcquisitionControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkTriggerControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkBinningControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkFrameSizeControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkFrameFormatControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkFlipControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( cameraInfo->colour ) {
		if ( _checkHueControls ( cameraNodeMapHandle, camera ) < 0 ) {
			( void ) ( *p_spinCameraDeInit )( cameraHandle );
			return -OA_ERR_SYSTEM_ERROR;
		}

		if ( _checkSaturationControls ( cameraNodeMapHandle, camera ) < 0 ) {
			( void ) ( *p_spinCameraDeInit )( cameraHandle );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

	if ( _checkUnknownControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

  return OA_ERR_NONE;
}


int
_checkGainControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		gain, autoGain;
  bool8_t						implemented, available, readable, writeable;
	double						min, max, curr;
	int								currInt;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	spinNodeHandle		valueHandle;
	size_t						enumValue;
	spinError					r;
	int								autoGainValid = 0;

  if ( _getNodeData ( nodeMap, "GainAuto", &autoGain, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }

	if ( implemented && available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
		if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto gain control", __func__ );
				_showEnumerationNode ( autoGain, 1 );
				if (( *p_spinEnumerationGetCurrentEntry )( autoGain,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto gain current entry",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( r = ( *p_spinEnumerationEntryGetEnumValue )( valueHandle,
						&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto gain current value, error %d", __func__, r );
				}
				switch ( enumValue ) {
					case GainAuto_Off:
						curr = 0;
						autoGainValid = 1;
						break;
					case GainAuto_Continuous:
						curr = 1;
						autoGainValid = 1;
						break;
					default:
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unhandled value '%d' for auto gain", __func__, enumValue );
				}
				if ( autoGainValid ) {
					camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_GAIN ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_GAIN ) = 0;
					commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_GAIN ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_GAIN ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAIN ) = curr ? 1 : 0;
					cameraInfo->autoGain = autoGain;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto gain", __func__,
						nodeTypes[ nodeType ] );
			}
		} else {
			oaLogError ( OA_LOG_CAMERA, "%s: auto gain is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto gain unavailable", __func__ );
  }

	// If auto gain is enabled then because there are controls to limit the
	// range of gain values in auto mode (the features AutoGainLowerLimit and
	// AutoGainUpperLimit, which are currently ignored), the range of available
	// gain values may not be correct, so auto gain must be disabled.

	if ( autoGainValid ) {
		if (( *p_spinEnumerationSetEnumValue )( autoGain, GainAuto_Off ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off auto gain", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "Gain", &gain, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( implemented && available ) {
    if ( readable || writeable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found gain control", __func__ );
        _showFloatNode ( gain, writeable );
				if (( *p_spinFloatGetValue )( gain, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get current gain value",
									__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMin )( gain, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min gain value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMax )( gain, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max gain value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				cameraInfo->minFloatGain = min;
				cameraInfo->maxFloatGain = max;
				// Potentially temporarily, convert this to a range from 0 to 400
				currInt = ( curr - min ) * 400.0 / ( max - min );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) =
						OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = 400;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = currInt;
				cameraInfo->gain = gain;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for gain", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: gain is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: gain unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkGammaControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		gamma, gammaEnabled;
  bool8_t						implemented, available, readable, writeable, currBool;
	double						min, max, curr;
	int								currInt;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								ctrl, gammaEnabledValid = 0;

  if ( _getNodeData ( nodeMap, "GammaEnabled", &gammaEnabled, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable || writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found gamma enabled control",
						__func__ );
				_showBooleanNode ( gammaEnabled );
				if (( *p_spinBooleanGetValue )( gammaEnabled, &currBool ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current gamma enabled value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_GAMMA );
				camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
				commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( ctrl ) = currBool ? 1 : 0;
				cameraInfo->gammaEnabled = gammaEnabled;
				gammaEnabledValid = 1;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for gamma enabled", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: gamma enabled is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: gamma enabled unavailable", __func__ );
  }

	// If gammaEnabled is off then reading the gamma values many not work
	// (the node might not be available, for a start), so enable it before
	// checking.

	if ( gammaEnabledValid ) {
		if (( *p_spinBooleanSetValue )( gammaEnabled, True ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn on gamma", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "Gamma", &gamma, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable || writeable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found gamma control", __func__ );
        _showFloatNode ( gamma, writeable );
				if (( *p_spinFloatGetValue )( gamma, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get current gamma value",
									__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMin )( gamma, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min gamma value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMax )( gamma, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max gamma value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				cameraInfo->minFloatGamma = min;
				cameraInfo->maxFloatGamma = max;
				// Potentially temporarily, convert this to a range from 0 to 100
				currInt = ( curr - min ) * 100.0 / ( max - min );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAMMA ) =
						OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) = 100;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAMMA ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) = currInt;
				cameraInfo->gamma = gamma;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for gamma", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: gamma is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: gamma unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkHueControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		hue, hueEnabled, autoHue;
	spinNodeHandle		valueHandle;
  bool8_t						available, readable, writeable, implemented, currBool;
	double						min, max, curr;
	int								currInt;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								ctrl;
	int								hueEnabledValid = 0, autoHueValid = 0;
	int64_t						intValue;
	spinError					r;

  if ( _getNodeData ( nodeMap, "HueEnabled", &hueEnabled, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable || writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found hue enabled control",
						__func__ );
				_showBooleanNode ( hueEnabled );
				if (( *p_spinBooleanGetValue )( hueEnabled, &currBool ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current hue enabled value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_HUE );
				camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
				commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( ctrl ) = currBool ? 1 : 0;
				hueEnabledValid = 1;
				cameraInfo->hueEnabled = hueEnabled;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for hue enabled", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: hue enabled is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: hue enabled unavailable", __func__ );
  }

	// If hueEnabled is off then reading the hue values many not work
	// (the node might not be available, for a start), so enable it before
	// checking.

	if ( hueEnabledValid ) {
		if (( *p_spinBooleanSetValue )( hueEnabled, True ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn on hue", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "HueAuto", &autoHue, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto hue control",
						__func__ );
				_showEnumerationNode ( autoHue, 0 );
				if (( *p_spinEnumerationGetCurrentEntry )( autoHue,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto hue current entry",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				// Have to use IntValue() here rather than EnumValue() because
				// hue doesn't appear to be part of the SFNC
				if (( r = ( *p_spinEnumerationEntryGetIntValue )( valueHandle,
						&intValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto hue current value, error %d", __func__, r );
				}
				switch ( intValue ) {
					case 0:
						curr = 0;
						autoHueValid = 1;
						break;
					case 2:
						curr = 1;
						autoHueValid = 1;
						break;
					default:
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unhandled value '%d' for auto hue", __func__, intValue );
				}
				if ( autoHueValid ) {
					camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_HUE ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_HUE ) = 0;
					commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_HUE ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_HUE ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_HUE ) = curr ? 1 : 0;
					cameraInfo->autoHue = autoHue;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto hue", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: auto hue is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto hue unavailable", __func__ );
  }

	// If auto hue is enabled disable it before we read the hue values in
	// case auto mode modifies the way hue works

	if ( autoHueValid ) {
		if (( *p_spinEnumerationSetIntValue )( autoHue, 0 ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off auto hue", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "Hue", &hue, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable || writeable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found hue control", __func__ );
        _showFloatNode ( hue, writeable );
				if (( *p_spinFloatGetValue )( hue, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get current hue value",
									__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMin )( hue, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min hue value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMax )( hue, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max hue value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				cameraInfo->minFloatHue = min;
				cameraInfo->maxFloatHue = max;
				// Potentially temporarily, convert this to a range from 0 to 100
				currInt = ( curr - min ) * 100.0 / ( max - min );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HUE ) =
						OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HUE ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HUE ) = 100;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HUE ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HUE ) = currInt;
				cameraInfo->hue = hue;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for hue", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: hue is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: hue unavailable", __func__ );
  }


	return OA_ERR_NONE;
}


int
_checkSaturationControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		saturation, saturationEnabled, autoSaturation;
	spinNodeHandle		valueHandle;
  bool8_t						available, readable, writeable, implemented, currBool;
	double						min, max, curr;
	int								currInt;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								ctrl;
	int								saturationEnabledValid = 0, autoSaturationValid = 0;
	int64_t						intValue;
	spinError					r;

  if ( _getNodeData ( nodeMap, "SaturationEnabled", &saturationEnabled,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable || writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found saturation enabled control",
						__func__ );
				_showBooleanNode ( saturationEnabled );
				if (( *p_spinBooleanGetValue )( saturationEnabled, &currBool ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current saturation enabled value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_SATURATION );
				camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
				commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( ctrl ) = currBool ? 1 : 0;
				saturationEnabledValid = 1;
				cameraInfo->saturationEnabled = saturationEnabled;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for saturation enabled", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: saturation enabled is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: saturation enabled unavailable", __func__ );
  }

	// If saturationEnabled is off then reading the saturation values many not
	// work (the node might not be available, for a start), so enable it before
	// checking.

	if ( saturationEnabledValid ) {
		if (( *p_spinBooleanSetValue )( saturationEnabled, True ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn on saturation", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "SaturationAuto", &autoSaturation, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto saturation control",
						__func__ );
				_showEnumerationNode ( autoSaturation, 0 );
				if (( *p_spinEnumerationGetCurrentEntry )( autoSaturation,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto saturation current entry", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				// Have to use IntValue() here rather than EnumValue() because
				// saturation doesn't appear to be part of the SFNC
				if (( r = ( *p_spinEnumerationEntryGetIntValue )( valueHandle,
						&intValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto saturation current value, error %d",
							__func__, r );
				}
				switch ( intValue ) {
					case 0:
						curr = 0;
						autoSaturationValid = 1;
						break;
					case 2:
						curr = 1;
						autoSaturationValid = 1;
						break;
					default:
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unhandled value '%d' for auto saturation",
								__func__, intValue );
				}
				if ( autoSaturationValid ) {
					camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_SATURATION ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_SATURATION ) = 0;
					commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_SATURATION ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_SATURATION ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_SATURATION ) =
							curr ? 1 : 0;
					cameraInfo->autoSaturation = autoSaturation;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto saturation", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: auto saturation is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto saturation unavailable", __func__ );
  }

	// If auto saturation is enabled disable it before we read the saturation
	// values in case auto mode modifies the way saturation works

	if ( autoSaturationValid ) {
		if (( *p_spinEnumerationSetIntValue )( autoSaturation, 0 ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off auto saturation",
					__func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "Saturation", &saturation, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable || writeable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found saturation control", __func__ );
        _showFloatNode ( saturation, writeable );
				if (( *p_spinFloatGetValue )( saturation, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get current saturation value",
									__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMin )( saturation, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min saturation value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMax )( saturation, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max saturation value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				cameraInfo->minFloatSaturation = min;
				cameraInfo->maxFloatSaturation = max;
				// Potentially temporarily, convert this to a range from 0 to 100
				currInt = ( curr - min ) * 100.0 / ( max - min );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SATURATION ) =
						OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SATURATION ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SATURATION ) = 100;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SATURATION ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SATURATION ) = currInt;
				cameraInfo->saturation = saturation;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for saturation", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: saturation is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: saturation unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkSharpnessControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		sharpness, sharpnessEnabled, autoSharpness;
	spinNodeHandle		valueHandle;
  bool8_t						available, readable, writeable, implemented, currBool;
	int64_t						min, max, curr, step;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								ctrl;
	int								sharpnessEnabledValid = 0, autoSharpnessValid = 0;
	int64_t						intValue;
	spinError					r;

  if ( _getNodeData ( nodeMap, "SharpnessEnabled", &sharpnessEnabled,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable || writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found sharpness enabled control",
						__func__ );
				_showBooleanNode ( sharpnessEnabled );
				if (( *p_spinBooleanGetValue )( sharpnessEnabled, &currBool ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current sharpness enabled value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_SHARPNESS );
				camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
				commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( ctrl ) = currBool ? 1 : 0;
				sharpnessEnabledValid = 1;
				cameraInfo->sharpnessEnabled = sharpnessEnabled;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for sharpness enabled", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: sharpness enabled is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: sharpness enabled unavailable", __func__ );
  }

	// If sharpnessEnabled is off then reading the sharpness values many not
	// work (the node might not be available, for a start), so enable it before
	// checking.

	if ( sharpnessEnabledValid ) {
		if (( *p_spinBooleanSetValue )( sharpnessEnabled, True ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn on sharpness", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "SharpnessAuto", &autoSharpness, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto sharpness control",
						__func__ );
				_showEnumerationNode ( autoSharpness, 0 );
				if (( *p_spinEnumerationGetCurrentEntry )( autoSharpness,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto sharpness current entry", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				// Have to use IntValue() here rather than EnumValue() because
				// sharpness doesn't appear to be part of the SFNC
				if (( r = ( *p_spinEnumerationEntryGetIntValue )( valueHandle,
						&intValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto sharpness current value, error %d",
							__func__, r );
				}
				switch ( intValue ) {
					case 0:
						curr = 0;
						autoSharpnessValid = 1;
						break;
					case 2:
						curr = 1;
						autoSharpnessValid = 1;
						break;
					default:
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unhandled value '%d' for auto sharpness",
								__func__, intValue );
				}
				if ( autoSharpnessValid ) {
					camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_SHARPNESS ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_SHARPNESS ) = 0;
					commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_SHARPNESS ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_SHARPNESS ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_SHARPNESS ) =
							curr ? 1 : 0;
					cameraInfo->autoSharpness = autoSharpness;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto sharpness", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: auto sharpness is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto sharpness unavailable", __func__ );
  }

	// If auto sharpness is enabled disable it before we read the sharpness
	// values in case auto mode modifies the way sharpness works

	if ( autoSharpnessValid ) {
		if (( *p_spinEnumerationSetIntValue )( autoSharpness, 0 ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off auto sharpness",
					__func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "Sharpness", &sharpness, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable || writeable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found sharpness control", __func__ );
        _showIntegerNode ( sharpness, writeable );
				if (( *p_spinIntegerGetValue )( sharpness, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get current sharpness value",
									__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinIntegerGetMin )( sharpness, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min sharpness value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinIntegerGetMax )( sharpness, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max sharpness value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinIntegerGetInc )( sharpness, &step ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get sharpness step value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHARPNESS ) =
						OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHARPNESS ) = min;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHARPNESS ) = max;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHARPNESS ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHARPNESS ) = step;
					cameraInfo->sharpness = sharpness;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for sharpness", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: sharpness is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: sharpness unavailable", __func__ );
  }

	// Perhaps this isn't the right thing to do, but I'm going to force
	// the sharpness control off if it exists

	if ( sharpnessEnabledValid ) {
		if (( *p_spinBooleanSetValue )( sharpnessEnabled, False ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off sharpness", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

	return OA_ERR_NONE;
}


int
_checkBlackLevelControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		blackLevel, blackLevelEnabled, autoBlackLevel;
	spinNodeHandle		valueHandle;
  bool8_t						available, readable, writeable, implemented, currBool;
	double						min, max, curr;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								ctrl;
	int								currInt;
	int								blackLevelEnabledValid = 0, autoBlackLevelValid = 0;
	size_t						enumValue;
	spinError					r;

  if ( _getNodeData ( nodeMap, "BlackLevelEnabled", &blackLevelEnabled,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found blackLevel enabled control",
						__func__ );
				_showBooleanNode ( blackLevelEnabled );
				if (( *p_spinBooleanGetValue )( blackLevelEnabled, &currBool ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current blackLevel enabled value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_BLACKLEVEL );
				camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
				commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( ctrl ) = currBool ? 1 : 0;
				blackLevelEnabledValid = 1;
				cameraInfo->blackLevelEnabled = blackLevelEnabled;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for blackLevel enabled", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: blackLevel enabled is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: blackLevel enabled unavailable", __func__ );
  }

	// If blackLevelEnabled is off then reading the sharpness values many not
	// work (the node might not be available, for a start), so enable it before
	// checking.

	if ( blackLevelEnabledValid ) {
		if (( *p_spinBooleanSetValue )( blackLevelEnabled, True ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn on black level", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "BlackLevelAuto", &autoBlackLevel, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto blackLevel control",
						__func__ );
				_showEnumerationNode ( autoBlackLevel, 1 );
				if (( *p_spinEnumerationGetCurrentEntry )( autoBlackLevel,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto blackLevel current entry", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( r = ( *p_spinEnumerationEntryGetEnumValue )( valueHandle,
						&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto blacklevel current value, error %d",
							__func__, r );
				}
				switch ( enumValue ) {
					case BlackLevelAuto_Off:
						curr = 0;
						autoBlackLevelValid = 1;
						break;
					case BlackLevelAuto_Continuous:
						curr = 1;
						autoBlackLevelValid = 1;
						break;
					default:
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unhandled value '%d' for auto blacklevel",
								__func__, enumValue );
				}
				if ( autoBlackLevelValid ) {
					camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_BLACKLEVEL ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_BLACKLEVEL ) = 0;
					commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_BLACKLEVEL ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_BLACKLEVEL ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_BLACKLEVEL ) =
							curr ? 1 : 0;
					cameraInfo->autoBlackLevel = autoBlackLevel;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto blacklevel", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: auto blacklevel is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto blacklevel unavailable", __func__ );
  }

	// If auto black level is enabled disable it before we read the black level
	// values in case auto mode modifies the way black level works

	if ( autoBlackLevelValid ) {
		if (( *p_spinEnumerationSetEnumValue )( autoBlackLevel,
				BlackLevelAuto_Off ) != SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off auto black level",
					__func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "BlackLevel", &blackLevel, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found blacklevel control", __func__ );
        _showFloatNode ( blackLevel, writeable );
				if (( *p_spinFloatGetValue )( blackLevel, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get current blacklevel value",
									__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMin )( blackLevel, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min blacklevel value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMax )( blackLevel, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max blacklevel value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				cameraInfo->minFloatBlacklevel = min;
				cameraInfo->maxFloatBlacklevel = max;
				// Potentially temporarily, convert this to a range from 0 to 100
				currInt = ( curr - min ) * 100.0 / ( max - min );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLACKLEVEL ) =
						OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLACKLEVEL ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLACKLEVEL ) = 100;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLACKLEVEL ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLACKLEVEL ) = currInt;
				cameraInfo->blackLevel = blackLevel;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for blacklevel", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: blacklevel is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: blacklevel unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkWhiteBalanceControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		autoWhiteBalance;
	spinNodeHandle		valueHandle;
  bool8_t						implemented, available, readable, writeable;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								autoWhiteBalanceValid = 0;
	size_t						enumValue;
	spinError					r;
	int								curr;

  if ( _getNodeData ( nodeMap, "BalanceWhiteAuto", &autoWhiteBalance,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto white balance control",
						__func__ );
				_showEnumerationNode ( autoWhiteBalance, 1 );
				if (( *p_spinEnumerationGetCurrentEntry )( autoWhiteBalance,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto white balance current entry", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( r = ( *p_spinEnumerationEntryGetEnumValue )( valueHandle,
						&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto white balance current value, error %d",
							__func__, r );
				}
				switch ( enumValue ) {
					case BalanceWhiteAuto_Off:
						curr = 0;
						autoWhiteBalanceValid = 1;
						break;
					case BalanceWhiteAuto_Continuous:
						curr = 1;
						autoWhiteBalanceValid = 1;
						break;
					default:
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unhandled value '%d' for auto white balance",
								__func__, enumValue );
				}
				if ( autoWhiteBalanceValid ) {
					camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_WHITE_BALANCE ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_WHITE_BALANCE ) = 0;
					commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_WHITE_BALANCE ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_WHITE_BALANCE ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_WHITE_BALANCE ) =
							curr ? 1 : 0;
					cameraInfo->autoWhiteBalance = autoWhiteBalance;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto white balance", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: auto white balance is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto white balance unavailable", __func__ );
  }

	if ( autoWhiteBalanceValid &&
			commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_WHITE_BALANCE )) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check auto white balance is "
				"disabled before checking white balance range", __func__ );
	}

	// FIX ME -- when auto white balance is present and off, there are the
	// balance ratio and balance ratio selector options, but as I don't have
	// access to a colour camera I can't really do much with those at the
	// moment

	return OA_ERR_NONE;
}


int
_checkResetControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		reset;
  bool8_t						implemented, available, readable, writeable;
  spinNodeType			nodeType;
  SPINNAKER_STATE*	cameraInfo = camera->_private;

  if ( _getNodeData ( nodeMap, "DeviceReset", &reset, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( writeable ) {
			if ( nodeType == CommandNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found reset control",
						__func__ );
				camera->features.flags |= OA_CAM_FEATURE_RESET;
				cameraInfo->reset = reset;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for reset", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: reset is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: reset unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkTemperatureControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		temperature;
  bool8_t						implemented, available, readable, writeable;
  spinNodeType			nodeType;
  SPINNAKER_STATE*	cameraInfo = camera->_private;

  if ( _getNodeData ( nodeMap, "DeviceTemperature", &temperature, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found temperature control",
						__func__ );
        _showFloatNode ( temperature, 0 );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) =
						OA_CTRL_TYPE_READONLY;
				cameraInfo->temperature = temperature;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for temperature", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: temperature is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: temperature unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkExposureControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		exposure, autoExposure, exposureMode;
  bool8_t						implemented, available, readable, writeable;
	double						min, max, curr;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	spinNodeHandle		valueHandle;
	size_t						enumValue;
	spinError					r;
	int								autoExposureValid = 0;

  if ( _getNodeData ( nodeMap, "ExposureAuto", &autoExposure, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto exposure control",
						__func__ );
				_showEnumerationNode ( autoExposure, 1 );
				if (( *p_spinEnumerationGetCurrentEntry )( autoExposure,
						&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto exposure current entry", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( r = ( *p_spinEnumerationEntryGetEnumValue )( valueHandle,
						&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get auto exposure current value, error %d",
							__func__, r );
				}
				switch ( enumValue ) {
					case ExposureAuto_Off:
						curr = 0;
						autoExposureValid = 1;
						break;
					case ExposureAuto_Continuous:
						curr = 1;
						autoExposureValid = 1;
						break;
					default:
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unhandled value '%d' for auto exposure", __func__,
								enumValue );
				}
				if ( autoExposureValid ) {
					camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_AUTO_MIN(
							OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 0;
					commonInfo->OA_CAM_CTRL_AUTO_MAX(
							OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_STEP(
							OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
					commonInfo->OA_CAM_CTRL_AUTO_DEF(
							OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = curr ? 1 : 0;
					cameraInfo->autoExposure = autoExposure;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto exposure", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: auto exposure is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto exposure unavailable", __func__ );
  }

	// If auto exposure is enabled then because there are controls to limit the
	// range of exposure values in auto mode (the features
	// AutoExposureTimeLowerLimit and AutoExposureTimeUpperLimit ), which are
	// currently ignored), the range of available exposure values may not be
	// correct, so auto exposure must be disabled.

	if ( autoExposureValid ) {
		if (( *p_spinEnumerationSetEnumValue )( autoExposure, ExposureAuto_Off ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off auto exposure",
					__func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

  if ( _getNodeData ( nodeMap, "ExposureTimeAbs", &exposure, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable || writeable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found exposure control", __func__ );
        _showFloatNode ( exposure, writeable );
				if (( *p_spinFloatGetValue )( exposure, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get current exposure value",
									__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMin )( exposure, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min exposure value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMax )( exposure, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max exposure value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
						OA_CTRL_TYPE_INT64;
				// FIX ME -- need to round min up to nearest microsecond?
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = min;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = max;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = curr;
				cameraInfo->exposure = exposure;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for exposure", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: exposure is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: exposure unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "ExposureMode", &exposureMode, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found exposure mode control",
						__func__ );
				_showEnumerationNode ( exposureMode, 1 );
				cameraInfo->exposureMode = exposureMode;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for exposure mode", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: exposure mode is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: exposure mode unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkAcquisitionControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		frameRateEnabled, acquisitionMode, acquisitionStart;
	spinNodeHandle		acquisitionStop;
	//spinNodeHandle		singleFrameMode;
  bool8_t						available, readable, writeable, implemented;
  spinNodeType			nodeType;
  SPINNAKER_STATE*	cameraInfo = camera->_private;

  if ( _getNodeData ( nodeMap, "AcquisitionFrameRateEnabled",
			&frameRateEnabled, &implemented, &available, &readable, &writeable,
			&nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found frame rate enabled control",
						__func__ );
				_showBooleanNode ( frameRateEnabled );
				cameraInfo->frameRateEnabled = frameRateEnabled;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for frame rate enabled", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: frame rate enabled is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: frame rate enabled unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "AcquisitionMode", &acquisitionMode,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found acquisition mode control",
						__func__ );
				_showEnumerationNode ( acquisitionMode, 1 );
				cameraInfo->acquisitionMode = acquisitionMode;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for acquisition mode", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: acquisition mode is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: acquisition mode unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "AcquisitionStart", &acquisitionStart,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( !available ) {
		// This is quite probably not a good thing, but we never actually
		// use the node directly
    oaLogWarning ( OA_LOG_CAMERA, "%s: acquisition start unavailable",
				__func__ );
  }

  if ( _getNodeData ( nodeMap, "AcquisitionStop", &acquisitionStop,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( !available ) {
		// This is quite probably not a good thing, but we never actually
		// use the node directly
    oaLogWarning ( OA_LOG_CAMERA, "%s: acquisition stop unavailable",
				__func__ );
  }

	/*
	 * The Grasshopper camera XML seems to claim this is a standard feature,
	 * but it doesn't appear in the v2.6 SNFC, so I'm unconvinced.  For the
	 * time being I'm going to ignore it.
	 *
  if ( _getNodeData ( nodeMap, "SingleFrameAcquisitionMode", &singleFrameMode,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found single frame acquisition mode control", __func__ );
				_showEnumerationNode ( singleFrameMode, 1 );
				cameraInfo->singleFrameMode = singleFrameMode;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for single frame acquisition mode",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: single frame acquisition mode is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: single frame acquisition mode unavailable",
				__func__ );
  }
	 */

	return OA_ERR_NONE;
}


int
_checkTriggerControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		triggerActivation, triggerDelayEnabled, triggerDelay;
	spinNodeHandle		triggerMode, triggerOverlap, triggerSelector;
	spinNodeHandle		triggerSource, tempHandle;
  bool8_t						available, readable, writeable, implemented, currBool;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								triggerModeValid = 0, overlapValid;
	double						min, max, curr;
	int								i, numberOfSources;
	size_t						numEntries, enumVal;
	spinError					err;

  if ( _getNodeData ( nodeMap, "TriggerMode", &triggerMode, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found trigger mode control", __func__ );
				_showEnumerationNode ( triggerMode, 1 );
				cameraInfo->triggerMode = triggerMode;
				triggerModeValid = 1;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for trigger mode",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: trigger mode is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger mode unavailable", __func__ );
  }
	if ( !triggerModeValid ) {
		// Anything else is pointless if we can't turn trigger mode on
		return OA_ERR_NONE;
	}

	// Other trigger features might only be available if trigger mode is on,
	// so make sure it's turned on now and turn it off afterwards.

	if (( *p_spinEnumerationSetEnumValue )( triggerMode, TriggerMode_On ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't turn on trigger mode", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

  if ( _getNodeData ( nodeMap, "TriggerActivation", &triggerActivation,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found trigger activation control", __func__ );
				_showEnumerationNode ( triggerActivation, 1 );
				cameraInfo->triggerActivation = triggerActivation;
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_POLARITY ) =
						OA_CTRL_TYPE_MENU;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_POLARITY ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_POLARITY ) = 1;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_POLARITY ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_POLARITY ) = 0;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for trigger activation",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: trigger activation is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger activation unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "TriggerDelayEnabled", &triggerDelayEnabled,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable && writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found trigger delay enabled control", __func__ );
				_showBooleanNode ( triggerDelayEnabled );
				cameraInfo->triggerDelayEnabled = triggerDelayEnabled;
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) =
						OA_CTRL_TYPE_BOOLEAN;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) = 1;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) = 0;
				if (( *p_spinBooleanGetValue )( triggerDelayEnabled, &currBool ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get trigger delay enabled current value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				cameraInfo->triggerDelayOn = currBool ? 1 : 0;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for trigger delay enabled",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: trigger delay enabled is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger delay enabled unavailable",
				__func__ );
  }

  if ( _getNodeData ( nodeMap, "TriggerDelay", &triggerDelay, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable && writeable ) {
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found trigger delay control", __func__ );
				_showFloatNode ( triggerDelay, writeable );
				cameraInfo->triggerDelay = triggerDelay;
				if (( *p_spinFloatGetValue )( triggerDelay, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current trigger delay value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMin )( triggerDelay, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min trigger delay value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinFloatGetMax )( triggerDelay, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max trigger delay value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}

				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_DELAY ) =
						OA_CTRL_TYPE_INT64;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_DELAY ) = min;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_DELAY ) = max;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_DELAY ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_DELAY ) = curr;
				cameraInfo->triggerDelay = triggerDelay;
				cameraInfo->triggerDelayValue = curr;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for trigger delay", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: trigger delay is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger delay unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "TriggerOverlap", &triggerOverlap, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found trigger overlap control", __func__ );
				_showEnumerationNode ( triggerOverlap, 1 );
				// All we really care about here is whether the overlap function
				// supports "off" and "readout" modes (there is also "previous
				// frame", but I'm not sure that's useful to us for the moment
				// anyhow.  flycapture offers other options, but it looks like
				// they may not be part of Genicam
				if (( *p_spinEnumerationGetNumEntries )( triggerOverlap, &numEntries )
						!= SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get number of enum entries for TriggerOverlap",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				overlapValid = 0;
				if ( numEntries > 1 ) {
					for ( i = 0; i < numEntries; i++ ) {
						if (( *p_spinEnumerationGetEntryByIndex )( triggerOverlap, i,
									&tempHandle ) != SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: get overlap node %d failed",
									__func__, i );
						} else {
							available = False;
							if (( err = ( *p_spinNodeIsAvailable )( tempHandle, &available ))
									!= SPINNAKER_ERR_SUCCESS ) {
								oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsAvailable failed "
										"for overlap node %d, error %d", __func__, i, err );
							} else {
								if ( available ) {
									overlapValid++;
								}
							}
						}
					}
				}
				if ( 2 == overlapValid ) {
					cameraInfo->triggerOverlap = triggerOverlap;
					camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_MODE ) =
							OA_CTRL_TYPE_MENU;
					commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_MODE ) = 0;
					commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_MODE ) = 1;
					commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_MODE ) = 1;
					commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_MODE ) = 0;
					oaLogInfo ( OA_LOG_CAMERA, "%s: Trigger overlap enabled", __func__ );
					cameraInfo->currentOverlapMode = TriggerOverlap_Off;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for trigger overlap",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: trigger overlap is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger overlap unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "TriggerSelector", &triggerSelector,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found trigger selector control", __func__ );
				_showEnumerationNode ( triggerSelector, 1 );
				cameraInfo->triggerSelector = triggerSelector;
				// FIX ME -- this needs to be set to TriggerSelector_AcquisitionStart
				// for our purposes
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for trigger selector", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: trigger selector is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger selector unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "TriggerSource", &triggerSource, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found trigger source control", __func__ );
				_showEnumerationNode ( triggerSource, 1 );
				// Now we need to walk through the options to see what's genuinely
				// available
				if (( *p_spinEnumerationGetNumEntries )( triggerSource, &numEntries )
						!= SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get number of enum entries for TriggerSource",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if ( numEntries ) {
					numberOfSources = 0;
					for ( i = 0; i < numEntries; i++ ) {
						if (( *p_spinEnumerationGetEntryByIndex )( triggerSource, i,
									&tempHandle ) != SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA,
									"%s: get trigger source node %d failed", __func__, i );
						} else {
							available = False;
							if (( err = ( *p_spinNodeIsAvailable )( tempHandle, &available ))
									!= SPINNAKER_ERR_SUCCESS ) {
								oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsAvailable failed "
										"for trigger source node %d, error %d", __func__, i, err );
							} else {
								if ( available ) {
									if (( *p_spinEnumerationEntryGetEnumValue )( tempHandle,
											&enumVal ) != SPINNAKER_ERR_SUCCESS ) {
										oaLogError ( OA_LOG_CAMERA,
												"%s: Can't get colour filter enum value", __func__ );
										return -OA_ERR_SYSTEM_ERROR;
									}
									// We can't handle non-contiguous sets of trigger sources
									// yet, hence the "if"s
									switch ( enumVal ) {
										case TriggerSource_Line0:
											numberOfSources = 1;
											break;
										case TriggerSource_Line1:
											if ( 1 == numberOfSources ) {
												numberOfSources++;
											} else {
												oaLogWarning ( OA_LOG_CAMERA, "%s: Non-contiguous "
														"trigger sources", __func__ );
												numberOfSources = 0;
											}
											break;
										case TriggerSource_Line2:
											if ( 2 == numberOfSources ) {
												numberOfSources++;
											} else {
												oaLogWarning ( OA_LOG_CAMERA, "%s: Non-contiguous "
														"trigger sources", __func__ );
												numberOfSources = 0;
											}
											break;
										case TriggerSource_Line3:
											if ( 3 == numberOfSources ) {
												numberOfSources++;
											} else {
												oaLogWarning ( OA_LOG_CAMERA, "%s: Non-contiguous "
														"trigger sources", __func__ );
												numberOfSources = 0;
											}
											break;
										default:
											// We don't actually care about these ones for now
											break;
									}
									if ( numberOfSources ) {
										cameraInfo->triggerSource = triggerSource;
										if ( numberOfSources > 1 ) {
											camera->OA_CAM_CTRL_TYPE(
													OA_CAM_CTRL_TRIGGER_SOURCE ) = OA_CTRL_TYPE_MENU;
											commonInfo->OA_CAM_CTRL_MIN(
													OA_CAM_CTRL_TRIGGER_SOURCE ) = 0;
											commonInfo->OA_CAM_CTRL_MAX(
													OA_CAM_CTRL_TRIGGER_SOURCE ) = numberOfSources - 1;
											commonInfo->OA_CAM_CTRL_STEP(
													OA_CAM_CTRL_TRIGGER_SOURCE ) = 1;
											oaLogWarning ( OA_LOG_CAMERA,
													"%s: Need to set default trigger source value",
													__func__ );
											commonInfo->OA_CAM_CTRL_DEF(
													OA_CAM_CTRL_TRIGGER_SOURCE ) = 0;
										}
									}
								}
							}
						}
					}
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for trigger source",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: trigger source is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger source unavailable", __func__ );
  }

	if (( *p_spinEnumerationSetEnumValue )( triggerMode, TriggerMode_Off ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't turn off trigger mode", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	cameraInfo->triggerEnabled = 0;
	camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_ENABLE ) = OA_CTRL_TYPE_BOOLEAN;
	commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_ENABLE ) = 0;
	commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_ENABLE ) = 1;
	commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_ENABLE ) = 1;
	// off appears to be the case for the Blackfly.  I'll assume for all
	commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_ENABLE ) = 0;

	return OA_ERR_NONE;
}


int
_checkBinningControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		binningType, verticalBin;
	//spinNodeHandle		horizontalBin;
  bool8_t						available, readable, writeable, implemented;
  spinNodeType			nodeType;
	int64_t						min, max, step, curr;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*			commonInfo = camera->_common;
	//int								hbinValid = 0, vbinValid = 0;

  if ( _getNodeData ( nodeMap, "BinningControl", &binningType,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found binning control", __func__ );
				_showEnumerationNode ( binningType, 1 );
				cameraInfo->binningType = binningType;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for binning control", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: binning control is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: binning control unavailable", __func__ );
  }

	// According to FLIR:
	//
	// "the binning modes which are valid for this camera are 2x2 and 4x4 modes.
	// Technically, spinnaker as latest sdk version help the customer to
	// implement more reliable code by letting the user avoid setting invalid
	// binning mode ( such as 1x2 or 2x3 ) on the camera. Hence, the horizontal
	// binning is locked and depends on current vertical binning while
	// configuring the camera with spinnaker API. 
	//
	// With that said, there is no advantage of letting the user to choose
	// horizontal binning."
	//
	// So, I think we can just ignore the "BinningHorizontal" feature and
	// treat "BinningVertical" as a straightforward binning control

	/*
  if ( _getNodeData ( nodeMap, "BinningHorizontal", &horizontalBin,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found horizontal bin control", __func__ );
				_showIntegerNode ( horizontalBin, writeable );
				cameraInfo->horizontalBin = horizontalBin;
				hbinValid = 1;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for horizontal binning",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogWarning ( OA_LOG_CAMERA,
					"%s: horizontal binning is inaccessible (read = %d, write = %d)",
					__func__, readable ? 1 : 0, writeable ? 1 : 0 );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: horizontal binning unavailable",
				__func__ );
  }
	*/

  if ( _getNodeData ( nodeMap, "BinningVertical", &verticalBin,
			&implemented, &available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found vertical bin control", __func__ );
				_showIntegerNode ( verticalBin, writeable );
				if (( *p_spinIntegerGetValue )( verticalBin, &curr ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get current vertical bin value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinIntegerGetMin )( verticalBin, &min ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get min vertical bin value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinIntegerGetMax )( verticalBin, &max ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get max vertical bin value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinIntegerGetInc )( verticalBin, &step ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get vertical bin step value",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				cameraInfo->verticalBin = verticalBin;
				camera->features.flags |= OA_CAM_FEATURE_BINNING;
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BINNING ) = min;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BINNING ) = max;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BINNING ) = step;
				// This seems a reasonable choice
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BINNING ) = min;
				cameraInfo->minBinning = min;
				cameraInfo->maxBinning = max;
				cameraInfo->binningStep = step;
				cameraInfo->binMode = curr;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for vertical binning",
						__func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: vertical binning is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: vertical binning unavailable",
				__func__ );
  }

	return OA_ERR_NONE;
}


int
_checkFrameSizeControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		height, width, maxHeight, maxWidth, xOffset, yOffset;
	spinNodeHandle		sensorHeight, sensorWidth;
  bool8_t						available, readable, writeable, implemented;
  spinNodeType			nodeType;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								maxHeightValid, maxWidthValid, heightValid, widthValid;
	int64_t						curr;

	heightValid = 0;
  if ( _getNodeData ( nodeMap, "Height", &height, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found height control", __func__ );
				_showIntegerNode ( height, writeable );
				cameraInfo->height = height;
				heightValid = 1;
				if ( writeable ) {
					camera->features.flags |= OA_CAM_FEATURE_ROI;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for height", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: height is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: height unavailable", __func__ );
  }

	widthValid = 0;
  if ( _getNodeData ( nodeMap, "Width", &width, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found width control", __func__ );
				_showIntegerNode ( width, writeable );
				cameraInfo->width = width;
				widthValid = 1;
				if ( writeable ) {
					camera->features.flags |= OA_CAM_FEATURE_ROI;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for width", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: width is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: width unavailable", __func__ );
  }

	maxHeightValid = 0;
  if ( _getNodeData ( nodeMap, "HeightMax", &maxHeight, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found max height control", __func__ );
				_showIntegerNode ( maxHeight, writeable );
				cameraInfo->maxHeight = maxHeight;
				maxHeightValid = 1;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for max height", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: max height is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: max height unavailable", __func__ );
  }

	maxWidthValid = 0;
  if ( _getNodeData ( nodeMap, "WidthMax", &maxWidth, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found max width control", __func__ );
				_showIntegerNode ( maxWidth, writeable );
				cameraInfo->maxWidth = maxWidth;
				maxWidthValid = 1;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for max width", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: max width is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: max width unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "OffsetX", &xOffset, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable && writeable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found X offset control", __func__ );
				_showIntegerNode ( xOffset, writeable );
				cameraInfo->xOffset = xOffset;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for x offset", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: x offset is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: x offset unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "OffsetY", &yOffset, &implemented, &available,
			&readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable && writeable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found Y offset control", __func__ );
				_showIntegerNode ( yOffset, writeable );
				cameraInfo->yOffset = yOffset;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for Y offset", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: Y offset is inaccessible", __func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: Y offset unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "SensorHeight", &sensorHeight, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found sensor height control",
						__func__ );
				_showIntegerNode ( sensorHeight, writeable );
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for sensor height", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: sensor height is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: sensor height unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "SensorWidth", &sensorWidth, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if ( readable ) {
			if ( nodeType == IntegerNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found sensor width control",
						__func__ );
				_showIntegerNode ( sensorWidth, writeable );
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for sensor width", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: sensor width is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: sensor width unavailable", __func__ );
  }

	if ( maxHeightValid && maxWidthValid ) {
		if (( *p_spinIntegerGetValue )( maxHeight, &curr ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't get max height value", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
		cameraInfo->maxResolutionY = curr;
		if (( *p_spinIntegerGetValue )( maxWidth, &curr ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't get max width value", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
		cameraInfo->maxResolutionX = curr;
		if ( heightValid && widthValid ) {
			if (( *p_spinIntegerGetValue )( height, &curr ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get height value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			cameraInfo->ySize = curr;
			if (( *p_spinIntegerGetValue )( width, &curr ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get width value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			cameraInfo->xSize = curr;
		} else {
			cameraInfo->xSize = cameraInfo->maxResolutionX;
			cameraInfo->ySize = cameraInfo->maxResolutionY;
		}
	} else {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to determine sensor size",
					__func__ );
    return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}


int
_checkFrameFormatControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		pixelFormat, pixelSize, colourFilter, pixelCoding;
	spinNodeHandle		bigEndian, tempHandle;
  bool8_t						available, readable, writeable, implemented;
  spinNodeType			nodeType;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								pixelFormatValid = 0, matched;
	size_t						enumVal, currentVal, numEntries;
	int								i;
	int								maxBytesPP, oaFormat;
	spinError					err;

  if ( _getNodeData ( nodeMap, "PixelFormat", &pixelFormat, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found pixel format", __func__ );
				_showEnumerationNode ( pixelFormat, 1 );
				cameraInfo->pixelFormat = pixelFormat;
				pixelFormatValid = 1;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for pixel format", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: pixel format is inaccessible",
				__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: pixel format unavailable", __func__ );
  }

  if ( _getNodeData ( nodeMap, "PixelColorFilter", &colourFilter, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable ) {
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found pixel CFA", __func__ );
				_showEnumerationNode ( colourFilter, 1 );
				if (( *p_spinEnumerationGetCurrentEntry )( colourFilter,
						&tempHandle ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't get colour filter value node",
							__func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( *p_spinEnumerationEntryGetEnumValue )( tempHandle, &enumVal ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get colour filter current value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
				switch ( enumVal ) {
					case PixelColorFilter_None:
						cameraInfo->colour = 0;
						break;
					case PixelColorFilter_BayerRG:
						cameraInfo->colour = 1;
						cameraInfo->cfa = OA_DEMOSAIC_RGGB;
						break;
					case PixelColorFilter_BayerGB:
						cameraInfo->colour = 1;
						cameraInfo->cfa = OA_DEMOSAIC_GBRG;
						break;
					case PixelColorFilter_BayerGR:
						cameraInfo->colour = 1;
						cameraInfo->cfa = OA_DEMOSAIC_GRBG;
						break;
					case PixelColorFilter_BayerBG:
						cameraInfo->colour = 1;
						cameraInfo->cfa = OA_DEMOSAIC_BGGR;
						break;
					default:
						oaLogError ( OA_LOG_CAMERA, "%s: Unrecognised value %ld for "
								"colour filter current value", __func__, enumVal );
						return -OA_ERR_SYSTEM_ERROR;
						break;
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for CFA", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: pixel CFA is inaccessible",
				__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: pixel CFA unavailable", __func__ );
  }

	if ( !pixelFormatValid ) {
		if ( _getNodeData ( nodeMap, "PixelSize", &pixelSize, &implemented,
				&available, &readable, &writeable, &nodeType ) < 0 ) {
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( implemented && available ) {
			if ( readable ) {
				if ( nodeType == EnumerationNode ) {
					oaLogInfo ( OA_LOG_CAMERA, "%s: Found pixel size", __func__ );
					_showEnumerationNode ( pixelSize, 1 );
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: PixelSize is available for this camera", __func__ );
				} else {
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unrecognised node type '%s' for pixel size", __func__,
							nodeTypes[ nodeType ] );
				}
			} else {
				oaLogError ( OA_LOG_CAMERA, "%s: pixel size is inaccessible",
					__func__ );
			}
		} else {
			oaLogInfo ( OA_LOG_CAMERA, "%s: pixel size unavailable", __func__ );
		}

		// This one is actually deprecated
		if ( _getNodeData ( nodeMap, "PixelCoding", &pixelCoding, &implemented,
				&available, &readable, &writeable, &nodeType ) < 0 ) {
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( implemented && available ) {
			if ( readable ) {
				if ( nodeType == EnumerationNode ) {
					oaLogInfo ( OA_LOG_CAMERA, "%s: Found pixel coding", __func__ );
					_showEnumerationNode ( pixelCoding, 1 );
					cameraInfo->pixelCoding = pixelCoding;
				} else {
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Unrecognised node type '%s' for pixel coding", __func__,
							nodeTypes[ nodeType ] );
				}
			} else {
				oaLogInfo ( OA_LOG_CAMERA, "%s: pixel coding is inaccessible",
					__func__ );
			}
		} else {
			oaLogInfo ( OA_LOG_CAMERA, "%s: pixel coding unavailable", __func__ );
		}
	}

  if ( _getNodeData ( nodeMap, "pgrPixelBigEndian", &bigEndian, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
		// Only need this one to be readable
    if ( readable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found bigEndian", __func__ );
				_showBooleanNode ( bigEndian );
				cameraInfo->bigEndian = bigEndian;
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for bigEndian", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: bigEndian is inaccessible",
				__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: bigEndian unavailable", __func__ );
  }

	if ( pixelFormatValid ) {
		if (( *p_spinEnumerationGetCurrentEntry )( pixelFormat, &tempHandle ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't get pixel format current node",
					__func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if (( *p_spinEnumerationEntryGetEnumValue )( tempHandle, &currentVal ) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't get value of pixel format enum",
					__func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if (( *p_spinEnumerationGetNumEntries )( pixelFormat, &numEntries ) !=
			 SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: Can't get number of enum entries for PixelFormat", __func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
		maxBytesPP = 0;
		matched = 0;
		for ( i = 0; i < numEntries; i++ ) {
			if (( *p_spinEnumerationGetEntryByIndex )( pixelFormat, i,
					&tempHandle ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get enum handle %d for PixelFormat", __func__, i );
				return -OA_ERR_SYSTEM_ERROR;
			}
			// There are (as of Spinnaker 2.3) apparently 252 possible values for
			// enumVal, most of which we just don't care about.  Unfortunately
			// some of the formats reported as present make little sense.
			// Fortunately these are "unavailable", so we need to check that too.
			available = False;
			if (( err = ( *p_spinNodeIsAvailable )( tempHandle, &available )) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: spinNodeIsAvailable failed for enum %d, error %d", __func__,
						i, err );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( available ) {
				if (( *p_spinEnumerationEntryGetEnumValue )( tempHandle, &enumVal ) !=
						SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA,
							"%s: Can't get value for PixelFormat enum node %d", __func__, i );
					return -OA_ERR_SYSTEM_ERROR;
				}
				if (( oaFormat = _spinFormatMap [ enumVal ] ) > 0 ) {
					// Let's not assume that just because an option is actually
					// available that it makes sense
					if (( oaFrameFormats[ oaFormat ].monochrome && !cameraInfo->colour )
							|| ( !oaFrameFormats[ oaFormat ].monochrome &&
							cameraInfo->colour )) {
						if ( oaFrameFormats[ oaFormat ].bytesPerPixel > maxBytesPP ) {
							maxBytesPP = oaFrameFormats[ oaFormat ].bytesPerPixel;
						}
						if ( oaFrameFormats[ oaFormat ].rawColour ) {
							camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
						}
						if ( oaFrameFormats[ oaFormat ].fullColour ) {
							camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
						}
						camera->frameFormats[ oaFormat ] = 1;
						if ( currentVal == enumVal ) {
							cameraInfo->currentBytesPerPixel =
									oaFrameFormats[ oaFormat ].bytesPerPixel;
							cameraInfo->currentFrameFormat = oaFormat;
							matched = 1;
						}
					} else {
						oaLogInfo ( OA_LOG_CAMERA,
								"%s: ignoring pixel format %ld for mismatch with camera type",
								__func__, enumVal );
					}
				} else {
					oaLogWarning ( OA_LOG_CAMERA, "%s: Unhandled pixel format %ld",
							__func__, enumVal );
				}
			}
		}
		if ( !matched ) {
			oaLogError ( OA_LOG_CAMERA, "%s: unable to match current frame format",
					__func__ );
			return -OA_ERR_SYSTEM_ERROR;
		}
		cameraInfo->maxBytesPerPixel = maxBytesPP;
	} else {
		oaLogError ( OA_LOG_CAMERA,
				"%s: No way to discover available frame formats", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

	camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

	return OA_ERR_NONE;
}


int
_checkFlipControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		flipX;
  bool8_t						available, readable, writeable, implemented;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	spinError					err;

  if ( _getNodeData ( nodeMap, "ReverseX", &flipX, &implemented,
			&available, &readable, &writeable, &nodeType ) < 0 ) {
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( implemented && available ) {
    if ( readable && writeable ) {
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found reverse X", __func__ );
				_showBooleanNode ( flipX );
				// It seems that sometimes, even though this node appears to be
				// available, trying to set it returns a GENICAM_ERR_ACCESS error.
				// Try that now and dump the control if it does.
				err = ( *p_spinBooleanSetValue )( flipX, False );
				if ( err == SPINNAKER_ERR_SUCCESS ) {
					cameraInfo->flipX = flipX;
					camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HFLIP ) = OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HFLIP ) = 0;
					commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HFLIP ) = 1;
					commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HFLIP ) = 1;
					commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HFLIP ) = 0;
				} else {
					if ( err == GENICAM_ERR_ACCESS ) {
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: ReverseX returns Genicam access error", __func__ );
					} else {
						oaLogError ( OA_LOG_CAMERA, "%s: Can't set reverse x, error %d",
								__func__, err );
						return -OA_ERR_SYSTEM_ERROR;
					}
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for reverse X", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: reverse X is inaccessible",
				__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: reverse X unavailable", __func__ );
  }

	return OA_ERR_NONE;
}


int
_checkUnknownControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle			rootHandle, categoryHandle, featureHandle;
	spinNodeType				nodeType;
	bool8_t							available, readable;
	size_t							numCategories, numFeatures, featureNameLen;
	unsigned int				i, j, k, found;
	char								featureName[ SPINNAKER_MAX_BUFF_LEN ];

  if (( *p_spinNodeMapGetNode )( nodeMap, "Root", &rootHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera root nodemap",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinCategoryGetNumFeatures )( rootHandle, &numCategories ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of root categories",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  for ( i = 0; i < numCategories; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( rootHandle, i, &categoryHandle )
        != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get category handle", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = False;
    if (( *p_spinNodeIsAvailable )( categoryHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get category available",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( categoryHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get category readable",
						__func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: unavailable category", __func__ );
      continue;
    }
    if ( !readable ) {
      oaLogError ( OA_LOG_CAMERA, "%s: unreadable category", __func__ );
      continue;
    }

    if (( *p_spinNodeGetType )( categoryHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get category node type",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( nodeType == CategoryNode ) {
			if (( *p_spinCategoryGetNumFeatures )( categoryHandle, &numFeatures ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of analogue features",
				__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

			if ( numFeatures > 0 ) {
				for ( j = 0; j < numFeatures; j++ ) {
					if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, j,
							&featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
						oaLogError ( OA_LOG_CAMERA, "%s: Can't get analogue feature handle",
								__func__ );
						return -OA_ERR_SYSTEM_ERROR;
					}

					featureNameLen = SPINNAKER_MAX_BUFF_LEN;
					if (( *p_spinNodeGetName )( featureHandle, featureName,
							&featureNameLen ) == SPINNAKER_ERR_SUCCESS ) {
						found = 0;
						for ( k = 0; !found && k < sizeof ( spinFeatureStrings ); k++ ) {
							if ( !strcmp ( featureName, spinFeatureStrings[k] )) {
								found = 1;
							}
						}
						if ( !found ) {
							oaLogWarning ( OA_LOG_CAMERA,
									"%s: unknown feature '%s' found", __func__, featureName );
						}
					}
				}
			}
		}
	}
	return OA_ERR_NONE;
}


static int
_getNodeData ( spinNodeMapHandle nodeMap, const char* nodeName,
		spinNodeHandle* handle, bool8_t* implemented, bool8_t* available,
		bool8_t* readable, bool8_t* writeable, spinNodeType* nodeType )
{
	spinError					err;

  if (( err = ( *p_spinNodeMapGetNode )( nodeMap, nodeName, handle )) !=
      SPINNAKER_ERR_SUCCESS ) {
		// Spinnaker appears to allow getting a node that doesn't even exist,
		// so this probably shouldn't ever fail unless the world has blown up
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get node for %s, error %d",
				__func__, nodeName, err );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( err = ( *p_spinNodeIsImplemented )( *handle, implemented )) !=
      SPINNAKER_ERR_SUCCESS ) {
		// An invalid handle error means a node of this name, even if it is
		// a valid name, is not present for the camera
		if ( err == SPINNAKER_ERR_INVALID_HANDLE ) {
			*implemented = False;
			return OA_ERR_NONE;
		}
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsImplemented failed for %s, error %d", __func__,
				nodeName, err );
    return -OA_ERR_SYSTEM_ERROR;
  }

	// "available" appears to mean that the node may exist, but for some reason
	// it can't be used at the moment
  *available = *readable = *writeable = False;
  if (( err = ( *p_spinNodeIsAvailable )( *handle, available )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for %s, error %d", __func__, nodeName,
				err );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if (( err = ( *p_spinNodeIsReadable )( *handle, readable )) !=
      SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA,
			"%s: spinNodeIsReadable failed for %s, error %d", __func__,
			nodeName, err );
      return -OA_ERR_SYSTEM_ERROR;
  }

  if (( err = ( *p_spinNodeIsWritable )( *handle, writeable )) !=
      SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA,
			"%s: spinNodeIsWritable failed for %s, error %d", __func__,
			nodeName, err );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( err = ( *p_spinNodeGetType )( *handle, nodeType )) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get node type for %s, error %d", __func__, nodeName, err );
				return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}


static void
_showIntegerNode ( spinNodeHandle intNode, bool8_t writeable )
{
  int64_t	min, max, step, curr;

  if (( *p_spinIntegerGetValue )( intNode, &curr ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get int current value", __func__ );
    return;
  }
  if (( *p_spinIntegerGetMin )( intNode, &min ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get int min value", __func__ );
    return;
  }
  if (( *p_spinIntegerGetMax )( intNode, &max ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get int max value", __func__ );
    return;
  }
  if (( *p_spinIntegerGetInc )( intNode, &step ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get int inc value", __func__ );
    return;
  }

  oaLogDebug ( OA_LOG_CAMERA, "%s:   [%ld:%ld]/[%ld] := %ld", __func__,
		min, max, step, curr );

  return;
}


static void
_showBooleanNode ( spinNodeHandle boolNode )
{
  bool8_t	curr;

  if (( *p_spinBooleanGetValue )( boolNode, &curr ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get bool current value",
				__func__ );
    return;
  }

  oaLogDebug ( OA_LOG_CAMERA, "%s:   [boolean] := %s", __func__,
			curr ? "true" : "false" );
  return;
}


static void
_showFloatNode ( spinNodeHandle floatNode, bool8_t writeable )
{
  double       min, max, curr;

  if (( *p_spinFloatGetValue )( floatNode, &curr ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get float current value", __func__ );
    return;
  }

  if (( *p_spinFloatGetMin )( floatNode, &min ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get float min value", __func__ );
    return;
  }
  if (( *p_spinFloatGetMax )( floatNode, &max ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get float max value", __func__ );
    return;
  }

  oaLogDebug ( OA_LOG_CAMERA, "%s:   [%f:%f] := %f", __func__,
		min, max, curr );
  
  return;
}

/*
static void
_showStringNode ( spinNodeHandle stringNode )
{
  char                  string[ SPINNAKER_MAX_BUFF_LEN ];
  size_t                stringLen = SPINNAKER_MAX_BUFF_LEN;

  if (( *p_spinNodeToString )( stringNode, string, &stringLen )
      != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get string value", __func__ );
    return;
  }

  oaLogDebug ( OA_LOG_CAMERA, "%s:   [%s]", __func__, string );
  return;
}
*/

static void
_showEnumerationNode ( spinNodeHandle enumNode, int inSNFC )
{
  size_t					numEntries;
  unsigned int		i;
	bool8_t					available;
  spinNodeHandle	entryHandle, currentHandle;
  char						entryName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t					entryNameLen;
  size_t					enumValue;
  int64_t					intValue;
	spinError				err;

  oaLogDebug ( OA_LOG_CAMERA, "%s:   ", __func__ );
  if (( *p_spinEnumerationGetNumEntries )( enumNode, &numEntries ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of enum node entries",
				__func__ );
    return;
  }

  for ( i = 0; i < numEntries; i++ ) {
    if (( *p_spinEnumerationGetEntryByIndex )( enumNode, i, &entryHandle )
        != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum handle", __func__ );
      return;
    }

    entryNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( entryHandle, entryName, &entryNameLen )
        != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA,
					"%s: Can't get enum name", __func__ );
      return;
    }
		available = False;
		if (( err = ( *p_spinNodeIsAvailable )( entryHandle, &available )) !=
				SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsAvailable failed for %s, error %d", __func__,
					entryName, err );
			return;
		}
    oaLogDebug ( OA_LOG_CAMERA, "%s: [%s] (%savailable)", __func__, entryName,
				available ? "" : "un" );
  }

  if (( *p_spinEnumerationGetCurrentEntry )( enumNode, &currentHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum current value", __func__ );
    return;
  }
	if ( inSNFC ) {
		if (( err = ( *p_spinEnumerationEntryGetEnumValue )( currentHandle,
				&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: Can't get value of enum, error %d", __func__, err );
			enumValue = -1;
		}
	}
	if (( err = ( *p_spinEnumerationEntryGetIntValue )( currentHandle,
			&intValue )) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get int value of enum, error %d", __func__, err );
	}
	if ( inSNFC ) {
		oaLogDebug ( OA_LOG_CAMERA, "%s: := %ld(enum), %ld(int)", __func__,
				enumValue, intValue );
	} else {
		oaLogDebug ( OA_LOG_CAMERA, "%s: := %ld(int)", __func__, intValue );
	}
  return;
}


static void
_spinInitFunctionPointers ( oaCamera* camera )
{

  camera->funcs.initCamera = oaSpinInitCamera;
  camera->funcs.closeCamera = oaSpinCloseCamera;

  camera->funcs.testControl = oaSpinCameraTestControl;
  camera->funcs.getControlRange = oaSpinCameraGetControlRange;
/*
  camera->funcs.getControlDiscreteSet = oaSpinCameraGetControlDiscreteSet;

  camera->funcs.testROISize = oaSpinCameraTestROISize;
*/
  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaSpinCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaSpinCameraGetFramePixelFormat;

/*
  camera->funcs.enumerateFrameRates = oaSpinCameraGetFrameRates;
  camera->funcs.setFrameInterval = oaSpinCameraSetFrameInterval;
*/
  camera->funcs.getMenuString = oaSpinCameraGetMenuString;
}


int
oaSpinCloseCamera ( oaCamera* camera )
{
  void*							dummy;
  SPINNAKER_STATE*	cameraInfo;
	int								j;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    ( void ) ( *p_spinCameraDeInit )( cameraInfo->cameraHandle );
		( void ) ( *p_spinCameraRelease )( cameraInfo->cameraHandle );
		( void ) ( *p_spinSystemReleaseInstance )( cameraInfo->systemHandle );

    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }

		//if ( cameraInfo->frameRates.numRates ) {
		//	free (( void* ) cameraInfo->frameRates.rates );
		//}

		for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
			if ( cameraInfo->frameSizes[ j ].numSizes ) {
				free (( void* ) cameraInfo->frameSizes[ j ].sizes );
				//if ( cameraInfo->frameModes[ j ] ) {
				//	free (( void* ) cameraInfo->frameModes[ j ]);
				//}
			}
		}

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );

    // free (( void* ) cameraInfo->metadataBuffers );
		free (( void* ) cameraInfo->buffers );
		//if ( cameraInfo->triggerModes ) {
		//	free (( void* ) cameraInfo->triggerModes );
		//}
    free (( void* ) camera->_common );
    free (( void* ) cameraInfo );
    free (( void* ) camera );

  } else {
    return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
