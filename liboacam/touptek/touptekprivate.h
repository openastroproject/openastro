/*****************************************************************************
 *
 * private.h -- private header for Touptek-based camera API
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

#ifndef OA_TOUPTEK_PRIVATE_H
#define OA_TOUPTEK_PRIVATE_H

#include <touptek-conf.h>

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

extern int				TT_FUNC( _, InitLibraryFunctionPointers )( void );

extern const char*	( *TT_LIB_FUNC( Version ))();
extern unsigned		( *TT_LIB_FUNC( EnumV2 ))( TT_VAR_TYPE ( InstV2* ));
extern TT_HANDLE		( *TT_LIB_FUNC( Open ))( const char* );
extern TT_HANDLE		( *TT_LIB_FUNC( OpenByIndex ))( unsigned );
void			( *TT_LIB_FUNC( Close ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( StartPullModeWithCallback ))( TT_HANDLE,
			    TT_FUNC_TYPE( P, EVENT_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_FUNC( PullImageV2 ))( TT_HANDLE, void*, int,
			    TT_VAR_TYPE( FrameInfoV2* ));
extern HRESULT		( *TT_LIB_FUNC( PullStillImageV2 ))( TT_HANDLE, void*, int,
			    TT_VAR_TYPE( FrameInfoV2* ));
extern HRESULT		( *TT_LIB_FUNC( StartPushModeV2 ))( TT_HANDLE,
			    TT_FUNC_TYPE( P, DATA_CALLBACK_V2 ), void* );
extern HRESULT		( *TT_LIB_FUNC( Stop ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( Pause ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( Snap ))( TT_HANDLE, unsigned );
extern HRESULT		( *TT_LIB_FUNC( Trigger ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( get_Size ))( TT_HANDLE, int*, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Size ))( TT_HANDLE, int, int );
extern HRESULT		( *TT_LIB_FUNC( get_eSize ))( TT_HANDLE, unsigned* );
extern HRESULT		( *TT_LIB_FUNC( put_eSize ))( TT_HANDLE, unsigned );
extern HRESULT		( *TT_LIB_FUNC( get_Resolution ))( TT_HANDLE, unsigned,
			    int*, int* );
extern HRESULT		( *TT_LIB_FUNC( get_ResolutionNumber ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( get_ResolutionRatio ))( TT_HANDLE, unsigned,
			    int*, int* );
extern HRESULT		( *TT_LIB_FUNC( get_RawFormat ))( TT_HANDLE, unsigned*,
			    unsigned* );
extern HRESULT		( *TT_LIB_FUNC( get_AutoExpoEnable ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( get_AutoExpoTarget ))( TT_HANDLE,
			    unsigned short* );
extern HRESULT		( *TT_LIB_FUNC( put_AutoExpoEnable ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( put_AutoExpoTarget ))( TT_HANDLE,
			    unsigned short );
extern HRESULT		( *TT_LIB_FUNC( get_ExpoTime ))( TT_HANDLE, unsigned* );
extern HRESULT		( *TT_LIB_FUNC( get_ExpTimeRange ))( TT_HANDLE, unsigned*,
			    unsigned*, unsigned* );
extern HRESULT		( *TT_LIB_FUNC( put_ExpoTime ))( TT_HANDLE, unsigned );
extern HRESULT		( *TT_LIB_FUNC( put_MaxAutoExpoTimeAGain ))( TT_HANDLE,
			    unsigned, unsigned short );
extern HRESULT		( *TT_LIB_FUNC( get_ExpoAGain ))( TT_HANDLE,
			    unsigned short* );
extern HRESULT		( *TT_LIB_FUNC( put_ExpoAGain ))( TT_HANDLE,
			    unsigned short );
extern HRESULT		( *TT_LIB_FUNC( get_ExpoAGainRange ))( TT_HANDLE,
			    unsigned short*, unsigned short*, unsigned short* );
extern HRESULT		( *TT_LIB_FUNC( AwbInit ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, WHITEBALANCE_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_FUNC( AwbOnePush ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, TEMPTINT_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_FUNC( get_TempTint ))( TT_HANDLE, int*, int* );
extern HRESULT		( *TT_LIB_FUNC( put_TempTint ))( TT_HANDLE, int, int );
extern HRESULT		( *TT_LIB_FUNC( get_WhiteBalanceGain ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_WhiteBalanceGain ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( get_Hue ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Hue ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_Saturation ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Saturation ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_Brightness ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Brightness ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_Contrast ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Contrast ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_Gamma ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Gamma ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_Chrome ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Chrome ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_VFlip ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_VFlip ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_HFlip ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_HFlip ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_Negative ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Negative ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_MaxSpeed ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( get_Speed ))( TT_HANDLE, unsigned short* );
extern HRESULT		( *TT_LIB_FUNC( put_Speed ))( TT_HANDLE, unsigned short );
extern HRESULT		( *TT_LIB_FUNC( get_MaxBitDepth ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( get_HZ ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_HZ ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_Mode ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_Mode ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( get_AWBAuxRect ))( TT_HANDLE, RECT* );
extern HRESULT		( *TT_LIB_FUNC( put_AWBAuxRect ))( TT_HANDLE, const RECT* );
extern HRESULT		( *TT_LIB_FUNC( get_AEAuxRect ))( TT_HANDLE, RECT* );
extern HRESULT		( *TT_LIB_FUNC( put_AEAuxRect ))( TT_HANDLE, const RECT* );
extern HRESULT		( *TT_LIB_FUNC( get_MonoMode ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( get_StillResolution ))( TT_HANDLE,
			    unsigned, int*, int* );
extern HRESULT		( *TT_LIB_FUNC( get_StillResolutionNumber ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( get_RealTime ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_FUNC( put_RealTime ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_FUNC( Flush ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( get_Temperature ))( TT_HANDLE, short* );
extern HRESULT		( *TT_LIB_FUNC( put_Temperature ))( TT_HANDLE, short );
extern HRESULT		( *TT_LIB_FUNC( get_SerialNumber ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_FUNC( get_FwVersion ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_FUNC( get_HwVersion ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_FUNC( get_ProductionDate ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_FUNC( get_LevelRange ))( TT_HANDLE,
			    unsigned short*, unsigned short* );
extern HRESULT		( *TT_LIB_FUNC( put_LevelRange ))( TT_HANDLE,
			    unsigned short*, unsigned short* );
extern HRESULT		( *TT_LIB_FUNC( put_ExpoCallback ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, EXPOSURE_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_FUNC( put_ChromeCallback ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, CHROME_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_FUNC( LevelRangeAuto ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_FUNC( GetHistogram ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, HISTOGRAM_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_FUNC( put_LEDState ))( TT_HANDLE, unsigned short,
			    unsigned short, unsigned short );
extern HRESULT		( *TT_LIB_FUNC( read_EEPROM ))( TT_HANDLE, unsigned,
			    unsigned char*, unsigned );
extern HRESULT		( *TT_LIB_FUNC( write_EEPROM ))( TT_HANDLE, unsigned,
			    const unsigned char*, unsigned );
extern HRESULT		( *TT_LIB_FUNC( get_Option ))( TT_HANDLE, unsigned,
			    unsigned* );
extern HRESULT		( *TT_LIB_FUNC( put_Option ))( TT_HANDLE, unsigned,
			    unsigned );
extern HRESULT		( *TT_LIB_FUNC( get_Roi ))( TT_HANDLE, unsigned*,
			    unsigned* );
extern HRESULT		( *TT_LIB_FUNC( put_Roi ))( TT_HANDLE, unsigned, unsigned,
                            unsigned, unsigned );
extern HRESULT		( *TT_LIB_FUNC( ST4PlusGuide ))( TT_HANDLE, unsigned,
			    unsigned );
extern HRESULT		( *TT_LIB_FUNC( ST4PlusGuideState ))( TT_HANDLE );
extern double		( *TT_LIB_FUNC( calc_ClarityFactor ))( const void*, int,
			    unsigned, unsigned );
extern void		( *TT_LIB_FUNC( deBayerV2 ))( unsigned, int, int,
			    const void*, void*, unsigned char );
extern void		( *TT_LIB_FUNC( HotPlug ))( TT_FUNC_TYPE( P, HOTPLUG ), void* );

#endif	/* OA_TOUPTEK_PRIVATE_H */
