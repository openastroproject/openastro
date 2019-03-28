/*****************************************************************************
 *
 * qhyccdoacam.c -- main entrypoint for libqhyccd camera support
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
#endif
#include <openastro/camera.h>
#include <qhyccd.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "qhyccdoacam.h"

// Pointers to libqhyccd functions so we can use them via libdl.

void						( *p_SetQHYCCDLogLevel )( uint8_t );
void						( *p_EnableQHYCCDMessage )( bool );
void						( *p_EnableQHYCCDLogFile )( bool );
const char*			( *p_GetTimeStamp )( void );
uint32_t				( *p_InitQHYCCDResource )( void );
uint32_t				( *p_ReleaseQHYCCDResource )( void );
uint32_t				( *p_ScanQHYCCD )( void );
uint32_t				( *p_GetQHYCCDId )( uint32_t, char* );
uint32_t				( *p_GetQHYCCDModel )( char*, char* );
qhyccd_handle*	( *p_OpenQHYCCD )( char* );
uint32_t				( *p_CloseQHYCCD )( qhyccd_handle* );
uint32_t				( *p_SetQHYCCDStreamMode )( qhyccd_handle*, uint8_t );
uint32_t				( *p_InitQHYCCD )( qhyccd_handle* );
uint32_t				( *p_IsQHYCCDControlAvailable )( qhyccd_handle*, CONTROL_ID );
uint32_t				( *p_SetQHYCCDParam )( qhyccd_handle*, CONTROL_ID, double );
double					( *p_GetQHYCCDParam )( qhyccd_handle*, CONTROL_ID );
uint32_t				( *p_GetQHYCCDParamMinMaxStep )( qhyccd_handle*, CONTROL_ID,
										double*, double*, double* );
uint32_t				( *p_SetQHYCCDResolution )( qhyccd_handle*, uint32_t, uint32_t,
										uint32_t, uint32_t );
uint32_t				( *p_GetQHYCCDMemLength )( qhyccd_handle* );
uint32_t				( *p_ExpQHYCCDSingleFrame )( qhyccd_handle* );
uint32_t				( *p_GetQHYCCDSingleFrame )( qhyccd_handle*, uint32_t*,
										uint32_t*, uint32_t*, uint32_t*, uint8_t* );
uint32_t				( *p_CancelQHYCCDExposing )( qhyccd_handle* );
uint32_t				( *p_CancelQHYCCDExposingAndReadout )( qhyccd_handle* );
uint32_t				( *p_BeginQHYCCDLive )( qhyccd_handle* );
uint32_t				( *p_GetQHYCCDLiveFrame )( qhyccd_handle*, uint32_t*, 
                    uint32_t*, uint32_t*, uint32_t*, uint8_t* );
uint32_t				( *p_StopQHYCCDLive )( qhyccd_handle* );
uint32_t				( *p_SetQHYCCDBinMode )( qhyccd_handle*, uint32_t, uint32_t );
uint32_t				( *p_SetQHYCCDBitsMode )( qhyccd_handle*, uint32_t );
uint32_t				( *p_ControlQHYCCDTemp )( qhyccd_handle*, double );
uint32_t				( *p_ControlQHYCCDGuide )( qhyccd_handle*, uint32_t, uint16_t );
uint32_t				( *p_SetQHYCCDTrigerMode )( qhyccd_handle*, uint32_t );
#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
uint32_t				( *p_OSXInitQHYCCDFirmware )( char* );
#endif
uint32_t				( *p_GetQHYCCDChipInfo )( qhyccd_handle*, double*, double*,
										uint32_t*, uint32_t*, double*, double*, uint32_t* );
uint32_t				( *p_GetQHYCCDEffectiveArea )( qhyccd_handle*, uint32_t*,
										uint32_t*, uint32_t*, uint32_t* );
uint32_t				( *p_GetQHYCCDOverScanArea )( qhyccd_handle*, uint32_t*,
										uint32_t*, uint32_t*, uint32_t* );
uint32_t				( *p_GetQHYCCDExposureRemaining )( qhyccd_handle* );
uint32_t				( *p_GetQHYCCDFWVersion )( qhyccd_handle*, uint8_t* );
uint32_t				( *p_GetQHYCCDCameraStatus )( qhyccd_handle*, uint8_t* );
uint32_t				( *p_GetQHYCCDShutterStatus )( qhyccd_handle* );
uint32_t				( *p_ControlQHYCCDShutter )( qhyccd_handle*, uint8_t );
uint32_t				( *p_GetQHYCCDHumidity )( qhyccd_handle*, double* );
uint32_t				( *p_QHYCCDI2CTwoWrite )( qhyccd_handle*, uint16_t, uint16_t );
uint32_t				( *p_QHYCCDI2CTwoRead )( qhyccd_handle*, uint16_t );
uint32_t				( *p_GetQHYCCDReadingProgress )( qhyccd_handle* );
/*
uint32_t				( *p_GetQHYCCDNumberOfReadModes )( qhyccd_handle*, int32_t* );
uint32_t				( *p_GetQHYCCDReadModeResolution )( qhyccd_handle*, int32_t,
										uint32_t*, uint32_t* );
uint32_t				( *p_GetQHYCCDReadModeName )( qhyccd_handle*, int32_t,
										char* );
uint32_t				( *p_SetQHYCCDReadMode )( qhyccd_handle*, int32_t );
uint32_t				( *p_GetQHYCCDReadMode )( qhyccd_handle*, int32_t* );
*/
uint32_t				( *p_SetQHYCCDDebayerOnOff )( qhyccd_handle*, bool );


