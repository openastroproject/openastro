/*****************************************************************************
 *
 * dynloader.c -- handle dynamic loading of interface library
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

#include "touptek-conf.h"
#include "oacamprivate.h"
#include "unimplemented.h"
#include "touptekoacam.h"
#include "touptekprivate.h"

// Pointers to library functions so we can use them via libdl.

const char*	( *TT_LIB_FUNC( Version ))();
unsigned	( *TT_LIB_FUNC( EnumV2 ))( TT_VAR_TYPE( InstV2* ));
TT_HANDLE	( *TT_LIB_FUNC( Open ))( const char* );
TT_HANDLE	( *TT_LIB_FUNC( OpenByIndex ))( unsigned );
void		( *TT_LIB_FUNC( Close ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( StartPullModeWithCallback ))( TT_HANDLE,
		    TT_FUNC_TYPE( P, EVENT_CALLBACK ), void* );
HRESULT		( *TT_LIB_FUNC( PullImageV2 ))( TT_HANDLE, void*, int,
		    TT_VAR_TYPE( FrameInfoV2* ));
HRESULT		( *TT_LIB_FUNC( PullStillImageV2 ))( TT_HANDLE, void*, int,
		    TT_VAR_TYPE( FrameInfoV2* ));
HRESULT		( *TT_LIB_FUNC( StartPushModeV2 ))( TT_HANDLE,
		    TT_FUNC_TYPE( P, DATA_CALLBACK_V2 ), void* );
HRESULT		( *TT_LIB_FUNC( Stop ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( Pause ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( Snap ))( TT_HANDLE, unsigned );
HRESULT		( *TT_LIB_FUNC( Trigger ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( get_Size ))( TT_HANDLE, int*, int* );
HRESULT		( *TT_LIB_FUNC( put_Size ))( TT_HANDLE, int, int );
HRESULT		( *TT_LIB_FUNC( get_eSize ))( TT_HANDLE, unsigned* );
HRESULT		( *TT_LIB_FUNC( put_eSize ))( TT_HANDLE, unsigned );
HRESULT		( *TT_LIB_FUNC( get_Resolution ))( TT_HANDLE, unsigned, int*,
		    int* );
HRESULT		( *TT_LIB_FUNC( get_ResolutionNumber ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( get_ResolutionRatio ))( TT_HANDLE, unsigned, int*,
		    int* );
HRESULT		( *TT_LIB_FUNC( get_RawFormat ))( TT_HANDLE, unsigned*,
		    unsigned* );
HRESULT		( *TT_LIB_FUNC( get_AutoExpoEnable ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( get_AutoExpoTarget ))( TT_HANDLE,
		    unsigned short* );
HRESULT		( *TT_LIB_FUNC( put_AutoExpoEnable ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( put_AutoExpoTarget ))( TT_HANDLE, unsigned short );
HRESULT		( *TT_LIB_FUNC( get_ExpoTime ))( TT_HANDLE, unsigned* );
HRESULT		( *TT_LIB_FUNC( get_ExpTimeRange ))( TT_HANDLE, unsigned*,
		    unsigned*, unsigned* );
HRESULT		( *TT_LIB_FUNC( put_ExpoTime ))( TT_HANDLE, unsigned );
HRESULT		( *TT_LIB_FUNC( put_MaxAutoExpoTimeAGain ))( TT_HANDLE, unsigned,
		    unsigned short );
HRESULT		( *TT_LIB_FUNC( get_ExpoAGain ))( TT_HANDLE, unsigned short* );
HRESULT		( *TT_LIB_FUNC( put_ExpoAGain ))( TT_HANDLE, unsigned short );
HRESULT		( *TT_LIB_FUNC( get_ExpoAGainRange ))( TT_HANDLE, unsigned short*,
		    unsigned short*, unsigned short* );
HRESULT		( *TT_LIB_FUNC( AwbInit ))( TT_HANDLE,
		    TT_FUNC_TYPE( PI, WHITEBALANCE_CALLBACK ), void* );
HRESULT		( *TT_LIB_FUNC( AwbOnePush ))( TT_HANDLE,
		    TT_FUNC_TYPE( PI, TEMPTINT_CALLBACK ), void* );
HRESULT		( *TT_LIB_FUNC( get_TempTint ))( TT_HANDLE, int*, int* );
HRESULT		( *TT_LIB_FUNC( put_TempTint ))( TT_HANDLE, int, int );
HRESULT		( *TT_LIB_FUNC( get_WhiteBalanceGain ))( TT_HANDLE, int[3] );
HRESULT		( *TT_LIB_FUNC( put_WhiteBalanceGain ))( TT_HANDLE, int[3] );
HRESULT		( *TT_LIB_FUNC( get_Hue ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Hue ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_Saturation ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Saturation ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_Brightness ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Brightness ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_Contrast ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Contrast ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_Gamma ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Gamma ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_Chrome ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Chrome ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_VFlip ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_VFlip ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_HFlip ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_HFlip ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_Negative ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Negative ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_MaxSpeed ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( get_Speed ))( TT_HANDLE, unsigned short* );
HRESULT		( *TT_LIB_FUNC( put_Speed ))( TT_HANDLE, unsigned short );
HRESULT		( *TT_LIB_FUNC( get_MaxBitDepth ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( get_HZ ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_HZ ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_Mode ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_Mode ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( get_AWBAuxRect ))( TT_HANDLE, RECT* );
HRESULT		( *TT_LIB_FUNC( put_AWBAuxRect ))( TT_HANDLE, const RECT* );
HRESULT		( *TT_LIB_FUNC( get_AEAuxRect ))( TT_HANDLE, RECT* );
HRESULT		( *TT_LIB_FUNC( put_AEAuxRect ))( TT_HANDLE, const RECT* );
HRESULT		( *TT_LIB_FUNC( get_MonoMode ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( get_StillResolution ))( TT_HANDLE, unsigned, int*,
		    int* );
HRESULT		( *TT_LIB_FUNC( get_StillResolutionNumber ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( get_RealTime ))( TT_HANDLE, int* );
HRESULT		( *TT_LIB_FUNC( put_RealTime ))( TT_HANDLE, int );
HRESULT		( *TT_LIB_FUNC( Flush ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( get_Temperature ))( TT_HANDLE, short* );
HRESULT		( *TT_LIB_FUNC( put_Temperature ))( TT_HANDLE, short );
HRESULT		( *TT_LIB_FUNC( get_SerialNumber ))( TT_HANDLE, char[32] );
HRESULT		( *TT_LIB_FUNC( get_FwVersion ))( TT_HANDLE, char[16] );
HRESULT		( *TT_LIB_FUNC( get_HwVersion ))( TT_HANDLE, char[16] );
HRESULT		( *TT_LIB_FUNC( get_ProductionDate ))( TT_HANDLE, char[10] );
HRESULT		( *TT_LIB_FUNC( get_LevelRange ))( TT_HANDLE, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *TT_LIB_FUNC( put_LevelRange ))( TT_HANDLE, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *TT_LIB_FUNC( put_ExpoCallback ))( TT_HANDLE,
		    TT_FUNC_TYPE( PI, EXPOSURE_CALLBACK ), void* );
HRESULT		( *TT_LIB_FUNC( put_ChromeCallback ))( TT_HANDLE,
		    TT_FUNC_TYPE( PI, CHROME_CALLBACK ), void* );
HRESULT		( *TT_LIB_FUNC( LevelRangeAuto ))( TT_HANDLE );
HRESULT		( *TT_LIB_FUNC( GetHistogram ))( TT_HANDLE,
		    TT_FUNC_TYPE( PI, HISTOGRAM_CALLBACK ), void* );
HRESULT		( *TT_LIB_FUNC( put_LEDState ))( TT_HANDLE, unsigned short,
		    unsigned short, unsigned short );
HRESULT		( *TT_LIB_FUNC( read_EEPROM ))( TT_HANDLE, unsigned,
		    unsigned char*, unsigned );
HRESULT		( *TT_LIB_FUNC( write_EEPROM ))( TT_HANDLE, unsigned,
		    const unsigned char*, unsigned );
HRESULT		( *TT_LIB_FUNC( get_Option ))( TT_HANDLE, unsigned, unsigned* );
HRESULT		( *TT_LIB_FUNC( put_Option ))( TT_HANDLE, unsigned, unsigned );
HRESULT		( *TT_LIB_FUNC( get_Roi ))( TT_HANDLE, unsigned*, unsigned* );
HRESULT		( *TT_LIB_FUNC( put_Roi ))( TT_HANDLE, unsigned, unsigned,
		    unsigned, unsigned );
HRESULT		( *TT_LIB_FUNC( ST4PlusGuide ))( TT_HANDLE, unsigned, unsigned );
HRESULT		( *TT_LIB_FUNC( ST4PlusGuideState ))( TT_HANDLE );
double		( *TT_LIB_FUNC( calc_ClarityFactor ))( const void*, int,
		    unsigned, unsigned );
void		( *TT_LIB_FUNC( deBayerV2 ))( unsigned, int, int, const void*,
		    void*, unsigned char );
void		( *TT_LIB_FUNC( HotPlug ))( TT_FUNC_TYPE( P, HOTPLUG ), void* );

// These are apparently obsolete
//
// ..._get_RoiMode
// ..._put_RoiMode
// ..._get_VignetAmountInt
// ..._get_VignetEnable
// ..._get_VignetMidPointInt
// ..._put_VignetAmountInt
// ..._put_VignetEnable
// ..._put_VignetMidPointInt

// And these are not documented as far as I can see
//
// ..._AbbOnePush(ToupcamT*, void (*)(unsigned short const*, void*), void*)
// ..._DfcOnePush(ToupcamT*)
// ..._FfcOnePush(ToupcamT*)
// ..._get_ABBAuxRect(ToupcamT*, RECT*)
// ..._get_BlackBalance(ToupcamT*, unsigned short*)
// ..._get_FanMaxSpeed(ToupcamT*)
// ..._get_Field(ToupcamT*)
// ..._get_FpgaVersion(ToupcamT*, char*)
// ..._get_FrameRate(ToupcamT*, unsigned int*, unsigned int*, unsigned int*)
// ..._get_PixelSize(ToupcamT*, unsigned int, float*, float*)
// ..._get_Revision(ToupcamT*, unsigned short*)
// ..._InitOcl()
// ..._IoControl(ToupcamT*, unsigned int, unsigned int, int, int*)
// ..._PullImageWithRowPitch(ToupcamT*, void*, int, int, unsigned int*, unsigned int*)
// ..._PullStillImageWithRowPitch(ToupcamT*, void*, int, int, unsigned int*, unsigned int*)
// ..._put_ABBAuxRect(ToupcamT*, RECT const*)
// ..._put_BlackBalance(ToupcamT*, unsigned short*)
// ..._put_ColorMatrix(ToupcamT*, double const*)
// ..._put_Curve(ToupcamT*, unsigned char const*, unsigned short const*)
// ..._put_Demosaic(ToupcamT*, void (*)(unsigned int, int, int, void const*, void*, unsigned char, void*), void*)
// ..._put_InitWBGain(ToupcamT*, unsigned short const*)
// ..._put_Linear(ToupcamT*, unsigned char const*, unsigned short const*)
// ..._read_UART(ToupcamT*, unsigned char*, unsigned int)
// ..._StartOclWithSharedTexture(ToupcamT*, ToupcamOclWithSharedTexture const*, void (*)(unsigned int, void*), void*)
// ..._write_UART(ToupcamT*, unsigned char const*, unsigned int)


static void*		_getDLSym ( void*, const char* );
#ifdef	TT_PATCH_BINARY
static void			_patchLibrary ( void* );
#endif

/**
 * Cycle through the list of cameras returned by the library
 */

