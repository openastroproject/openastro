/*****************************************************************************
 *
 * SVBdynloader.c -- handle dynamic loading of libSVBCameraSDK
 *
 * Copyright 2020,2021 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBSVBCAMERASDK

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif

#include <SVBCameraSDK.h>

#include <openastro/errno.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "SVBprivate.h"


int							( *p_SVBGetNumOfConnectedCameras )( void ); 
SVB_ERROR_CODE	( *p_SVBGetCameraInfo )( SVB_CAMERA_INFO*, int );
SVB_ERROR_CODE	( *p_SVBGetCameraProperty )( int, SVB_CAMERA_PROPERTY* );
SVB_ERROR_CODE	( *p_SVBOpenCamera )( int );
SVB_ERROR_CODE	( *p_SVBCloseCamera )( int );
SVB_ERROR_CODE	( *p_SVBGetNumOfControls )( int, int* );
SVB_ERROR_CODE	( *p_SVBGetControlCaps )( int, int, SVB_CONTROL_CAPS* );
SVB_ERROR_CODE	( *p_SVBGetControlValue )( int, SVB_CONTROL_TYPE, long*,
										SVB_BOOL* );
SVB_ERROR_CODE	( *p_SVBSetControlValue )( int, SVB_CONTROL_TYPE, long,
										SVB_BOOL );
SVB_ERROR_CODE	( *p_SVBGetOutputImageType )( int, SVB_IMG_TYPE* );
SVB_ERROR_CODE	( *p_SVBSetOutputImageType )( int, SVB_IMG_TYPE );
SVB_ERROR_CODE	( *p_SVBSetROIFormat )( int, int, int, int, int, int ); 
SVB_ERROR_CODE	( *p_SVBGetROIFormat )( int, int*, int*,  int*, int*, int* );
SVB_ERROR_CODE	( *p_SVBGetDroppedFrames )( int, int* ); 
SVB_ERROR_CODE	( *p_SVBStartVideoCapture )( int );
SVB_ERROR_CODE	( *p_SVBStopVideoCapture )( int );
SVB_ERROR_CODE	( *p_SVBGetVideoData )( int, unsigned char*, long, int );
//SVB_ERROR_CODE	( *p_SVBPulseGuideOn )( int, SVB_GUIDE_DIRECTION );
//SVB_ERROR_CODE	( *p_SVBPulseGuideOff )( int, SVB_GUIDE_DIRECTION );
SVB_ERROR_CODE	( *p_SVBStartExposure )( int, SVB_BOOL );
SVB_ERROR_CODE	( *p_SVBStopExposure )( int );
char*						( *p_SVBGetSDKVersion )( void );
SVB_ERROR_CODE	( *p_SVBGetCameraSupportMode )( int, SVB_SUPPORTED_MODE* );
SVB_ERROR_CODE	( *p_SVBGetCameraMode )( int, SVB_CAMERA_MODE* );
SVB_ERROR_CODE	( *p_SVBSetCameraMode )( int, SVB_CAMERA_MODE );
SVB_ERROR_CODE	( *p_SVBSendSoftTrigger )( int );
SVB_ERROR_CODE	( *p_SVBGetSerialNumber )( int, SVB_SN* );
SVB_ERROR_CODE	( *p_SVBSetTriggerOutputIOConf )( int, SVB_TRIG_OUTPUT_PIN,
										SVB_BOOL, long, long);
SVB_ERROR_CODE	( *p_SVBGetTriggerOutputIOConf )( int, SVB_TRIG_OUTPUT_PIN,
										SVB_BOOL*, long*, long* );
SVB_ERROR_CODE	( *p_SVBGetSensorPixelSize )( int, float* );

#if HAVE_LIBDL && !HAVE_STATIC_LIBSVBCAMERASDK
static void*    _getDLSym ( void*, const char* );
#endif

int
_svbInitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL && !HAVE_STATIC_LIBSVBCAMERASDK
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*		libName = "libSVBCameraSDK.dylib";
#else
  const char*		libName = "libSVBCameraSDK.so";
#endif
#ifdef RETRY_SO_WITHOUT_PATH
	int						tryWithoutPath = 1;
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
#ifdef RETRY_SO_WITHOUT_PATH
retry:
#endif
		( void ) strncat ( libPath, libName, PATH_MAX );

    if (!( libHandle = dlopen ( libPath, RTLD_LAZY ))) {
#ifdef RETRY_SO_WITHOUT_PATH
			if ( tryWithoutPath ) {
				tryWithoutPath = 0;
				*libPath = 0;
				goto retry;
			}
#endif
      oaLogWarning ( OA_LOG_CAMERA, "%s: can't load %s, error '%s'", __func__,
					libPath, dlerror());
      return OA_ERR_LIBRARY_NOT_FOUND;
    }

		if (!( *( void** )( &p_SVBGetNumOfConnectedCameras ) = _getDLSym ( libHandle,
		    "SVBGetNumOfConnectedCameras" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetCameraProperty ) = _getDLSym ( libHandle,
		    "SVBGetCameraProperty" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBOpenCamera ) = _getDLSym ( libHandle,
		    "SVBOpenCamera" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBCloseCamera ) = _getDLSym ( libHandle,
		    "SVBCloseCamera" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetNumOfControls ) = _getDLSym ( libHandle,
		    "SVBGetNumOfControls" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetControlCaps ) = _getDLSym ( libHandle,
		    "SVBGetControlCaps" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetControlValue ) = _getDLSym ( libHandle,
		    "SVBGetControlValue" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBSetControlValue ) = _getDLSym ( libHandle,
		    "SVBSetControlValue" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetOutputImageType ) = _getDLSym ( libHandle,
		    "SVBGetOutputImageType" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBSetOutputImageType ) = _getDLSym ( libHandle,
		    "SVBSetOutputImageType" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBSetROIFormat ) = _getDLSym ( libHandle,
		    "SVBSetROIFormat" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetROIFormat ) = _getDLSym ( libHandle,
		    "SVBGetROIFormat" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetDroppedFrames ) = _getDLSym ( libHandle,
		    "SVBGetDroppedFrames" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBStartVideoCapture ) = _getDLSym ( libHandle,
		    "SVBStartVideoCapture" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBStopVideoCapture ) = _getDLSym ( libHandle,
		    "SVBStopVideoCapture" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetVideoData ) = _getDLSym ( libHandle,
		    "SVBGetVideoData" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetSDKVersion ) = _getDLSym ( libHandle,
		    "SVBGetSDKVersion" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetCameraSupportMode ) = _getDLSym ( libHandle,
		    "SVBGetCameraSupportMode" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetCameraMode ) = _getDLSym ( libHandle,
		    "SVBGetCameraMode" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBSetCameraMode ) = _getDLSym ( libHandle,
		    "SVBSetCameraMode" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBSendSoftTrigger ) = _getDLSym ( libHandle,
		    "SVBSendSoftTrigger" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetSerialNumber ) = _getDLSym ( libHandle,
		    "SVBGetSerialNumber" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBSetTriggerOutputIOConf ) = _getDLSym ( libHandle,
		    "SVBSetTriggerOutputIOConf" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}

		if (!( *( void** )( &p_SVBGetTriggerOutputIOConf ) = _getDLSym ( libHandle,
		    "SVBGetTriggerOutputIOConf" ))) {
		  dlclose ( libHandle );
		  libHandle = 0;
		  return OA_ERR_SYMBOL_NOT_FOUND;
		}
	}

#else
#if HAVE_STATIC_LIBSVBCAMERASDK

	p_SVBGetNumOfConnectedCameras = SVBGetNumOfConnectedCameras;
	p_SVBGetCameraProperty = SVBGetCameraProperty;
	p_SVBOpenCamera = SVBOpenCamera;
	p_SVBCloseCamera = SVBCloseCamera;
	p_SVBGetNumOfControls = SVBGetNumOfControls;
	p_SVBGetControlCaps = SVBGetControlCaps;
	p_SVBGetControlValue = SVBGetControlValue;
	p_SVBSetControlValue = SVBSetControlValue;
	p_SVBGetOutputImageType = SVBGetOutputImageType;
	p_SVBSetOutputImageType = SVBSetOutputImageType;
	p_SVBSetROIFormat = SVBSetROIFormat;
	p_SVBGetROIFormat = SVBGetROIFormat;
	p_SVBGetDroppedFrames = SVBGetDroppedFrames;
	p_SVBStartVideoCapture = SVBStartVideoCapture;
	p_SVBStopVideoCapture = SVBStopVideoCapture;
	p_SVBGetVideoData = SVBGetVideoData;
//	p_SVBPulseGuideOn = SVBPulseGuideOn;
//	p_SVBPulseGuideOff = SVBPulseGuideOff;
	p_SVBGetSDKVersion = SVBGetSDKVersion;
	p_SVBGetCameraSupportMode = SVBGetCameraSupportMode;
	p_SVBGetCameraMode = SVBGetCameraMode;
	p_SVBSetCameraMode = SVBSetCameraMode;
	p_SVBSendSoftTrigger = SVBSendSoftTrigger;
	p_SVBGetSerialNumber = SVBGetSerialNumber;
	p_SVBSetTriggerOutputIOConf = SVBSetTriggerOutputIOConf;
	p_SVBGetTriggerOutputIOConf = SVBGetTriggerOutputIOConf;

#else
	return OA_ERR_LIBRARY_NOT_FOUND;
#endif	/* HAVE_STATIC_LIBSVBCAMERASDK */
#endif	/* HAVE_LIBDL && !HAVE_STATIC_LIBSVBCAMERASDK */
	return OA_ERR_NONE;
}

#if HAVE_LIBDL && !HAVE_STATIC_LIBSVBCAMERASDK
static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libSVBCameraSDK DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif
#endif	/* HAVE_LIBSVBCAMERASDK */
