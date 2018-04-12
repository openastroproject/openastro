/*****************************************************************************
 *
 * Spinconnect.c -- Initialise Point Grey Spinnaker-based cameras
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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

#include <spinnaker/spinc/SpinnakerC.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/demosaic.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "Spinoacam.h"
#include "Spinstate.h"


static void	_spinInitFunctionPointers ( oaCamera* );
static int	_processCameraEntry ( spinCamera, oaCamera* );
static int	_processAnalogueControls ( spinNodeHandle, oaCamera* );
static int	_processDeviceControls ( spinNodeHandle, oaCamera* );
static int	_processAquisitionControls ( spinNodeHandle, oaCamera* );
static int	_processFormatControls ( spinNodeHandle, oaCamera* );

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


  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }

  if (!( cameraInfo = ( SPINNAKER_STATE* ) malloc ( sizeof (
      SPINNAKER_STATE )))) {
    free (( void* ) camera );
    perror ( "malloc FC2_STATE failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }
  OA_CLEAR ( *camera );
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  _oaInitCameraFunctionPointers ( camera );
  _spinInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;

  if (( *p_spinSystemGetInstance )( &systemHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker system instance\n" );
    return 0;
  }

  if (( *p_spinInterfaceListCreateEmpty )( &ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't create empty Spinnaker interface list\n" );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (( *p_spinSystemGetInterfaces )( systemHandle, ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker interfaces\n" );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (( *p_spinInterfaceListGetSize )( ifaceListHandle, &numInterfaces ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get size of Spinnaker interface list\n" );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if ( !numInterfaces ) {
    fprintf ( stderr, "No Spinnaker interfaces found\n" );
    ( void ) ( *p_spinInterfaceListClear  )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy  )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance  )( systemHandle );
    return 0;
  }

  if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't create empty Spinnaker camera list\n" );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (( *p_spinSystemGetCameras )( systemHandle, cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker camera list\n" );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get size of Spinnaker camera list\n" );
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
      fprintf ( stderr, "Can't get Spinnaker interface from list\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }

    if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't create empty Spinnaker camera list\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }

    if (( *p_spinInterfaceGetCameras )( ifaceHandle, cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker interface camera list\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }

    if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker interface camera count\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }

    if ( numCameras ) {
      for ( j = 0; j < numCameras && !found; j++ ) {
        if (( *p_spinCameraListGet )( cameraListHandle, j, &cameraHandle ) !=
            SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr, "Can't get Spinnaker interface camera\n" );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return 0;
        }

        if (( *p_spinCameraGetTLDeviceNodeMap )( cameraHandle,
            &cameraNodeMapHandle ) != SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr, "Can't get Spinnaker camera node map\n" );
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
          fprintf ( stderr, "Can't get Spinnaker camera id node\n" );
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
          fprintf ( stderr,
              "Can't get Spinnaker camera id availability\n" );
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
            fprintf ( stderr, "Can't get Spinnaker camera id readability\n" );
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
              fprintf ( stderr, "Can't get Spinnaker camera id string\n" );
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
fprintf ( stderr, "matched camera entry\n" );
              if ( _processCameraEntry ( cameraHandle, camera ) < 0 ) {
                fprintf ( stderr, "Failed to process camera nodemap\n" );
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
            fprintf ( stderr, "Can't release Spinnaker camera\n" );
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
        fprintf ( stderr, "Can't release Spinnaker camera list\n" );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return 0;
      }

      if (( *p_spinCameraListDestroy )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't destroy Spinnaker camera list\n" );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return 0;
      }
    } else {
      ( void ) ( *p_spinCameraListClear )( cameraListHandle );
      ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
      fprintf ( stderr, "Interface %d has no cameras\n", i );
    }

    if (( *p_spinInterfaceRelease )( ifaceHandle ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't release Spinnaker interface\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return 0;
    }
  }

  ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
  ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );

  if ( !found ) {
    fprintf ( stderr, "Can't find Spinnaker camera\n" );
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
  int			i, ret;
  bool8_t		available, readable;
  int			err;

fprintf ( stderr, "processing camera entry\n" );
  if (( *p_spinCameraInit )( cameraHandle ) != SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't initialise Spinnaker camera\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( err = ( *p_spinCameraGetNodeMap )( cameraHandle, &cameraNodeMapHandle )) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker camera nodemap: err %d\n", err );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "Root", &rootHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker camera root nodemap\n" );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinCategoryGetNumFeatures )( rootHandle, &numCategories ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker number of root categories\n" );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  for ( i = 0; i < numCategories; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( rootHandle, i, &categoryHandle )
        != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker category handle\n" );
    ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = False;
    if (( *p_spinNodeIsAvailable )( categoryHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker category available\n" );
      ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( categoryHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker category readable\n" );
        ( void ) ( *p_spinCameraDeInit )( cameraHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      fprintf ( stderr, "unavailable Spinnaker category\n" );
      continue;
    }
    if ( !readable ) {
      fprintf ( stderr, "unreadable Spinnaker category\n" );
      continue;
    }

    if (( *p_spinNodeGetType )( categoryHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker category node type\n" );
      ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    categoryNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( categoryHandle, categoryName,
        &categoryNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker category name\n" );
      ( void ) ( *p_spinCameraDeInit )( cameraHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( nodeType == CategoryNode ) {
      if ( !strcmp ( "Analog Control", categoryName )) {
        if (( ret = _processAnalogueControls ( categoryHandle, camera )) < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
      }
      if ( !strcmp ( "Device Control", categoryName )) {
        if (( ret = _processDeviceControls ( categoryHandle, camera )) < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
      }
      if ( !strcmp ( "Acquisition Control", categoryName )) {
        if (( ret = _processAquisitionControls ( categoryHandle, camera ))
            < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
      }
      if ( !strcmp ( "Image Format Control", categoryName )) {
        if (( ret = _processFormatControls ( categoryHandle, camera )) < 0 ) {
          ( void ) ( *p_spinCameraDeInit )( cameraHandle );
          return ret;
        }
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
    } else {
      fprintf ( stderr, "Unhandled Spinnaker camera node '%s', type %d\n",
          categoryName, nodeType );
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
  int			i, ret;
  bool8_t		available, readable;

  if (( *p_spinCategoryGetNumFeatures )( categoryHandle, &numFeatures ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker number of analogue features\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    fprintf ( stderr, "number of analogue features: %ld\n", numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker analogue feature handle\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker analogue feature available\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker analogue feature readable\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable feature nodes
      // fprintf ( stderr, "unavailable Spinnaker analogue feature %d\n", i );
      continue;
    }
    if ( !readable ) {
      fprintf ( stderr, "unreadable Spinnaker analogue feature %d\n", i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker analogue feature node type\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker analogue feature %d name\n", i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    fprintf ( stderr, "feature '%s', type %d found\n", featureName, nodeType );
  }

  return -OA_ERR_NONE;
}


static int
_processDeviceControls ( spinNodeHandle featureHandle, oaCamera* camera )
{
  return -OA_ERR_NONE;
}


static int
_processAquisitionControls ( spinNodeHandle featureHandle, oaCamera* camera )
{
  return -OA_ERR_NONE;
}


static int
_processFormatControls ( spinNodeHandle featureHandle, oaCamera* camera )
{
  return -OA_ERR_NONE;
}


static void
_spinInitFunctionPointers ( oaCamera* camera )
{
/*
  camera->funcs.initCamera = oaSpinInitCamera;
  camera->funcs.closeCamera = oaSpinCloseCamera;

  camera->funcs.setControl = oaSpinCameraSetControl;
  camera->funcs.readControl = oaSpinCameraReadControl;
  camera->funcs.testControl = oaSpinCameraTestControl;
  camera->funcs.getControlRange = oaSpinCameraGetControlRange;
  camera->funcs.getControlDiscreteSet = oaSpinCameraGetControlDiscreteSet;

  camera->funcs.startStreaming = oaSpinCameraStartStreaming;
  camera->funcs.stopStreaming = oaSpinCameraStopStreaming;
  camera->funcs.isStreaming = oaSpinCameraIsStreaming;

  camera->funcs.setResolution = oaSpinCameraSetResolution;
  camera->funcs.setROI = oaSpinCameraSetROI;
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
