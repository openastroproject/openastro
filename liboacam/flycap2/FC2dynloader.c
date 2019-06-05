/*****************************************************************************
 *
 * FC2dynloader.c -- dynamic loader for flycapture libraries
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif
#include <openastro/errno.h>
#include <flycapture/C/FlyCapture2_C.h>

#include "FC2private.h"

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
fc2Error				      ( *p_fc2GetImageMetadata )( fc2Image*,
													fc2ImageMetadata* );
fc2Error							( *p_fc2GetEmbeddedImageInfo )( fc2Context,
													fc2EmbeddedImageInfo* );
fc2Error							( *p_fc2SetEmbeddedImageInfo )( fc2Context,
													fc2EmbeddedImageInfo* );

#if HAVE_LIBDL
static void*		_getDLSym ( void*, const char* );
#endif

/**
 * Cycle through the list of cameras returned by the Flycapture library
 */

int
_fc2InitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL
  static void*		libHandle = 0;

  if ( !libHandle ) {
    if (!( libHandle = dlopen( "/usr/lib/libflycapture-c.so.2", RTLD_LAZY ))) {
      return OA_ERR_LIBRARY_NOT_FOUND;
    }
  }

  dlerror();

  if (!( *( void** )( &p_fc2Connect ) = _getDLSym ( libHandle,
      "fc2Connect" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2CreateGigEContext ) = _getDLSym ( libHandle,
      "fc2CreateGigEContext" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2DestroyContext ) = _getDLSym ( libHandle,
      "fc2DestroyContext" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2DiscoverGigECameras ) = _getDLSym ( libHandle,
      "fc2DiscoverGigECameras" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetCameraFromIndex ) = _getDLSym ( libHandle,
      "fc2GetCameraFromIndex" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetCameraFromIPAddress ) = _getDLSym ( libHandle,
      "fc2GetCameraFromIPAddress" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetCameraInfo ) = _getDLSym ( libHandle,
      "fc2GetCameraInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetGigEImageBinningSettings ) =
      _getDLSym ( libHandle, "fc2GetGigEImageBinningSettings" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetGigEImageSettings ) = _getDLSym ( libHandle,
      "fc2GetGigEImageSettings" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetGigEImageSettingsInfo ) = _getDLSym ( libHandle,
      "fc2GetGigEImageSettingsInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetInterfaceTypeFromGuid ) = _getDLSym ( libHandle,
      "fc2GetInterfaceTypeFromGuid" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetNumOfCameras ) = _getDLSym ( libHandle,
      "fc2GetNumOfCameras" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetProperty ) = _getDLSym ( libHandle,
      "fc2GetProperty" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetPropertyInfo ) = _getDLSym ( libHandle,
      "fc2GetPropertyInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetStrobe ) = _getDLSym ( libHandle,
      "fc2GetStrobe" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetStrobeInfo ) = _getDLSym ( libHandle,
      "fc2GetStrobeInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetTriggerDelay ) = _getDLSym ( libHandle,
      "fc2GetTriggerDelay" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetTriggerDelayInfo ) = _getDLSym ( libHandle,
      "fc2GetTriggerDelayInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetTriggerMode ) = _getDLSym ( libHandle,
      "fc2GetTriggerMode" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetTriggerModeInfo ) = _getDLSym ( libHandle,
      "fc2GetTriggerModeInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2QueryGigEImagingMode ) = _getDLSym ( libHandle,
      "fc2QueryGigEImagingMode" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2ReadRegister ) = _getDLSym ( libHandle,
      "fc2ReadRegister" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetGigEImageBinningSettings ) =
      _getDLSym ( libHandle, "fc2SetGigEImageBinningSettings" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetGigEImageSettings ) = _getDLSym ( libHandle,
      "fc2SetGigEImageSettings" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetGigEImagingMode ) = _getDLSym ( libHandle,
      "fc2SetGigEImagingMode" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetProperty ) = _getDLSym ( libHandle,
      "fc2SetProperty" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetStrobe ) = _getDLSym ( libHandle,
      "fc2SetStrobe" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetTriggerDelay ) = _getDLSym ( libHandle,
      "fc2SetTriggerDelay" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetTriggerMode ) = _getDLSym ( libHandle,
      "fc2SetTriggerMode" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2StartCaptureCallback ) = _getDLSym ( libHandle,
      "fc2StartCaptureCallback" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2StopCapture ) = _getDLSym ( libHandle,
      "fc2StopCapture" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetImageMetadata ) = _getDLSym ( libHandle,
      "fc2GetImageMetadata" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2GetEmbeddedImageInfo ) = _getDLSym ( libHandle,
      "fc2GetEmbeddedImageInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_fc2SetEmbeddedImageInfo ) = _getDLSym ( libHandle,
      "fc2SetEmbeddedImageInfo" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
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
  p_fc2GetImageMetadata = fc2GetImageMetadata;
  p_fc2GetEmbeddedImageInfo = fc2GetEmbeddedImageInfo;
  p_fc2SetEmbeddedImageInfo = fc2SetEmbeddedImageInfo;

#endif /* HAVE_LIBDL */

  return OA_ERR_NONE;
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
