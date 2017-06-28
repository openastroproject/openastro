/*****************************************************************************
 *
 * PGEoacam.c -- main entrypoint for Point Grey Gig-E Cameras
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif
#include <openastro/camera.h>
#include <flycapture/C/FlyCapture2_C.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "PGEoacam.h"

// Pointers to libflycapture functions so we can use them via libdl.

fc2Error              ( *p_fc2Connect )( fc2Context, fc2PGRGuid* );
fc2Error              ( *p_fc2CreateGigEContext )( fc2Context* );
fc2Error              ( *p_fc2DestroyContext )( fc2Context );
fc2Error              ( *p_fc2DiscoverGigECameras )( fc2Context, fc2CameraInfo*,
                          unsigned int* );
fc2Error              ( *p_fc2GetCameraFromIndex )( fc2Context, unsigned int,
                          fc2PGRGuid* );
fc2Error              ( *p_fc2GetCameraFromIPAddress )( fc2Context,
                          fc2IPAddress, fc2PGRGuid* );
fc2Error              ( *p_fc2GetCameraInfo )( fc2Context, fc2CameraInfo* );
fc2Error              ( *p_fc2GetGigEImageBinningSettings )( fc2Context,
                          unsigned int*, unsigned int* );
fc2Error              ( *p_fc2GetGigEImageSettings )( fc2Context,
                          fc2GigEImageSettings* );
fc2Error              ( *p_fc2GetGigEImageSettingsInfo )( fc2Context,
                          fc2GigEImageSettingsInfo* );
fc2Error              ( *p_fc2GetInterfaceTypeFromGuid )( fc2Context,
                          fc2PGRGuid*, fc2InterfaceType* );
fc2Error              ( *p_fc2GetNumOfCameras )( fc2Context, unsigned int* );
fc2Error              ( *p_fc2GetProperty )( fc2Context, fc2Property* );
fc2Error              ( *p_fc2GetPropertyInfo )( fc2Context, fc2PropertyInfo* );
fc2Error              ( *p_fc2GetStrobe )( fc2Context, fc2StrobeControl* );
fc2Error              ( *p_fc2GetStrobeInfo )( fc2Context, fc2StrobeInfo* );
fc2Error              ( *p_fc2GetTriggerDelay )( fc2Context, fc2TriggerDelay* );
fc2Error              ( *p_fc2GetTriggerDelayInfo )( fc2Context,
                          fc2TriggerDelayInfo* );
fc2Error              ( *p_fc2GetTriggerMode )( fc2Context, fc2TriggerMode* );
fc2Error              ( *p_fc2GetTriggerModeInfo )( fc2Context,
                          fc2TriggerModeInfo* );
fc2Error              ( *p_fc2QueryGigEImagingMode )( fc2Context, fc2Mode,
                          BOOL* );
fc2Error              ( *p_fc2ReadRegister )( fc2Context, unsigned int,
                          unsigned int* );
fc2Error              ( *p_fc2SetGigEImageBinningSettings )( fc2Context,
                          unsigned int, unsigned int );
fc2Error              ( *p_fc2SetGigEImageSettings )( fc2Context,
                          const fc2GigEImageSettings* );
fc2Error              ( *p_fc2SetGigEImagingMode )( fc2Context, fc2Mode );
fc2Error              ( *p_fc2SetProperty )( fc2Context, fc2Property* );
fc2Error              ( *p_fc2SetStrobe )( fc2Context, fc2StrobeControl* );
fc2Error              ( *p_fc2SetTriggerDelay )( fc2Context, fc2TriggerDelay* );
fc2Error              ( *p_fc2SetTriggerMode )( fc2Context, fc2TriggerMode* );
fc2Error              ( *p_fc2StartCaptureCallback )( fc2Context,
                          fc2ImageEventCallback, void* );
fc2Error              ( *p_fc2StopCapture )( fc2Context );

#if HAVE_LIBDL
static void*		_getDLSym ( void*, const char* );
#endif

/**
 * Cycle through the list of cameras returned by the Flycapture library
 */

