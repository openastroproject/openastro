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
static void	_showIntegerNode ( spinNodeHandle, bool8_t );
static void	_showBooleanNode ( spinNodeHandle );
static void	_showFloatNode ( spinNodeHandle, bool8_t );
//static void	_showStringNode ( spinNodeHandle );
static void	_showEnumerationNode ( spinNodeHandle );
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

	oaLogWarning ( OA_LOG_CAMERA,
			"%s: Should only be testing for colour controls on colour camera?",
			__func__ );

	if ( _checkHueControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkSaturationControls ( cameraNodeMapHandle, camera ) < 0 ) {
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( _checkTriggerControls ( cameraNodeMapHandle, camera ) < 0 ) {
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
_checkGainControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		gain, autoGain;
  bool8_t						available, readable, writeable;
	double						min, max, curr;
	int								currInt;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
	spinNodeHandle		valueHandle;
	size_t						enumValue;
	spinError					r;
	int								autoGainValid = 0;

  if (( *p_spinNodeMapGetNode )( nodeMap, "GainAuto", &autoGain ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto gain node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( autoGain, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for auto gain", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( autoGain, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for auto gain", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( autoGain, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for auto gain", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( autoGain, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for auto gain", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto gain control",
						__func__ );
				_showEnumerationNode ( autoGain );
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
				}
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for auto gain", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: auto gain is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: auto gain unavailable", __func__ );
  }

	if ( autoGainValid && commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAIN )) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check auto gain is disabled "
				"before checking gain range", __func__ );
	}

  if (( *p_spinNodeMapGetNode )( nodeMap, "Gain", &gain ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get gain node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( gain, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsAvailable failed for gain",
			__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( gain, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsReadable failed for gain",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( gain, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsWritable failed for gain",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( gain, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get node type for gain",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

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

				// Potentially temporarily, convert this to a range from 0 to 400
				currInt = ( curr - min ) * 400.0 / ( max - min );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) =
						OA_CTRL_TYPE_INT32;
				commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = 0;
				commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = 400;
				commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
				commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = currInt;
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

  if (( r = ( *p_spinNodeMapGetNode )( nodeMap, "HueEnabled", &hueEnabled )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get hue enabled node, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( r = ( *p_spinNodeIsImplemented )( hueEnabled, &implemented )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsImplemented failed for hue enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( r = ( *p_spinNodeIsAvailable )( hueEnabled, &available )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for hue enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( hueEnabled, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for hue enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( hueEnabled, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for hue enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( hueEnabled, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for hue enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
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

	if ( hueEnabledValid ) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check hue enabled is set "
				"before checking other hue controls", __func__ );
	}

  if (( *p_spinNodeMapGetNode )( nodeMap, "Hue", &hue ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get hue node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinNodeMapGetNode )( nodeMap, "HueAuto", &autoHue ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto hue node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( autoHue, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for auto hue", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( autoHue, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for auto hue", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( autoHue, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for auto hue", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( autoHue, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for auto hue", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto hue control",
						__func__ );
				_showEnumerationNode ( autoHue );
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

	if ( autoHueValid && commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_HUE )) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check auto hue is disabled "
				"before checking hue range", __func__ );
	}

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( hue, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsAvailable failed for hue",
			__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( hue, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsReadable failed for hue",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( hue, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsWritable failed for hue",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( hue, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get node type for hue",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

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

  if (( r = ( *p_spinNodeMapGetNode )( nodeMap, "SaturationEnabled",
			&saturationEnabled )) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get saturation enabled node, error %d", __func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( r = ( *p_spinNodeIsImplemented )( saturationEnabled, &implemented )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsImplemented failed for saturation enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( r = ( *p_spinNodeIsAvailable )( saturationEnabled, &available )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for saturation enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( saturationEnabled, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for saturation enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( saturationEnabled, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for saturation enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( saturationEnabled, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for saturation enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
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

	if ( saturationEnabledValid ) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check saturation enabled is "
				"set before checking other saturation controls", __func__ );
	}

  if (( *p_spinNodeMapGetNode )( nodeMap, "Saturation", &saturation ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get saturation node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinNodeMapGetNode )( nodeMap, "SaturationAuto",
				&autoSaturation ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto saturation node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( autoSaturation, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for auto saturation", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( autoSaturation, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for auto saturation", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( autoSaturation, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for auto saturation", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( autoSaturation, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for auto saturation", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto saturation control",
						__func__ );
				_showEnumerationNode ( autoSaturation );
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

	if ( autoSaturationValid &&
			commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_SATURATION )) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check auto saturation is "
				"disabled before checking saturation range", __func__ );
	}

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( saturation, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for saturation", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( saturation, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for saturation", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( saturation, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for saturation", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( saturation, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get node type for saturation",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

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
	int								ctrl;
	int								sharpnessEnabledValid = 0, autoSharpnessValid = 0;
	int64_t						intValue;
	spinError					r;

  if (( r = ( *p_spinNodeMapGetNode )( nodeMap, "SharpnessEnabled",
			&sharpnessEnabled )) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get sharpness enabled node, error %d", __func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( r = ( *p_spinNodeIsImplemented )( sharpnessEnabled, &implemented )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsImplemented failed for sharpness enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( r = ( *p_spinNodeIsAvailable )( sharpnessEnabled, &available )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for sharpness enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( sharpnessEnabled, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for sharpness enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( sharpnessEnabled, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for sharpness enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( sharpnessEnabled, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for sharpness enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
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

	if ( sharpnessEnabledValid ) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check sharpness enabled is "
				"set before checking other sharpness controls", __func__ );
	}

  if (( *p_spinNodeMapGetNode )( nodeMap, "Sharpness", &sharpness ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get sharpness node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinNodeMapGetNode )( nodeMap, "SharpnessAuto",
				&autoSharpness ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto sharpness node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( autoSharpness, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for auto sharpness", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( autoSharpness, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for auto sharpness", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( autoSharpness, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for auto sharpness", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( autoSharpness, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for auto sharpness", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto sharpness control",
						__func__ );
				_showEnumerationNode ( autoSharpness );
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

	if ( autoSharpnessValid &&
			commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_SHARPNESS )) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check auto sharpness is "
				"disabled before checking sharpness range", __func__ );
	}

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( sharpness, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for sharpness", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( sharpness, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for sharpness", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( sharpness, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for sharpness", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( sharpness, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get node type for sharpness",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

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

  if (( r = ( *p_spinNodeMapGetNode )( nodeMap, "BlackLevelEnabled",
			&blackLevelEnabled )) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get blackLevel enabled node, error %d", __func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( r = ( *p_spinNodeIsImplemented )( blackLevelEnabled, &implemented )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsImplemented failed for blackLevel enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( r = ( *p_spinNodeIsAvailable )( blackLevelEnabled, &available )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for blackLevel enabled, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( blackLevelEnabled, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for blackLevel enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( blackLevelEnabled, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for blackLevel enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( blackLevelEnabled, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for blackLevel enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
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

	if ( blackLevelEnabledValid ) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check blackLevel enabled is "
				"set before checking other blackLevel controls", __func__ );
	}

  if (( *p_spinNodeMapGetNode )( nodeMap, "BlackLevel", &blackLevel ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get blackLevel node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinNodeMapGetNode )( nodeMap, "BlackLevelAuto",
				&autoBlackLevel ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto blackLevel node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( autoBlackLevel, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for auto blackLevel", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( autoBlackLevel, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for auto blackLevel", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( autoBlackLevel, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for auto blackLevel", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( autoBlackLevel, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for auto blackLevel", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto blackLevel control",
						__func__ );
				_showEnumerationNode ( autoBlackLevel );
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

	if ( autoBlackLevelValid &&
			commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_BLACKLEVEL )) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check auto blacklevel is "
				"disabled before checking blacklevel range", __func__ );
	}

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( blackLevel, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for blacklevel", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( blackLevel, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for blacklevel", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( blackLevel, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for blacklevel", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( blackLevel, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get node type for blacklevel",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

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
  bool8_t						available, readable, writeable;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
	int								autoWhiteBalanceValid = 0;
	size_t						enumValue;
	spinError					r;
	int								curr;

  if (( *p_spinNodeMapGetNode )( nodeMap, "BalanceWhiteAuto",
				&autoWhiteBalance ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto white balance node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( autoWhiteBalance, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for auto white balance", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( autoWhiteBalance, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for auto white balance", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( autoWhiteBalance, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for auto white balance", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( autoWhiteBalance, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for auto white balance", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto white balance control",
						__func__ );
				_showEnumerationNode ( autoWhiteBalance );
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

	return OA_ERR_NONE;
}


int
_checkResetControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		reset;
  bool8_t						available, readable, writeable;
  spinNodeType			nodeType;

  if (( *p_spinNodeMapGetNode )( nodeMap, "DeviceReset",
				&reset ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get reset node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( reset, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for reset", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( reset, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for reset", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( reset, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for reset", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( writeable ) {
			if (( *p_spinNodeGetType )( reset, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for reset", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == CommandNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found reset control",
						__func__ );
				camera->features.flags |= OA_CAM_FEATURE_RESET;
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
  bool8_t						available, readable;
  spinNodeType			nodeType;

  if (( *p_spinNodeMapGetNode )( nodeMap, "DeviceTemperature",
				&temperature ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get temperature node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = False;
  if (( *p_spinNodeIsAvailable )( temperature, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for temperature", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( temperature, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for temperature", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( readable ) {
			if (( *p_spinNodeGetType )( temperature, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for temperature", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == FloatNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found temperature control",
						__func__ );
        _showFloatNode ( temperature, 0 );
				camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) =
						OA_CTRL_TYPE_READONLY;
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
  bool8_t						available, readable, writeable;
	double						min, max, curr;
  spinNodeType			nodeType;
  COMMON_INFO*			commonInfo = camera->_common;
	spinNodeHandle		valueHandle;
	size_t						enumValue;
	spinError					r;
	int								autoExposureValid = 0;

  if (( *p_spinNodeMapGetNode )( nodeMap, "ExposureAuto", &autoExposure ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get auto exposure node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( autoExposure, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for auto exposure", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( autoExposure, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for auto exposure", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( autoExposure, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for auto exposure", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( autoExposure, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for auto exposure", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found auto exposure control",
						__func__ );
				_showEnumerationNode ( autoExposure );
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

	if ( autoExposureValid &&
			commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) {
		oaLogWarning ( OA_LOG_CAMERA, "%s: need to check auto exposure is disabled "
				"before checking exposure range", __func__ );
	}

  if (( *p_spinNodeMapGetNode )( nodeMap, "ExposureTimeAbs", &exposure ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get exposure node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( exposure, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsAvailable failed for exposure",
			__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( exposure, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsReadable failed for exposure",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( exposure, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA, "%s: spinNodeIsWritable failed for exposure",
				__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( readable || writeable ) {
			if (( *p_spinNodeGetType )( exposure, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get node type for exposure",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}

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

  if (( *p_spinNodeMapGetNode )( nodeMap, "ExposureMode",
			&exposureMode ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get exposure mode node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( exposureMode, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for exposure mode", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( exposureMode, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for exposure mode", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( exposureMode, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for exposure mode", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( exposureMode, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for exposure mode", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found exposure mode control",
						__func__ );
				_showEnumerationNode ( exposureMode );
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
	spinNodeHandle		acquisitionStop, singleFrameMode;
  bool8_t						available, readable, writeable;
  spinNodeType			nodeType;

  if (( *p_spinNodeMapGetNode )( nodeMap, "AcquisitionFrameRateEnabled",
			&frameRateEnabled ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get frame rate enabled node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( frameRateEnabled, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for frame rate enabled", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( frameRateEnabled, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for frame rate enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( frameRateEnabled, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for frame rate enabled", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( frameRateEnabled, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for frame rate enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == BooleanNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found frame rate enabled control",
						__func__ );
				_showBooleanNode ( frameRateEnabled );
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

  if (( *p_spinNodeMapGetNode )( nodeMap, "AcquisitionMode",
			&acquisitionMode ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get acquisition mode node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( acquisitionMode, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for acquisition mode", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( acquisitionMode, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for acquisition mode", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( acquisitionMode, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for acquisition mode", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( acquisitionMode, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for acquisition mode", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found acquisition mode control",
						__func__ );
				_showEnumerationNode ( acquisitionMode );
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

  if (( *p_spinNodeMapGetNode )( nodeMap, "AcquisitionStart",
			&acquisitionStart ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get acquisition start node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( acquisitionStart, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for acquisition start", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( acquisitionStart, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for acquisition start", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( acquisitionStart, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for acquisition start", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( acquisitionStart, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for acquisition start", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == CommandNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found acquisition start control",
						__func__ );
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for acquisition start", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: acquisition start is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: acquisition start unavailable", __func__ );
  }

  if (( *p_spinNodeMapGetNode )( nodeMap, "AcquisitionStop",
			&acquisitionStop ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get acquisition start node",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( acquisitionStop, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for acquisition stop", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( acquisitionStop, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for acquisition stop", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( acquisitionStop, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for acquisition stop", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( acquisitionStop, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for acquisition stop", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == CommandNode ) {
				oaLogInfo ( OA_LOG_CAMERA, "%s: Found acquisition stop control",
						__func__ );
			} else {
				oaLogWarning ( OA_LOG_CAMERA,
						"%s: Unrecognised node type '%s' for acquisition stop", __func__,
						nodeTypes[ nodeType ] );
			}
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: acquisition stop is inaccessible",
					__func__ );
		}
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: acquisition stop unavailable", __func__ );
  }

  if (( *p_spinNodeMapGetNode )( nodeMap, "SingleFrameAcquisitionMode",
			&singleFrameMode ) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get single frame acquisition mode node", __func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( *p_spinNodeIsAvailable )( singleFrameMode, &available ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for single frame acquisition mode",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( singleFrameMode, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for single frame acquisition mode",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( singleFrameMode, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for single frame acquisition mode",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

		// Doesn't make much sense that this node not be readable and
		// writeable?
    if ( readable && writeable ) {
			if (( *p_spinNodeGetType )( singleFrameMode, &nodeType ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get node type for single frame acquisition mode",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( nodeType == EnumerationNode ) {
				oaLogInfo ( OA_LOG_CAMERA,
						"%s: Found single frame acquisition mode control", __func__ );
				_showEnumerationNode ( singleFrameMode );
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

	return OA_ERR_NONE;
}


int
_checkTriggerControls ( spinNodeMapHandle nodeMap, oaCamera* camera )
{
	spinNodeHandle		triggerOverlap;
  bool8_t						available, readable, writeable, implemented;
	spinError					r;

  if (( r = ( *p_spinNodeMapGetNode )( nodeMap, "TriggerOverlap",
			&triggerOverlap )) != SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get trigger overlap node, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( r = ( *p_spinNodeIsImplemented )( triggerOverlap, &implemented )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsImplemented failed for trigger overlap, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }

  available = readable = writeable = False;
  if (( r = ( *p_spinNodeIsAvailable )( triggerOverlap, &available )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: spinNodeIsAvailable failed for trigger overlap, error %d",
				__func__, r );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( available ) {
    if (( *p_spinNodeIsReadable )( triggerOverlap, &readable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsReadable failed for trigger overlap", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if (( *p_spinNodeIsWritable )( triggerOverlap, &writeable ) !=
        SPINNAKER_ERR_SUCCESS ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: spinNodeIsWritable failed for trigger overlap", __func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
		oaLogInfo ( OA_LOG_CAMERA, "%s: trigger overlap readable/writeable: %d/%d",
				__func__, readable, writeable );
  } else {
    oaLogInfo ( OA_LOG_CAMERA, "%s: trigger overlap unavailable", __func__ );
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

  oaLogInfo ( OA_LOG_CAMERA, "%s:   [%ld:%ld]/[%ld] := %ld", __func__,
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
*/

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
