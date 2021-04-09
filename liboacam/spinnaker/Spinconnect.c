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

#include "unimplemented.h"
#include "oacamprivate.h"
#include "Spinoacam.h"
#include "Spin.h"
#include "Spinstate.h"
#include "Spinstrings.h"


static void	_spinInitFunctionPointers ( oaCamera* );
static int	_processCameraEntry ( spinCamera, oaCamera* );
//static void	_showIntegerNode ( spinNodeHandle, bool8_t );
static void	_showBooleanNode ( spinNodeHandle );
static void	_showFloatNode ( spinNodeHandle, bool8_t );
//static void	_showStringNode ( spinNodeHandle );
//static void	_showEnumerationNode ( spinNodeHandle );
static int	_readGainControls ( spinNodeMapHandle, oaCamera* );
static int	_readGammaControls ( spinNodeMapHandle, oaCamera* );


oaCamera*
oaSpinInitCamera ( oaCameraDevice* device )
{
  oaCamera*		camera;
  SPINNAKER_STATE*	cameraInfo;
  COMMON_INFO*		commonInfo;
  DEVICE_INFO*		devInfo;
  spinSystem		systemHandle;
  spinInterfaceList	ifaceListHandle = 0;
  size_t		numInterfaces = 0;
  spinCameraList	cameraListHandle = 0;
  size_t		numCameras = 0;
  spinInterface		ifaceHandle = 0;
  spinCamera		cameraHandle;
  spinNodeMapHandle	cameraNodeMapHandle = 0;
  spinNodeHandle	deviceIdHandle = 0;
  bool8_t		deviceIdAvailable = False;
  bool8_t		deviceIdReadable = False;
  char			deviceId[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		deviceIdLen = SPINNAKER_MAX_BUFF_LEN;
  unsigned int		i, j, found;

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( SPINNAKER_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  _spinInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;

  camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;

  if (( *p_spinSystemGetInstance )( &systemHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get system instance", __func__ );
    return 0;
  }

  if (( *p_spinInterfaceListCreateEmpty )( &ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't create empty interface list", __func__ );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (( *p_spinSystemGetInterfaces )( systemHandle, ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get interfaces", __func__ );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (( *p_spinInterfaceListGetSize )( ifaceListHandle, &numInterfaces ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get size of interface list",
				__func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if ( !numInterfaces ) {
    oaLogError ( OA_LOG_CAMERA, "%s: No interfaces found", __func__ );
    ( void ) ( *p_spinInterfaceListClear  )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy  )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance  )( systemHandle );
    return 0;
  }

  if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't create empty camera list",
				__func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (( *p_spinSystemGetCameras )( systemHandle, cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera list", __func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
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
    return 0;
  }

  ( void ) ( *p_spinCameraListClear )( cameraListHandle );
  ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
  cameraListHandle = 0;
  if ( !numCameras ) {
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
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
      return 0;
    }

    if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't create empty camera list",
					__func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }

    if (( *p_spinInterfaceGetCameras )( ifaceHandle, cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera list",
					__func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }

    if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera count",
					__func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
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
          return 0;
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
              return 0;
            }

            if ( !strcmp ( deviceId, devInfo->deviceId )) {
              found = 1;
              if ( _processCameraEntry ( cameraHandle, camera ) < 0 ) {
                oaLogError ( OA_LOG_CAMERA,
										"%s: Failed to process camera nodemap", __func__ );
                ( void ) ( *p_spinCameraRelease )( cameraHandle );
                ( void ) ( *p_spinCameraListClear )( cameraListHandle );
                ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
                ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
                ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
                ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
                return 0;
              }
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
        return 0;
      }

      if (( *p_spinCameraListDestroy )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't destroy camera list", __func__ );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return 0;
      }
    } else {
      ( void ) ( *p_spinCameraListClear )( cameraListHandle );
      ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
      oaLogError ( OA_LOG_CAMERA, "%s: Interface %d has no cameras", __func__,
					i );
    }

    if (( *p_spinInterfaceRelease )( ifaceHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't release interface", __func__ );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }
  }

  ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
  ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );

  if ( !found ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't find camera", __func__ );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  // Don't want to do this here, eventually
  ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );

  return 0;
}


static int
_processCameraEntry ( spinCamera cameraHandle, oaCamera* camera )
{
  spinNodeMapHandle	cameraNodeMapHandle = 0;
  int			err;

  if (( *p_spinCameraInit )( cameraHandle ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't initialise camera", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( err = ( *p_spinCameraGetNodeMap )( cameraHandle,
      &cameraNodeMapHandle )) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera nodemap: err %d",
				__func__, err );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if ( _readGainControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _readGammaControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	oaLogWarning ( OA_LOG_CAMERA,
			"%s: Should we check for unrecognised features?", __func__ );

  // Won't eventually want to do this here
  ( void ) ( *p_spinCameraDeInit )( cameraHandle );

  return -OA_ERR_SYSTEM_ERROR;
}


int
_readGainControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	return OA_ERR_NONE;
}


int
_readGammaControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		gamma, gammaEnabled;
  bool8_t						available, readable, writeable, currBool;
	double						min, max, curr;
	int								currInt;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								ctrl;

  if (( *p_spinNodeMapGetNode )( nodeMap, "Gamma", &gamma ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get gamma node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( gamma, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsAvailable failed for gamma",
			__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( gamma, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsReadable failed for gamma",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( gamma, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsWritable failed for gamma",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( gamma, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get node type for gamma",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

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

  if (( *p_spinNodeMapGetNode )( nodeMap, "GammaEnabled", &gammaEnabled ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get gamma enabled node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( gammaEnabled, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for gamma enabled", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( gammaEnabled, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for gamma enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( gammaEnabled, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for gamma enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( gammaEnabled, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for gamma enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
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

	return OA_ERR_NONE;
}

/*
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

  oaLogInfo ( OA_LOG_CAMERA, "%s:   [%ld:%ld]/[%ld] := %ld", __func__,
		min, max, step, curr );

  return;
}
*/

static void
_showBooleanNode ( spinNodeHandle boolNode )
{
  bool8_t	curr;

  if (( *p_spinBooleanGetValue )( boolNode, &curr ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get bool current value",
				__func__ );
    return;
  }

  oaLogInfo ( OA_LOG_CAMERA, "%s:   [boolean] := %s", __func__,
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

  oaLogInfo ( OA_LOG_CAMERA, "%s:   [%f:%f] := %f", __func__,
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

  oaLogInfo ( OA_LOG_CAMERA, "%s:   [%s]", __func__, string );
  return;
}


static void
_showEnumerationNode ( spinNodeHandle enumNode )
{
  size_t		numEntries;
  unsigned int		i;
  spinNodeHandle	entryHandle, currentHandle;
  char			entryName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		entryNameLen;
  char			value[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		valueLen;

  oaLogInfo ( OA_LOG_CAMERA, "%s:   ", __func__ );
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

    oaLogInfo ( OA_LOG_CAMERA, "%s: [%s] ", __func__, entryName );
  }

  if (( *p_spinEnumerationGetCurrentEntry )( enumNode, &currentHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum current value", __func__ );
    return;
  }
  valueLen = SPINNAKER_MAX_BUFF_LEN;
  if (( *p_spinNodeToString )( currentHandle, value, &valueLen ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum value as string",
				__func__ );
    return;
  }
  oaLogInfo ( OA_LOG_CAMERA, "%s: := %s", __func__, value );
  return;
}
*/

static void
_spinInitFunctionPointers ( oaCamera* camera )
{
/*
  camera->funcs.initCamera = oaSpinInitCamera;
  camera->funcs.closeCamera = oaSpinCloseCamera;

  camera->funcs.testControl = oaSpinCameraTestControl;
  camera->funcs.getControlRange = oaSpinCameraGetControlRange;
  camera->funcs.getControlDiscreteSet = oaSpinCameraGetControlDiscreteSet;

  camera->funcs.testROISize = oaSpinCameraTestROISize;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaSpinCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaSpinCameraGetFramePixelFormat;

  camera->funcs.enumerateFrameRates = oaSpinCameraGetFrameRates;
  camera->funcs.setFrameInterval = oaSpinCameraSetFrameInterval;

  camera->funcs.getMenuString = oaSpinCameraGetMenuString;
*/
}
