/*****************************************************************************
 *
 * Altairdynloader.c -- handle dynamic loading of libaltaircam
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
#include <altaircam.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Altairoacam.h"
#include "Altairprivate.h"

// Pointers to libaltaircam functions so we can use them via libdl.

const char*	( *p_Altaircam_Version )();
unsigned	( *p_Altaircam_EnumV2 )( AltaircamInstV2* );
HAltairCam	( *p_Altaircam_Open )( const char* );
HAltairCam	( *p_Altaircam_OpenByIndex )( unsigned );
void		( *p_Altaircam_Close )( HAltairCam );
HRESULT		( *p_Altaircam_StartPullModeWithCallback )( HAltairCam,
		    PALTAIRCAM_EVENT_CALLBACK, void* );
HRESULT		( *p_Altaircam_PullImageV2 )( HAltairCam, void*, int,
		    AltaircamFrameInfoV2* );
HRESULT		( *p_Altaircam_PullStillImageV2 )( HAltairCam, void*, int,
		    AltaircamFrameInfoV2* );
HRESULT		( *p_Altaircam_StartPushModeV2 )( HAltairCam,
		    PALTAIRCAM_DATA_CALLBACK_V2, void* );
HRESULT		( *p_Altaircam_Stop )( HAltairCam );
HRESULT		( *p_Altaircam_Pause )( HAltairCam, int );
HRESULT		( *p_Altaircam_Snap )( HAltairCam, unsigned );
HRESULT		( *p_Altaircam_Trigger )( HAltairCam );
HRESULT		( *p_Altaircam_get_Size )( HAltairCam, int*, int* );
HRESULT		( *p_Altaircam_put_Size )( HAltairCam, int, int );
HRESULT		( *p_Altaircam_get_eSize )( HAltairCam, unsigned* );
HRESULT		( *p_Altaircam_put_eSize )( HAltairCam, unsigned );
HRESULT		( *p_Altaircam_get_Resolution )( HAltairCam, unsigned, int*,
		    int* );
HRESULT		( *p_Altaircam_get_ResolutionNumber )( HAltairCam );
HRESULT		( *p_Altaircam_get_ResolutionRatio )( HAltairCam, unsigned, int*,
		    int* );
HRESULT		( *p_Altaircam_get_RawFormat )( HAltairCam, unsigned*,
		    unsigned* );
HRESULT		( *p_Altaircam_get_AutoExpoEnable )( HAltairCam, int* );
HRESULT		( *p_Altaircam_get_AutoExpoTarget )( HAltairCam,
		    unsigned short* );
HRESULT		( *p_Altaircam_put_AutoExpoEnable )( HAltairCam, int );
HRESULT		( *p_Altaircam_put_AutoExpoTarget )( HAltairCam, unsigned short );
HRESULT		( *p_Altaircam_get_ExpoTime )( HAltairCam, unsigned* );
HRESULT		( *p_Altaircam_get_ExpTimeRange )( HAltairCam, unsigned*,
		    unsigned*, unsigned* );
HRESULT		( *p_Altaircam_put_ExpoTime )( HAltairCam, unsigned );
HRESULT		( *p_Altaircam_put_MaxAutoExpoTimeAGain )( HAltairCam, unsigned,
		    unsigned short );
HRESULT		( *p_Altaircam_get_ExpoAGain )( HAltairCam, unsigned short* );
HRESULT		( *p_Altaircam_put_ExpoAGain )( HAltairCam, unsigned short );
HRESULT		( *p_Altaircam_get_ExpoAGainRange )( HAltairCam, unsigned short*,
		    unsigned short*, unsigned short* );
HRESULT		( *p_Altaircam_AwbInit )( HAltairCam,
		    PIALTAIRCAM_WHITEBALANCE_CALLBACK, void* );
HRESULT		( *p_Altaircam_AwbOnePush )( HAltairCam,
		    PIALTAIRCAM_TEMPTINT_CALLBACK, void* );
HRESULT		( *p_Altaircam_get_TempTint )( HAltairCam, int*, int* );
HRESULT		( *p_Altaircam_put_TempTint )( HAltairCam, int, int );
HRESULT		( *p_Altaircam_get_WhiteBalanceGain )( HAltairCam, int[3] );
HRESULT		( *p_Altaircam_put_WhiteBalanceGain )( HAltairCam, int[3] );
HRESULT		( *p_Altaircam_get_Hue )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Hue )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_Saturation )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Saturation )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_Brightness )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Brightness )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_Contrast )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Contrast )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_Gamma )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Gamma )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_Chrome )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Chrome )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_VFlip )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_VFlip )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_HFlip )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_HFlip )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_Negative )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Negative )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_MaxSpeed )( HAltairCam );
HRESULT		( *p_Altaircam_get_Speed )( HAltairCam, unsigned short* );
HRESULT		( *p_Altaircam_put_Speed )( HAltairCam, unsigned short );
HRESULT		( *p_Altaircam_get_MaxBitDepth )( HAltairCam );
HRESULT		( *p_Altaircam_get_HZ )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_HZ )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_Mode )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_Mode )( HAltairCam, int );
HRESULT		( *p_Altaircam_get_AWBAuxRect )( HAltairCam, RECT* );
HRESULT		( *p_Altaircam_put_AWBAuxRect )( HAltairCam, const RECT* );
HRESULT		( *p_Altaircam_get_AEAuxRect )( HAltairCam, RECT* );
HRESULT		( *p_Altaircam_put_AEAuxRect )( HAltairCam, const RECT* );
HRESULT		( *p_Altaircam_get_MonoMode )( HAltairCam );
HRESULT		( *p_Altaircam_get_StillResolution )( HAltairCam, unsigned, int*,
		    int* );
HRESULT		( *p_Altaircam_get_StillResolutionNumber )( HAltairCam );
HRESULT		( *p_Altaircam_get_RealTime )( HAltairCam, int* );
HRESULT		( *p_Altaircam_put_RealTime )( HAltairCam, int );
HRESULT		( *p_Altaircam_Flush )( HAltairCam );
HRESULT		( *p_Altaircam_get_Temperature )( HAltairCam, short* );
HRESULT		( *p_Altaircam_put_Temperature )( HAltairCam, short );
HRESULT		( *p_Altaircam_get_SerialNumber )( HAltairCam, char[32] );
HRESULT		( *p_Altaircam_get_FwVersion )( HAltairCam, char[16] );
HRESULT		( *p_Altaircam_get_HwVersion )( HAltairCam, char[16] );
HRESULT		( *p_Altaircam_get_ProductionDate )( HAltairCam, char[10] );
HRESULT		( *p_Altaircam_get_LevelRange )( HAltairCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Altaircam_put_LevelRange )( HAltairCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Altaircam_put_ExpoCallback )( HAltairCam,
		    PIALTAIRCAM_EXPOSURE_CALLBACK, void* );
HRESULT		( *p_Altaircam_put_ChromeCallback )( HAltairCam,
		    PIALTAIRCAM_CHROME_CALLBACK, void* );
HRESULT		( *p_Altaircam_LevelRangeAuto )( HAltairCam );
HRESULT		( *p_Altaircam_GetHistogram )( HAltairCam,
		    PIALTAIRCAM_HISTOGRAM_CALLBACK, void* );
HRESULT		( *p_Altaircam_put_LEDState )( HAltairCam, unsigned short,
		    unsigned short, unsigned short );
HRESULT		( *p_Altaircam_read_EEPROM )( HAltairCam, unsigned,
		    unsigned char*, unsigned );
HRESULT		( *p_Altaircam_write_EEPROM )( HAltairCam, unsigned,
		    const unsigned char*, unsigned );
HRESULT		( *p_Altaircam_get_Option )( HAltairCam, unsigned, unsigned* );
HRESULT		( *p_Altaircam_put_Option )( HAltairCam, unsigned, unsigned );
HRESULT		( *p_Altaircam_get_Roi )( HAltairCam, unsigned*, unsigned* );
HRESULT		( *p_Altaircam_put_Roi )( HAltairCam, unsigned, unsigned,
		    unsigned, unsigned );
HRESULT		( *p_Altaircam_ST4PlusGuide )( HAltairCam, unsigned, unsigned );
HRESULT		( *p_Altaircam_ST4PlusGuideState )( HAltairCam );
double		( *p_Altaircam_calc_ClarityFactor )( const void*, int,
		    unsigned, unsigned );
void		( *p_Altaircam_deBayerV2 )( unsigned, int, int, const void*,
		    void*, unsigned char );
void		( *p_Altaircam_HotPlug )( PALTAIRCAM_HOTPLUG, void* );

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


static void*		_getDLSym ( void*, const char* );
static void			_patchLibrary ( void* );

/**
 * Cycle through the list of cameras returned by the altaircam library
 */

