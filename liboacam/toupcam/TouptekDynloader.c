/*****************************************************************************
 *
 * TouptekDynloader.c -- handle dynamic loading for libtopcam
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif
#include <openastro/errno.h>
#include <toupcam.h>

#include "oacamprivate.h"
#include "Touptekprivate.h"

// Pointers to libtoupcam functions so we can use them via libdl.

const char*	( *p_Toupcam_Version )();
unsigned	( *p_Toupcam_EnumV2 )( ToupcamInstV2* );
HToupCam	( *p_Toupcam_Open )( const char* );
HToupCam	( *p_Toupcam_OpenByIndex )( unsigned );
void		( *p_Toupcam_Close )( HToupCam );
HRESULT		( *p_Toupcam_StartPullModeWithCallback )( HToupCam,
		    PTOUPCAM_EVENT_CALLBACK, void* );
HRESULT		( *p_Toupcam_PullImageV2 )( HToupCam, void*, int,
		    ToupcamFrameInfoV2* );
HRESULT		( *p_Toupcam_PullStillImageV2 )( HToupCam, void*, int,
		    ToupcamFrameInfoV2* );
HRESULT		( *p_Toupcam_StartPushModeV2 )( HToupCam, PTOUPCAM_DATA_CALLBACK_V2,
		    void* );
HRESULT		( *p_Toupcam_Stop )( HToupCam );
HRESULT		( *p_Toupcam_Pause )( HToupCam, int );
HRESULT		( *p_Toupcam_Snap )( HToupCam, unsigned );
HRESULT		( *p_Toupcam_Trigger )( HToupCam );
HRESULT		( *p_Toupcam_get_Size )( HToupCam, int*, int* );
HRESULT		( *p_Toupcam_put_Size )( HToupCam, int, int );
HRESULT		( *p_Toupcam_get_eSize )( HToupCam, unsigned* );
HRESULT		( *p_Toupcam_put_eSize )( HToupCam, unsigned );
HRESULT		( *p_Toupcam_get_Resolution )( HToupCam, unsigned, int*, int* );
HRESULT		( *p_Toupcam_get_ResolutionNumber )( HToupCam );
HRESULT		( *p_Toupcam_get_ResolutionRatio )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Toupcam_get_RawFormat )( HToupCam, unsigned*, unsigned* );
HRESULT		( *p_Toupcam_get_AutoExpoEnable )( HToupCam, int* );
HRESULT		( *p_Toupcam_get_AutoExpoTarget )( HToupCam, unsigned short* );
HRESULT		( *p_Toupcam_put_AutoExpoEnable )( HToupCam, int );
HRESULT		( *p_Toupcam_put_AutoExpoTarget )( HToupCam, unsigned short );
HRESULT		( *p_Toupcam_get_ExpoTime )( HToupCam, unsigned* );
HRESULT		( *p_Toupcam_get_ExpTimeRange )( HToupCam, unsigned*,
		    unsigned*, unsigned* );
HRESULT		( *p_Toupcam_put_ExpoTime )( HToupCam, unsigned );
HRESULT		( *p_Toupcam_put_MaxAutoExpoTimeAGain )( HToupCam, unsigned,
		    unsigned short );
HRESULT		( *p_Toupcam_get_ExpoAGain )( HToupCam, unsigned short* );
HRESULT		( *p_Toupcam_put_ExpoAGain )( HToupCam, unsigned short );
HRESULT		( *p_Toupcam_get_ExpoAGainRange )( HToupCam, unsigned short*,
		    unsigned short*, unsigned short* );
HRESULT		( *p_Toupcam_AwbInit )( HToupCam,
		    PITOUPCAM_WHITEBALANCE_CALLBACK, void* );
HRESULT		( *p_Toupcam_AwbOnePush )( HToupCam,
		    PITOUPCAM_TEMPTINT_CALLBACK, void* );
HRESULT		( *p_Toupcam_get_TempTint )( HToupCam, int*, int* );
HRESULT		( *p_Toupcam_put_TempTint )( HToupCam, int, int );
HRESULT		( *p_Toupcam_get_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_Toupcam_put_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_Toupcam_get_Hue )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Hue )( HToupCam, int );
HRESULT		( *p_Toupcam_get_Saturation )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Saturation )( HToupCam, int );
HRESULT		( *p_Toupcam_get_Brightness )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Brightness )( HToupCam, int );
HRESULT		( *p_Toupcam_get_Contrast )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Contrast )( HToupCam, int );
HRESULT		( *p_Toupcam_get_Gamma )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Gamma )( HToupCam, int );
HRESULT		( *p_Toupcam_get_Chrome )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Chrome )( HToupCam, int );
HRESULT		( *p_Toupcam_get_VFlip )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_VFlip )( HToupCam, int );
HRESULT		( *p_Toupcam_get_HFlip )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_HFlip )( HToupCam, int );
HRESULT		( *p_Toupcam_get_Negative )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Negative )( HToupCam, int );
HRESULT		( *p_Toupcam_get_MaxSpeed )( HToupCam );
HRESULT		( *p_Toupcam_get_Speed )( HToupCam, unsigned short* );
HRESULT		( *p_Toupcam_put_Speed )( HToupCam, unsigned short );
HRESULT		( *p_Toupcam_get_MaxBitDepth )( HToupCam );
HRESULT		( *p_Toupcam_get_HZ )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_HZ )( HToupCam, int );
HRESULT		( *p_Toupcam_get_Mode )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_Mode )( HToupCam, int );
HRESULT		( *p_Toupcam_get_AWBAuxRect )( HToupCam, RECT* );
HRESULT		( *p_Toupcam_put_AWBAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_Toupcam_get_AEAuxRect )( HToupCam, RECT* );
HRESULT		( *p_Toupcam_put_AEAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_Toupcam_get_MonoMode )( HToupCam );
HRESULT		( *p_Toupcam_get_StillResolution )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Toupcam_get_StillResolutionNumber )( HToupCam );
HRESULT		( *p_Toupcam_get_RealTime )( HToupCam, int* );
HRESULT		( *p_Toupcam_put_RealTime )( HToupCam, int );
HRESULT		( *p_Toupcam_Flush )( HToupCam );
HRESULT		( *p_Toupcam_get_Temperature )( HToupCam, short* );
HRESULT		( *p_Toupcam_put_Temperature )( HToupCam, short );
HRESULT		( *p_Toupcam_get_SerialNumber )( HToupCam, char[32] );
HRESULT		( *p_Toupcam_get_FwVersion )( HToupCam, char[16] );
HRESULT		( *p_Toupcam_get_HwVersion )( HToupCam, char[16] );
HRESULT		( *p_Toupcam_get_ProductionDate )( HToupCam, char[10] );
HRESULT		( *p_Toupcam_get_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Toupcam_put_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Toupcam_put_ExpoCallback )( HToupCam,
		    PITOUPCAM_EXPOSURE_CALLBACK, void* );
HRESULT		( *p_Toupcam_put_ChromeCallback )( HToupCam,
		    PITOUPCAM_CHROME_CALLBACK, void* );
HRESULT		( *p_Toupcam_LevelRangeAuto )( HToupCam );
HRESULT		( *p_Toupcam_GetHistogram )( HToupCam,
		    PITOUPCAM_HISTOGRAM_CALLBACK, void* );
HRESULT		( *p_Toupcam_put_LEDState )( HToupCam, unsigned short,
		    unsigned short, unsigned short );
HRESULT		( *p_Toupcam_read_EEPROM )( HToupCam, unsigned, unsigned char*,
		    unsigned );
HRESULT		( *p_Toupcam_write_EEPROM )( HToupCam, unsigned,
		    const unsigned char*, unsigned );
HRESULT		( *p_Toupcam_get_Option )( HToupCam, unsigned, unsigned* );
HRESULT		( *p_Toupcam_put_Option )( HToupCam, unsigned, unsigned );
HRESULT		( *p_Toupcam_get_Roi )( HToupCam, unsigned*, unsigned* );
HRESULT		( *p_Toupcam_put_Roi )( HToupCam, unsigned, unsigned, unsigned,
                  unsigned );
HRESULT		( *p_Toupcam_ST4PlusGuide )( HToupCam, unsigned, unsigned );
HRESULT		( *p_Toupcam_ST4PlusGuideState )( HToupCam );
double		( *p_Toupcam_calc_ClarityFactor )( const void*, int, unsigned,
		    unsigned );
void		( *p_Toupcam_deBayerV2 )( unsigned, int, int, const void*, void*,
		    unsigned char );
void		( *p_Toupcam_HotPlug )( PTOUPCAM_HOTPLUG, void* );

// These are apparently obsolete
//
// Toupcam_get_RoiMode
// Toupcam_put_RoiMode
// Toupcam_get_VignetAmountInt
// Toupcam_get_VignetEnable
// Toupcam_get_VignetMidPointInt
// Toupcam_put_VignetAmountInt
// Toupcam_put_VignetEnable
// Toupcam_put_VignetMidPointInt

// And these are not documented as far as I can see
// Toupcam_get_FanMaxSpeed
// Toupcam_get_Field
// Toupcam_get_PixelSize
// Toupcam_read_UART
// Toupcam_write_UART

static void*		_getDLSym ( void*, const char* );

/**
 * Cycle through the list of cameras returned by the toupcam library
 */

