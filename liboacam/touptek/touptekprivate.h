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

extern const char*	( *TT_LIB_PTR( Version ))();
extern unsigned		( *TT_LIB_PTR( EnumV2 ))( TT_VAR_TYPE ( InstV2* ));
extern TT_HANDLE		( *TT_LIB_PTR( Open ))( const char* );
extern TT_HANDLE		( *TT_LIB_PTR( OpenByIndex ))( unsigned );
void			( *TT_LIB_PTR( Close ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( StartPullModeWithCallback ))( TT_HANDLE,
			    TT_FUNC_TYPE( P, EVENT_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_PTR( PullImageV2 ))( TT_HANDLE, void*, int,
			    TT_VAR_TYPE( FrameInfoV2* ));
extern HRESULT		( *TT_LIB_PTR( PullStillImageV2 ))( TT_HANDLE, void*, int,
			    TT_VAR_TYPE( FrameInfoV2* ));
extern HRESULT		( *TT_LIB_PTR( StartPushModeV2 ))( TT_HANDLE,
			    TT_FUNC_TYPE( P, DATA_CALLBACK_V2 ), void* );
extern HRESULT		( *TT_LIB_PTR( Stop ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( Pause ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( Snap ))( TT_HANDLE, unsigned );
extern HRESULT		( *TT_LIB_PTR( Trigger ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( get_Size ))( TT_HANDLE, int*, int* );
extern HRESULT		( *TT_LIB_PTR( put_Size ))( TT_HANDLE, int, int );
extern HRESULT		( *TT_LIB_PTR( get_eSize ))( TT_HANDLE, unsigned* );
extern HRESULT		( *TT_LIB_PTR( put_eSize ))( TT_HANDLE, unsigned );
extern HRESULT		( *TT_LIB_PTR( get_Resolution ))( TT_HANDLE, unsigned,
			    int*, int* );
extern HRESULT		( *TT_LIB_PTR( get_ResolutionNumber ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( get_ResolutionRatio ))( TT_HANDLE, unsigned,
			    int*, int* );
extern HRESULT		( *TT_LIB_PTR( get_RawFormat ))( TT_HANDLE, unsigned*,
			    unsigned* );
extern HRESULT		( *TT_LIB_PTR( get_AutoExpoEnable ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( get_AutoExpoTarget ))( TT_HANDLE,
			    unsigned short* );
extern HRESULT		( *TT_LIB_PTR( put_AutoExpoEnable ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( put_AutoExpoTarget ))( TT_HANDLE,
			    unsigned short );
extern HRESULT		( *TT_LIB_PTR( get_ExpoTime ))( TT_HANDLE, unsigned* );
extern HRESULT		( *TT_LIB_PTR( get_ExpTimeRange ))( TT_HANDLE, unsigned*,
			    unsigned*, unsigned* );
extern HRESULT		( *TT_LIB_PTR( put_ExpoTime ))( TT_HANDLE, unsigned );
extern HRESULT		( *TT_LIB_PTR( put_MaxAutoExpoTimeAGain ))( TT_HANDLE,
			    unsigned, unsigned short );
extern HRESULT		( *TT_LIB_PTR( get_ExpoAGain ))( TT_HANDLE,
			    unsigned short* );
extern HRESULT		( *TT_LIB_PTR( put_ExpoAGain ))( TT_HANDLE,
			    unsigned short );
extern HRESULT		( *TT_LIB_PTR( get_ExpoAGainRange ))( TT_HANDLE,
			    unsigned short*, unsigned short*, unsigned short* );
extern HRESULT		( *TT_LIB_PTR( AwbInit ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, WHITEBALANCE_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_PTR( AwbOnePush ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, TEMPTINT_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_PTR( get_TempTint ))( TT_HANDLE, int*, int* );
extern HRESULT		( *TT_LIB_PTR( put_TempTint ))( TT_HANDLE, int, int );
extern HRESULT		( *TT_LIB_PTR( get_WhiteBalanceGain ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_WhiteBalanceGain ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( get_Hue ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Hue ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_Saturation ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Saturation ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_Brightness ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Brightness ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_Contrast ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Contrast ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_Gamma ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Gamma ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_Chrome ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Chrome ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_VFlip ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_VFlip ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_HFlip ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_HFlip ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_Negative ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Negative ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_MaxSpeed ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( get_Speed ))( TT_HANDLE, unsigned short* );
extern HRESULT		( *TT_LIB_PTR( put_Speed ))( TT_HANDLE, unsigned short );
extern HRESULT		( *TT_LIB_PTR( get_MaxBitDepth ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( get_HZ ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_HZ ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_Mode ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_Mode ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( get_AWBAuxRect ))( TT_HANDLE, RECT* );
extern HRESULT		( *TT_LIB_PTR( put_AWBAuxRect ))( TT_HANDLE, const RECT* );
extern HRESULT		( *TT_LIB_PTR( get_AEAuxRect ))( TT_HANDLE, RECT* );
extern HRESULT		( *TT_LIB_PTR( put_AEAuxRect ))( TT_HANDLE, const RECT* );
extern HRESULT		( *TT_LIB_PTR( get_MonoMode ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( get_StillResolution ))( TT_HANDLE,
			    unsigned, int*, int* );
extern HRESULT		( *TT_LIB_PTR( get_StillResolutionNumber ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( get_RealTime ))( TT_HANDLE, int* );
extern HRESULT		( *TT_LIB_PTR( put_RealTime ))( TT_HANDLE, int );
extern HRESULT		( *TT_LIB_PTR( Flush ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( get_Temperature ))( TT_HANDLE, short* );
extern HRESULT		( *TT_LIB_PTR( put_Temperature ))( TT_HANDLE, short );
extern HRESULT		( *TT_LIB_PTR( get_SerialNumber ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_PTR( get_FwVersion ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_PTR( get_HwVersion ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_PTR( get_ProductionDate ))( TT_HANDLE, char* );
extern HRESULT		( *TT_LIB_PTR( get_LevelRange ))( TT_HANDLE,
			    unsigned short*, unsigned short* );
extern HRESULT		( *TT_LIB_PTR( put_LevelRange ))( TT_HANDLE,
			    unsigned short*, unsigned short* );
extern HRESULT		( *TT_LIB_PTR( put_ExpoCallback ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, EXPOSURE_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_PTR( put_ChromeCallback ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, CHROME_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_PTR( LevelRangeAuto ))( TT_HANDLE );
extern HRESULT		( *TT_LIB_PTR( GetHistogram ))( TT_HANDLE,
			    TT_FUNC_TYPE( PI, HISTOGRAM_CALLBACK ), void* );
extern HRESULT		( *TT_LIB_PTR( put_LEDState ))( TT_HANDLE, unsigned short,
			    unsigned short, unsigned short );
extern HRESULT		( *TT_LIB_PTR( read_EEPROM ))( TT_HANDLE, unsigned,
			    unsigned char*, unsigned );
extern HRESULT		( *TT_LIB_PTR( write_EEPROM ))( TT_HANDLE, unsigned,
			    const unsigned char*, unsigned );
extern HRESULT		( *TT_LIB_PTR( get_Option ))( TT_HANDLE, unsigned,
			    unsigned* );
extern HRESULT		( *TT_LIB_PTR( put_Option ))( TT_HANDLE, unsigned,
			    unsigned );
extern HRESULT		( *TT_LIB_PTR( get_Roi ))( TT_HANDLE, unsigned*,
			    unsigned* );
extern HRESULT		( *TT_LIB_PTR( put_Roi ))( TT_HANDLE, unsigned, unsigned,
                            unsigned, unsigned );
extern HRESULT		( *TT_LIB_PTR( ST4PlusGuide ))( TT_HANDLE, unsigned,
			    unsigned );
extern HRESULT		( *TT_LIB_PTR( ST4PlusGuideState ))( TT_HANDLE );
extern double		( *TT_LIB_PTR( calc_ClarityFactor ))( const void*, int,
			    unsigned, unsigned );
extern void		( *TT_LIB_PTR( deBayerV2 ))( unsigned, int, int,
			    const void*, void*, unsigned char );
extern void		( *TT_LIB_PTR( HotPlug ))( TT_FUNC_TYPE( P, HOTPLUG ), void* );

#endif	/* OA_TOUPTEK_PRIVATE_H */
