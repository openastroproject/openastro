/*****************************************************************************
 *
 * Spinoacam.c -- main entrypoint for Point Grey Spinnaker interface
 *
 * Copyright 2018,2019,2020,2021
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

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/util.h>
#include <spinc/SpinnakerC.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Spinoacam.h"
#include "Spin.h"


/**
 * Cycle through the list of cameras returned by the Spinnaker library
 */

int
oaSpinGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  spinSystem		systemHandle;
  spinInterfaceList	ifaceListHandle = 0;
  size_t		numInterfaces = 0;
  spinCameraList	cameraListHandle = 0;
  size_t		numCameras = 0;
  spinInterface		ifaceHandle = 0;
  spinNodeMapHandle	ifaceNodeMapHandle = 0;
  spinNodeHandle	ifaceNameHandle = 0;
  bool8_t		ifaceNameAvailable = False;
  bool8_t		ifaceNameReadable = False;
  char			ifaceName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		ifaceNameLen;
  spinCamera		cameraHandle;
  spinNodeMapHandle	cameraNodeMapHandle = 0;
  spinNodeHandle	vendorNameHandle = 0;
  bool8_t		vendorNameAvailable = False;
  bool8_t		vendorNameReadable = False;
  char			vendorName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		vendorNameLen;
  spinNodeHandle	modelNameHandle = 0;
  bool8_t		modelNameAvailable = False;
  bool8_t		modelNameReadable = False;
  char			modelName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		modelNameLen;
  spinNodeHandle	deviceIdHandle = 0;
  bool8_t		deviceIdAvailable = False;
  bool8_t		deviceIdReadable = False;
  char			deviceId[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		deviceIdLen;
  spinNodeHandle        deviceTypeHandle = 0;
  spinNodeHandle        currentEntryHandle = 0;
  bool8_t               deviceTypeAvailable = False;
  bool8_t               deviceTypeReadable = False;
  // size_t		deviceType;
  uint64_t		deviceType;
  spinNodeHandle        ipAddrHandle = 0;
  bool8_t               ipAddrAvailable = False;
  bool8_t               ipAddrReadable = False;
  uint64_t		ipAddr;
  unsigned int		i, j, numFound;
  oaCameraDevice*       devices;
  DEVICE_INFO*		_private;
  int			ret;
	spinError		err;


	if (( ret = _spinInitLibraryFunctionPointers()) != OA_ERR_NONE ) {
		return ret;
	}

  if (( err = ( *p_spinSystemGetInstance )( &systemHandle )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get system instance, error %d", __func__, err );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( err = ( *p_spinInterfaceListCreateEmpty )( &ifaceListHandle )) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: Can't create empty interface list, error %d", __func__, err );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinSystemGetInterfaces )( systemHandle, ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get interfaces", __func__ );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinInterfaceListGetSize )( ifaceListHandle, &numInterfaces ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get size of interface list",
				__func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  oaLogInfo ( OA_LOG_CAMERA, "%s: %ld interfaces found", __func__,
			numInterfaces );

  if ( !numInterfaces ) {
    oaLogWarning ( OA_LOG_CAMERA, "%s: No Spinnaker interfaces found",
				__func__ );
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
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinSystemGetCameras )( systemHandle, cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera list", __func__ );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
      SPINNAKER_ERR_SUCCESS ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't get size of camera list", __func__ );
    ( void ) ( *p_spinCameraListClear )( cameraListHandle );
    ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  ( void ) ( *p_spinCameraListClear )( cameraListHandle );
  ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
  cameraListHandle = 0;

  oaLogInfo ( OA_LOG_CAMERA, "%s: %ld cameras found", __func__, numCameras );

  if ( !numCameras ) {
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (!( devices = malloc ( sizeof ( oaCameraDevice ) * numCameras ))) {
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_MEM_ALLOC;
  }

  if (!( _private = malloc ( sizeof ( DEVICE_INFO ) * numCameras ))) {
    ( void ) free (( void* ) devices );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_MEM_ALLOC;
  }

  numFound = 0;
  for ( i = 0; i < numInterfaces; i++ ) {
    if (( *p_spinInterfaceListGet )( ifaceListHandle, i, &ifaceHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface from list",
					__func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinInterfaceGetTLNodeMap )( ifaceHandle, &ifaceNodeMapHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get TL node map", __func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinNodeMapGetNode )( ifaceNodeMapHandle, "InterfaceDisplayName",
        &ifaceNameHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get TL map node", __func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinNodeIsAvailable )( ifaceNameHandle, &ifaceNameAvailable ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface name availability",
					__func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( ifaceNameAvailable ) {
      if (( *p_spinNodeIsReadable )( ifaceNameHandle, &ifaceNameReadable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface name readability",
						__func__ );
        ( void ) free (( void* ) devices );
        ( void ) free (( void* ) _private );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if ( ifaceNameReadable ) {
				ifaceNameLen = SPINNAKER_MAX_BUFF_LEN;
        if (( *p_spinStringGetValue )( ifaceNameHandle, ifaceName,
            &ifaceNameLen ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get string value", __func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }
      } else {
        ( void ) strcpy ( ifaceName, "name unreadable" );
      }
    } else {
      ( void ) strcpy ( ifaceName, "name unavailable" );
    }

    if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't create empty camera list",
					__func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinInterfaceGetCameras )( ifaceHandle, cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera list",
					__func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
        SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera count",
					__func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( numCameras ) {
      for ( j = 0; j < numCameras; j++ ) {
        if (( *p_spinCameraListGet )( cameraListHandle, j, &cameraHandle ) !=
            SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get interface camera",
							__func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinCameraGetTLDeviceNodeMap )( cameraHandle,
            &cameraNodeMapHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera node map",
							__func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "DeviceVendorName",
            &vendorNameHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera name node",
							__func__ );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinNodeIsAvailable )( vendorNameHandle,
            &vendorNameAvailable ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get vendor name availability",
							__func__ );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( vendorNameAvailable ) {
          if (( *p_spinNodeIsReadable )( vendorNameHandle,
              &vendorNameReadable ) != SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA, "%s: Can't get vendor name readability",
								__func__ );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }
          if ( vendorNameReadable ) {
						vendorNameLen = SPINNAKER_MAX_BUFF_LEN;
            if (( *p_spinStringGetValue )( vendorNameHandle, vendorName,
                &vendorNameLen ) != SPINNAKER_ERR_SUCCESS ) {
              oaLogError ( OA_LOG_CAMERA, "%s: Can't get vendor name string",
									__func__ );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }
          } else {
            ( void ) strcpy ( vendorName, "vendor unreadable" );
          }
        } else {
          ( void ) strcpy ( vendorName, "vendor unavailable" );
        }

        if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle,
            "DeviceModelName", &modelNameHandle ) !=
            SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera model node",
							__func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinNodeIsAvailable )( modelNameHandle, &modelNameAvailable )
            != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA,
              "%s: Can't get vendor model availability", __func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( modelNameAvailable ) {
          if (( *p_spinNodeIsReadable )( modelNameHandle, &modelNameReadable )
              != SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA,
								"%s: Can't get vendor model readability", __func__ );
            ( void ) free (( void* ) devices );
            ( void ) free (( void* ) _private );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }

          if ( modelNameReadable ) {
						modelNameLen = SPINNAKER_MAX_BUFF_LEN;
            if (( err = ( *p_spinStringGetValue )( modelNameHandle, modelName,
                &modelNameLen )) != SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA,
									"%s: Can't get model name string, error %d", __func__, err );
              ( void ) free (( void* ) devices );
              ( void ) free (( void* ) _private );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }
          } else {
            ( void ) strcpy ( modelName, "model unreadable" );
          }
        } else {
          ( void ) strcpy ( modelName, "model unavailable" );
        }

        if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "DeviceID",
            &deviceIdHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id node",
							__func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        *deviceId = 0;
        if (( *p_spinNodeIsAvailable )( deviceIdHandle,
            &deviceIdAvailable ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id availability",
							__func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( deviceIdAvailable ) {
          if (( *p_spinNodeIsReadable )( deviceIdHandle,
              &deviceIdReadable ) != SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id readability",
								__func__ );
            ( void ) free (( void* ) devices );
            ( void ) free (( void* ) _private );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }
          if ( deviceIdReadable ) {
						deviceIdLen = SPINNAKER_MAX_BUFF_LEN;
            if (( *p_spinStringGetValue )( deviceIdHandle, deviceId,
                &deviceIdLen ) != SPINNAKER_ERR_SUCCESS ) {
              oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera id string",
									__func__ );
              ( void ) free (( void* ) devices );
              ( void ) free (( void* ) _private );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }
          }
        }

        if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "DeviceType",
            &deviceTypeHandle ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera type node",
							__func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinNodeIsAvailable )( deviceTypeHandle,
            &deviceTypeAvailable ) != SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera type availability",
							__func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( deviceTypeAvailable ) {
          if (( *p_spinNodeIsReadable )( deviceTypeHandle,
              &deviceTypeReadable ) != SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera type readability",
								__func__ );
            ( void ) free (( void* ) devices );
            ( void ) free (( void* ) _private );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }
          if ( deviceTypeReadable ) {
            if (( *p_spinEnumerationGetCurrentEntry )( deviceTypeHandle,
              &currentEntryHandle ) != SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA,
									"%s: Can't get enumeration current value", __func__ );
              ( void ) free (( void* ) devices );
              ( void ) free (( void* ) _private );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }

            // Not entirely sure why this works when ...GetEnumValue doesn't
            // as I'd have thought it's the enum value I want to check in the
            // "if" condition below.  The Enum function throws an error (-1009)
            // though.
            if (( *p_spinEnumerationEntryGetIntValue )( currentEntryHandle,
                &deviceType ) != SPINNAKER_ERR_SUCCESS ) {
              oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera type value",
									__func__ );
              ( void ) free (( void* ) devices );
              ( void ) free (( void* ) _private );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }
          }
        }

        if ( deviceType == DeviceType_GigEVision ) {
          if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle,
             "GevDeviceIPAddress", &ipAddrHandle ) != SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA, "%s: Can't get camera address node",
								__func__ );
            ( void ) free (( void* ) devices );
            ( void ) free (( void* ) _private );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }

          if (( *p_spinNodeIsAvailable )( ipAddrHandle, &ipAddrAvailable )
              != SPINNAKER_ERR_SUCCESS ) {
            oaLogError ( OA_LOG_CAMERA,
								"%s: Can't get camera address availability", __func__ );
            ( void ) free (( void* ) devices );
            ( void ) free (( void* ) _private );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }

          if ( ipAddrAvailable ) {
            if (( *p_spinNodeIsReadable )( ipAddrHandle, &ipAddrReadable ) !=
                SPINNAKER_ERR_SUCCESS ) {
							oaLogError ( OA_LOG_CAMERA,
									"%s: Can't get camera address readability", __func__ );
              ( void ) free (( void* ) devices );
              ( void ) free (( void* ) _private );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }
            if ( ipAddrReadable ) {
              if (( *p_spinIntegerGetValue )( ipAddrHandle, &ipAddr ) !=
                  SPINNAKER_ERR_SUCCESS ) {
								oaLogError ( OA_LOG_CAMERA,
										"%s: Can't get camera address value", __func__ );
                ( void ) free (( void* ) devices );
                ( void ) free (( void* ) _private );
                ( void ) ( *p_spinCameraRelease )( cameraHandle );
                ( void ) ( *p_spinCameraListClear )( cameraListHandle );
                ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
                ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
                ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
                ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
                return -OA_ERR_SYSTEM_ERROR;
              }
            }
          }
        }

        if ( *deviceId ) {
          _oaInitCameraDeviceFunctionPointers ( &devices[ numFound ]);
          devices[ numFound ].interface = OA_CAM_IF_SPINNAKER;
          if ( deviceType == DeviceType_GigEVision ) {
            ( void ) snprintf ( devices[ numFound ].deviceName,
                OA_MAX_NAME_LEN+1, "%.*s (%d.%d.%d.%d)", OA_MAX_NAME_LEN - 20,
								modelName, ( uint8_t )( ipAddr >> 24 ),
								( uint8_t )(( ipAddr >> 16 ) & 0xff ),
								( uint8_t )(( ipAddr >> 8 ) & 0xff ),
                ( uint8_t )( ipAddr & 0xff ));
          } else {
            ( void ) snprintf ( devices[ numFound ].deviceName,
                OA_MAX_NAME_LEN+1, "%.*s", OA_MAX_NAME_LEN, modelName );
          }

          ( void ) strncpy ( _private[ numFound ].deviceId, deviceId,
							OA_MAX_DEVICEID_LEN );
          _private[ numFound ].ipAddress = ipAddr;

          devices[ numFound ]._private = &_private[ numFound ];
          devices[ numFound ].initCamera = oaSpinInitCamera;
          if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
            ( void ) free (( void* ) devices );
            ( void ) free (( void* ) _private );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return ret;
          }
          deviceList->cameraList[ deviceList->numCameras++ ] =
              &devices[ numFound ];
          numFound++;

          oaLogInfo ( OA_LOG_CAMERA, "%s: Interface: %s, Camera: %s",
							__func__, ifaceName, modelName );
        }
 
        if (( *p_spinCameraRelease )( cameraHandle ) !=
            SPINNAKER_ERR_SUCCESS ) {
          oaLogError ( OA_LOG_CAMERA, "%s: Can't release camera", __func__ );
          ( void ) free (( void* ) devices );
          ( void ) free (( void* ) _private );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }
      }
      if (( *p_spinCameraListClear )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) { 
        oaLogError ( OA_LOG_CAMERA, "%s: Can't release camera list", __func__ );
        ( void ) free (( void* ) devices );
        ( void ) free (( void* ) _private );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }

      if (( *p_spinCameraListDestroy )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) {  
        oaLogError ( OA_LOG_CAMERA, "%s: Can't destroy camera list", __func__ );
        ( void ) free (( void* ) devices );
        ( void ) free (( void* ) _private );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      ( void ) ( *p_spinCameraListClear )( cameraListHandle );
      ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
      oaLogInfo ( OA_LOG_CAMERA, "%s: Interface %s has no cameras",
          __func__, ifaceName );
    }

    if (( *p_spinInterfaceRelease )( ifaceHandle ) != SPINNAKER_ERR_SUCCESS ) {
      oaLogError ( OA_LOG_CAMERA, "%s: Can't release interface", __func__ );
      ( void ) free (( void* ) devices );
      ( void ) free (( void* ) _private );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }
  }

  ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
  ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
  ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );

  return numFound;
}
