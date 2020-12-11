/*****************************************************************************
 *
 * SVBprivate.h -- private declarations
 *
 * Copyright 2020 James Fidell (james@openastroproject.org)
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

#ifndef SVB_PRIVATE_H
#define SVB_PRIVATE_H

#include <SVBCameraSDK.h>

extern int							_svbInitLibraryFunctionPointers ( void );

extern int							( *p_SVBGetNumOfConnectedCameras )( void ); 
extern int							( *p_SVBGetProductIDs )( int* );
extern SVB_ERROR_CODE	( *p_SVBGetCameraInfo )( SVB_CAMERA_INFO*, int );
extern SVB_ERROR_CODE	( *p_SVBGetCameraProperty )( int, SVB_CAMERA_PROPERTY* );
extern SVB_ERROR_CODE	( *p_SVBOpenCamera )( int );
extern SVB_ERROR_CODE	( *p_SVBCloseCamera )( int );
extern SVB_ERROR_CODE	( *p_SVBGetNumOfControls )( int, int* );
extern SVB_ERROR_CODE	( *p_SVBGetControlCaps )( int, int, SVB_CONTROL_CAPS* );
extern SVB_ERROR_CODE	( *p_SVBGetControlValue )( int, SVB_CONTROL_TYPE, long*,
													SVB_BOOL* );
extern SVB_ERROR_CODE	( *p_SVBSetControlValue )( int, SVB_CONTROL_TYPE, long,
													SVB_BOOL );
extern SVB_ERROR_CODE	( *p_SVBSetROIFormat )( int, int, int, int, int, int );
extern SVB_ERROR_CODE	( *p_SVBGetROIFormat )( int, int*, int*,  int*, int*,
													int* );
extern SVB_ERROR_CODE	( *p_SVBSetOutputImageType )( int, int );
extern SVB_ERROR_CODE	( *p_SVBGetOutputImageType )( int, int* );
//extern SVB_ERROR_CODE	( *p_SVBSetStartPos )( int, int, int ); 
//extern SVB_ERROR_CODE	( *p_SVBGetStartPos )( int, int*, int* ); 
extern SVB_ERROR_CODE	( *p_SVBGetDroppedFrames )( int,int* ); 
//extern SVB_ERROR_CODE	( *p_SVBEnableDarkSubtract )( int, char* );
//extern SVB_ERROR_CODE	( *p_SVBDisableDarkSubtract )( int );
extern SVB_ERROR_CODE	( *p_SVBStartVideoCapture )( int );
extern SVB_ERROR_CODE	( *p_SVBStopVideoCapture )( int );
extern SVB_ERROR_CODE	( *p_SVBGetVideoData )( int, unsigned char*, long, int );
//extern SVB_ERROR_CODE	( *p_SVBPulseGuideOn )( int, SVB_GUIDE_DIRECTION );
//extern SVB_ERROR_CODE	( *p_SVBPulseGuideOff )( int, SVB_GUIDE_DIRECTION );
//extern SVB_ERROR_CODE	( *p_SVBStartExposure )( int, SVB_BOOL );
//extern SVB_ERROR_CODE	( *p_SVBStopExposure )( int );
//extern SVB_ERROR_CODE	( *p_SVBGetExpStatus )( int, SVB_EXPOSURE_STATUS* );
//extern SVB_ERROR_CODE	( *p_SVBGetDataAfterExp )( int, unsigned char*, long );
//extern SVB_ERROR_CODE	( *p_SVBGetID )( int, SVB_ID* );
//extern SVB_ERROR_CODE	( *p_SVBSetID )( int, SVB_ID );
extern SVB_ERROR_CODE	( *p_SVBGetGainOffset )( int, int*, int*, int*, int* );
extern char*					( *p_SVBGetSDKVersion )( void );
extern SVB_ERROR_CODE	( *p_SVBGetCameraSupportMode )( int,
													SVB_SUPPORTED_MODE* );
extern SVB_ERROR_CODE	( *p_SVBGetCameraMode )( int, SVB_CAMERA_MODE* );
extern SVB_ERROR_CODE	( *p_SVBSetCameraMode )( int, SVB_CAMERA_MODE );
extern SVB_ERROR_CODE	( *p_SVBSendSoftTrigger )( int );
extern SVB_ERROR_CODE	( *p_SVBGetSerialNumber )( int, SVB_SN* );
extern SVB_ERROR_CODE	( *p_SVBSetTriggerOutputIOConf )( int,
													SVB_TRIG_OUTPUT_PIN, SVB_BOOL, long, long);
extern SVB_ERROR_CODE	( *p_SVBGetTriggerOutputIOConf )( int,
													SVB_TRIG_OUTPUT_PIN, SVB_BOOL*, long*, long* );

#endif /* SVB_PRIVATE_H */