int
oaPGEGetCameras ( CAMERA_LIST* deviceList, int flags )
{
  fc2Context		pgeContext;
  fc2CameraInfo*	devList;
  fc2PGRGuid		guid;
  fc2InterfaceType	interfaceType;
  unsigned int		numCameras, slotsAvailable;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int                   numFound, ret;

#if HAVE_LIBDL
  static void*		libHandle = 0;

  if ( !libHandle ) {
    if (!( libHandle = dlopen( "/usr/lib/libflycapture-c.so.2", RTLD_LAZY ))) {
      return 0;
    }
  }

  dlerror();

  if (!( *( void** )( &p_fc2Connect ) = _getDLSym ( libHandle,
      "fc2Connect" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2CreateGigEContext ) = _getDLSym ( libHandle,
      "fc2CreateGigEContext" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2DestroyContext ) = _getDLSym ( libHandle,
      "fc2DestroyContext" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2DiscoverGigECameras ) = _getDLSym ( libHandle,
      "fc2DiscoverGigECameras" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetCameraFromIndex ) = _getDLSym ( libHandle,
      "fc2GetCameraFromIndex" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetCameraFromIPAddress ) = _getDLSym ( libHandle,
      "fc2GetCameraFromIPAddress" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetCameraInfo ) = _getDLSym ( libHandle,
      "fc2GetCameraInfo" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetGigEImageBinningSettings ) =
      _getDLSym ( libHandle, "fc2GetGigEImageBinningSettings" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetGigEImageSettings ) = _getDLSym ( libHandle,
      "fc2GetGigEImageSettings" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetGigEImageSettingsInfo ) = _getDLSym ( libHandle,
      "fc2GetGigEImageSettingsInfo" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetInterfaceTypeFromGuid ) = _getDLSym ( libHandle,
      "fc2GetInterfaceTypeFromGuid" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetNumOfCameras ) = _getDLSym ( libHandle,
      "fc2GetNumOfCameras" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetProperty ) = _getDLSym ( libHandle,
      "fc2GetProperty" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetPropertyInfo ) = _getDLSym ( libHandle,
      "fc2GetPropertyInfo" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetStrobe ) = _getDLSym ( libHandle,
      "fc2GetStrobe" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetStrobeInfo ) = _getDLSym ( libHandle,
      "fc2GetStrobeInfo" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetTriggerDelay ) = _getDLSym ( libHandle,
      "fc2GetTriggerDelay" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetTriggerDelayInfo ) = _getDLSym ( libHandle,
      "fc2GetTriggerDelayInfo" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetTriggerMode ) = _getDLSym ( libHandle,
      "fc2GetTriggerMode" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2GetTriggerModeInfo ) = _getDLSym ( libHandle,
      "fc2GetTriggerModeInfo" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2QueryGigEImagingMode ) = _getDLSym ( libHandle,
      "fc2QueryGigEImagingMode" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2ReadRegister ) = _getDLSym ( libHandle,
      "fc2ReadRegister" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2SetGigEImageBinningSettings ) =
      _getDLSym ( libHandle, "fc2SetGigEImageBinningSettings" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2SetGigEImageSettings ) = _getDLSym ( libHandle,
      "fc2SetGigEImageSettings" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2SetGigEImagingMode ) = _getDLSym ( libHandle,
      "fc2SetGigEImagingMode" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2SetProperty ) = _getDLSym ( libHandle,
      "fc2SetProperty" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2SetStrobe ) = _getDLSym ( libHandle,
      "fc2SetStrobe" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2SetTriggerDelay ) = _getDLSym ( libHandle,
      "fc2SetTriggerDelay" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2SetTriggerMode ) = _getDLSym ( libHandle,
      "fc2SetTriggerMode" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2StartCaptureCallback ) = _getDLSym ( libHandle,
      "fc2StartCaptureCallback" ))) {
    return 0;
  }
  if (!( *( void** )( &p_fc2StopCapture ) = _getDLSym ( libHandle,
      "fc2StopCapture" ))) {
    return 0;
  }

#else /* HAVE_LIBDL */

  p_fc2Connect = fc2Connect;
  p_fc2CreateGigEContext = fc2CreateGigEContext;
  p_fc2DestroyContext = fc2DestroyContext;
  p_fc2DiscoverGigECameras = fc2DiscoverGigECameras;
  p_fc2GetCameraFromIndex = fc2GetCameraFromIndex;
  p_fc2GetCameraFromIPAddress = fc2GetCameraFromIPAddress;
  p_fc2GetCameraInfo = fc2GetCameraInfo;
  p_fc2GetGigEImageBinningSettings = fc2GetGigEImageBinningSettings;
  p_fc2GetGigEImageSettings = fc2GetGigEImageSettings;
  p_fc2GetGigEImageSettingsInfo = fc2GetGigEImageSettingsInfo;
  p_fc2GetInterfaceTypeFromGuid = fc2GetInterfaceTypeFromGuid;
  p_fc2GetNumOfCameras = fc2GetNumOfCameras;
  p_fc2GetProperty = fc2GetProperty;
  p_fc2GetPropertyInfo = fc2GetPropertyInfo;
  p_fc2GetStrobe = fc2GetStrobe;
  p_fc2GetStrobeInfo = fc2GetStrobeInfo;
  p_fc2GetTriggerDelay = fc2GetTriggerDelay;
  p_fc2GetTriggerDelayInfo = fc2GetTriggerDelayInfo;
  p_fc2GetTriggerMode = fc2GetTriggerMode;
  p_fc2GetTriggerModeInfo = fc2GetTriggerModeInfo;
  p_fc2QueryGigEImagingMode = fc2QueryGigEImagingMode;
  p_fc2ReadRegister = fc2ReadRegister;
  p_fc2SetGigEImageBinningSettings = fc2SetGigEImageBinningSettings;
  p_fc2SetGigEImageSettings = fc2SetGigEImageSettings;
  p_fc2SetGigEImagingMode = fc2SetGigEImagingMode;
  p_fc2SetProperty = fc2SetProperty;
  p_fc2SetStrobe = fc2SetStrobe;
  p_fc2SetTriggerDelay = fc2SetTriggerDelay;
  p_fc2SetTriggerMode = fc2SetTriggerMode;
  p_fc2StartCaptureCallback = fc2StartCaptureCallback;
  p_fc2StopCapture = fc2StopCapture;

#endif /* HAVE_LIBDL */

  if (( *p_fc2CreateGigEContext )( &pgeContext ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get PGE context\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  slotsAvailable = 0;
  devList = 0;
  do {
    slotsAvailable += 4;
    if (!( devList = realloc ( devList, sizeof ( fc2CameraInfo ) *
        slotsAvailable ))) {
      return -OA_ERR_MEM_ALLOC;
    }
    numCameras = slotsAvailable;
    ret = ( *p_fc2DiscoverGigECameras )( pgeContext, devList, &numCameras );
    if ( ret != FC2_ERROR_OK && ret != FC2_ERROR_BUFFER_TOO_SMALL ) {
      ( *p_fc2DestroyContext )( pgeContext );
      fprintf ( stderr, "Can't enumerate PGE devices\n" );
      free (( void* ) devList );
      return -OA_ERR_SYSTEM_ERROR;
    }
  } while ( ret != FC2_ERROR_OK );

  if ( !numCameras ) {
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) devList );
    return 0;
  }

/*
  if (( * p_fc2GetNumOfCameras )( pgeContext, &numCameras ) != FC2_ERROR_OK ) {
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) devList );
    fprintf ( stderr, "Error fetching number of cameras\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( !numCameras ) {
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) devList );
    return 0;
  }
*/

  numFound = 0;
  for ( i = 0; i < numCameras; i++ ) {
    // if (( *p_fc2GetCameraFromIndex )( pgeContext, i, &guid ) != FC2_ERROR_OK ) {
    if (( *p_fc2GetCameraFromIPAddress )( pgeContext, devList[i].ipAddress,
        &guid ) != FC2_ERROR_OK ) {
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) devList );
      fprintf ( stderr, "Error fetching details for camera %d\n", i );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_fc2GetInterfaceTypeFromGuid )( pgeContext, &guid,
        &interfaceType ) != FC2_ERROR_OK ) {
      ( *p_fc2DestroyContext )( pgeContext );
      fprintf ( stderr, "Error getting interface type for camera %d\n", i );
      free (( void* ) devList );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( interfaceType != FC2_INTERFACE_GIGE ) {
      continue;
    }
    /*
    fprintf(
        stderr,
        "GigE major version - %u\n"
        "GigE minor version - %u\n"
        "User-defined name - %s\n"
        "Model name - %s\n"
        "XML URL1 - %s\n"
        "XML URL2 - %s\n"
        "Firmware version - %s\n"
        "IIDC version - %1.2f\n"
        "MAC address - %02X:%02X:%02X:%02X:%02X:%02X\n"
        "IP address - %u.%u.%u.%u\n"
        "Subnet mask - %u.%u.%u.%u\n"
        "Default gateway - %u.%u.%u.%u\n\n",
        devList[i].gigEMajorVersion,
        devList[i].gigEMinorVersion,
        devList[i].userDefinedName,
        devList[i].modelName,
        devList[i].xmlURL1,
        devList[i].xmlURL2,
        devList[i].firmwareVersion,
        devList[i].iidcVer / 100.0f,
        devList[i].macAddress.octets[0],
        devList[i].macAddress.octets[1],
        devList[i].macAddress.octets[2],
        devList[i].macAddress.octets[3],
        devList[i].macAddress.octets[4],
        devList[i].macAddress.octets[5],
        devList[i].ipAddress.octets[0],
        devList[i].ipAddress.octets[1],
        devList[i].ipAddress.octets[2],
        devList[i].ipAddress.octets[3],
        devList[i].subnetMask.octets[0],
        devList[i].subnetMask.octets[1],
        devList[i].subnetMask.octets[2],
        devList[i].subnetMask.octets[3],
        devList[i].defaultGateway.octets[0],
        devList[i].defaultGateway.octets[1],
        devList[i].defaultGateway.octets[2],
        devList[i].defaultGateway.octets[3]);
     */

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
      _oaFreeCameraDeviceList ( deviceList );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) devList );
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      free (( void* ) dev );
      _oaFreeCameraDeviceList ( deviceList );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) devList );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_PGE;
    ( void ) snprintf ( dev->deviceName, OA_MAX_NAME_LEN+1,
        "%s (%d.%d.%d.%d)", devList[i].modelName,
        devList[i].ipAddress.octets[0], devList[i].ipAddress.octets[1],
        devList[i].ipAddress.octets[2], devList[i].ipAddress.octets[3] );
    memcpy (( void* ) &_private->pgeGuid, ( void* ) &guid, sizeof ( guid ));
    dev->_private = _private;
    dev->initCamera = oaPGEInitCamera;
    dev->hasLoadableFirmware = 0;
    if ((( _private->colour = devList[i].isColorCamera ) ? 1 : 0 )) {
      switch ( devList[i].bayerTileFormat ) {
        case FC2_BT_RGGB:
          _private->cfaPattern = OA_PIX_FMT_RGGB8;
          break;
        case FC2_BT_GRBG:
          _private->cfaPattern = OA_PIX_FMT_GRBG8;
          break;
        case FC2_BT_GBRG:
          _private->cfaPattern = OA_PIX_FMT_GBRG8;
          break;
        case FC2_BT_BGGR:
          _private->cfaPattern = OA_PIX_FMT_BGGR8;
          break;
        default:
          break;
      }
    }
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      free (( void* ) dev );
      free (( void* ) _private );
      _oaFreeCameraDeviceList ( deviceList );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) devList );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
    numFound++;
  }

  ( *p_fc2DestroyContext )( pgeContext );
  free (( void* ) devList );
  return numFound;
}


#if HAVE_LIBDL
static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libflycapture DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif
