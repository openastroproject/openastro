/*****************************************************************************
 *
 * qhyccdprivate.h -- private header for qhyccd camera API
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

#ifndef OA_QHYCCD_PRIVATE_H
#define OA_QHYCCD_PRIVATE_H

#include <qhyccd/qhyccd.h>

extern int					_qhyccdInitLibraryFunctionPointers ( void );
extern void					( *p_SetQHYCCDLogLevel )( uint8_t );
extern void					( *p_EnableQHYCCDMessage )( bool );
extern void					( *p_EnableQHYCCDLogFile )( bool );
extern const char*	( *p_GetTimeStamp )( void );
extern uint32_t			( *p_InitQHYCCDResource )( void );
extern uint32_t			( *p_ReleaseQHYCCDResource )( void );
extern uint32_t			( *p_ScanQHYCCD )( void );
extern uint32_t			( *p_GetQHYCCDId )( uint32_t, char* );
extern uint32_t			( *p_GetQHYCCDModel )( char*, char* );
extern qhyccd_handle*	( *p_OpenQHYCCD )( char* );
extern uint32_t			( *p_CloseQHYCCD )( qhyccd_handle* );
extern uint32_t			( *p_SetQHYCCDStreamMode )( qhyccd_handle*, uint8_t );
extern uint32_t			( *p_InitQHYCCD )( qhyccd_handle* );
extern uint32_t			( *p_IsQHYCCDControlAvailable )( qhyccd_handle*,
												CONTROL_ID );
extern uint32_t			( *p_SetQHYCCDParam )( qhyccd_handle*, CONTROL_ID, double );
extern double				( *p_GetQHYCCDParam )( qhyccd_handle*, CONTROL_ID );
extern uint32_t			( *p_GetQHYCCDParamMinMaxStep )( qhyccd_handle*, CONTROL_ID,
												double*, double*, double* );
extern uint32_t			( *p_SetQHYCCDResolution )( qhyccd_handle*, uint32_t,
												uint32_t, uint32_t, uint32_t );
extern uint32_t			( *p_GetQHYCCDMemLength )( qhyccd_handle* );
extern uint32_t			( *p_ExpQHYCCDSingleFrame )( qhyccd_handle* );
extern uint32_t			( *p_GetQHYCCDSingleFrame )( qhyccd_handle*, uint32_t*,
												uint32_t*, uint32_t*, uint32_t*, uint8_t* );
extern uint32_t			( *p_CancelQHYCCDExposing )( qhyccd_handle* );
extern uint32_t			( *p_CancelQHYCCDExposingAndReadout )( qhyccd_handle* );
extern uint32_t			( *p_BeginQHYCCDLive )( qhyccd_handle* );
extern uint32_t			( *p_GetQHYCCDLiveFrame )( qhyccd_handle*, uint32_t*, 
												uint32_t*, uint32_t*, uint32_t*, uint8_t* );
extern uint32_t			( *p_StopQHYCCDLive )( qhyccd_handle* );
extern uint32_t			( *p_SetQHYCCDBinMode )( qhyccd_handle*, uint32_t,
												uint32_t );
extern uint32_t			( *p_SetQHYCCDBitsMode )( qhyccd_handle*, uint32_t );
extern uint32_t			( *p_ControlQHYCCDTemp )( qhyccd_handle*, double );
extern uint32_t			( *p_ControlQHYCCDGuide )( qhyccd_handle*, uint32_t,
												uint16_t );
extern uint32_t			( *p_SetQHYCCDTrigerMode )( qhyccd_handle*, uint32_t );
#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
extern uint32_t			( *p_OSXInitQHYCCDFirmware )( char* );
#endif
extern uint32_t			( *p_GetQHYCCDChipInfo )( qhyccd_handle*, double*, double*,
												uint32_t*, uint32_t*, double*, double*, uint32_t* );
extern uint32_t			( *p_GetQHYCCDEffectiveArea )( qhyccd_handle*, uint32_t*,
												uint32_t*, uint32_t*, uint32_t* );
extern uint32_t			( *p_GetQHYCCDOverScanArea )( qhyccd_handle*, uint32_t*,
												uint32_t*, uint32_t*, uint32_t* );
extern uint32_t			( *p_GetQHYCCDExposureRemaining )( qhyccd_handle* );
extern uint32_t			( *p_GetQHYCCDFWVersion )( qhyccd_handle*, uint8_t* );
extern uint32_t			( *p_GetQHYCCDCameraStatus )( qhyccd_handle*, uint8_t* );
extern uint32_t			( *p_GetQHYCCDShutterStatus )( qhyccd_handle* );
extern uint32_t			( *p_ControlQHYCCDShutter )( qhyccd_handle*, uint8_t );
extern uint32_t			( *p_GetQHYCCDHumidity )( qhyccd_handle*, double* );
extern uint32_t			( *p_QHYCCDI2CTwoWrite )( qhyccd_handle*, uint16_t,
												uint16_t );
extern uint32_t			( *p_QHYCCDI2CTwoRead )( qhyccd_handle*, uint16_t );
extern uint32_t			( *p_GetQHYCCDReadingProgress )( qhyccd_handle* );
/*
extern uint32_t			( *p_GetQHYCCDNumberOfReadModes )( qhyccd_handle*,
												int32_t* );
extern uint32_t			( *p_GetQHYCCDReadModeResolution )( qhyccd_handle*, int32_t,
												uint32_t*, uint32_t* );
extern uint32_t			( *p_GetQHYCCDReadModeName )( qhyccd_handle*, int32_t,
												char* );
extern uint32_t			( *p_SetQHYCCDReadMode )( qhyccd_handle*, int32_t );
extern uint32_t			( *p_GetQHYCCDReadMode )( qhyccd_handle*, int32_t* );
*/
extern uint32_t			( *p_SetQHYCCDDebayerOnOff )( qhyccd_handle*, bool );

struct qhyCtrl {
  uint8_t qhyControl;
  int   oaControl;
  int   oaControlType;
	int		multiplier;
};

extern struct qhyCtrl	QHYControlData[];
extern unsigned int		numQHYControls;

#endif	/* OA_QHYCCD_PRIVATE_H */