int
TT_FUNC( _, InitLibraryFunctionPointers )( void )
{
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];
#ifdef	TT_PATCH_BINARY
	int							oalib;
#endif

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*		libName = "lib" TT_SOLIB ".dylib";
#else
  const char*		libName = "lib" TT_SOLIB ".so.1";
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

	  if (!( *( void** )( &TT_LIB_FUNC( AwbInit )) = _getDLSym ( libHandle,
	      TT_DRIVER "_AwbInit" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( AwbOnePush )) = _getDLSym ( libHandle,
	      TT_DRIVER "_AwbOnePush" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( calc_ClarityFactor )) =
				_getDLSym ( libHandle, TT_DRIVER "_calc_ClarityFactor" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( Close )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Close" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

		/*
	  if (!( *( void** )( &TT_LIB_FUNC( deBayerV2 )) = _getDLSym ( libHandle,
	      TT_DRIVER "_deBayerV2" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
		 */

	  if (!( *( void** )( &TT_LIB_FUNC( EnumV2 )) = _getDLSym ( libHandle,
	      TT_DRIVER "_EnumV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( Flush )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Flush" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_AEAuxRect )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_AEAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_AutoExpoEnable )) =
				_getDLSym ( libHandle, TT_DRIVER "_get_AutoExpoEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_AutoExpoTarget )) =
				_getDLSym ( libHandle, TT_DRIVER "_get_AutoExpoTarget" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_AWBAuxRect )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_AWBAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Brightness )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Brightness" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Chrome )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Chrome" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Contrast )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Contrast" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_eSize )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_eSize" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_ExpoAGain )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_ExpoAGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_ExpoAGainRange )) =
				_getDLSym ( libHandle, TT_DRIVER "_get_ExpoAGainRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_ExpoTime )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_ExpoTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_ExpTimeRange )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_ExpTimeRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( get_FanMaxSpeed )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_FanMaxSpeed" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Field )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Field" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &TT_LIB_FUNC( get_FwVersion )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_FwVersion" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Gamma )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Gamma" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_HFlip )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_HFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( GetHistogram )) = _getDLSym ( libHandle,
	      TT_DRIVER "_GetHistogram" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Hue )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Hue" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_HwVersion )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_HwVersion" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_HZ )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_HZ" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_LevelRange )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_LevelRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_MaxBitDepth )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_MaxBitDepth" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_MaxSpeed )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_MaxSpeed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Mode )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Mode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_MonoMode )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_MonoMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Negative )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Negative" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Option )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Option" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( get_PixelSize )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_PixelSize" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &TT_LIB_FUNC( get_ProductionDate )) =
				_getDLSym ( libHandle, TT_DRIVER "_get_ProductionDate" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_RawFormat )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_RawFormat" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_RealTime )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_RealTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Resolution )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Resolution" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_ResolutionNumber )) = _getDLSym (
	      libHandle, TT_DRIVER "_get_ResolutionNumber" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_ResolutionRatio )) = _getDLSym (
	      libHandle, TT_DRIVER "_get_ResolutionRatio" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Roi )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Roi" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( get_RoiMode )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_RoiMode" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &TT_LIB_FUNC( get_Saturation )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Saturation" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_SerialNumber )) =
				_getDLSym ( libHandle, TT_DRIVER "_get_SerialNumber" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Size )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Size" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Speed )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Speed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_StillResolution )) = _getDLSym (
	      libHandle, TT_DRIVER "_get_StillResolution" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_StillResolutionNumber )) = _getDLSym (
	      libHandle, TT_DRIVER "_get_StillResolutionNumber" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_Temperature )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_Temperature" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_TempTint )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_TempTint" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_VFlip )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_VFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( get_VignetAmountInt )) = _getDLSym (
	      libHandle, TT_DRIVER "_get_VignetAmountInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_VignetEnable )) = _getDLSym ( libHandle,
	      TT_DRIVER "_get_VignetEnable" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( get_VignetMidPointInt )) = _getDLSym (
	      libHandle, TT_DRIVER "_get_VignetMidPointInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &TT_LIB_FUNC( get_WhiteBalanceGain )) = _getDLSym (
	      libHandle, TT_DRIVER "_get_WhiteBalanceGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( HotPlug )) = _getDLSym ( libHandle,
	      TT_DRIVER "_HotPlug" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( LevelRangeAuto )) = _getDLSym ( libHandle,
	      TT_DRIVER "_LevelRangeAuto" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( Open )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Open" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( OpenByIndex )) = _getDLSym ( libHandle,
	      TT_DRIVER "_OpenByIndex" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( Pause )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Pause" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( PullImageV2 )) = _getDLSym ( libHandle,
	      TT_DRIVER "_PullImageV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( PullStillImageV2 )) = _getDLSym ( libHandle,
	      TT_DRIVER "_PullStillImageV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_AEAuxRect )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_AEAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_AutoExpoEnable )) =
				_getDLSym ( libHandle, TT_DRIVER "_put_AutoExpoEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_AutoExpoTarget )) =
				_getDLSym ( libHandle, TT_DRIVER "_put_AutoExpoTarget" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_AWBAuxRect )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_AWBAuxRect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Brightness )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Brightness" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Chrome )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Chrome" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_ChromeCallback )) =
				_getDLSym ( libHandle, TT_DRIVER "_put_ChromeCallback" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Contrast )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Contrast" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_eSize )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_eSize" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_ExpoAGain )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_ExpoAGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_ExpoCallback )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_ExpoCallback" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_ExpoTime )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_ExpoTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Gamma )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Gamma" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_HFlip )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_HFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Hue )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Hue" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_HZ )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_HZ" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_LEDState )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_LEDState" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_LevelRange )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_LevelRange" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_MaxAutoExpoTimeAGain )) = _getDLSym (
	      libHandle, TT_DRIVER "_put_MaxAutoExpoTimeAGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Mode )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Mode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Negative )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Negative" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Option )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Option" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_RealTime )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_RealTime" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Roi )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Roi" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( put_RoiMode )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_RoiMode" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &TT_LIB_FUNC( put_Saturation )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Saturation" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Size )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Size" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Speed )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Speed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_Temperature )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_Temperature" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_TempTint )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_TempTint" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_VFlip )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_VFlip" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( put_VignetAmountInt )) = _getDLSym (
	      libHandle, TT_DRIVER "_put_VignetAmountInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_VignetEnable )) = _getDLSym ( libHandle,
	      TT_DRIVER "_put_VignetEnable" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( put_VignetMidPointInt )) = _getDLSym (
	      libHandle, TT_DRIVER "_put_VignetMidPointInt" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &TT_LIB_FUNC( put_WhiteBalanceGain )) = _getDLSym (
	      libHandle, TT_DRIVER "_put_WhiteBalanceGain" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( read_EEPROM )) = _getDLSym ( libHandle,
	      TT_DRIVER "_read_EEPROM" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( read_UART )) = _getDLSym ( libHandle,
	      TT_DRIVER "_read_UART" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

	  if (!( *( void** )( &TT_LIB_FUNC( Snap )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Snap" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( ST4PlusGuide )) = _getDLSym ( libHandle,
	      TT_DRIVER "_ST4PlusGuide" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( ST4PlusGuideState )) =
				_getDLSym ( libHandle, TT_DRIVER "_ST4PlusGuideState" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( StartPullModeWithCallback )) = _getDLSym (
	      libHandle, TT_DRIVER "_StartPullModeWithCallback" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( StartPushModeV2 )) = _getDLSym ( libHandle,
	      TT_DRIVER "_StartPushModeV2" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( Stop )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Stop" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( Trigger )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Trigger" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( Version )) = _getDLSym ( libHandle,
	      TT_DRIVER "_Version" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &TT_LIB_FUNC( write_EEPROM )) = _getDLSym ( libHandle,
	      TT_DRIVER "_write_EEPROM" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  /*
	  if (!( *( void** )( &TT_LIB_FUNC( write_UART )) = _getDLSym ( libHandle,
	      TT_DRIVER "_write_UART" ))) {
				dlclose ( libHandle );
				libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	   */

#ifdef	TT_PATCH_BINARY
		oalib = !strcmp ( "32.13483.20181206", TT_LIB_FUNC( Version()));
	  if ( oalib ) {
			unsigned				( *p_Toupcam_EnumV2 )( TT_VAR_TYPE( InstV2* ));
			if (( *( void** )( &p_Toupcam_EnumV2 ) = _getDLSym ( libHandle,
				  "_Z14Toupcam_EnumV2P13ToupcamInstV2" ))) {
				// Now comes the really ugly bit.  Patch the data section of the loaded
				// Touptek library to match the new USB product IDs.  Actually, this
				// probably even gives "ugly" a bad name.
				_patchLibrary ( p_Toupcam_EnumV2 );
			}
		}
#endif	/* TT_PATCH_BINARY */
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
    fprintf ( stderr, "lib" TT_SOLIB " DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}


#ifdef	TT_PATCH_BINARY
#ifdef	DYNLIB_EXT_DYLIB

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

#else	/* DYNLIB_EXT_DYLIB */

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
#endif	/* DYNLIB_EXT_DYLIB */
#endif	/* TT_PATCH_BINARY */