static void*		_getDLSym ( void*, const char* );

/**
 * Cycle through the list of cameras returned by libqhyccd
 */

int
oaQHYCCDGetCameras ( CAMERA_LIST* deviceList, int flags )
{
  unsigned int		numCameras;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int                   ret;
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];
	char						qhyccdId[ 64 ]; // size is a guess
	char						qhyccdModel[ 64 ]; // size is a guess

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
	char					firmwarePath[ PATH_MAX+1 ];
  const char*		libName = "libqhyccd.dylib";
#else
  const char*		libName = "libqhyccd.so.4";
#endif

	*libPath = 0;
	dlerror();
  if ( !libHandle ) {
		if ( installPathRoot ) {
			( void ) strncpy ( libPath, installPathRoot, PATH_MAX );
		}
#ifdef SHLIB_PATH
		( void ) strncat ( libPath, SHLIB_PATH, PATH_MAX );
#endif
		( void ) strncat ( libPath, libName, PATH_MAX );

    if (!( libHandle = dlopen ( libPath, RTLD_LAZY ))) {
      // fprintf ( stderr, "can't load %s:\n%s\n", libPath, dlerror());
      return 0;
    }

	  if (!( *( void** )( &p_SetQHYCCDLogLevel ) = _getDLSym ( libHandle,
	      "SetQHYCCDLogLevel" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_EnableQHYCCDMessage ) = _getDLSym ( libHandle,
	      "EnableQHYCCDMessage" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_EnableQHYCCDLogFile ) = _getDLSym ( libHandle,
	      "EnableQHYCCDLogFile" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetTimeStamp ) = _getDLSym ( libHandle,
	      "GetTimeStamp" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_InitQHYCCDResource ) = _getDLSym ( libHandle,
	      "InitQHYCCDResource" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_ReleaseQHYCCDResource ) = _getDLSym ( libHandle,
	      "ReleaseQHYCCDResource" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_ScanQHYCCD ) = _getDLSym ( libHandle,
	      "ScanQHYCCD" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDId ) = _getDLSym ( libHandle,
	      "GetQHYCCDId" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDModel ) = _getDLSym ( libHandle,
	      "GetQHYCCDModel" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_OpenQHYCCD ) = _getDLSym ( libHandle,
	      "OpenQHYCCD" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_CloseQHYCCD ) = _getDLSym ( libHandle,
	      "CloseQHYCCD" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_SetQHYCCDStreamMode ) = _getDLSym ( libHandle,
	      "SetQHYCCDStreamMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_InitQHYCCD ) = _getDLSym ( libHandle,
	      "InitQHYCCD" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_IsQHYCCDControlAvailable ) = _getDLSym ( libHandle,
	      "IsQHYCCDControlAvailable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_SetQHYCCDParam ) = _getDLSym ( libHandle,
	      "SetQHYCCDParam" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDParam ) = _getDLSym ( libHandle,
	      "GetQHYCCDParam" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDParamMinMaxStep ) = _getDLSym ( libHandle,
	      "GetQHYCCDParamMinMaxStep" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_SetQHYCCDResolution ) = _getDLSym ( libHandle,
	      "SetQHYCCDResolution" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDMemLength ) = _getDLSym ( libHandle,
	      "GetQHYCCDMemLength" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_ExpQHYCCDSingleFrame ) = _getDLSym ( libHandle,
	      "ExpQHYCCDSingleFrame" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDSingleFrame ) = _getDLSym ( libHandle,
	      "GetQHYCCDSingleFrame" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_CancelQHYCCDExposing ) = _getDLSym ( libHandle,
	      "CancelQHYCCDExposing" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_CancelQHYCCDExposingAndReadout ) = _getDLSym (
				libHandle, "CancelQHYCCDExposingAndReadout" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_BeginQHYCCDLive ) = _getDLSym ( libHandle,
	      "BeginQHYCCDLive" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDLiveFrame ) = _getDLSym ( libHandle,
	      "GetQHYCCDLiveFrame" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_StopQHYCCDLive ) = _getDLSym ( libHandle,
	      "StopQHYCCDLive" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_SetQHYCCDBinMode ) = _getDLSym ( libHandle,
	      "SetQHYCCDBinMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_SetQHYCCDBitsMode ) = _getDLSym ( libHandle,
	      "SetQHYCCDBitsMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_ControlQHYCCDTemp ) = _getDLSym ( libHandle,
	      "ControlQHYCCDTemp" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_ControlQHYCCDGuide ) = _getDLSym ( libHandle,
	      "ControlQHYCCDGuide" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_SetQHYCCDTrigerMode ) = _getDLSym ( libHandle,
	      "SetQHYCCDTrigerMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
	  if (!( *( void** )( &p_OSXInitQHYCCDFirmware ) = _getDLSym ( libHandle,
	      "OSXInitQHYCCDFirmware" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }
#endif

	  if (!( *( void** )( &p_GetQHYCCDChipInfo ) = _getDLSym ( libHandle,
	      "GetQHYCCDChipInfo" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDEffectiveArea ) = _getDLSym ( libHandle,
	      "GetQHYCCDEffectiveArea" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDOverScanArea ) = _getDLSym ( libHandle,
	      "GetQHYCCDOverScanArea" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDExposureRemaining ) = _getDLSym ( libHandle,
	      "GetQHYCCDExposureRemaining" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDFWVersion ) = _getDLSym ( libHandle,
	      "GetQHYCCDFWVersion" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDCameraStatus ) = _getDLSym ( libHandle,
	      "GetQHYCCDCameraStatus" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDShutterStatus ) = _getDLSym ( libHandle,
	      "GetQHYCCDShutterStatus" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_ControlQHYCCDShutter ) = _getDLSym ( libHandle,
	      "ControlQHYCCDShutter" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDHumidity ) = _getDLSym ( libHandle,
	      "GetQHYCCDHumidity" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_QHYCCDI2CTwoWrite ) = _getDLSym ( libHandle,
	      "QHYCCDI2CTwoWrite" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_QHYCCDI2CTwoRead ) = _getDLSym ( libHandle,
	      "QHYCCDI2CTwoRead" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDReadingProgress ) = _getDLSym ( libHandle,
	      "GetQHYCCDReadingProgress" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

		/*
		 * This function isn't used and isn't present in some releases (4.0.1?) of
		 * libqhyccd, so skip it for now
		 *
	  if (!( *( void** )( &p_GetQHYCCDNumberOfReadModes ) = _getDLSym ( libHandle,
	      "GetQHYCCDNumberOfReadModes" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }
		 */

		/*
		 * Ditto this one
		 *
	  if (!( *( void** )( &p_GetQHYCCDReadModeResolution ) = _getDLSym ( libHandle,
	      "GetQHYCCDReadModeResolution" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }
		 */

		/*
		 * And this
		 *
	  if (!( *( void** )( &p_GetQHYCCDReadModeName ) = _getDLSym ( libHandle,
	      "GetQHYCCDReadModeName" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }
		 */

		/*
		 * And these two...
		 *
	  if (!( *( void** )( &p_SetQHYCCDReadMode ) = _getDLSym ( libHandle,
	      "SetQHYCCDReadMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }

	  if (!( *( void** )( &p_GetQHYCCDReadMode ) = _getDLSym ( libHandle,
	      "GetQHYCCDReadMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }
		 */

	  if (!( *( void** )( &p_SetQHYCCDDebayerOnOff ) = _getDLSym ( libHandle,
	      "SetQHYCCDDebayerOnOff" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return 0;
	  }
	}
	
	if ( p_InitQHYCCDResource() != QHYCCD_SUCCESS ) {
		fprintf ( stderr, "can't init libqhyccd\n" );
		return 0;
	}

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  if ( installPathRoot ) {
		( void ) strcpy ( firmwarePath, installPathRoot );
  }
	( void ) strcat ( firmwarePath, FIRMWARE_QHY_PATH );
	// because, stupidly, the firmware directory has to be called
	// "firmware"
	// This may in fact have changed in the v4.0.14 release, but it's
	// not entirely clear from the "documentation"
	( void ) strcat ( firmwarePath, "/firmware" );
	p_OSXInitQHYCCDFirmware ( firmwarePath );
	// Don't really know how long this should be, but a short delay appears
	// to be required to allow the camera(s) to reset
	sleep ( 3 );
#endif

  numCameras = ( p_ScanQHYCCD )();
  if ( numCameras < 1 ) {
		p_ReleaseQHYCCDResource();
    return 0;
  }

  for ( i = 0; i < numCameras; i++ ) {

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
			p_ReleaseQHYCCDResource();
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
			p_ReleaseQHYCCDResource();
      return -OA_ERR_MEM_ALLOC;
    }

		if ( p_GetQHYCCDId ( i, qhyccdId ) != QHYCCD_SUCCESS ) {
			p_ReleaseQHYCCDResource();
			fprintf ( stderr, "can't get id for camera %d\n", i );
			return 0;
		} 

		if ( p_GetQHYCCDModel ( qhyccdId, qhyccdModel ) != QHYCCD_SUCCESS ) {
			p_ReleaseQHYCCDResource();
			fprintf ( stderr, "can't get model for camera %d\n", i );
			return 0;
		} 

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_QHYCCD;
    ( void ) strncpy ( dev->deviceName, qhyccdModel, OA_MAX_NAME_LEN+1 );
    _private->devIndex = i;
    ( void ) strcpy ( _private->deviceId, qhyccdId );
    dev->_private = _private;
    dev->initCamera = oaQHYCCDInitCamera;
    dev->hasLoadableFirmware = 0;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
			p_ReleaseQHYCCDResource();
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
  }

	p_ReleaseQHYCCDResource();
  return numCameras;
}


static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libqhyccd DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
