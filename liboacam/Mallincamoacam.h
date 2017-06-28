/*****************************************************************************
 *
 * Mallincamoacam.h -- header for Mallincam camera API
 *
 * Copyright 2016,2017 James Fidell (james@openastroproject.org)
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

#ifndef OA_MALLINCAM_OACAM_H
#define OA_MALLINCAM_OACAM_H

#include <toupcam.h>

extern int		oaMallincamGetCameras ( CAMERA_LIST*, int );
extern oaCamera*	oaMallincamInitCamera ( oaCameraDevice* );

extern int		oaMallincamCloseCamera ( oaCamera* );

extern int		oaMallincamCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaMallincamCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaMallincamCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaMallincamCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern int		oaMallincamCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int), void* );
extern int		oaMallincamCameraStopStreaming ( oaCamera* );
extern int		oaMallincamCameraIsStreaming ( oaCamera* );

extern int		oaMallincamCameraSetResolution ( oaCamera*, int, int );
extern int		oaMallincamCameraSetROI ( oaCamera*, int, int );
extern int		oaMallincamCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamMallincamcontroller ( void* );
extern void*		oacamMallincamcallbackHandler ( void* );

extern const FRAMESIZES* oaMallincamCameraGetFrameSizes ( oaCamera* );
extern int		oaMallincamCameraGetFramePixelFormat ( oaCamera*, int );


extern const char*	( *p_Mallincam_Version )();
extern unsigned		( *p_Mallincam_Enum )( ToupcamInst* );
extern HToupCam		( *p_Mallincam_Open )( const char* );
extern HToupCam		( *p_Mallincam_OpenByIndex )( unsigned );
void			( *p_Mallincam_Close )( HToupCam );
extern HRESULT		( *p_Mallincam_StartPullModeWithCallback )( HToupCam,
			    PTOUPCAM_EVENT_CALLBACK, void* );
extern HRESULT		( *p_Mallincam_PullImage )( HToupCam, void*, int,
			    unsigned*, unsigned* );
extern HRESULT		( *p_Mallincam_PullStillImage )( HToupCam, void*, int,
			    unsigned*, unsigned* );
extern HRESULT		( *p_Mallincam_StartPushMode )( HToupCam,
			    PTOUPCAM_DATA_CALLBACK, void* );
extern HRESULT		( *p_Mallincam_Stop )( HToupCam );
extern HRESULT		( *p_Mallincam_Pause )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_Snap )( HToupCam, unsigned );
extern HRESULT		( *p_Mallincam_Trigger )( HToupCam );
extern HRESULT		( *p_Mallincam_get_Size )( HToupCam, int*, int* );
extern HRESULT		( *p_Mallincam_put_Size )( HToupCam, int, int );
extern HRESULT		( *p_Mallincam_get_eSize )( HToupCam, unsigned* );
extern HRESULT		( *p_Mallincam_put_eSize )( HToupCam, unsigned );
extern HRESULT		( *p_Mallincam_get_Resolution )( HToupCam, unsigned,
			    int*, int* );
extern HRESULT		( *p_Mallincam_get_ResolutionNumber )( HToupCam );
extern HRESULT		( *p_Mallincam_get_ResolutionRatio )( HToupCam, unsigned,
			    int*, int* );
extern HRESULT		( *p_Mallincam_get_RawFormat )( HToupCam, unsigned*,
			    unsigned* );
extern HRESULT		( *p_Mallincam_get_AutoExpoEnable )( HToupCam, BOOL* );
extern HRESULT		( *p_Mallincam_get_AutoExpoTarget )( HToupCam,
			    unsigned short* );
extern HRESULT		( *p_Mallincam_put_AutoExpoEnable )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_put_AutoExpoTarget )( HToupCam,
			    unsigned short );
extern HRESULT		( *p_Mallincam_get_ExpoTime )( HToupCam, unsigned* );
extern HRESULT		( *p_Mallincam_get_ExpTimeRange )( HToupCam, unsigned*,
			    unsigned*, unsigned* );
extern HRESULT		( *p_Mallincam_put_ExpoTime )( HToupCam, unsigned );
extern HRESULT		( *p_Mallincam_put_MaxAutoExpoTimeAGain )( HToupCam,
			    unsigned, unsigned short );
extern HRESULT		( *p_Mallincam_get_ExpoAGain )( HToupCam,
			    unsigned short* );
extern HRESULT		( *p_Mallincam_put_ExpoAGain )( HToupCam,
			    unsigned short );
extern HRESULT		( *p_Mallincam_get_ExpoAGainRange )( HToupCam,
			    unsigned short*, unsigned short*, unsigned short* );
extern HRESULT		( *p_Mallincam_AwbInit )( HToupCam,
			    PITOUPCAM_WHITEBALANCE_CALLBACK, void* );
extern HRESULT		( *p_Mallincam_AwbOnePush )( HToupCam,
			    PITOUPCAM_TEMPTINT_CALLBACK, void* );
extern HRESULT		( *p_Mallincam_get_TempTint )( HToupCam, int*, int* );
extern HRESULT		( *p_Mallincam_put_TempTint )( HToupCam, int, int );
extern HRESULT		( *p_Mallincam_get_WhiteBalanceGain )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_put_WhiteBalanceGain )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_get_Hue )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_put_Hue )( HToupCam, int );
extern HRESULT		( *p_Mallincam_get_Saturation )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_put_Saturation )( HToupCam, int );
extern HRESULT		( *p_Mallincam_get_Brightness )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_put_Brightness )( HToupCam, int );
extern HRESULT		( *p_Mallincam_get_Contrast )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_put_Contrast )( HToupCam, int );
extern HRESULT		( *p_Mallincam_get_Gamma )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_put_Gamma )( HToupCam, int );
extern HRESULT		( *p_Mallincam_get_Chrome )( HToupCam, BOOL* );
extern HRESULT		( *p_Mallincam_put_Chrome )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_get_VFlip )( HToupCam, BOOL* );
extern HRESULT		( *p_Mallincam_put_VFlip )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_get_HFlip )( HToupCam, BOOL* );
extern HRESULT		( *p_Mallincam_put_HFlip )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_get_Negative )( HToupCam, BOOL* );
extern HRESULT		( *p_Mallincam_put_Negative )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_get_MaxSpeed )( HToupCam );
extern HRESULT		( *p_Mallincam_get_Speed )( HToupCam, unsigned short* );
extern HRESULT		( *p_Mallincam_put_Speed )( HToupCam, unsigned short );
extern HRESULT		( *p_Mallincam_get_MaxBitDepth )( HToupCam );
extern HRESULT		( *p_Mallincam_get_HZ )( HToupCam, int* );
extern HRESULT		( *p_Mallincam_put_HZ )( HToupCam, int );
extern HRESULT		( *p_Mallincam_get_Mode )( HToupCam, BOOL* );
extern HRESULT		( *p_Mallincam_put_Mode )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_get_AWBAuxRect )( HToupCam, RECT* );
extern HRESULT		( *p_Mallincam_put_AWBAuxRect )( HToupCam, const RECT* );
extern HRESULT		( *p_Mallincam_get_AEAuxRect )( HToupCam, RECT* );
extern HRESULT		( *p_Mallincam_put_AEAuxRect )( HToupCam, const RECT* );
extern HRESULT		( *p_Mallincam_get_MonoMode )( HToupCam );
extern HRESULT		( *p_Mallincam_get_StillResolution )( HToupCam,
			    unsigned, int*, int* );
extern HRESULT		( *p_Mallincam_get_StillResolutionNumber )( HToupCam );
extern HRESULT		( *p_Mallincam_get_RealTime )( HToupCam, BOOL* );
extern HRESULT		( *p_Mallincam_put_RealTime )( HToupCam, BOOL );
extern HRESULT		( *p_Mallincam_Flush )( HToupCam );
extern HRESULT		( *p_Mallincam_get_Temperature )( HToupCam, short* );
extern HRESULT		( *p_Mallincam_put_Temperature )( HToupCam, short );
extern HRESULT		( *p_Mallincam_get_SerialNumber )( HToupCam, char* );
extern HRESULT		( *p_Mallincam_get_FwVersion )( HToupCam, char* );
extern HRESULT		( *p_Mallincam_get_HwVersion )( HToupCam, char* );
extern HRESULT		( *p_Mallincam_get_ProductionDate )( HToupCam, char* );
extern HRESULT		( *p_Mallincam_get_LevelRange )( HToupCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *p_Mallincam_put_LevelRange )( HToupCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *p_Mallincam_put_ExpoCallback )( HToupCam,
			    PITOUPCAM_EXPOSURE_CALLBACK, void* );
extern HRESULT		( *p_Mallincam_put_ChromeCallback )( HToupCam,
			    PITOUPCAM_CHROME_CALLBACK, void* );
extern HRESULT		( *p_Mallincam_LevelRangeAuto )( HToupCam );
extern HRESULT		( *p_Mallincam_GetHistogram )( HToupCam,
			    PITOUPCAM_HISTOGRAM_CALLBACK, void* );
extern HRESULT		( *p_Mallincam_put_LEDState )( HToupCam, unsigned short,
			    unsigned short, unsigned short );
extern HRESULT		( *p_Mallincam_read_EEPROM )( HToupCam, unsigned,
			    unsigned char*, unsigned );
extern HRESULT		( *p_Mallincam_write_EEPROM )( HToupCam, unsigned,
			    const unsigned char*, unsigned );
extern HRESULT		( *p_Mallincam_get_Option )( HToupCam, unsigned,
			    unsigned* );
extern HRESULT		( *p_Mallincam_put_Option )( HToupCam, unsigned,
			    unsigned );
extern HRESULT		( *p_Mallincam_get_Roi )( HToupCam, unsigned*,
			    unsigned* );
extern HRESULT		( *p_Mallincam_put_Roi )( HToupCam, unsigned, unsigned,
                            unsigned, unsigned );
extern HRESULT		( *p_Mallincam_ST4PlusGuide )( HToupCam, unsigned,
			    unsigned );
extern HRESULT		( *p_Mallincam_ST4PlusGuideState )( HToupCam );
extern double		( *p_Mallincam_calc_ClarityFactor )( const void*, int,
			    unsigned, unsigned );
extern void		( *p_Mallincam_deBayer )( unsigned, int, int,
			    const void*, void*, unsigned char );
extern void		( *p_Mallincam_HotPlug )( PTOUPCAM_HOTPLUG, void* );


#endif	/* OA_MALLINCAM_OACAM_H */
