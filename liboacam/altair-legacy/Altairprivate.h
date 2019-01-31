/*****************************************************************************
 *
 * Altairprivate.h -- private header for legacy Altair camera API
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

#ifndef OA_ALTAIRCAM_LEGACY_PRIVATE_H
#define OA_ALTAIRCAM_LEGACY_PRIVATE_H

#include <altaircamlegacy.h>

extern const char*	( *pl_Altaircam_Version )();
extern unsigned		( *pl_Altaircam_Enum )( ToupcamInst* );
extern HToupCam		( *pl_Altaircam_Open )( const char* );
extern HToupCam		( *pl_Altaircam_OpenByIndex )( unsigned );
void			( *pl_Altaircam_Close )( HToupCam );
extern HRESULT		( *pl_Altaircam_StartPullModeWithCallback )( HToupCam,
			    PTOUPCAM_EVENT_CALLBACK, void* );
extern HRESULT		( *pl_Altaircam_PullImage )( HToupCam, void*, int,
			    unsigned*, unsigned* );
extern HRESULT		( *pl_Altaircam_PullStillImage )( HToupCam, void*, int,
			    unsigned*, unsigned* );
extern HRESULT		( *pl_Altaircam_StartPushMode )( HToupCam,
			    PTOUPCAM_DATA_CALLBACK, void* );
extern HRESULT		( *pl_Altaircam_Stop )( HToupCam );
extern HRESULT		( *pl_Altaircam_Pause )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_Snap )( HToupCam, unsigned );
extern HRESULT		( *pl_Altaircam_Trigger )( HToupCam );
extern HRESULT		( *pl_Altaircam_get_Size )( HToupCam, int*, int* );
extern HRESULT		( *pl_Altaircam_put_Size )( HToupCam, int, int );
extern HRESULT		( *pl_Altaircam_get_eSize )( HToupCam, unsigned* );
extern HRESULT		( *pl_Altaircam_put_eSize )( HToupCam, unsigned );
extern HRESULT		( *pl_Altaircam_get_Resolution )( HToupCam, unsigned,
			    int*, int* );
extern HRESULT		( *pl_Altaircam_get_ResolutionNumber )( HToupCam );
extern HRESULT		( *pl_Altaircam_get_ResolutionRatio )( HToupCam, unsigned,
			    int*, int* );
extern HRESULT		( *pl_Altaircam_get_RawFormat )( HToupCam, unsigned*,
			    unsigned* );
extern HRESULT		( *pl_Altaircam_get_AutoExpoEnable )( HToupCam, BOOL* );
extern HRESULT		( *pl_Altaircam_get_AutoExpoTarget )( HToupCam,
			    unsigned short* );
extern HRESULT		( *pl_Altaircam_put_AutoExpoEnable )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_put_AutoExpoTarget )( HToupCam,
			    unsigned short );
extern HRESULT		( *pl_Altaircam_get_ExpoTime )( HToupCam, unsigned* );
extern HRESULT		( *pl_Altaircam_get_ExpTimeRange )( HToupCam, unsigned*,
			    unsigned*, unsigned* );
extern HRESULT		( *pl_Altaircam_put_ExpoTime )( HToupCam, unsigned );
extern HRESULT		( *pl_Altaircam_put_MaxAutoExpoTimeAGain )( HToupCam,
			    unsigned, unsigned short );
extern HRESULT		( *pl_Altaircam_get_ExpoAGain )( HToupCam,
			    unsigned short* );
extern HRESULT		( *pl_Altaircam_put_ExpoAGain )( HToupCam,
			    unsigned short );
extern HRESULT		( *pl_Altaircam_get_ExpoAGainRange )( HToupCam,
			    unsigned short*, unsigned short*, unsigned short* );
extern HRESULT		( *pl_Altaircam_AwbInit )( HToupCam,
			    PITOUPCAM_WHITEBALANCE_CALLBACK, void* );
extern HRESULT		( *pl_Altaircam_AwbOnePush )( HToupCam,
			    PITOUPCAM_TEMPTINT_CALLBACK, void* );
extern HRESULT		( *pl_Altaircam_get_TempTint )( HToupCam, int*, int* );
extern HRESULT		( *pl_Altaircam_put_TempTint )( HToupCam, int, int );
extern HRESULT		( *pl_Altaircam_get_WhiteBalanceGain )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_put_WhiteBalanceGain )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_get_Hue )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_put_Hue )( HToupCam, int );
extern HRESULT		( *pl_Altaircam_get_Saturation )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_put_Saturation )( HToupCam, int );
extern HRESULT		( *pl_Altaircam_get_Brightness )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_put_Brightness )( HToupCam, int );
extern HRESULT		( *pl_Altaircam_get_Contrast )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_put_Contrast )( HToupCam, int );
extern HRESULT		( *pl_Altaircam_get_Gamma )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_put_Gamma )( HToupCam, int );
extern HRESULT		( *pl_Altaircam_get_Chrome )( HToupCam, BOOL* );
extern HRESULT		( *pl_Altaircam_put_Chrome )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_get_VFlip )( HToupCam, BOOL* );
extern HRESULT		( *pl_Altaircam_put_VFlip )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_get_HFlip )( HToupCam, BOOL* );
extern HRESULT		( *pl_Altaircam_put_HFlip )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_get_Negative )( HToupCam, BOOL* );
extern HRESULT		( *pl_Altaircam_put_Negative )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_get_MaxSpeed )( HToupCam );
extern HRESULT		( *pl_Altaircam_get_Speed )( HToupCam, unsigned short* );
extern HRESULT		( *pl_Altaircam_put_Speed )( HToupCam, unsigned short );
extern HRESULT		( *pl_Altaircam_get_MaxBitDepth )( HToupCam );
extern HRESULT		( *pl_Altaircam_get_HZ )( HToupCam, int* );
extern HRESULT		( *pl_Altaircam_put_HZ )( HToupCam, int );
extern HRESULT		( *pl_Altaircam_get_Mode )( HToupCam, BOOL* );
extern HRESULT		( *pl_Altaircam_put_Mode )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_get_AWBAuxRect )( HToupCam, RECT* );
extern HRESULT		( *pl_Altaircam_put_AWBAuxRect )( HToupCam, const RECT* );
extern HRESULT		( *pl_Altaircam_get_AEAuxRect )( HToupCam, RECT* );
extern HRESULT		( *pl_Altaircam_put_AEAuxRect )( HToupCam, const RECT* );
extern HRESULT		( *pl_Altaircam_get_MonoMode )( HToupCam );
extern HRESULT		( *pl_Altaircam_get_StillResolution )( HToupCam,
			    unsigned, int*, int* );
extern HRESULT		( *pl_Altaircam_get_StillResolutionNumber )( HToupCam );
extern HRESULT		( *pl_Altaircam_get_RealTime )( HToupCam, BOOL* );
extern HRESULT		( *pl_Altaircam_put_RealTime )( HToupCam, BOOL );
extern HRESULT		( *pl_Altaircam_Flush )( HToupCam );
extern HRESULT		( *pl_Altaircam_get_Temperature )( HToupCam, short* );
extern HRESULT		( *pl_Altaircam_put_Temperature )( HToupCam, short );
extern HRESULT		( *pl_Altaircam_get_SerialNumber )( HToupCam, char* );
extern HRESULT		( *pl_Altaircam_get_FwVersion )( HToupCam, char* );
extern HRESULT		( *pl_Altaircam_get_HwVersion )( HToupCam, char* );
extern HRESULT		( *pl_Altaircam_get_ProductionDate )( HToupCam, char* );
extern HRESULT		( *pl_Altaircam_get_LevelRange )( HToupCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *pl_Altaircam_put_LevelRange )( HToupCam,
			    unsigned short*, unsigned short* );
extern HRESULT		( *pl_Altaircam_put_ExpoCallback )( HToupCam,
			    PITOUPCAM_EXPOSURE_CALLBACK, void* );
extern HRESULT		( *pl_Altaircam_put_ChromeCallback )( HToupCam,
			    PITOUPCAM_CHROME_CALLBACK, void* );
extern HRESULT		( *pl_Altaircam_LevelRangeAuto )( HToupCam );
extern HRESULT		( *pl_Altaircam_GetHistogram )( HToupCam,
			    PITOUPCAM_HISTOGRAM_CALLBACK, void* );
extern HRESULT		( *pl_Altaircam_put_LEDState )( HToupCam, unsigned short,
			    unsigned short, unsigned short );
extern HRESULT		( *pl_Altaircam_read_EEPROM )( HToupCam, unsigned,
			    unsigned char*, unsigned );
extern HRESULT		( *pl_Altaircam_write_EEPROM )( HToupCam, unsigned,
			    const unsigned char*, unsigned );
extern HRESULT		( *pl_Altaircam_get_Option )( HToupCam, unsigned,
			    unsigned* );
extern HRESULT		( *pl_Altaircam_put_Option )( HToupCam, unsigned,
			    unsigned );
extern HRESULT		( *pl_Altaircam_get_Roi )( HToupCam, unsigned*,
			    unsigned* );
extern HRESULT		( *pl_Altaircam_put_Roi )( HToupCam, unsigned, unsigned,
                            unsigned, unsigned );
extern HRESULT		( *pl_Altaircam_ST4PlusGuide )( HToupCam, unsigned,
			    unsigned );
extern HRESULT		( *pl_Altaircam_ST4PlusGuideState )( HToupCam );
extern double		( *pl_Altaircam_calc_ClarityFactor )( const void*, int,
			    unsigned, unsigned );
extern void		( *pl_Altaircam_deBayer )( unsigned, int, int,
			    const void*, void*, unsigned char );
extern void		( *pl_Altaircam_HotPlug )( PTOUPCAM_HOTPLUG, void* );

#endif	/* OA_ALTAIRCAM_LEGACY_PRIVATE_H */
