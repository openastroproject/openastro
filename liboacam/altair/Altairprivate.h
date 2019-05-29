/*****************************************************************************
 *
 * Altairprivate.h -- private header for Altair camera API
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

#ifndef OA_ALTAIRCAM_PRIVATE_H
#define OA_ALTAIRCAM_PRIVATE_H

#include <altaircam.h>

// Handle change of name of flags and options
#ifndef TOUPCAM_FLAG_TEC_ONOFF
#ifdef TOUPCAM_FLAG_COOLERONOFF
#define TOUPCAM_FLAG_TEC_ONOFF TOUPCAM_FLAG_COOLERONOFF
#endif
#endif

#ifndef TOUPCAM_OPTION_TEC
#ifdef TOUPCAM_OPTION_COOLER
#define TOUPCAM_OPTION_TEC TOUPCAM_OPTION_COOLER
#endif
#endif

#ifndef TOUPCAM_OPTION_RGB
#ifdef TOUPCAM_OPTION_RGB48
#define TOUPCAM_OPTION_RGB TOUPCAM_OPTION_RGB48
#endif
#endif

extern int				_altairInitLibraryFunctionPointers ( void );

extern const char*	( *p_Altaircam_Version )();
extern unsigned		( *p_Altaircam_EnumV2 )( AltaircamInstV2* );
extern HAltairCam		( *p_Altaircam_Open )( const char* );
extern HAltairCam		( *p_Altaircam_OpenByIndex )( unsigned );
void			( *p_Altaircam_Close )( HAltairCam );
extern HRESULT		( *p_Altaircam_StartPullModeWithCallback )( HAltairCam,
			    PALTAIRCAM_EVENT_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_PullImageV2 )( HAltairCam, void*, int,
			    AltaircamFrameInfoV2* );
extern HRESULT		( *p_Altaircam_PullStillImageV2 )( HAltairCam, void*, int,
			    AltaircamFrameInfoV2* );
extern HRESULT		( *p_Altaircam_StartPushModeV2 )( HAltairCam,
			    PALTAIRCAM_DATA_CALLBACK_V2, void* );
extern HRESULT		( *p_Altaircam_Stop )( HAltairCam );
extern HRESULT		( *p_Altaircam_Pause )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_Snap )( HAltairCam, unsigned );
extern HRESULT		( *p_Altaircam_Trigger )( HAltairCam );
extern HRESULT		( *p_Altaircam_get_Size )( HAltairCam, int*, int* );
extern HRESULT		( *p_Altaircam_put_Size )( HAltairCam, int, int );
extern HRESULT		( *p_Altaircam_get_eSize )( HAltairCam, unsigned* );
extern HRESULT		( *p_Altaircam_put_eSize )( HAltairCam, unsigned );
extern HRESULT		( *p_Altaircam_get_Resolution )( HAltairCam, unsigned,
			    int*, int* );
extern HRESULT		( *p_Altaircam_get_ResolutionNumber )( HAltairCam );
extern HRESULT		( *p_Altaircam_get_ResolutionRatio )( HAltairCam, unsigned,
			    int*, int* );
extern HRESULT		( *p_Altaircam_get_RawFormat )( HAltairCam, unsigned*,
			    unsigned* );
extern HRESULT		( *p_Altaircam_get_AutoExpoEnable )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_get_AutoExpoTarget )( HAltairCam,
			    unsigned short* );
extern HRESULT		( *p_Altaircam_put_AutoExpoEnable )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_put_AutoExpoTarget )( HAltairCam,
			    unsigned short );
extern HRESULT		( *p_Altaircam_get_ExpoTime )( HAltairCam, unsigned* );
extern HRESULT		( *p_Altaircam_get_ExpTimeRange )( HAltairCam, unsigned*,
			    unsigned*, unsigned* );
extern HRESULT		( *p_Altaircam_put_ExpoTime )( HAltairCam, unsigned );
extern HRESULT		( *p_Altaircam_put_MaxAutoExpoTimeAGain )( HAltairCam,
			    unsigned, unsigned short );
extern HRESULT		( *p_Altaircam_get_ExpoAGain )( HAltairCam,
			    unsigned short* );
extern HRESULT		( *p_Altaircam_put_ExpoAGain )( HAltairCam,
			    unsigned short );
extern HRESULT		( *p_Altaircam_get_ExpoAGainRange )( HAltairCam,
			    unsigned short*, unsigned short*, unsigned short* );
extern HRESULT		( *p_Altaircam_AwbInit )( HAltairCam,
			    PIALTAIRCAM_WHITEBALANCE_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_AwbOnePush )( HAltairCam,
			    PIALTAIRCAM_TEMPTINT_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_get_TempTint )( HAltairCam, int*, int* );
extern HRESULT		( *p_Altaircam_put_TempTint )( HAltairCam, int, int );
extern HRESULT		( *p_Altaircam_get_WhiteBalanceGain )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_WhiteBalanceGain )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_get_Hue )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Hue )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_Saturation )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Saturation )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_Brightness )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Brightness )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_Contrast )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Contrast )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_Gamma )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Gamma )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_Chrome )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Chrome )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_VFlip )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_VFlip )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_HFlip )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_HFlip )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_Negative )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Negative )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_MaxSpeed )( HAltairCam );
extern HRESULT		( *p_Altaircam_get_Speed )( HAltairCam, unsigned short* );
extern HRESULT		( *p_Altaircam_put_Speed )( HAltairCam, unsigned short );
extern HRESULT		( *p_Altaircam_get_MaxBitDepth )( HAltairCam );
extern HRESULT		( *p_Altaircam_get_HZ )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_HZ )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_Mode )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_Mode )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_get_AWBAuxRect )( HAltairCam, RECT* );
extern HRESULT		( *p_Altaircam_put_AWBAuxRect )( HAltairCam, const RECT* );
extern HRESULT		( *p_Altaircam_get_AEAuxRect )( HAltairCam, RECT* );
extern HRESULT		( *p_Altaircam_put_AEAuxRect )( HAltairCam, const RECT* );
extern HRESULT		( *p_Altaircam_get_MonoMode )( HAltairCam );
extern HRESULT		( *p_Altaircam_get_StillResolution )( HAltairCam,
			    unsigned, int*, int* );
extern HRESULT		( *p_Altaircam_get_StillResolutionNumber )( HAltairCam );
extern HRESULT		( *p_Altaircam_get_RealTime )( HAltairCam, int* );
extern HRESULT		( *p_Altaircam_put_RealTime )( HAltairCam, int );
extern HRESULT		( *p_Altaircam_Flush )( HAltairCam );
extern HRESULT		( *p_Altaircam_get_Temperature )( HAltairCam, short* );
extern HRESULT		( *p_Altaircam_put_Temperature )( HAltairCam, short );
extern HRESULT		( *p_Altaircam_get_SerialNumber )( HAltairCam, char* );
extern HRESULT		( *p_Altaircam_get_FwVersion )( HAltairCam, char* );
extern HRESULT		( *p_Altaircam_get_HwVersion )( HAltairCam, char* );
extern HRESULT		( *p_Altaircam_get_ProductionDate )( HAltairCam, char* );
extern HRESULT		( *p_Altaircam_get_LevelRange )( HAltairCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *p_Altaircam_put_LevelRange )( HAltairCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *p_Altaircam_put_ExpoCallback )( HAltairCam,
			    PIALTAIRCAM_EXPOSURE_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_put_ChromeCallback )( HAltairCam,
			    PIALTAIRCAM_CHROME_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_LevelRangeAuto )( HAltairCam );
extern HRESULT		( *p_Altaircam_GetHistogram )( HAltairCam,
			    PIALTAIRCAM_HISTOGRAM_CALLBACK, void* );
extern HRESULT		( *p_Altaircam_put_LEDState )( HAltairCam, unsigned short,
			    unsigned short, unsigned short );
extern HRESULT		( *p_Altaircam_read_EEPROM )( HAltairCam, unsigned,
			    unsigned char*, unsigned );
extern HRESULT		( *p_Altaircam_write_EEPROM )( HAltairCam, unsigned,
			    const unsigned char*, unsigned );
extern HRESULT		( *p_Altaircam_get_Option )( HAltairCam, unsigned,
			    unsigned* );
extern HRESULT		( *p_Altaircam_put_Option )( HAltairCam, unsigned,
			    unsigned );
extern HRESULT		( *p_Altaircam_get_Roi )( HAltairCam, unsigned*,
			    unsigned* );
extern HRESULT		( *p_Altaircam_put_Roi )( HAltairCam, unsigned, unsigned,
                            unsigned, unsigned );
extern HRESULT		( *p_Altaircam_ST4PlusGuide )( HAltairCam, unsigned,
			    unsigned );
extern HRESULT		( *p_Altaircam_ST4PlusGuideState )( HAltairCam );
extern double		( *p_Altaircam_calc_ClarityFactor )( const void*, int,
			    unsigned, unsigned );
extern void		( *p_Altaircam_deBayerV2 )( unsigned, int, int,
			    const void*, void*, unsigned char );
extern void		( *p_Altaircam_HotPlug )( PALTAIRCAM_HOTPLUG, void* );


#endif	/* OA_ALTAIRCAM_PRIVATE_H */
