/*****************************************************************************
 *
 * Altairoacam.h -- header for Altair camera API
 *
 * Copyright 2016,2017,2018 James Fidell (james@openastroproject.org)
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

#ifndef OA_ALTAIRCAM_OACAM_H
#define OA_ALTAIRCAM_OACAM_H

#include <toupcam.h>

extern int		oaAltairGetCameras ( CAMERA_LIST*, int );
extern oaCamera*	oaAltairInitCamera ( oaCameraDevice* );

extern int		oaAltairCloseCamera ( oaCamera* );

extern int		oaAltairCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaAltairCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaAltairCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaAltairCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int              oaAltairCameraGetControlDiscreteSet ( oaCamera*, int,
                                int32_t*, int64_t** );

extern int		oaAltairCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int), void* );
extern int		oaAltairCameraStopStreaming ( oaCamera* );
extern int		oaAltairCameraIsStreaming ( oaCamera* );

extern int		oaAltairCameraSetResolution ( oaCamera*, int, int );
extern int		oaAltairCameraSetROI ( oaCamera*, int, int );
extern int		oaAltairCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamAltaircontroller ( void* );
extern void*		oacamAltaircallbackHandler ( void* );

extern const FRAMESIZES* oaAltairCameraGetFrameSizes ( oaCamera* );
extern int		oaAltairCameraGetFramePixelFormat ( oaCamera* );

extern const char*      oaAltairCameraGetMenuString ( oaCamera*, int, int );


extern const char*	( *p_Altaircam_Version )();
extern unsigned		( *p_Altaircam_Enum )( ToupcamInst* );
extern HToupCam		( *p_Altaircam_Open )( const char* );
extern HToupCam		( *p_Altaircam_OpenByIndex )( unsigned );
void			( *p_Altaircam_Close )( HToupCam );
extern HRESULT		( *p_Altaircam_StartPullModeWithCallback )( HToupCam,
			    PTOUPCAM_EVENT_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_PullImage )( HToupCam, void*, int,
			    unsigned*, unsigned* );
extern HRESULT		( *p_Altaircam_PullStillImage )( HToupCam, void*, int,
			    unsigned*, unsigned* );
extern HRESULT		( *p_Altaircam_StartPushMode )( HToupCam,
			    PTOUPCAM_DATA_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_Stop )( HToupCam );
extern HRESULT		( *p_Altaircam_Pause )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_Snap )( HToupCam, unsigned );
extern HRESULT		( *p_Altaircam_Trigger )( HToupCam );
extern HRESULT		( *p_Altaircam_get_Size )( HToupCam, int*, int* );
extern HRESULT		( *p_Altaircam_put_Size )( HToupCam, int, int );
extern HRESULT		( *p_Altaircam_get_eSize )( HToupCam, unsigned* );
extern HRESULT		( *p_Altaircam_put_eSize )( HToupCam, unsigned );
extern HRESULT		( *p_Altaircam_get_Resolution )( HToupCam, unsigned,
			    int*, int* );
extern HRESULT		( *p_Altaircam_get_ResolutionNumber )( HToupCam );
extern HRESULT		( *p_Altaircam_get_ResolutionRatio )( HToupCam, unsigned,
			    int*, int* );
extern HRESULT		( *p_Altaircam_get_RawFormat )( HToupCam, unsigned*,
			    unsigned* );
extern HRESULT		( *p_Altaircam_get_AutoExpoEnable )( HToupCam, BOOL* );
extern HRESULT		( *p_Altaircam_get_AutoExpoTarget )( HToupCam,
			    unsigned short* );
extern HRESULT		( *p_Altaircam_put_AutoExpoEnable )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_put_AutoExpoTarget )( HToupCam,
			    unsigned short );
extern HRESULT		( *p_Altaircam_get_ExpoTime )( HToupCam, unsigned* );
extern HRESULT		( *p_Altaircam_get_ExpTimeRange )( HToupCam, unsigned*,
			    unsigned*, unsigned* );
extern HRESULT		( *p_Altaircam_put_ExpoTime )( HToupCam, unsigned );
extern HRESULT		( *p_Altaircam_put_MaxAutoExpoTimeAGain )( HToupCam,
			    unsigned, unsigned short );
extern HRESULT		( *p_Altaircam_get_ExpoAGain )( HToupCam,
			    unsigned short* );
extern HRESULT		( *p_Altaircam_put_ExpoAGain )( HToupCam,
			    unsigned short );
extern HRESULT		( *p_Altaircam_get_ExpoAGainRange )( HToupCam,
			    unsigned short*, unsigned short*, unsigned short* );
extern HRESULT		( *p_Altaircam_AwbInit )( HToupCam,
			    PITOUPCAM_WHITEBALANCE_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_AwbOnePush )( HToupCam,
			    PITOUPCAM_TEMPTINT_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_get_TempTint )( HToupCam, int*, int* );
extern HRESULT		( *p_Altaircam_put_TempTint )( HToupCam, int, int );
extern HRESULT		( *p_Altaircam_get_WhiteBalanceGain )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_put_WhiteBalanceGain )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_get_Hue )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_put_Hue )( HToupCam, int );
extern HRESULT		( *p_Altaircam_get_Saturation )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_put_Saturation )( HToupCam, int );
extern HRESULT		( *p_Altaircam_get_Brightness )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_put_Brightness )( HToupCam, int );
extern HRESULT		( *p_Altaircam_get_Contrast )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_put_Contrast )( HToupCam, int );
extern HRESULT		( *p_Altaircam_get_Gamma )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_put_Gamma )( HToupCam, int );
extern HRESULT		( *p_Altaircam_get_Chrome )( HToupCam, BOOL* );
extern HRESULT		( *p_Altaircam_put_Chrome )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_get_VFlip )( HToupCam, BOOL* );
extern HRESULT		( *p_Altaircam_put_VFlip )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_get_HFlip )( HToupCam, BOOL* );
extern HRESULT		( *p_Altaircam_put_HFlip )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_get_Negative )( HToupCam, BOOL* );
extern HRESULT		( *p_Altaircam_put_Negative )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_get_MaxSpeed )( HToupCam );
extern HRESULT		( *p_Altaircam_get_Speed )( HToupCam, unsigned short* );
extern HRESULT		( *p_Altaircam_put_Speed )( HToupCam, unsigned short );
extern HRESULT		( *p_Altaircam_get_MaxBitDepth )( HToupCam );
extern HRESULT		( *p_Altaircam_get_HZ )( HToupCam, int* );
extern HRESULT		( *p_Altaircam_put_HZ )( HToupCam, int );
extern HRESULT		( *p_Altaircam_get_Mode )( HToupCam, BOOL* );
extern HRESULT		( *p_Altaircam_put_Mode )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_get_AWBAuxRect )( HToupCam, RECT* );
extern HRESULT		( *p_Altaircam_put_AWBAuxRect )( HToupCam, const RECT* );
extern HRESULT		( *p_Altaircam_get_AEAuxRect )( HToupCam, RECT* );
extern HRESULT		( *p_Altaircam_put_AEAuxRect )( HToupCam, const RECT* );
extern HRESULT		( *p_Altaircam_get_MonoMode )( HToupCam );
extern HRESULT		( *p_Altaircam_get_StillResolution )( HToupCam,
			    unsigned, int*, int* );
extern HRESULT		( *p_Altaircam_get_StillResolutionNumber )( HToupCam );
extern HRESULT		( *p_Altaircam_get_RealTime )( HToupCam, BOOL* );
extern HRESULT		( *p_Altaircam_put_RealTime )( HToupCam, BOOL );
extern HRESULT		( *p_Altaircam_Flush )( HToupCam );
extern HRESULT		( *p_Altaircam_get_Temperature )( HToupCam, short* );
extern HRESULT		( *p_Altaircam_put_Temperature )( HToupCam, short );
extern HRESULT		( *p_Altaircam_get_SerialNumber )( HToupCam, char* );
extern HRESULT		( *p_Altaircam_get_FwVersion )( HToupCam, char* );
extern HRESULT		( *p_Altaircam_get_HwVersion )( HToupCam, char* );
extern HRESULT		( *p_Altaircam_get_ProductionDate )( HToupCam, char* );
extern HRESULT		( *p_Altaircam_get_LevelRange )( HToupCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *p_Altaircam_put_LevelRange )( HToupCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *p_Altaircam_put_ExpoCallback )( HToupCam,
			    PITOUPCAM_EXPOSURE_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_put_ChromeCallback )( HToupCam,
			    PITOUPCAM_CHROME_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_LevelRangeAuto )( HToupCam );
extern HRESULT		( *p_Altaircam_GetHistogram )( HToupCam,
			    PITOUPCAM_HISTOGRAM_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_put_LEDState )( HToupCam, unsigned short,
			    unsigned short, unsigned short );
extern HRESULT		( *p_Altaircam_read_EEPROM )( HToupCam, unsigned,
			    unsigned char*, unsigned );
extern HRESULT		( *p_Altaircam_write_EEPROM )( HToupCam, unsigned,
			    const unsigned char*, unsigned );
extern HRESULT		( *p_Altaircam_get_Option )( HToupCam, unsigned,
			    unsigned* );
extern HRESULT		( *p_Altaircam_put_Option )( HToupCam, unsigned,
			    unsigned );
extern HRESULT		( *p_Altaircam_get_Roi )( HToupCam, unsigned*,
			    unsigned* );
extern HRESULT		( *p_Altaircam_put_Roi )( HToupCam, unsigned, unsigned,
                            unsigned, unsigned );
extern HRESULT		( *p_Altaircam_ST4PlusGuide )( HToupCam, unsigned,
			    unsigned );
extern HRESULT		( *p_Altaircam_ST4PlusGuideState )( HToupCam );
extern double		( *p_Altaircam_calc_ClarityFactor )( const void*, int,
			    unsigned, unsigned );
extern void		( *p_Altaircam_deBayer )( unsigned, int, int,
			    const void*, void*, unsigned char );
extern void		( *p_Altaircam_HotPlug )( PTOUPCAM_HOTPLUG, void* );


#endif	/* OA_ALTAIRCAM_OACAM_H */
