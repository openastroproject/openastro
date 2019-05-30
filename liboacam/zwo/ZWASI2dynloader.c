/*****************************************************************************
 *
 * ZWASI2dynloader.c -- handle dynamic loading of libASICamera2
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

#if HAVE_LIBASI2

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif

#include <openastro/errno.h>
#include <ASICamera2.h>

#include "ZWASI2private.h"


int							( *p_ASIGetNumOfConnectedCameras )( void ); 
int							( *p_ASIGetProductIDs )( int* );
ASI_ERROR_CODE	( *p_ASIGetCameraProperty )( ASI_CAMERA_INFO*, int );
ASI_ERROR_CODE	( *p_ASIGetCameraPropertyByID )( int, ASI_CAMERA_INFO* );
ASI_ERROR_CODE	( *p_ASIOpenCamera )( int );
ASI_ERROR_CODE	( *p_ASIInitCamera )( int );
ASI_ERROR_CODE	( *p_ASICloseCamera )( int );
ASI_ERROR_CODE	( *p_ASIGetNumOfControls )( int, int* );
ASI_ERROR_CODE	( *p_ASIGetControlCaps )( int, int, ASI_CONTROL_CAPS* );
ASI_ERROR_CODE	( *p_ASIGetControlValue )( int, ASI_CONTROL_TYPE, long*,
										ASI_BOOL* );
ASI_ERROR_CODE	( *p_ASISetControlValue )( int, ASI_CONTROL_TYPE, long,
										ASI_BOOL );
ASI_ERROR_CODE	( *p_ASISetROIFormat )( int, int, int, int, ASI_IMG_TYPE ); 
ASI_ERROR_CODE	( *p_ASIGetROIFormat )( int, int*, int*,  int*, ASI_IMG_TYPE* );
ASI_ERROR_CODE	( *p_ASISetStartPos )( int, int, int ); 
ASI_ERROR_CODE	( *p_ASIGetStartPos )( int, int*, int* ); 
ASI_ERROR_CODE	( *p_ASIGetDroppedFrames )( int,int* ); 
ASI_ERROR_CODE	( *p_ASIEnableDarkSubtract )( int, char* );
ASI_ERROR_CODE	( *p_ASIDisableDarkSubtract )( int );
ASI_ERROR_CODE	( *p_ASIStartVideoCapture )( int );
ASI_ERROR_CODE	( *p_ASIStopVideoCapture )( int );
ASI_ERROR_CODE	( *p_ASIGetVideoData )( int, unsigned char*, long, int );
ASI_ERROR_CODE	( *p_ASIPulseGuideOn )( int, ASI_GUIDE_DIRECTION );
ASI_ERROR_CODE	( *p_ASIPulseGuideOff )( int, ASI_GUIDE_DIRECTION );
ASI_ERROR_CODE	( *p_ASIStartExposure )( int, ASI_BOOL );
ASI_ERROR_CODE	( *p_ASIStopExposure )( int );
ASI_ERROR_CODE	( *p_ASIGetExpStatus )( int, ASI_EXPOSURE_STATUS* );
ASI_ERROR_CODE	( *p_ASIGetDataAfterExp )( int, unsigned char*, long );
ASI_ERROR_CODE	( *p_ASIGetID )( int, ASI_ID* );
ASI_ERROR_CODE	( *p_ASISetID )( int, ASI_ID );
ASI_ERROR_CODE	( *p_ASIGetGainOffset )( int, int*, int*, int*, int* );
char*						( *p_ASIGetSDKVersion )( void );
ASI_ERROR_CODE	( *p_ASIGetCameraSupportMode )( int, ASI_SUPPORTED_MODE* );
ASI_ERROR_CODE	( *p_ASIGetCameraMode )( int, ASI_CAMERA_MODE* );
ASI_ERROR_CODE	( *p_ASISetCameraMode )( int, ASI_CAMERA_MODE );
ASI_ERROR_CODE	( *p_ASISendSoftTrigger )( int, ASI_BOOL );
ASI_ERROR_CODE	( *p_ASIGetSerialNumber )( int, ASI_SN* );
ASI_ERROR_CODE	( *p_ASISetTriggerOutputIOConf )( int, ASI_TRIG_OUTPUT_PIN,
										ASI_BOOL, long, long);
ASI_ERROR_CODE	( *p_ASIGetTriggerOutputIOConf )( int, ASI_TRIG_OUTPUT_PIN,
										ASI_BOOL*, long*, long* );

#if HAVE_LIBDL && !HAVE_STATIC_LIBASICAMERA2
static void*    _getDLSym ( void*, const char* );
#endif

int
_asiInitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL && !HAVE_STATIC_LIBASICAMERA2
	static void*		libHandle = 0;

	if ( !libHandle ) {
		if (!( libHandle = dlopen( "libASICamera2.so", RTLD_LAZY ))) {
			return OA_ERR_LIBRARY_NOT_FOUND;
		}
	}

	dlerror();

  if (!( *( void** )( &p_ASIGetNumOfConnectedCameras ) = _getDLSym ( libHandle,
      "ASIGetNumOfConnectedCameras" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetProductIDs ) = _getDLSym ( libHandle,
      "ASIGetProductIDs" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetCameraProperty ) = _getDLSym ( libHandle,
      "ASIGetCameraProperty" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetCameraPropertyByID ) = _getDLSym ( libHandle,
      "ASIGetCameraPropertyByID" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIOpenCamera ) = _getDLSym ( libHandle,
      "ASIOpenCamera" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIInitCamera ) = _getDLSym ( libHandle,
      "ASIInitCamera" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASICloseCamera ) = _getDLSym ( libHandle,
      "ASICloseCamera" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetNumOfControls ) = _getDLSym ( libHandle,
      "ASIGetNumOfControls" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetControlCaps ) = _getDLSym ( libHandle,
      "ASIGetControlCaps" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetControlValue ) = _getDLSym ( libHandle,
      "ASIGetControlValue" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASISetControlValue ) = _getDLSym ( libHandle,
      "ASISetControlValue" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASISetROIFormat ) = _getDLSym ( libHandle,
      "ASISetROIFormat" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetROIFormat ) = _getDLSym ( libHandle,
      "ASIGetROIFormat" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASISetStartPos ) = _getDLSym ( libHandle,
      "ASISetStartPos" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetStartPos ) = _getDLSym ( libHandle,
      "ASIGetStartPos" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetDroppedFrames ) = _getDLSym ( libHandle,
      "ASIGetDroppedFrames" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIEnableDarkSubtract ) = _getDLSym ( libHandle,
      "ASIEnableDarkSubtract" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIDisableDarkSubtract ) = _getDLSym ( libHandle,
      "ASIDisableDarkSubtract" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIStartVideoCapture ) = _getDLSym ( libHandle,
      "ASIStartVideoCapture" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIStopVideoCapture ) = _getDLSym ( libHandle,
      "ASIStopVideoCapture" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetVideoData ) = _getDLSym ( libHandle,
      "ASIGetVideoData" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIPulseGuideOn ) = _getDLSym ( libHandle,
      "ASIPulseGuideOn" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIPulseGuideOff ) = _getDLSym ( libHandle,
      "ASIPulseGuideOff" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIStartExposure ) = _getDLSym ( libHandle,
      "ASIStartExposure" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIStopExposure ) = _getDLSym ( libHandle,
      "ASIStopExposure" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetExpStatus ) = _getDLSym ( libHandle,
      "ASIGetExpStatus" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetDataAfterExp ) = _getDLSym ( libHandle,
      "ASIGetDataAfterExp" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetID ) = _getDLSym ( libHandle,
      "ASIGetID" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASISetID ) = _getDLSym ( libHandle,
      "ASISetID" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetGainOffset ) = _getDLSym ( libHandle,
      "ASIGetGainOffset" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetSDKVersion ) = _getDLSym ( libHandle,
      "ASIGetSDKVersion" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetCameraSupportMode ) = _getDLSym ( libHandle,
      "ASIGetCameraSupportMode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetCameraMode ) = _getDLSym ( libHandle,
      "ASIGetCameraMode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASISetCameraMode ) = _getDLSym ( libHandle,
      "ASISetCameraMode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASISendSoftTrigger ) = _getDLSym ( libHandle,
      "ASISendSoftTrigger" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetSerialNumber ) = _getDLSym ( libHandle,
      "ASIGetSerialNumber" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASISetTriggerOutputIOConf ) = _getDLSym ( libHandle,
      "ASISetTriggerOutputIOConf" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_ASIGetTriggerOutputIOConf ) = _getDLSym ( libHandle,
      "ASIGetTriggerOutputIOConf" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

#else
#if HAVE_STATIC_LIBASICAMERA2

	p_ASIGetNumOfConnectedCameras = ASIGetNumOfConnectedCameras;
	p_ASIGetProductIDs = ASIGetProductIDs;
	p_ASIGetCameraProperty = ASIGetCameraProperty;
	p_ASIGetCameraPropertyByID = ASIGetCameraPropertyByID;
	p_ASIOpenCamera = ASIOpenCamera;
	p_ASIInitCamera = ASIInitCamera;
	p_ASICloseCamera = ASICloseCamera;
	p_ASIGetNumOfControls = ASIGetNumOfControls;
	p_ASIGetControlCaps = ASIGetControlCaps;
	p_ASIGetControlValue = ASIGetControlValue;
	p_ASISetControlValue = ASISetControlValue;
	p_ASISetROIFormat = ASISetROIFormat;
	p_ASIGetROIFormat = ASIGetROIFormat;
	p_ASISetStartPos = ASISetStartPos;
	p_ASIGetStartPos = ASIGetStartPos;
	p_ASIGetDroppedFrames = ASIGetDroppedFrames;
	p_ASIEnableDarkSubtract = ASIEnableDarkSubtract;
	p_ASIDisableDarkSubtract = ASIDisableDarkSubtract;
	p_ASIStartVideoCapture = ASIStartVideoCapture;
	p_ASIStopVideoCapture = ASIStopVideoCapture;
	p_ASIGetVideoData = ASIGetVideoData;
	p_ASIPulseGuideOn = ASIPulseGuideOn;
	p_ASIPulseGuideOff = ASIPulseGuideOff;
	p_ASIStartExposure = ASIStartExposure;
	p_ASIStopExposure = ASIStopExposure;
	p_ASIGetExpStatus = ASIGetExpStatus;
	p_ASIGetDataAfterExp = ASIGetDataAfterExp;
	p_ASIGetID = ASIGetID;
	p_ASISetID = ASISetID;
	p_ASIGetGainOffset = ASIGetGainOffset;
	p_ASIGetSDKVersion = ASIGetSDKVersion;
	p_ASIGetCameraSupportMode = ASIGetCameraSupportMode;
	p_ASIGetCameraMode = ASIGetCameraMode;
	p_ASISetCameraMode = ASISetCameraMode;
	p_ASISendSoftTrigger = ASISendSoftTrigger;
	p_ASIGetSerialNumber = ASIGetSerialNumber;
	p_ASISetTriggerOutputIOConf = ASISetTriggerOutputIOConf;
	p_ASIGetTriggerOutputIOConf = ASIGetTriggerOutputIOConf;

#else
	return OA_ERR_LIBRARY_NOT_FOUND;
#endif	/* HAVE_STATIC_LIBASICAMERA2 */
#endif	/* HAVE_LIBDL && !HAVE_STATIC_LIBASICAMERA2 */
	return OA_ERR_NONE;
}

#if HAVE_LIBDL && !HAVE_STATIC_LIBASICAMERA2
static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libASICamera2 DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif
#endif	/* HAVE_LIBASI2 */
