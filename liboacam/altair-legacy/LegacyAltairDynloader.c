/*****************************************************************************
 *
 * LegacyAltairDynloader.c -- handle dynamic loading of libaltaircamlegacy
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
#include <openastro/camera.h>
#include <altaircamlegacy.h>

#include "oacamprivate.h"
#include "LegacyAltairprivate.h"


// Pointers to libaltaircamlegacy functions so we can use them via libdl.

const char*	( *p_legacyAltaircam_Version )();
unsigned	( *p_legacyAltaircam_Enum )( ToupcamInst* );
HToupCam	( *p_legacyAltaircam_Open )( const char* );
HToupCam	( *p_legacyAltaircam_OpenByIndex )( unsigned );
void		( *p_legacyAltaircam_Close )( HToupCam );
HRESULT		( *p_legacyAltaircam_StartPullModeWithCallback )( HToupCam,
		    PTOUPCAM_EVENT_CALLBACK, void* );
HRESULT		( *p_legacyAltaircam_PullImage )( HToupCam, void*, int, unsigned*,
		    unsigned* );
HRESULT		( *p_legacyAltaircam_PullStillImage )( HToupCam, void*, int,
		    unsigned*, unsigned* );
HRESULT		( *p_legacyAltaircam_StartPushMode )( HToupCam,
		    PTOUPCAM_DATA_CALLBACK, void* );
HRESULT		( *p_legacyAltaircam_Stop )( HToupCam );
HRESULT		( *p_legacyAltaircam_Pause )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_Snap )( HToupCam, unsigned );
HRESULT		( *p_legacyAltaircam_Trigger )( HToupCam );
HRESULT		( *p_legacyAltaircam_get_Size )( HToupCam, int*, int* );
HRESULT		( *p_legacyAltaircam_put_Size )( HToupCam, int, int );
HRESULT		( *p_legacyAltaircam_get_eSize )( HToupCam, unsigned* );
HRESULT		( *p_legacyAltaircam_put_eSize )( HToupCam, unsigned );
HRESULT		( *p_legacyAltaircam_get_Resolution )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_legacyAltaircam_get_ResolutionNumber )( HToupCam );
HRESULT		( *p_legacyAltaircam_get_ResolutionRatio )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_legacyAltaircam_get_RawFormat )( HToupCam, unsigned*,
		    unsigned* );
HRESULT		( *p_legacyAltaircam_get_AutoExpoEnable )( HToupCam, BOOL* );
HRESULT		( *p_legacyAltaircam_get_AutoExpoTarget )( HToupCam,
		    unsigned short* );
HRESULT		( *p_legacyAltaircam_put_AutoExpoEnable )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_put_AutoExpoTarget )( HToupCam, unsigned short );
HRESULT		( *p_legacyAltaircam_get_ExpoTime )( HToupCam, unsigned* );
HRESULT		( *p_legacyAltaircam_get_ExpTimeRange )( HToupCam, unsigned*,
		    unsigned*, unsigned* );
HRESULT		( *p_legacyAltaircam_put_ExpoTime )( HToupCam, unsigned );
HRESULT		( *p_legacyAltaircam_put_MaxAutoExpoTimeAGain )( HToupCam, unsigned,
		    unsigned short );
HRESULT		( *p_legacyAltaircam_get_ExpoAGain )( HToupCam, unsigned short* );
HRESULT		( *p_legacyAltaircam_put_ExpoAGain )( HToupCam, unsigned short );
HRESULT		( *p_legacyAltaircam_get_ExpoAGainRange )( HToupCam, unsigned short*,
		    unsigned short*, unsigned short* );
HRESULT		( *p_legacyAltaircam_AwbInit )( HToupCam,
		    PITOUPCAM_WHITEBALANCE_CALLBACK, void* );
HRESULT		( *p_legacyAltaircam_AwbOnePush )( HToupCam,
		    PITOUPCAM_TEMPTINT_CALLBACK, void* );
HRESULT		( *p_legacyAltaircam_get_TempTint )( HToupCam, int*, int* );
HRESULT		( *p_legacyAltaircam_put_TempTint )( HToupCam, int, int );
HRESULT		( *p_legacyAltaircam_get_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_legacyAltaircam_put_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_legacyAltaircam_get_Hue )( HToupCam, int* );
HRESULT		( *p_legacyAltaircam_put_Hue )( HToupCam, int );
HRESULT		( *p_legacyAltaircam_get_Saturation )( HToupCam, int* );
HRESULT		( *p_legacyAltaircam_put_Saturation )( HToupCam, int );
HRESULT		( *p_legacyAltaircam_get_Brightness )( HToupCam, int* );
HRESULT		( *p_legacyAltaircam_put_Brightness )( HToupCam, int );
HRESULT		( *p_legacyAltaircam_get_Contrast )( HToupCam, int* );
HRESULT		( *p_legacyAltaircam_put_Contrast )( HToupCam, int );
HRESULT		( *p_legacyAltaircam_get_Gamma )( HToupCam, int* );
HRESULT		( *p_legacyAltaircam_put_Gamma )( HToupCam, int );
HRESULT		( *p_legacyAltaircam_get_Chrome )( HToupCam, BOOL* );
HRESULT		( *p_legacyAltaircam_put_Chrome )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_get_VFlip )( HToupCam, BOOL* );
HRESULT		( *p_legacyAltaircam_put_VFlip )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_get_HFlip )( HToupCam, BOOL* );
HRESULT		( *p_legacyAltaircam_put_HFlip )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_get_Negative )( HToupCam, BOOL* );
HRESULT		( *p_legacyAltaircam_put_Negative )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_get_MaxSpeed )( HToupCam );
HRESULT		( *p_legacyAltaircam_get_Speed )( HToupCam, unsigned short* );
HRESULT		( *p_legacyAltaircam_put_Speed )( HToupCam, unsigned short );
HRESULT		( *p_legacyAltaircam_get_MaxBitDepth )( HToupCam );
HRESULT		( *p_legacyAltaircam_get_HZ )( HToupCam, int* );
HRESULT		( *p_legacyAltaircam_put_HZ )( HToupCam, int );
HRESULT		( *p_legacyAltaircam_get_Mode )( HToupCam, BOOL* );
HRESULT		( *p_legacyAltaircam_put_Mode )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_get_AWBAuxRect )( HToupCam, RECT* );
HRESULT		( *p_legacyAltaircam_put_AWBAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_legacyAltaircam_get_AEAuxRect )( HToupCam, RECT* );
HRESULT		( *p_legacyAltaircam_put_AEAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_legacyAltaircam_get_MonoMode )( HToupCam );
HRESULT		( *p_legacyAltaircam_get_StillResolution )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_legacyAltaircam_get_StillResolutionNumber )( HToupCam );
HRESULT		( *p_legacyAltaircam_get_RealTime )( HToupCam, BOOL* );
HRESULT		( *p_legacyAltaircam_put_RealTime )( HToupCam, BOOL );
HRESULT		( *p_legacyAltaircam_Flush )( HToupCam );
HRESULT		( *p_legacyAltaircam_get_Temperature )( HToupCam, short* );
HRESULT		( *p_legacyAltaircam_put_Temperature )( HToupCam, short );
HRESULT		( *p_legacyAltaircam_get_SerialNumber )( HToupCam, char[32] );
HRESULT		( *p_legacyAltaircam_get_FwVersion )( HToupCam, char[16] );
HRESULT		( *p_legacyAltaircam_get_HwVersion )( HToupCam, char[16] );
HRESULT		( *p_legacyAltaircam_get_ProductionDate )( HToupCam, char[10] );
HRESULT		( *p_legacyAltaircam_get_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_legacyAltaircam_put_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_legacyAltaircam_put_ExpoCallback )( HToupCam,
		    PITOUPCAM_EXPOSURE_CALLBACK, void* );
HRESULT		( *p_legacyAltaircam_put_ChromeCallback )( HToupCam,
		    PITOUPCAM_CHROME_CALLBACK, void* );
HRESULT		( *p_legacyAltaircam_LevelRangeAuto )( HToupCam );
HRESULT		( *p_legacyAltaircam_GetHistogram )( HToupCam,
		    PITOUPCAM_HISTOGRAM_CALLBACK, void* );
HRESULT		( *p_legacyAltaircam_put_LEDState )( HToupCam, unsigned short,
		    unsigned short, unsigned short );
HRESULT		( *p_legacyAltaircam_read_EEPROM )( HToupCam, unsigned,
		    unsigned char*, unsigned );
HRESULT		( *p_legacyAltaircam_write_EEPROM )( HToupCam, unsigned,
		    const unsigned char*, unsigned );
HRESULT		( *p_legacyAltaircam_get_Option )( HToupCam, unsigned, unsigned* );
HRESULT		( *p_legacyAltaircam_put_Option )( HToupCam, unsigned, unsigned );
HRESULT		( *p_legacyAltaircam_get_Roi )( HToupCam, unsigned*, unsigned* );
HRESULT		( *p_legacyAltaircam_put_Roi )( HToupCam, unsigned, unsigned,
		    unsigned, unsigned );
HRESULT		( *p_legacyAltaircam_ST4PlusGuide )( HToupCam, unsigned, unsigned );
HRESULT		( *p_legacyAltaircam_ST4PlusGuideState )( HToupCam );
double		( *p_legacyAltaircam_calc_ClarityFactor )( const void*, int,
		    unsigned, unsigned );
void		( *p_legacyAltaircam_deBayer )( unsigned, int, int, const void*,
		    void*, unsigned char );
void		( *p_legacyAltaircam_HotPlug )( PTOUPCAM_HOTPLUG, void* );

// These are apparently obsolete
//
// Altaircam_get_RoiMode
// Altaircam_put_RoiMode
// Altaircam_get_VignetAmountInt
// Altaircam_get_VignetEnable
// Altaircam_get_VignetMidPointInt
// Altaircam_put_VignetAmountInt
// Altaircam_put_VignetEnable
// Altaircam_put_VignetMidPointInt

// And these are not documented as far as I can see
//
// Altaircam_AbbOnePush(ToupcamT*, void (*)(unsigned short const*, void*), void*)
// Altaircam_DfcOnePush(ToupcamT*)
// Altaircam_EnumV2(ToupcamInstV2*)
// Altaircam_FfcOnePush(ToupcamT*)
// Altaircam_get_ABBAuxRect(ToupcamT*, RECT*)
// Altaircam_get_BlackBalance(ToupcamT*, unsigned short*)
// Altaircam_get_FanMaxSpeed(ToupcamT*)
// Altaircam_get_Field(ToupcamT*)
// Altaircam_get_FpgaVersion(ToupcamT*, char*)
// Altaircam_get_FrameRate(ToupcamT*, unsigned int*, unsigned int*, unsigned int*)
// Altaircam_get_PixelSize(ToupcamT*, unsigned int, float*, float*)
// Altaircam_get_Revision(ToupcamT*, unsigned short*)
// Altaircam_InitOcl()
// Altaircam_IoControl(ToupcamT*, unsigned int, unsigned int, int, int*)
// Altaircam_PullImageWithRowPitch(ToupcamT*, void*, int, int, unsigned int*, unsigned int*)
// Altaircam_PullStillImageWithRowPitch(ToupcamT*, void*, int, int, unsigned int*, unsigned int*)
// Altaircam_put_ABBAuxRect(ToupcamT*, RECT const*)
// Altaircam_put_BlackBalance(ToupcamT*, unsigned short*)
// Altaircam_put_ColorMatrix(ToupcamT*, double const*)
// Altaircam_put_Curve(ToupcamT*, unsigned char const*, unsigned short const*)
// Altaircam_put_Demosaic(ToupcamT*, void (*)(unsigned int, int, int, void const*, void*, unsigned char, void*), void*)
// Altaircam_put_InitWBGain(ToupcamT*, unsigned short const*)
// Altaircam_put_Linear(ToupcamT*, unsigned char const*, unsigned short const*)
// Altaircam_read_UART(ToupcamT*, unsigned char*, unsigned int)
// Altaircam_StartOclWithSharedTexture(ToupcamT*, ToupcamOclWithSharedTexture const*, void (*)(unsigned int, void*), void*)
// Altaircam_write_UART(ToupcamT*, unsigned char const*, unsigned int)


static void*		_getDLSym ( void*, const char*, int );

#define ALTAIR_PREFIX	1
#define TOUPCAM_PREFIX	2

/**
 * Cycle through the list of cameras returned by the altaircam library
 */

