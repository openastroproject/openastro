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
static int	_processAnalogueControls ( spinNodeHandle, oaCamera* );
static int	_processDeviceControls ( spinNodeHandle, oaCamera* );
static int	_processAquisitionControls ( spinNodeHandle, oaCamera* );
static int	_processFormatControls ( spinNodeHandle, oaCamera* );
static void	_showIntegerNode ( spinNodeHandle, bool8_t );
static void	_showBooleanNode ( spinNodeHandle );
static void	_showFloatNode ( spinNodeHandle, bool8_t );
static void	_showStringNode ( spinNodeHandle );
static void	_showEnumerationNode ( spinNodeHandle );

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
  spinNodeHandle	rootHandle = 0;
  spinNodeHandle	categoryHandle = 0;
  spinNodeType		nodeType;
  char			categoryName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		categoryNameLen;
  size_t		numCategories;
  int			ret;
  bool8_t		available, readable;
  int			err;
  unsigned int		i;

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

  if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "Root", &rootHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera root nodemap",
				__func__ );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinCategoryGetNumFeatures )( rootHandle, &numCategories ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of root categories",
				__func__ );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  for ( i = 0; i < numCategories; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( rootHandle, i, &categoryHandle )
        != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get category handle", __func__ );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = False;
    if (( *p_spinNodeIsAvailable )( categoryHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get category available",
					__func__ );
      ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( categoryHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get category readable",
						__func__ );
        ( void ) ( *p_spinCameraDeInit )( cameraHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      oaLogError ( OA_LOG_CAMERA,
					"%s: unavailable category", __func__ );
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
      ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    categoryNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( categoryHandle, categoryName,
        &categoryNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get category name", __func__ );
      ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( nodeType == CategoryNode ) {
			int		handled = 0;

      if ( !strcmp ( "Analog Control", categoryName )) {
        if (( ret = _processAnalogueControls ( categoryHandle, camera )) < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
				handled = 1;
      }
      if ( !strcmp ( "Device Control", categoryName )) {
        if (( ret = _processDeviceControls ( categoryHandle, camera )) < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
				handled = 1;
      }
      if ( !strcmp ( "Acquisition Control", categoryName )) {
        if (( ret = _processAquisitionControls ( categoryHandle, camera ))
            < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
				handled = 1;
      }
      if ( !strcmp ( "Image Format Control", categoryName )) {
        if (( ret = _processFormatControls ( categoryHandle, camera )) < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
				handled = 1;
      }
      if ( !strcmp ( "User Set Control", categoryName ) ||
          !strcmp ( "Digital I/O Control", categoryName ) ||
          !strcmp ( "LUT Control", categoryName ) ||
          !strcmp ( "Transport Layer Control", categoryName ) ||
          !strcmp ( "Chunk Data Control", categoryName ) ||
          !strcmp ( "Event Control", categoryName )) {
        // For the time being we ignore these
        continue;
      }
			if ( !handled ) {
				oaLogWarning ( OA_LOG_CAMERA, "%s: Unhandled category '%s'", __func__,
						categoryName );
			}
    } else {
      oaLogWarning ( OA_LOG_CAMERA,
					"%s: Unhandled camera node '%s', type %d", __func__, categoryName,
					nodeType );
    }
  }

  // Won't eventually want to do this here
  ( void ) ( *p_spinCameraDeInit )( cameraHandle );

  return -OA_ERR_SYSTEM_ERROR;
}


static int
_processAnalogueControls ( spinNodeHandle categoryHandle, oaCamera* camera )
{
  spinNodeHandle	featureHandle = 0;
  spinNodeType		nodeType;
  char			featureName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		featureNameLen;
  size_t		numFeatures;
  bool8_t		available, readable, writeable;
  unsigned int		i, j;
  int			featureId;
  COMMON_INFO*			commonInfo = camera->_common;
  SPINNAKER_STATE*	cameraInfo = camera->_private;

  if (( *p_spinCategoryGetNumFeatures )( categoryHandle, &numFeatures ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of analogue features",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    oaLogError ( OA_LOG_CAMERA, "%s: number of analogue features: %ld",
				__func__, numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get analogue feature handle",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = writeable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get analogue feature available",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get analogue feature readable",
						__func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get analogue feature writeable", __func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable feature nodes
      // oaLogError ( OA_LOG_CAMERA, "%s: unavailable analogue feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      oaLogError ( OA_LOG_CAMERA, "%s: inaccessible analogue feature %d",
					__func__, i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get analogue feature node type",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get analogue feature %d name",
					__func__, i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    oaLogInfo ( OA_LOG_CAMERA,
				"%s: analogue feature %d '%s', type %s [%s] found", __func__, i,
				featureName, ( nodeType >= 0 ) ? nodeTypes [ nodeType ] : "unknown",
				readable ? ( writeable ? "RW" : "RO" ) :
        ( writeable ? "WO" : "??" ));

    switch ( nodeType ) {
      case IntegerNode:
        _showIntegerNode ( featureHandle, writeable );
        break;
      case BooleanNode:
        _showBooleanNode ( featureHandle );
        break;
      case FloatNode:
        _showFloatNode ( featureHandle, writeable );
        break;
      case CommandNode:
        oaLogError ( OA_LOG_CAMERA, "%s:   [command]", __func__ );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        oaLogError ( OA_LOG_CAMERA, "%s:   unhandled node type", __func__ );
        break;
    }

    // It's not clear if features are always numbered in the same order for
    // all cameras, but the fact that feature numbers are skipped suggests
    // that might be so.  In case it isn't, do things the hard way :(

    for ( j = 0, featureId = -1; j < ANALOGUE_MAX_FEATURES && featureId < 0;
        j++ ) {
      if ( !strcmp ( featureName, analogueFeatures[ j ] )) {
        featureId = j;
      }
    }

    if ( featureId >= 0 ) {
      switch ( featureId ) {
        case ANALOGUE_GAIN_SELECTOR: // enumeration
          // FIX ME -- Ignore this for the time being.  I have no useful test
					// cases to try it with
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Ignoring ANALOGUE_GAIN_SELECTOR feature", __func__ );
          break;

        case ANALOGUE_GAIN_AUTO: // boolean or enumerated value?
        {
          bool8_t	curr, valid;

					valid = 0;
					if ( nodeType == BooleanNode ) {
						if (( *p_spinBooleanGetValue )( featureHandle, &curr ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA,
									"%s: Can't get bool current value for ANALOGUE_GAIN_AUTO",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						valid = 1;
					} else {
						if ( nodeType == EnumerationNode ) {
							spinNodeHandle	valueHandle;
							size_t					enumValue;
							spinError				r;

							if (( *p_spinEnumerationGetCurrentEntry )( featureHandle,
									&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
								oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum current value",
										__func__ );
								return -OA_ERR_SYSTEM_ERROR;
							}
							if (( r = ( *p_spinEnumerationEntryGetEnumValue )( valueHandle,
									&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
								oaLogError ( OA_LOG_CAMERA,
										"%s: Can't get enum value, error %d", __func__, r );
							}
							switch ( enumValue ) {
								case GainAuto_Off:
									curr = 0;
									valid = 1;
									break;
								case GainAuto_Continuous:
									curr = 1;
									valid = 1;
									break;
								default:
									oaLogWarning ( OA_LOG_CAMERA,
											"%s: Unhandled value '%d' for ANALOGUE_GAIN_AUTO",
											__func__, enumValue );
									curr = 0;
							}
						} else {
							oaLogWarning ( OA_LOG_CAMERA,
									"%s: Unrecognised node type '%s' for ANALOGUE_GAIN_AUTO",
									__func__, nodeTypes[ nodeType ] );
						}
					}
					if ( valid ) {
						camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_GAIN ) =
								OA_CTRL_TYPE_BOOLEAN;
						commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_GAIN ) = 0;
						commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_GAIN ) = 1;
						commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_GAIN ) = 1;
						commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAIN ) = curr ? 1 : 0;
					}
          break;
        }

        case ANALOGUE_GAIN: // float
        {
          // Can't get the actual min/max values available if we have auto
          // gain unless auto gain is turned off.  Assume we'll already have
          // seen auto gain before this if it exists

          break;
        }

				case ANALOGUE_AUTO_GAIN_UPPER_LIMIT:
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: ignoring ANALOGUE_AUTO_GAIN_UPPER_LIMIT control", __func__ );
					break;

				case ANALOGUE_AUTO_GAIN_LOWER_LIMIT:
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: ignoring ANALOGUE_AUTO_GAIN_LOWER_LIMIT control", __func__ );
					break;

				case ANALOGUE_BLACK_LEVEL: // float on Blackfly.  Could be int?
        {
          bool8_t	valid;
					double	min, max, curr;

					valid = 0;
					if ( nodeType == FloatNode ) {
						if (( *p_spinFloatGetValue )( featureHandle, &curr ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get current float value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						if (( *p_spinFloatGetMin )( featureHandle, &min ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get min float value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						if (( *p_spinFloatGetMax )( featureHandle, &max ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get max float value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						valid = 1;
					} else {
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unrecognised node type '%s' for ANALOGUE_BLACK_LEVEL",
								__func__, nodeTypes[ nodeType ] );
					}
					if ( valid ) {
						int	intCurr;

						cameraInfo->minFloatBlacklevel = min;
						cameraInfo->maxFloatBlacklevel = max;
						// Potentially temporarily, convert this to a range from 0 to 100
						intCurr = ( curr - min ) * 100.0 / ( max - min );
						camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLACKLEVEL ) =
								OA_CTRL_TYPE_INT32;
						commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLACKLEVEL ) = 0;
						commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLACKLEVEL ) = 100;
						commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLACKLEVEL ) = 1;
						commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLACKLEVEL ) = intCurr;
					}
          break;
				}

				case ANALOGUE_BLACK_LEVEL_ENABLED: // boolean
        {
          bool8_t	curr, valid;

					if ( nodeType == BooleanNode ) {
						if (( *p_spinBooleanGetValue )( featureHandle, &curr ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get bool current value "
									"for ANALOGUE_BLACK_LEVEL_ENABLED", __func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						valid = 1;
					} else {
						oaLogWarning ( OA_LOG_CAMERA, "%s: Unrecognised node type '%s' "
								"for ANALOGUE_BLACK_LEVEL_ENABLED", __func__,
								nodeTypes[ nodeType ] );
					}
					if ( valid ) {
						int ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_BLACKLEVEL );
						camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
						commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
						commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
						commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
						commonInfo->OA_CAM_CTRL_DEF( ctrl ) = curr ? 1 : 0;
					}
          break;
				}

				case ANALOGUE_GAMMA:
				// This is a float on Blackfly.  It doesn't appear to be part of
				// the Genicam standard, so perhaps that's all it will ever be.
        {
          bool8_t	valid;
					double	min, max, curr;

					valid = 0;
					if ( nodeType == FloatNode ) {
						if (( *p_spinFloatGetValue )( featureHandle, &curr ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get current float value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						if (( *p_spinFloatGetMin )( featureHandle, &min ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get min float value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						if (( *p_spinFloatGetMax )( featureHandle, &max ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get max float value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						valid = 1;
					} else {
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unrecognised node type '%s' for ANALOGUE_GAMMA",
								__func__, nodeTypes[ nodeType ] );
					}
					if ( valid ) {
						int	intCurr;

						cameraInfo->minFloatGamma = min;
						cameraInfo->maxFloatGamma = max;
						// Potentially temporarily, convert this to a range from 0 to 100
						intCurr = ( curr - min ) * 100.0 / ( max - min );
						camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAMMA ) =
								OA_CTRL_TYPE_INT32;
						commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) = 0;
						commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) = 100;
						commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAMMA ) = 1;
						commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) = intCurr;
					}
          break;
				}

				case ANALOGUE_GAMMA_ENABLED: // boolean
        {
          bool8_t	curr, valid;

					if ( nodeType == BooleanNode ) {
						if (( *p_spinBooleanGetValue )( featureHandle, &curr ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get bool current value "
									"for ANALOGUE_GAMMA_ENABLED", __func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						valid = 1;
					} else {
						oaLogWarning ( OA_LOG_CAMERA, "%s: Unrecognised node type '%s' "
								"for ANALOGUE_GAMMA_ENABLED", __func__,
								nodeTypes[ nodeType ] );
					}
					if ( valid ) {
						int ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_GAMMA );
						camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
						commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
						commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
						commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
						commonInfo->OA_CAM_CTRL_DEF( ctrl ) = curr ? 1 : 0;
					}
          break;
				}

				case ANALOGUE_SHARPNESS:
				// int on Blackfly, and again not apparently a part of the Genicam
				// standard
        {
          bool8_t	valid;
					int64_t	min, max, curr, step;

					valid = 0;
					if ( nodeType == IntegerNode ) {
						if (( *p_spinIntegerGetValue )( featureHandle, &curr ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get current int value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						if (( *p_spinIntegerGetMin )( featureHandle, &min ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get min int value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						if (( *p_spinIntegerGetMax )( featureHandle, &max ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get max int value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						if (( *p_spinIntegerGetInc )( featureHandle, &step ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get max int value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						valid = 1;
					} else {
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unrecognised node type '%s' for ANALOGUE_SHARPNESS",
								__func__, nodeTypes[ nodeType ] );
					}
					if ( valid ) {
						camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHARPNESS ) =
								OA_CTRL_TYPE_INT32;
						commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHARPNESS ) = min;
						commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHARPNESS ) = max;
						commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHARPNESS ) = step;
						commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHARPNESS ) = curr;
					}
          break;
				}

				case ANALOGUE_SHARPNESS_ENABLED: // boolean
        {
          bool8_t	curr, valid;

					if ( nodeType == BooleanNode ) {
						if (( *p_spinBooleanGetValue )( featureHandle, &curr ) !=
								SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get bool current value "
									"for ANALOGUE_SHARPNESS_ENABLED", __func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						valid = 1;
					} else {
						oaLogWarning ( OA_LOG_CAMERA, "%s: Unrecognised node type '%s' "
								"for ANALOGUE_SHARPNESS_ENABLED", __func__,
								nodeTypes[ nodeType ] );
					}
					if ( valid ) {
						int ctrl = OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_SHARPNESS );
						camera->OA_CAM_CTRL_TYPE( ctrl ) = OA_CTRL_TYPE_BOOLEAN;
						commonInfo->OA_CAM_CTRL_MIN( ctrl ) = 0;
						commonInfo->OA_CAM_CTRL_MAX( ctrl ) = 1;
						commonInfo->OA_CAM_CTRL_STEP( ctrl ) = 1;
						commonInfo->OA_CAM_CTRL_DEF( ctrl ) = curr ? 1 : 0;
					}
          break;
				}

        case ANALOGUE_SHARPNESS_AUTO:
				// Seems to be enumerated value on the Blackfly.  Not apparently part
				// of the Genicam standard, so hopefully it will be the same for
				// all Point Grey/FLIR cameras
        {
          bool8_t	curr, valid;

					valid = 0;
					if ( nodeType == EnumerationNode ) {
						spinNodeHandle	valueHandle;
						int64_t					intValue;
						spinError				r;

						if (( *p_spinEnumerationGetCurrentEntry )( featureHandle,
								&valueHandle ) != SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum current value",
									__func__ );
							return -OA_ERR_SYSTEM_ERROR;
						}
						/*
						 * Getting the enum value doesn't work, presumably because
						 * "sharpness" is not a standard Genicam control.  Have to
						 * use the integer value instead
						 *
						if (( r = ( *p_spinEnumerationEntryGetEnumValue )( valueHandle,
								&enumValue )) != SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA,
									"%s: Can't get enum value, error %d", __func__, r );
						}
						 */
						if (( r = ( *p_spinEnumerationEntryGetIntValue )( valueHandle,
								&intValue )) != SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA,
									"%s: Can't get int value, error %d", __func__, r );
						}
						switch ( intValue ) {
							case AUTO_SHARPNESS_OFF:
								curr = 0;
								valid = 1;
								break;
							case AUTO_SHARPNESS_CONTINUOUS:
								curr = 1;
								valid = 1;
								break;
							default:
								oaLogWarning ( OA_LOG_CAMERA,
										"%s: Unhandled value '%d' for ANALOGUE_SHARPNESS_AUTO",
										__func__, intValue );
								curr = 0;
						}
					} else {
						oaLogWarning ( OA_LOG_CAMERA,
								"%s: Unrecognised node type '%s' for ANALOGUE_SHARPNESS_AUTO",
								__func__, nodeTypes[ nodeType ] );
					}
					if ( valid ) {
						camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_SHARPNESS ) =
								OA_CTRL_TYPE_BOOLEAN;
						commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_SHARPNESS ) = 0;
						commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_SHARPNESS ) = 1;
						commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_SHARPNESS ) = 1;
						commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_SHARPNESS ) =
							curr ? 1 : 0;
					}
          break;
        }

        default:
          oaLogError ( OA_LOG_CAMERA, "%s: Unhandled analogue feature '%s'",
							__func__, featureName );
          break;
      }
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: Unrecognised analogue feature %d, '%s'",
					__func__, i, featureName );
    }
  }

  return -OA_ERR_NONE;
}


static int
_processDeviceControls ( spinNodeHandle categoryHandle, oaCamera* camera )
{
  spinNodeHandle	featureHandle = 0;
  spinNodeType		nodeType;
  char			featureName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		featureNameLen;
  size_t		numFeatures;
  unsigned int		i, j;
  int			featureId;
  bool8_t		available, readable, writeable;

  if (( *p_spinCategoryGetNumFeatures )( categoryHandle, &numFeatures ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of device features",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    oaLogError ( OA_LOG_CAMERA, "%s: number of device features: %ld", __func__,
				numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get device feature handle",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = writeable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get device feature available",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get device feature readable",
						__func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get device feature writeable",
						__func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable device feature nodes
      // oaLogError ( OA_LOG_CAMERA, "%s: unavailable Spinnaker device feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      oaLogError ( OA_LOG_CAMERA, "%s: inaccessible device feature %d",
					__func__, i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get device feature node type",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get device feature %d name",
					__func__, i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    oaLogInfo ( OA_LOG_CAMERA,
				"%s: device feature %d '%s', type %s [%s] found", __func__, i,
				featureName, ( nodeType >= 0 ) ? nodeTypes [ nodeType ] : "unknown",
				readable ? ( writeable ? "RW" : "RO" ) :
        ( writeable ? "WO" : "??" ));

    // It's not clear if features are always numbered in the same order for
    // all cameras, but the fact that feature numbers are skipped suggests
    // that might be so

    switch ( nodeType ) {
      case IntegerNode:
        _showIntegerNode ( featureHandle, writeable );
        break;
      case BooleanNode:
        _showBooleanNode ( featureHandle );
        break;
      case FloatNode:
        _showFloatNode ( featureHandle, writeable );
        break;
      case CommandNode:
        oaLogError ( OA_LOG_CAMERA, "%s:   [command]", __func__ );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        oaLogError ( OA_LOG_CAMERA, "%s:   unhandled node type", __func__ );
        break;
    }

    // It's not clear if features are always numbered in the same order for
    // all cameras, but the fact that feature numbers are skipped suggests
    // that might be so.  In case it isn't, do things the hard way :(

    for ( j = 0, featureId = -1; j < DEVICE_MAX_FEATURES && featureId < 0;
        j++ ) {
      if ( !strcmp ( featureName, deviceFeatures[ j ] )) {
        featureId = j;
      }
    }

    if ( featureId >= 0 ) {
      switch ( featureId ) {
				case DEVICE_INDICATOR_MODE:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_INDICATOR_MODE feature", __func__ );
          break;
				case DEVICE_VENDOR_NAME:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_VENDOR_NAME feature", __func__ );
          break;
				case DEVICE_SENSOR_DESC:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_SENSOR_DESC feature", __func__ );
          break;
				case DEVICE_MODEL_NAME:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_MODEL_NAME feature", __func__ );
          break;
				case DEVICE_VERSION:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_VERSION feature", __func__ );
          break;
				case DEVICE_SVN_VERSION:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_SVN_VERSION feature", __func__ );
          break;
				case DEVICE_FW_VERSION:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_FW_VERSION feature", __func__ );
          break;
				case DEVICE_ID:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_ID feature", __func__ );
          break;
				case DEVICE_SERIAL_NO:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_SERIAL_NO feature", __func__ );
          break;
				case DEVICE_USER_ID:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_USER_ID feature", __func__ );
          break;
				case DEVICE_SCAN_TYPE:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_SCAN_TYPE feature", __func__ );
          break;

				case DEVICE_TEMPERATURE:
				  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) =
							OA_CTRL_TYPE_READONLY;
					break;

				case DEVICE_RESET:
					camera->features.flags |= OA_CAM_FEATURE_RESET;
					break;

				case DEVICE_UPTIME:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_UPTIME feature", __func__ );
          break;
				case DEVICE_AUTO_FUNC_AOIS_CONTROL:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_AOIS_CONTROL feature", __func__ );
          break;
				case DEVICE_POWER_SUPPLY_SELECTOR:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_SUPPLY_SELECTOR feature", __func__ );
          break;
				case DEVICE_POWER_SUPPLY_VOLTAGE:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_SUPPLY_VOLTAGE feature", __func__ );
          break;
				case DEVICE_POWER_SUPPLY_CURRENT:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_SUPPLY_CURRENT feature", __func__ );
          break;
				case DEVICE_MAX_THROUGHPUT:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_MAX_THROUGHPUT feature", __func__ );
          break;
				case DEVICE_LINK_THROUGHPUT_LIMIT:
          // Ignore this for the time being.
					oaLogInfo ( OA_LOG_CAMERA,
							"%s: Ignoring DEVICE_LINK_THROUGHPUT_LIMIT feature", __func__ );
          break;

        default:
          oaLogError ( OA_LOG_CAMERA, "%s: Unhandled device feature '%s'",
							__func__, featureName );
          break;
      }
    } else {
      oaLogError ( OA_LOG_CAMERA, "%s: Unrecognised device feature %d, '%s'",
					__func__, i, featureName );
		}
  }

  return -OA_ERR_NONE;
}


static int
_processAquisitionControls ( spinNodeHandle categoryHandle, oaCamera* camera )
{
  spinNodeHandle	featureHandle = 0;
  spinNodeType		nodeType;
  char			featureName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		featureNameLen;
  size_t		numFeatures;
  unsigned int		i;
  bool8_t		available, readable, writeable;

  if (( *p_spinCategoryGetNumFeatures )( categoryHandle, &numFeatures ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of aquisition features",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    oaLogWarning ( OA_LOG_CAMERA, "%s: number of aquisition features: %ld",
				__func__, numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get aquisition feature handle",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = writeable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get aquisition feature available",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get aquisition feature readable",
						__func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get aquisition feature writeable", __func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable feature nodes
      // oaLogError ( OA_LOG_CAMERA, "%s: unavailable Spinnaker aquisition feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      oaLogError ( OA_LOG_CAMERA, "%s: inaccessible aquisition feature %d",
					__func__, i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get aquisition feature node type",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get aquisition feature %d name",
					__func__, i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    oaLogInfo ( OA_LOG_CAMERA,
				"%s: acquisition feature %d '%s', type %d [%s] found", __func__, i,
        featureName, nodeType, readable ? ( writeable ? "RW" : "RO" ) :
        ( writeable ? "WO" : "??" ));

    // It's not clear if features are always numbered in the same order for
    // all cameras, but the fact that feature numbers are skipped suggests
    // that might be so

    switch ( nodeType ) {
      case IntegerNode:
        _showIntegerNode ( featureHandle, writeable );
        break;
      case BooleanNode:
        _showBooleanNode ( featureHandle );
        break;
      case FloatNode:
        _showFloatNode ( featureHandle, writeable );
        break;
      case CommandNode:
        oaLogError ( OA_LOG_CAMERA, "%s:   [command]", __func__ );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        oaLogError ( OA_LOG_CAMERA, "%s:   unhandled node type", __func__ );
        break;
    }
  }

  return -OA_ERR_NONE;
}


static int
_processFormatControls ( spinNodeHandle categoryHandle, oaCamera* camera )
{
  spinNodeHandle	featureHandle = 0;
  spinNodeType		nodeType;
  char			featureName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		featureNameLen;
  size_t		numFeatures;
  unsigned int		i;
  bool8_t		available, readable, writeable;

  if (( *p_spinCategoryGetNumFeatures )( categoryHandle, &numFeatures ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of format features",
				__func__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    oaLogWarning ( OA_LOG_CAMERA, "%s: number of format features: %ld",
				__func__, numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get format feature handle",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get format feature available",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get format feature readable",
						__func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get format feature writeable",
						__func__ );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable feature nodes
      // oaLogError ( OA_LOG_CAMERA, "%s: unavailable Spinnaker format feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      oaLogWarning ( OA_LOG_CAMERA, "%s: inaccessible format feature %d",
					__func__, i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get format feature node type",
					__func__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get format feature %d name",
					__func__, i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    oaLogInfo ( OA_LOG_CAMERA, "%s: format feature %d '%s', type %d [%s] found",
				__func__, i, featureName, nodeType,
				readable ? ( writeable ? "RW" : "RO" ) : ( writeable ? "WO" : "??" ));

    // It's not clear if features are always numbered in the same order for
    // all cameras, but the fact that feature numbers are skipped suggests
    // that might be so

    switch ( nodeType ) {
      case IntegerNode:
        _showIntegerNode ( featureHandle, writeable );
        break;
      case BooleanNode:
        _showBooleanNode ( featureHandle );
        break;
      case FloatNode:
        _showFloatNode ( featureHandle, writeable );
        break;
      case CommandNode:
        oaLogWarning ( OA_LOG_CAMERA, "%s:   [command]", __func__ );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        oaLogWarning ( OA_LOG_CAMERA, "%s:   unhandled node type", __func__ );
        break;
    }
  }

  return -OA_ERR_NONE;
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