int
_altairInitLibraryFunctionPointers ( void )
{
	unsigned				( *p_Toupcam_EnumV2 )( AltaircamInstV2* );
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];
	int							oalib;

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*		libName = "libaltaircam.dylib";
#else
  const char*		libName = "libaltaircam.so.1";
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

	  dlerror();

	  if (!( *( void** )( &p_Altaircam_AwbInit ) = _getDLSym ( libHandle,
	      "Altaircam_AwbInit" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_AwbOnePush ) = _getDLSym ( libHandle,
	      "Altaircam_AwbOnePush" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_calc_ClarityFactor ) =
				_getDLSym ( libHandle, "Altaircam_calc_ClarityFactor" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_Close ) = _getDLSym ( libHandle,
	      "Altaircam_Close" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

		/*
	  if (!( *( void** )( &p_Altaircam_deBayerV2 ) = _getDLSym ( libHandle,
	      "Altaircam_deBayerV2" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
		 */

	  if (!( *( void** )( &p_Altaircam_EnumV2 ) = _getDLSym ( libHandle,
	      "Altaircam_EnumV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_Flush ) = _getDLSym ( libHandle,
	      "Altaircam_Flush" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_AEAuxRect ) = _getDLSym ( libHandle,
	      "Altaircam_get_AEAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_AutoExpoEnable ) =
				_getDLSym ( libHandle, "Altaircam_get_AutoExpoEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_AutoExpoTarget ) =
				_getDLSym ( libHandle, "Altaircam_get_AutoExpoTarget" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_AWBAuxRect ) = _getDLSym ( libHandle,
	      "Altaircam_get_AWBAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Brightness ) = _getDLSym ( libHandle,
	      "Altaircam_get_Brightness" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Chrome ) = _getDLSym ( libHandle,
	      "Altaircam_get_Chrome" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Contrast ) = _getDLSym ( libHandle,
	      "Altaircam_get_Contrast" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_eSize ) = _getDLSym ( libHandle,
	      "Altaircam_get_eSize" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_ExpoAGain ) = _getDLSym ( libHandle,
	      "Altaircam_get_ExpoAGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_ExpoAGainRange ) =
				_getDLSym ( libHandle, "Altaircam_get_ExpoAGainRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_ExpoTime ) = _getDLSym ( libHandle,
	      "Altaircam_get_ExpoTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_ExpTimeRange ) = _getDLSym ( libHandle,
	      "Altaircam_get_ExpTimeRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_get_FanMaxSpeed ) = _getDLSym ( libHandle,
	      "Altaircam_get_FanMaxSpeed" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Field ) = _getDLSym ( libHandle,
	      "Altaircam_get_Field" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &p_Altaircam_get_FwVersion ) = _getDLSym ( libHandle,
	      "Altaircam_get_FwVersion" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Gamma ) = _getDLSym ( libHandle,
	      "Altaircam_get_Gamma" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_HFlip ) = _getDLSym ( libHandle,
	      "Altaircam_get_HFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_GetHistogram ) = _getDLSym ( libHandle,
	      "Altaircam_GetHistogram" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Hue ) = _getDLSym ( libHandle,
	      "Altaircam_get_Hue" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_HwVersion ) = _getDLSym ( libHandle,
	      "Altaircam_get_HwVersion" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_HZ ) = _getDLSym ( libHandle,
	      "Altaircam_get_HZ" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_LevelRange ) = _getDLSym ( libHandle,
	      "Altaircam_get_LevelRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_MaxBitDepth ) = _getDLSym ( libHandle,
	      "Altaircam_get_MaxBitDepth" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_MaxSpeed ) = _getDLSym ( libHandle,
	      "Altaircam_get_MaxSpeed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Mode ) = _getDLSym ( libHandle,
	      "Altaircam_get_Mode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_MonoMode ) = _getDLSym ( libHandle,
	      "Altaircam_get_MonoMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Negative ) = _getDLSym ( libHandle,
	      "Altaircam_get_Negative" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Option ) = _getDLSym ( libHandle,
	      "Altaircam_get_Option" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_get_PixelSize ) = _getDLSym ( libHandle,
	      "Altaircam_get_PixelSize" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &p_Altaircam_get_ProductionDate ) =
				_getDLSym ( libHandle, "Altaircam_get_ProductionDate" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_RawFormat ) = _getDLSym ( libHandle,
	      "Altaircam_get_RawFormat" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_RealTime ) = _getDLSym ( libHandle,
	      "Altaircam_get_RealTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Resolution ) = _getDLSym ( libHandle,
	      "Altaircam_get_Resolution" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_ResolutionNumber ) = _getDLSym (
	      libHandle, "Altaircam_get_ResolutionNumber" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_ResolutionRatio ) = _getDLSym (
	      libHandle, "Altaircam_get_ResolutionRatio" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Roi ) = _getDLSym ( libHandle,
	      "Altaircam_get_Roi" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_get_RoiMode ) = _getDLSym ( libHandle,
	      "Altaircam_get_RoiMode" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &p_Altaircam_get_Saturation ) = _getDLSym ( libHandle,
	      "Altaircam_get_Saturation" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_SerialNumber ) =
				_getDLSym ( libHandle, "Altaircam_get_SerialNumber" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Size ) = _getDLSym ( libHandle,
	      "Altaircam_get_Size" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Speed ) = _getDLSym ( libHandle,
	      "Altaircam_get_Speed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_StillResolution ) = _getDLSym (
	      libHandle, "Altaircam_get_StillResolution" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_StillResolutionNumber ) = _getDLSym (
	      libHandle, "Altaircam_get_StillResolutionNumber" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_Temperature ) = _getDLSym ( libHandle,
	      "Altaircam_get_Temperature" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_TempTint ) = _getDLSym ( libHandle,
	      "Altaircam_get_TempTint" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_VFlip ) = _getDLSym ( libHandle,
	      "Altaircam_get_VFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_get_VignetAmountInt ) = _getDLSym (
	      libHandle, "Altaircam_get_VignetAmountInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_VignetEnable ) = _getDLSym ( libHandle,
	      "Altaircam_get_VignetEnable" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_get_VignetMidPointInt ) = _getDLSym (
	      libHandle, "Altaircam_get_VignetMidPointInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &p_Altaircam_get_WhiteBalanceGain ) = _getDLSym (
	      libHandle, "Altaircam_get_WhiteBalanceGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_HotPlug ) = _getDLSym ( libHandle,
	      "Altaircam_HotPlug" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_LevelRangeAuto ) = _getDLSym ( libHandle,
	      "Altaircam_LevelRangeAuto" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_Open ) = _getDLSym ( libHandle,
	      "Altaircam_Open" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_OpenByIndex ) = _getDLSym ( libHandle,
	      "Altaircam_OpenByIndex" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_Pause ) = _getDLSym ( libHandle,
	      "Altaircam_Pause" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_PullImageV2 ) = _getDLSym ( libHandle,
	      "Altaircam_PullImageV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_PullStillImageV2 ) = _getDLSym ( libHandle,
	      "Altaircam_PullStillImageV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_AEAuxRect ) = _getDLSym ( libHandle,
	      "Altaircam_put_AEAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_AutoExpoEnable ) =
				_getDLSym ( libHandle, "Altaircam_put_AutoExpoEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_AutoExpoTarget ) =
				_getDLSym ( libHandle, "Altaircam_put_AutoExpoTarget" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_AWBAuxRect ) = _getDLSym ( libHandle,
	      "Altaircam_put_AWBAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Brightness ) = _getDLSym ( libHandle,
	      "Altaircam_put_Brightness" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Chrome ) = _getDLSym ( libHandle,
	      "Altaircam_put_Chrome" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_ChromeCallback ) =
				_getDLSym ( libHandle, "Altaircam_put_ChromeCallback" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Contrast ) = _getDLSym ( libHandle,
	      "Altaircam_put_Contrast" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_eSize ) = _getDLSym ( libHandle,
	      "Altaircam_put_eSize" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_ExpoAGain ) = _getDLSym ( libHandle,
	      "Altaircam_put_ExpoAGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_ExpoCallback ) = _getDLSym ( libHandle,
	      "Altaircam_put_ExpoCallback" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_ExpoTime ) = _getDLSym ( libHandle,
	      "Altaircam_put_ExpoTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Gamma ) = _getDLSym ( libHandle,
	      "Altaircam_put_Gamma" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_HFlip ) = _getDLSym ( libHandle,
	      "Altaircam_put_HFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Hue ) = _getDLSym ( libHandle,
	      "Altaircam_put_Hue" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_HZ ) = _getDLSym ( libHandle,
	      "Altaircam_put_HZ" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_LEDState ) = _getDLSym ( libHandle,
	      "Altaircam_put_LEDState" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_LevelRange ) = _getDLSym ( libHandle,
	      "Altaircam_put_LevelRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_MaxAutoExpoTimeAGain ) = _getDLSym (
	      libHandle, "Altaircam_put_MaxAutoExpoTimeAGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Mode ) = _getDLSym ( libHandle,
	      "Altaircam_put_Mode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Negative ) = _getDLSym ( libHandle,
	      "Altaircam_put_Negative" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Option ) = _getDLSym ( libHandle,
	      "Altaircam_put_Option" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_RealTime ) = _getDLSym ( libHandle,
	      "Altaircam_put_RealTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Roi ) = _getDLSym ( libHandle,
	      "Altaircam_put_Roi" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_put_RoiMode ) = _getDLSym ( libHandle,
	      "Altaircam_put_RoiMode" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &p_Altaircam_put_Saturation ) = _getDLSym ( libHandle,
	      "Altaircam_put_Saturation" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Size ) = _getDLSym ( libHandle,
	      "Altaircam_put_Size" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Speed ) = _getDLSym ( libHandle,
	      "Altaircam_put_Speed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_Temperature ) = _getDLSym ( libHandle,
	      "Altaircam_put_Temperature" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_TempTint ) = _getDLSym ( libHandle,
	      "Altaircam_put_TempTint" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_VFlip ) = _getDLSym ( libHandle,
	      "Altaircam_put_VFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_put_VignetAmountInt ) = _getDLSym (
	      libHandle, "Altaircam_put_VignetAmountInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_VignetEnable ) = _getDLSym ( libHandle,
	      "Altaircam_put_VignetEnable" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_put_VignetMidPointInt ) = _getDLSym (
	      libHandle, "Altaircam_put_VignetMidPointInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &p_Altaircam_put_WhiteBalanceGain ) = _getDLSym (
	      libHandle, "Altaircam_put_WhiteBalanceGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_read_EEPROM ) = _getDLSym ( libHandle,
	      "Altaircam_read_EEPROM" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_read_UART ) = _getDLSym ( libHandle,
	      "Altaircam_read_UART" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &p_Altaircam_Snap ) = _getDLSym ( libHandle,
	      "Altaircam_Snap" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_ST4PlusGuide ) = _getDLSym ( libHandle,
	      "Altaircam_ST4PlusGuide" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_ST4PlusGuideState ) =
				_getDLSym ( libHandle, "Altaircam_ST4PlusGuideState" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_StartPullModeWithCallback ) = _getDLSym (
	      libHandle, "Altaircam_StartPullModeWithCallback" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_StartPushModeV2 ) = _getDLSym ( libHandle,
	      "Altaircam_StartPushModeV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_Stop ) = _getDLSym ( libHandle,
	      "Altaircam_Stop" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_Trigger ) = _getDLSym ( libHandle,
	      "Altaircam_Trigger" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_Version ) = _getDLSym ( libHandle,
	      "Altaircam_Version" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_Altaircam_write_EEPROM ) = _getDLSym ( libHandle,
	      "Altaircam_write_EEPROM" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &p_Altaircam_write_UART ) = _getDLSym ( libHandle,
	      "Altaircam_write_UART" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

		oalib = !strcmp ( "32.13483.20181206", p_Altaircam_Version());
	  if ( oalib ) {
			if (( *( void** )( &p_Toupcam_EnumV2 ) = _getDLSym ( libHandle,
				  "_Z14Toupcam_EnumV2P13ToupcamInstV2" ))) {
				// Now comes the really ugly bit.  Patch the data section of the loaded
				// Altair library to match the new USB product IDs.  Actually, this
				// probably even gives "ugly" a bad name.
				_patchLibrary ( p_Toupcam_EnumV2 );
			}
		}
	}
	
	return OA_ERR_NONE;
}


static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libaltaircam DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}


#ifdef DYNLIB_EXT_DYLIB

// This version is for MacOS, possibly 64-bit only

void
_patchLibrary ( void* p )
{
  uint8_t*		enumFunction;
  uint8_t*		lea;
  uint8_t*		rip;
  uint8_t*		pidTableStart = 0;
  uint8_t*		pidTableEnd = 0;
  uint8_t*		pidPos;
  uint8_t*		nextPid;
  int32_t		offset;
	int					found = 0;
  uint16_t		pid;

  enumFunction = (uint8_t*) p;

  lea = enumFunction + 0x8b;
  rip = lea + 0x07;

  if ( *lea == 0x4c && *(lea+1) == 0x8d && *(lea+2) == 0x3d ) {
    lea += 3;
    offset = *lea++;
    offset |= ( *lea++ ) << 8;
    offset |= ( *lea++ ) << 16;
    offset |= ( *lea++ ) << 24;

    pidTableStart = rip + offset;
  } else {
    fprintf ( stderr, "lea instruction not found at address %p\n", lea );
    for ( offset = 0; offset < 16; offset++ ) {
      fprintf ( stderr, "lea + %02x: %02x\n", offset, *( lea + offset ));
    }
    return;
  }

	/*
  offset = 0;
  while ( offset < 0x200 ) {
    if ( offset % 16 == 0 ) {
      fprintf ( stderr, "%04x  ", offset );
    }
    fprintf ( stderr, "%02x ", *pidTableStart++ );
    offset++;
    if ( offset % 16 == 0 ) {
      fprintf ( stderr, "\n" );
    }
  }
	 */

  pidPos = pidTableStart;
	pidTableEnd = pidTableStart + ( 0x200 * 0x20 );
  while (( pidPos < pidTableEnd ) && !found ) {
    nextPid = pidPos;
    pid = *nextPid;
    pid |= *( nextPid + 1 ) << 8;
    // fprintf ( stderr, "pid found: 0x%04x\n", pid );
    if ( pid == 0xb135 ) {
      nextPid += 2;
      pid = *nextPid;
      pid |= *( nextPid + 1 ) << 8;
      if ( pid ) {
        nextPid += 2;
        pid = *nextPid;
        pid |= *( nextPid + 1 ) << 8;
        if ( pid ) {
          fprintf ( stderr, "no spare spaces in PID table after 0xb135\n" );
          return;
        }
      }
      *nextPid = 0x2a;
      *( nextPid + 1 ) = 0x0b;
      found = 1;
      // fprintf ( stderr, "0x0b2a PID added at address %p\n", nextPid );
    }
    pidPos += 0x20;
  }

  if ( !found ) {
    fprintf ( stderr, "PID 0xb135 not found in PID table\n" );
  }
}

#else

// And this one for x86 Linux (possibly 64-bit only)

void
_patchLibrary ( void* p )
{
  uint8_t*		enumFunction;
  uint8_t*		lea;
  uint8_t*		rip;
  uint8_t*		pidTableStart = 0;
  uint8_t*		pidTableEnd = 0;
  uint8_t*		pidPos;
  uint8_t*		nextPid;
  int32_t		offset;
  uint16_t		pid;
	int					found = 0;

  enumFunction = (uint8_t*) p;

	/*
	for ( offset = 0; offset < 256; offset++ ) {
		fprintf ( stderr, "%02x ", *( enumFunction + offset ));
		if ( offset % 16 == 15 ) {
			fprintf ( stderr, "\n" );
		}
	}
	*/

  lea = enumFunction + 0xa5;
  rip = lea + 0x07;

  if ( *lea == 0x48 && *(lea+1) == 0x8d && *(lea+2) == 0x1d ) {
    lea += 3;
    offset = *lea++;
    offset |= ( *lea++ ) << 8;
    offset |= ( *lea++ ) << 16;
    offset |= ( *lea++ ) << 24;

    // fprintf ( stderr, "offset = %04x\n", offset );

    pidTableStart = rip + offset;
  } else {
		fprintf ( stderr, "lea instruction #1 not found at address %p\n", lea );
		for ( offset = 0; offset < 16; offset++ ) {
			fprintf ( stderr, "lea + %02x: %02x\n", offset, *( lea + offset ));
		}
		return;
	}

  lea = enumFunction + 0x87;
  rip = lea + 0x07;

  if ( *lea == 0x4c && *(lea+1) == 0x8d && *(lea+2) == 0x35 ) {
    lea += 3;
    offset = *lea++;
    offset |= ( *lea++ ) << 8;
    offset |= ( *lea++ ) << 16;
    offset |= ( *lea++ ) << 24;

    // fprintf ( stderr, "offset = %04x\n", offset );

    pidTableEnd = rip + offset;
  } else {
		fprintf ( stderr, "lea instruction #2 not found at address %p\n", lea );
		for ( offset = 0; offset < 16; offset++ ) {
			fprintf ( stderr, "lea + %02x: %02x\n", offset, *( lea + offset ));
		}
		return;
  }

  // fprintf ( stderr, "pid = %p to %p\n", pidTableStart, pidTableEnd );

  pidPos = pidTableStart;
  while (( pidPos < pidTableEnd ) && !found ) {
		nextPid = pidPos;
    pid = *nextPid;
    pid |= *( nextPid + 1 ) << 8;
		// fprintf ( stderr, "pid found: 0x%04x\n", pid );
		if ( pid == 0xb135 ) {
			nextPid += 2;
			pid = *nextPid;
			pid |= *( nextPid + 1 ) << 8;
			if ( pid ) {
				nextPid += 2;
				pid = *nextPid;
				pid |= *( nextPid + 1 ) << 8;
				if ( pid ) {
					fprintf ( stderr, "no spare spaces in PID table after 0xb135\n" );
					return;
				}
			}
			*nextPid = 0x2a;
			*( nextPid + 1 ) = 0x0b;
			found = 1;
			// fprintf ( stderr, "0x0b2a PID added at address %p\n", nextPid );
    }
    pidPos += 0x20;
  }

	if ( !found ) {
		fprintf ( stderr, "PID 0xb135 not found in PID table\n" );
	}

	/*
  offset = 0;
  while ( pidTableStart < pidTableEnd ) {
    if ( offset % 16 == 0 ) {
      fprintf ( stderr, "%04x  ", offset );
    }
    fprintf ( stderr, "%02x ", *pidTableStart++ );
    offset++;
    if ( offset % 16 == 0 ) {
      fprintf ( stderr, "\n" );
    }
  }
	*/
}
#endif