int
_altairLegacyInitLibraryFunctionPointers ( void )
{
  int             prefix;
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*   libName = "libaltaircamlegacy.dylib";
#else
  const char*   libName = "libaltaircamlegacy.so.1";
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
      // fprintf ( stderr, "can't load %s:\n%s\n", libPath, dlerror());
      return OA_ERR_LIBRARY_NOT_FOUND;
    }
  }

  dlerror();

  // At some point the Altair library changed.  All the old Toupcam_
  // functions appear to be compiled using C++ now, and the entrypoints
  // names changed to be "Altair_...".  Handling both possibilities leads
  // to the less than desirable mess that we have here...

  // First we look for the "Altaircam_" prefix.  If that isn't found then
  // try the "Toupcam_" prefix.  And if that is also missing, give up

  prefix = ALTAIR_PREFIX;
  if (!( *( void** )( &p_legacyAltaircam_AwbInit ) = _getDLSym ( libHandle,
      "AwbInit", prefix ))) {
    prefix = TOUPCAM_PREFIX;
    if (!( *( void** )( &p_legacyAltaircam_AwbInit ) = _getDLSym ( libHandle,
        "AwbInit", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
      return OA_ERR_SYMBOL_NOT_FOUND;
    }
  }

  if (!( *( void** )( &p_legacyAltaircam_AwbOnePush ) = _getDLSym ( libHandle,
      "AwbOnePush", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_calc_ClarityFactor ) =
			_getDLSym ( libHandle, "calc_ClarityFactor", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Close ) = _getDLSym ( libHandle,
      "Close", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_deBayer ) = _getDLSym ( libHandle,
      "deBayer", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Enum ) = _getDLSym ( libHandle,
      "Enum", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Flush ) = _getDLSym ( libHandle,
      "Flush", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_AEAuxRect ) =
			_getDLSym ( libHandle, "get_AEAuxRect", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_AutoExpoEnable ) =
			_getDLSym ( libHandle, "get_AutoExpoEnable", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_AutoExpoTarget ) =
			_getDLSym ( libHandle, "get_AutoExpoTarget", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_AWBAuxRect ) =
			_getDLSym ( libHandle, "get_AWBAuxRect", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Brightness ) =
			_getDLSym ( libHandle, "get_Brightness", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Chrome ) =
			_getDLSym ( libHandle, "get_Chrome", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Contrast ) =
			_getDLSym ( libHandle, "get_Contrast", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_eSize ) =
			_getDLSym ( libHandle, "get_eSize", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_ExpoAGain ) =
			_getDLSym ( libHandle, "get_ExpoAGain", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_ExpoAGainRange ) =
			_getDLSym ( libHandle, "get_ExpoAGainRange", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_ExpoTime ) =
			_getDLSym ( libHandle, "get_ExpoTime", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_ExpTimeRange ) =
			_getDLSym ( libHandle, "get_ExpTimeRange", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_get_FanMaxSpeed ) =
				_getDLSym ( libHandle, "get_FanMaxSpeed", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Field ) = _getDLSym ( libHandle,
      "get_Field", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_legacyAltaircam_get_FwVersion ) =
			_getDLSym ( libHandle, "get_FwVersion", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Gamma ) = _getDLSym ( libHandle,
      "get_Gamma", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_HFlip ) = _getDLSym ( libHandle,
      "get_HFlip", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_GetHistogram ) =
			_getDLSym ( libHandle, "GetHistogram", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Hue ) = _getDLSym ( libHandle,
      "get_Hue", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_HwVersion ) =
			_getDLSym ( libHandle, "get_HwVersion", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_HZ ) = _getDLSym ( libHandle,
      "get_HZ", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_LevelRange ) =
			_getDLSym ( libHandle, "get_LevelRange", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_MaxBitDepth ) =
			_getDLSym ( libHandle, "get_MaxBitDepth", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_MaxSpeed ) =
			_getDLSym ( libHandle, "get_MaxSpeed", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Mode ) = _getDLSym ( libHandle,
      "get_Mode", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_MonoMode ) = _getDLSym ( libHandle,
      "get_MonoMode", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Negative ) = _getDLSym ( libHandle,
      "get_Negative", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Option ) = _getDLSym ( libHandle,
      "get_Option", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_get_PixelSize ) =
				_getDLSym ( libHandle, "get_PixelSize", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_legacyAltaircam_get_ProductionDate ) =
			_getDLSym ( libHandle, "get_ProductionDate", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_RawFormat ) =
			_getDLSym ( libHandle, "get_RawFormat", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_RealTime ) =
			_getDLSym ( libHandle, "get_RealTime", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Resolution ) =
			_getDLSym ( libHandle, "get_Resolution", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_ResolutionNumber ) = _getDLSym (
      libHandle, "get_ResolutionNumber", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_ResolutionRatio ) = _getDLSym (
      libHandle, "get_ResolutionRatio", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Roi ) = _getDLSym ( libHandle,
      "get_Roi", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_get_RoiMode ) = _getDLSym ( libHandle,
      "get_RoiMode", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_legacyAltaircam_get_Saturation ) =
			_getDLSym ( libHandle, "get_Saturation", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_SerialNumber ) =
			_getDLSym ( libHandle, "get_SerialNumber", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Size ) = _getDLSym ( libHandle,
      "get_Size", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Speed ) = _getDLSym ( libHandle,
      "get_Speed", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_StillResolution ) = _getDLSym (
      libHandle, "get_StillResolution", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_StillResolutionNumber ) =
			_getDLSym ( libHandle, "get_StillResolutionNumber", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_Temperature ) =
			_getDLSym ( libHandle, "get_Temperature", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_TempTint ) =
			_getDLSym ( libHandle, "get_TempTint", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_VFlip ) = _getDLSym ( libHandle,
      "get_VFlip", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_get_VignetAmountInt ) = _getDLSym (
      libHandle, "get_VignetAmountInt", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_VignetEnable ) =
				_getDLSym ( libHandle, "get_VignetEnable", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_get_VignetMidPointInt ) =
				_getDLSym ( libHandle, "get_VignetMidPointInt", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_legacyAltaircam_get_WhiteBalanceGain ) = _getDLSym (
      libHandle, "get_WhiteBalanceGain", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_HotPlug ) = _getDLSym ( libHandle,
      "HotPlug", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_LevelRangeAuto ) =
			_getDLSym ( libHandle, "LevelRangeAuto", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Open ) = _getDLSym ( libHandle,
      "Open", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_OpenByIndex ) = _getDLSym ( libHandle,
      "OpenByIndex", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Pause ) = _getDLSym ( libHandle,
      "Pause", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_PullImage ) = _getDLSym ( libHandle,
      "PullImage", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_PullStillImage ) =
			_getDLSym ( libHandle, "PullStillImage", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_AEAuxRect ) =
			_getDLSym ( libHandle, "put_AEAuxRect", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_AutoExpoEnable ) =
			_getDLSym ( libHandle, "put_AutoExpoEnable", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_AutoExpoTarget ) =
			_getDLSym ( libHandle, "put_AutoExpoTarget", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_AWBAuxRect ) =
			_getDLSym ( libHandle, "put_AWBAuxRect", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Brightness ) =
			_getDLSym ( libHandle, "put_Brightness", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Chrome ) = _getDLSym ( libHandle,
      "put_Chrome", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_ChromeCallback ) =
			_getDLSym ( libHandle, "put_ChromeCallback", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Contrast ) =
			_getDLSym ( libHandle, "put_Contrast", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_eSize ) = _getDLSym ( libHandle,
      "put_eSize", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_ExpoAGain ) =
			_getDLSym ( libHandle, "put_ExpoAGain", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_ExpoCallback ) =
			_getDLSym ( libHandle, "put_ExpoCallback", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_ExpoTime ) =
			_getDLSym ( libHandle, "put_ExpoTime", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Gamma ) = _getDLSym ( libHandle,
      "put_Gamma", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_HFlip ) = _getDLSym ( libHandle,
      "put_HFlip", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Hue ) = _getDLSym ( libHandle,
      "put_Hue", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_HZ ) = _getDLSym ( libHandle,
      "put_HZ", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_LEDState ) =
			_getDLSym ( libHandle, "put_LEDState", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_LevelRange ) =
			_getDLSym ( libHandle, "put_LevelRange", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_MaxAutoExpoTimeAGain ) =
			_getDLSym ( libHandle, "put_MaxAutoExpoTimeAGain", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Mode ) = _getDLSym ( libHandle,
      "put_Mode", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Negative ) = _getDLSym ( libHandle,
      "put_Negative", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Option ) = _getDLSym ( libHandle,
      "put_Option", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_RealTime ) = _getDLSym ( libHandle,
      "put_RealTime", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Roi ) = _getDLSym ( libHandle,
      "put_Roi", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_put_RoiMode ) = _getDLSym ( libHandle,
      "put_RoiMode", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_legacyAltaircam_put_Saturation ) =
			_getDLSym ( libHandle, "put_Saturation", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Size ) = _getDLSym ( libHandle,
      "put_Size", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Speed ) = _getDLSym ( libHandle,
      "put_Speed", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_Temperature ) =
			_getDLSym ( libHandle, "put_Temperature", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_TempTint ) = _getDLSym ( libHandle,
      "put_TempTint", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_VFlip ) = _getDLSym ( libHandle,
      "put_VFlip", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_put_VignetAmountInt ) = _getDLSym (
      libHandle, "put_VignetAmountInt", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_VignetEnable ) =
				_getDLSym ( libHandle, "put_VignetEnable", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_put_VignetMidPointInt ) = _getDLSym (
      libHandle, "put_VignetMidPointInt", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_legacyAltaircam_put_WhiteBalanceGain ) = _getDLSym (
      libHandle, "put_WhiteBalanceGain", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_read_EEPROM ) = _getDLSym ( libHandle,
      "read_EEPROM", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_read_UART ) = _getDLSym ( libHandle,
      "read_UART", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

  if (!( *( void** )( &p_legacyAltaircam_Snap ) = _getDLSym ( libHandle,
      "Snap", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_ST4PlusGuide ) = _getDLSym ( libHandle,
      "ST4PlusGuide", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_ST4PlusGuideState ) =
			_getDLSym ( libHandle, "ST4PlusGuideState", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_StartPullModeWithCallback ) =
			_getDLSym ( libHandle, "StartPullModeWithCallback", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_StartPushMode ) =
			_getDLSym ( libHandle, "StartPushMode", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Stop ) = _getDLSym ( libHandle,
      "Stop", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Trigger ) = _getDLSym ( libHandle,
      "Trigger", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_Version ) = _getDLSym ( libHandle,
      "Version", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_legacyAltaircam_write_EEPROM ) = _getDLSym ( libHandle,
      "write_EEPROM", prefix ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  /*
  if (!( *( void** )( &p_legacyAltaircam_write_UART ) = _getDLSym ( libHandle,
      "write_UART", prefix ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }
   */

	return OA_ERR_NONE;
}


static void*
_getDLSym ( void* libHandle, const char* symbol, int prefix )
{
  void* addr;
  char* error;
  char symbolBuffer[ 80 ]; // This really has to be long enough

  switch ( prefix ) {
    case ALTAIR_PREFIX:
      ( void ) strcpy ( symbolBuffer, "Altaircam_" );
      break;
    case TOUPCAM_PREFIX:
      ( void ) strcpy ( symbolBuffer, "Toupcam_" );
      break;
    default:
      return 0;
  }
  ( void ) strncat ( symbolBuffer, symbol, 79 );

  addr = dlsym ( libHandle, symbolBuffer );
  if (( error = dlerror())) {
    fprintf ( stderr, "libaltaircamlegacy DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
