/*****************************************************************************
 *
 * ZWASI2private.h -- private declarations
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

#ifndef ZWASI2_PRIVATE_H
#define ZWASI2_PRIVATE_H

#include <ASICamera2.h>

extern int							_asiInitLibraryFunctionPointers ( void );

extern int							( *p_ASIGetNumOfConnectedCameras )( void ); 
extern int							( *p_ASIGetProductIDs )( int* );
extern ASI_ERROR_CODE	( *p_ASIGetCameraProperty )( ASI_CAMERA_INFO*, int );
extern ASI_ERROR_CODE	( *p_ASIGetCameraPropertyByID )( int, ASI_CAMERA_INFO* );
extern ASI_ERROR_CODE	( *p_ASIOpenCamera )( int );
extern ASI_ERROR_CODE	( *p_ASIInitCamera )( int );
extern ASI_ERROR_CODE	( *p_ASICloseCamera )( int );
extern ASI_ERROR_CODE	( *p_ASIGetNumOfControls )( int, int* );
extern ASI_ERROR_CODE	( *p_ASIGetControlCaps )( int, int, ASI_CONTROL_CAPS* );
extern ASI_ERROR_CODE	( *p_ASIGetControlValue )( int, ASI_CONTROL_TYPE, long*,
													ASI_BOOL* );
extern ASI_ERROR_CODE	( *p_ASISetControlValue )( int, ASI_CONTROL_TYPE, long,
													ASI_BOOL );
extern ASI_ERROR_CODE	( *p_ASISetROIFormat )( int, int, int, int,
													ASI_IMG_TYPE ); 
extern ASI_ERROR_CODE	( *p_ASIGetROIFormat )( int, int*, int*,  int*,
													ASI_IMG_TYPE* );
extern ASI_ERROR_CODE	( *p_ASISetStartPos )( int, int, int ); 
extern ASI_ERROR_CODE	( *p_ASIGetStartPos )( int, int*, int* ); 
extern ASI_ERROR_CODE	( *p_ASIGetDroppedFrames )( int,int* ); 
extern ASI_ERROR_CODE	( *p_ASIEnableDarkSubtract )( int, char* );
extern ASI_ERROR_CODE	( *p_ASIDisableDarkSubtract )( int );
extern ASI_ERROR_CODE	( *p_ASIStartVideoCapture )( int );
extern ASI_ERROR_CODE	( *p_ASIStopVideoCapture )( int );
extern ASI_ERROR_CODE	( *p_ASIGetVideoData )( int, unsigned char*, long, int );
extern ASI_ERROR_CODE	( *p_ASIPulseGuideOn )( int, ASI_GUIDE_DIRECTION );
extern ASI_ERROR_CODE	( *p_ASIPulseGuideOff )( int, ASI_GUIDE_DIRECTION );
extern ASI_ERROR_CODE	( *p_ASIStartExposure )( int, ASI_BOOL );
extern ASI_ERROR_CODE	( *p_ASIStopExposure )( int );
extern ASI_ERROR_CODE	( *p_ASIGetExpStatus )( int, ASI_EXPOSURE_STATUS* );
extern ASI_ERROR_CODE	( *p_ASIGetDataAfterExp )( int, unsigned char*, long );
extern ASI_ERROR_CODE	( *p_ASIGetID )( int, ASI_ID* );
extern ASI_ERROR_CODE	( *p_ASISetID )( int, ASI_ID );
extern ASI_ERROR_CODE	( *p_ASIGetGainOffset )( int, int*, int*, int*, int* );
extern char*					( *p_ASIGetSDKVersion )( void );
extern ASI_ERROR_CODE	( *p_ASIGetCameraSupportMode )( int,
													ASI_SUPPORTED_MODE* );
extern ASI_ERROR_CODE	( *p_ASIGetCameraMode )( int, ASI_CAMERA_MODE* );
extern ASI_ERROR_CODE	( *p_ASISetCameraMode )( int, ASI_CAMERA_MODE );
extern ASI_ERROR_CODE	( *p_ASISendSoftTrigger )( int, ASI_BOOL );
extern ASI_ERROR_CODE	( *p_ASIGetSerialNumber )( int, ASI_SN* );
extern ASI_ERROR_CODE	( *p_ASISetTriggerOutputIOConf )( int,
													ASI_TRIG_OUTPUT_PIN, ASI_BOOL, long, long);
extern ASI_ERROR_CODE	( *p_ASIGetTriggerOutputIOConf )( int,
													ASI_TRIG_OUTPUT_PIN, ASI_BOOL*, long*, long* );

#endif /* ZWASI2_PRIVATE_H */