int
_toupcamInitLibraryFunctionPointers ( void )
{
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*   libName = "libtoupcam.dylib";
#else
  const char*   libName = "libtoupcam.so.1";
#endif

  *libPath = 0;
  if ( !libHandle ) {
		if ( installPathRoot ) {
			( void ) strncpy ( libPath, installPathRoot, PATH_MAX );
		}
#ifdef SHLIB_PATH
		( void ) strncat ( libPath, SHLIB_PATH, PATH_MAX );
#endif
		( void ) strncat ( libPath, libName, PATH_MAX );

    if (!( libHandle = dlopen ( libPath, RTLD_LAZY ))) {
      fprintf ( stderr, "can't load %s:\n%s\n", libPath, dlerror());
      return OA_ERR_LIBRARY_NOT_FOUND;
    }
  }

  dlerror();

  if (!( *( void** )( &p_Toupcam_AwbInit ) = _getDLSym ( libHandle,
      "Toupcam_AwbInit" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_AwbOnePush ) = _getDLSym ( libHandle,
      "Toupcam_AwbOnePush" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_calc_ClarityFactor ) = _getDLSym ( libHandle,
      "Toupcam_calc_ClarityFactor" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_Close ) = _getDLSym ( libHandle,
      "Toupcam_Close" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

	/*
  if (!( *( void** )( &p_Toupcam_deBayerV2 ) = _getDLSym ( libHandle,
      "Toupcam_deBayerV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
	*/

  if (!( *( void** )( &p_Toupcam_EnumV2 ) = _getDLSym ( libHandle,
      "Toupcam_EnumV2" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_Flush ) = _getDLSym ( libHandle,
      "Toupcam_Flush" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_AEAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_get_AEAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_AutoExpoEnable ) = _getDLSym ( libHandle,
      "Toupcam_get_AutoExpoEnable" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_AutoExpoTarget ) = _getDLSym ( libHandle,
      "Toupcam_get_AutoExpoTarget" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_AWBAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_get_AWBAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Brightness ) = _getDLSym ( libHandle,
      "Toupcam_get_Brightness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Chrome ) = _getDLSym ( libHandle,
      "Toupcam_get_Chrome" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Contrast ) = _getDLSym ( libHandle,
      "Toupcam_get_Contrast" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_eSize ) = _getDLSym ( libHandle,
      "Toupcam_get_eSize" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_ExpoAGain ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpoAGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_ExpoAGainRange ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpoAGainRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_ExpoTime ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpoTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_ExpTimeRange ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpTimeRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_get_FanMaxSpeed ) = _getDLSym ( libHandle,
      "Toupcam_get_FanMaxSpeed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Field ) = _getDLSym ( libHandle,
      "Toupcam_get_Field" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_Toupcam_get_FwVersion ) = _getDLSym ( libHandle,
      "Toupcam_get_FwVersion" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Gamma ) = _getDLSym ( libHandle,
      "Toupcam_get_Gamma" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_HFlip ) = _getDLSym ( libHandle,
      "Toupcam_get_HFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_GetHistogram ) = _getDLSym ( libHandle,
      "Toupcam_GetHistogram" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Hue ) = _getDLSym ( libHandle,
      "Toupcam_get_Hue" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_HwVersion ) = _getDLSym ( libHandle,
      "Toupcam_get_HwVersion" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_HZ ) = _getDLSym ( libHandle,
      "Toupcam_get_HZ" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_LevelRange ) = _getDLSym ( libHandle,
      "Toupcam_get_LevelRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_MaxBitDepth ) = _getDLSym ( libHandle,
      "Toupcam_get_MaxBitDepth" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_MaxSpeed ) = _getDLSym ( libHandle,
      "Toupcam_get_MaxSpeed" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Mode ) = _getDLSym ( libHandle,
      "Toupcam_get_Mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_MonoMode ) = _getDLSym ( libHandle,
      "Toupcam_get_MonoMode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Negative ) = _getDLSym ( libHandle,
      "Toupcam_get_Negative" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Option ) = _getDLSym ( libHandle,
      "Toupcam_get_Option" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_get_PixelSize ) = _getDLSym ( libHandle,
      "Toupcam_get_PixelSize" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_Toupcam_get_ProductionDate ) = _getDLSym ( libHandle,
      "Toupcam_get_ProductionDate" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_RawFormat ) = _getDLSym ( libHandle,
      "Toupcam_get_RawFormat" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_RealTime ) = _getDLSym ( libHandle,
      "Toupcam_get_RealTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Resolution ) = _getDLSym ( libHandle,
      "Toupcam_get_Resolution" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_ResolutionNumber ) = _getDLSym ( libHandle,
      "Toupcam_get_ResolutionNumber" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_ResolutionRatio ) = _getDLSym ( libHandle,
      "Toupcam_get_ResolutionRatio" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Roi ) = _getDLSym ( libHandle,
      "Toupcam_get_Roi" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_get_RoiMode ) = _getDLSym ( libHandle,
      "Toupcam_get_RoiMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_Toupcam_get_Saturation ) = _getDLSym ( libHandle,
      "Toupcam_get_Saturation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_SerialNumber ) = _getDLSym ( libHandle,
      "Toupcam_get_SerialNumber" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Size ) = _getDLSym ( libHandle,
      "Toupcam_get_Size" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Speed ) = _getDLSym ( libHandle,
      "Toupcam_get_Speed" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_StillResolution ) = _getDLSym ( libHandle,
      "Toupcam_get_StillResolution" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_StillResolutionNumber ) = _getDLSym (
      libHandle, "Toupcam_get_StillResolutionNumber" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_Temperature ) = _getDLSym ( libHandle,
      "Toupcam_get_Temperature" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_TempTint ) = _getDLSym ( libHandle,
      "Toupcam_get_TempTint" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_VFlip ) = _getDLSym ( libHandle,
      "Toupcam_get_VFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_get_VignetAmountInt ) = _getDLSym ( libHandle,
      "Toupcam_get_VignetAmountInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_VignetEnable ) = _getDLSym ( libHandle,
      "Toupcam_get_VignetEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_get_VignetMidPointInt ) = _getDLSym (
      libHandle, "Toupcam_get_VignetMidPointInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_Toupcam_get_WhiteBalanceGain ) = _getDLSym ( libHandle,
      "Toupcam_get_WhiteBalanceGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_HotPlug ) = _getDLSym ( libHandle,
      "Toupcam_HotPlug" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_LevelRangeAuto ) = _getDLSym ( libHandle,
      "Toupcam_LevelRangeAuto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_Open ) = _getDLSym ( libHandle,
      "Toupcam_Open" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_OpenByIndex ) = _getDLSym ( libHandle,
      "Toupcam_OpenByIndex" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_Pause ) = _getDLSym ( libHandle,
      "Toupcam_Pause" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_PullImageV2 ) = _getDLSym ( libHandle,
      "Toupcam_PullImageV2" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_PullStillImageV2 ) = _getDLSym ( libHandle,
      "Toupcam_PullStillImageV2" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_AEAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_put_AEAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_AutoExpoEnable ) = _getDLSym ( libHandle,
      "Toupcam_put_AutoExpoEnable" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_AutoExpoTarget ) = _getDLSym ( libHandle,
      "Toupcam_put_AutoExpoTarget" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_AWBAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_put_AWBAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Brightness ) = _getDLSym ( libHandle,
      "Toupcam_put_Brightness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Chrome ) = _getDLSym ( libHandle,
      "Toupcam_put_Chrome" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_ChromeCallback ) = _getDLSym ( libHandle,
      "Toupcam_put_ChromeCallback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Contrast ) = _getDLSym ( libHandle,
      "Toupcam_put_Contrast" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_eSize ) = _getDLSym ( libHandle,
      "Toupcam_put_eSize" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_ExpoAGain ) = _getDLSym ( libHandle,
      "Toupcam_put_ExpoAGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_ExpoCallback ) = _getDLSym ( libHandle,
      "Toupcam_put_ExpoCallback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_ExpoTime ) = _getDLSym ( libHandle,
      "Toupcam_put_ExpoTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Gamma ) = _getDLSym ( libHandle,
      "Toupcam_put_Gamma" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_HFlip ) = _getDLSym ( libHandle,
      "Toupcam_put_HFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Hue ) = _getDLSym ( libHandle,
      "Toupcam_put_Hue" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_HZ ) = _getDLSym ( libHandle,
      "Toupcam_put_HZ" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_LEDState ) = _getDLSym ( libHandle,
      "Toupcam_put_LEDState" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_LevelRange ) = _getDLSym ( libHandle,
      "Toupcam_put_LevelRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_MaxAutoExpoTimeAGain ) = _getDLSym (
      libHandle, "Toupcam_put_MaxAutoExpoTimeAGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Mode ) = _getDLSym ( libHandle,
      "Toupcam_put_Mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Negative ) = _getDLSym ( libHandle,
      "Toupcam_put_Negative" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Option ) = _getDLSym ( libHandle,
      "Toupcam_put_Option" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_RealTime ) = _getDLSym ( libHandle,
      "Toupcam_put_RealTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Roi ) = _getDLSym ( libHandle,
      "Toupcam_put_Roi" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_put_RoiMode ) = _getDLSym ( libHandle,
      "Toupcam_put_RoiMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_Toupcam_put_Saturation ) = _getDLSym ( libHandle,
      "Toupcam_put_Saturation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Size ) = _getDLSym ( libHandle,
      "Toupcam_put_Size" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Speed ) = _getDLSym ( libHandle,
      "Toupcam_put_Speed" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_Temperature ) = _getDLSym ( libHandle,
      "Toupcam_put_Temperature" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_TempTint ) = _getDLSym ( libHandle,
      "Toupcam_put_TempTint" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_VFlip ) = _getDLSym ( libHandle,
      "Toupcam_put_VFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_put_VignetAmountInt ) = _getDLSym ( libHandle,
      "Toupcam_put_VignetAmountInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_VignetEnable ) = _getDLSym ( libHandle,
      "Toupcam_put_VignetEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_put_VignetMidPointInt ) = _getDLSym (
      libHandle, "Toupcam_put_VignetMidPointInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_Toupcam_put_WhiteBalanceGain ) = _getDLSym ( libHandle,
      "Toupcam_put_WhiteBalanceGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_read_EEPROM ) = _getDLSym ( libHandle,
      "Toupcam_read_EEPROM" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_read_UART ) = _getDLSym ( libHandle,
      "Toupcam_read_UART" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_Toupcam_Snap ) = _getDLSym ( libHandle,
      "Toupcam_Snap" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_ST4PlusGuide ) = _getDLSym ( libHandle,
      "Toupcam_ST4PlusGuide" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_ST4PlusGuideState ) = _getDLSym ( libHandle,
      "Toupcam_ST4PlusGuideState" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_StartPullModeWithCallback ) = _getDLSym (
      libHandle, "Toupcam_StartPullModeWithCallback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_StartPushModeV2 ) = _getDLSym ( libHandle,
      "Toupcam_StartPushModeV2" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_Stop ) = _getDLSym ( libHandle,
      "Toupcam_Stop" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_Trigger ) = _getDLSym ( libHandle,
      "Toupcam_Trigger" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_Version ) = _getDLSym ( libHandle,
      "Toupcam_Version" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_Toupcam_write_EEPROM ) = _getDLSym ( libHandle,
      "Toupcam_write_EEPROM" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_Toupcam_write_UART ) = _getDLSym ( libHandle,
      "Toupcam_write_UART" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  return OA_ERR_NONE;
}


static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libtoupcam DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
