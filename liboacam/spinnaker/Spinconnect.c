/*****************************************************************************
 *
 * Spinconnect.c -- Initialise Point Grey Spinnaker-based cameras
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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
  int			ret;
  bool8_t		available, readable;
  int			err;
  unsigned int		i;

  if (( *p_spinCameraInit )( cameraHandle ) != SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't initialise Spinnaker camera\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( err = ( *p_spinCameraGetNodeMap )( cameraHandle,
      &cameraNodeMapHandle )) != SPINNAKER_ERR_SUCCESS ) {
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
  bool8_t		available, readable, writeable;
  unsigned int		i, j;
  int			featureId;
  COMMON_INFO*		commonInfo = camera->_common;

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

    available = readable = writeable = False;
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
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker analogue feature writeable\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable feature nodes
      // fprintf ( stderr, "unavailable Spinnaker analogue feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      fprintf ( stderr, "inaccessible Spinnaker analogue feature %d\n", i );
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

    fprintf ( stderr, "analogue feature %d '%s', type %d [%s] found\n", i,
        featureName, nodeType, readable ? ( writeable ? "RW" : "RO" ) :
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
        fprintf ( stderr, "  [command]\n" );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        fprintf ( stderr, "  unhandled node type\n" );
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
          // Ignore this for the time being.  I have no useful test cases
          // to try it with
          break;

        case ANALOGUE_GAIN_AUTO: // boolean
        {
          bool8_t	curr;

          camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_GAIN ) =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_GAIN ) = 0;
          commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_GAIN ) = 1;
          commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_GAIN ) = 1;
          if (( *p_spinBooleanGetValue )( featureHandle, &curr ) !=
              SPINNAKER_ERR_SUCCESS ) {
            fprintf ( stderr, "Can't get Spinnaker bool current value\n" );
            return -OA_ERR_SYSTEM_ERROR;
          }
          commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAIN ) = curr ? 1 : 0;
          break;
        }

        case ANALOGUE_GAIN: // float
        {
          // Can't get the actual min/max values available if we have auto
          // gain unless auto gain is turned off.  Assume we'll already have
          // seen auto gain before this if it exists

          break;
        }

        default:
          fprintf ( stderr, "Unhandled analogue feature '%s'\n", featureName );
          break;
      }
    } else {
      fprintf ( stderr, "Unrecognised analogue feature %d, '%s'\n", i,
          featureName );
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
  unsigned int		i;
  bool8_t		available, readable, writeable;

  if (( *p_spinCategoryGetNumFeatures )( categoryHandle, &numFeatures ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker number of device features\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    fprintf ( stderr, "number of device features: %ld\n", numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker device feature handle\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = writeable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker device feature available\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker device feature readable\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker device feature writeable\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable device feature nodes
      // fprintf ( stderr, "unavailable Spinnaker device feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      fprintf ( stderr, "inaccessible Spinnaker device feature %d\n", i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker device feature node type\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker device feature %d name\n", i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    fprintf ( stderr, "device feature %d '%s', type %d [%s] found\n", i,
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
        fprintf ( stderr, "  [command]\n" );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        fprintf ( stderr, "  unhandled node type\n" );
        break;
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
    fprintf ( stderr, "Can't get Spinnaker number of aquisition features\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    fprintf ( stderr, "number of aquisition features: %ld\n", numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker aquisition feature handle\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = writeable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker aquisition feature available\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker aquisition feature readable\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker aquisition feature writeable\n"
            );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable feature nodes
      // fprintf ( stderr, "unavailable Spinnaker aquisition feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      fprintf ( stderr, "inaccessible Spinnaker aquisition feature %d\n", i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker aquisition feature node type\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker aquisition feature %d name\n", i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    fprintf ( stderr, "acquisition feature %d '%s', type %d [%s] found\n", i,
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
        fprintf ( stderr, "  [command]\n" );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        fprintf ( stderr, "  unhandled node type\n" );
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
    fprintf ( stderr, "Can't get Spinnaker number of format features\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( numFeatures < 1 ) {
    fprintf ( stderr, "number of format features: %ld\n", numFeatures );
    return OA_ERR_NONE;
  }

  for ( i = 0; i < numFeatures; i++ ) {
    if (( *p_spinCategoryGetFeatureByIndex )( categoryHandle, i,
        &featureHandle ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker format feature handle\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    available = readable = False;
    if (( *p_spinNodeIsAvailable )( featureHandle, &available ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker format feature available\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }
    if ( available ) {
      if (( *p_spinNodeIsReadable )( featureHandle, &readable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker format feature readable\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if (( *p_spinNodeIsWritable )( featureHandle, &writeable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker format feature writeable\n" );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      // No real benefit in showing this.  It seems to be normal behaviour
      // to have unavailable feature nodes
      // fprintf ( stderr, "unavailable Spinnaker format feature %d\n", i );
      continue;
    }
    if ( !readable && !writeable ) {
      fprintf ( stderr, "inaccessible Spinnaker format feature %d\n", i );
      continue;
    }

    if (( *p_spinNodeGetType )( featureHandle, &nodeType ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker format feature node type\n" );
      return -OA_ERR_SYSTEM_ERROR;
    }

    featureNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( featureHandle, featureName,
        &featureNameLen ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker format feature %d name\n", i );
      ( void ) strcpy ( featureName, "unknown" );
    }

    fprintf ( stderr, "format feature %d '%s', type %d [%s] found\n", i,
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
        fprintf ( stderr, "  [command]\n" );
        break;
      case StringNode:
        _showStringNode ( featureHandle );
        break;
      case EnumerationNode:
        _showEnumerationNode ( featureHandle );
        break;
      default:
        fprintf ( stderr, "  unhandled node type\n" );
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
    fprintf ( stderr, "Can't get Spinnaker int current value\n" );
    return;
  }
  if ( writeable ) {
    if (( *p_spinIntegerGetMin )( intNode, &min ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker int min value\n" );
      return;
    }
    if (( *p_spinIntegerGetMax )( intNode, &max ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker int max value\n" );
      return;
    }
    if (( *p_spinIntegerGetInc )( intNode, &step ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker int inc value\n" );
      return;
    }

    fprintf ( stderr, "  [%ld:%ld]/[%ld] := %ld\n", min, max, step, curr );

  } else {
    fprintf ( stderr, "  [readonly] := %ld\n", curr );
  }

  return;
}


static void
_showBooleanNode ( spinNodeHandle boolNode )
{
  bool8_t	curr;

  if (( *p_spinBooleanGetValue )( boolNode, &curr ) != SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker bool current value\n" );
    return;
  }

  fprintf ( stderr, "  [boolean] := %s\n", curr ? "true" : "false" );
  return;
}


static void
_showFloatNode ( spinNodeHandle floatNode, bool8_t writeable )
{
  double       min, max, curr;

  if (( *p_spinFloatGetValue )( floatNode, &curr ) != SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker float current value\n" );
    return;
  }

  if ( writeable ) {
    if (( *p_spinFloatGetMin )( floatNode, &min ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker float min value\n" );
      return;
    }
    if (( *p_spinFloatGetMax )( floatNode, &max ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker float max value\n" );
      return;
    }

    fprintf ( stderr, "  [%f:%f] := %f\n", min, max, curr );

  } else {
    fprintf ( stderr, "  [readonly] := %f\n", curr );
  }
  
  return;
}


static void
_showStringNode ( spinNodeHandle stringNode )
{
  char                  string[ SPINNAKER_MAX_BUFF_LEN ];
  size_t                stringLen = SPINNAKER_MAX_BUFF_LEN;

  if (( *p_spinNodeToString )( stringNode, string, &stringLen )
      != SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker string value\n" );
    return;
  }

  fprintf ( stderr, "  [%s]\n", string );
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

  fprintf ( stderr, "  " );
  if (( *p_spinEnumerationGetNumEntries )( enumNode, &numEntries ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker number of enum node entries\n" );
    return;
  }

  for ( i = 0; i < numEntries; i++ ) {
    if (( *p_spinEnumerationGetEntryByIndex )( enumNode, i, &entryHandle )
        != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker enum handle\n" );
      return;
    }

    entryNameLen = SPINNAKER_MAX_BUFF_LEN;
    if (( *p_spinNodeGetDisplayName )( entryHandle, entryName, &entryNameLen )
        != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker enum name\n" );
      return;
    }

    fprintf ( stderr, "[%s] ", entryName );
  }

  if (( *p_spinEnumerationGetCurrentEntry )( enumNode, &currentHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "\nCan't get Spinnaker enum current value\n" );
    return;
  }
  valueLen = SPINNAKER_MAX_BUFF_LEN;
  if (( *p_spinNodeToString )( currentHandle, value, &valueLen ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "\nCan't get Spinnaker enum value as string\n" );
    return;
  }
  fprintf ( stderr, ":= %s\n", value );
  return;
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
