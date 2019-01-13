/*****************************************************************************
 *
 * Altairoacam.c -- main entrypoint for Altair cameras
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

#include <oa_common.h>

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif
#include <openastro/camera.h>
#include <toupcam.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Altairoacam.h"

// Pointers to libaltaircam functions so we can use them via libdl.

const char*	( *p_Altaircam_Version )();
unsigned	( *p_Altaircam_Enum )( ToupcamInst* );
HToupCam	( *p_Altaircam_Open )( const char* );
HToupCam	( *p_Altaircam_OpenByIndex )( unsigned );
void		( *p_Altaircam_Close )( HToupCam );
HRESULT		( *p_Altaircam_StartPullModeWithCallback )( HToupCam,
		    PTOUPCAM_EVENT_CALLBACK, void* );
HRESULT		( *p_Altaircam_PullImage )( HToupCam, void*, int, unsigned*,
		    unsigned* );
HRESULT		( *p_Altaircam_PullStillImage )( HToupCam, void*, int,
		    unsigned*, unsigned* );
HRESULT		( *p_Altaircam_StartPushMode )( HToupCam,
		    PTOUPCAM_DATA_CALLBACK, void* );
HRESULT		( *p_Altaircam_Stop )( HToupCam );
HRESULT		( *p_Altaircam_Pause )( HToupCam, int );
HRESULT		( *p_Altaircam_Snap )( HToupCam, unsigned );
HRESULT		( *p_Altaircam_Trigger )( HToupCam );
HRESULT		( *p_Altaircam_get_Size )( HToupCam, int*, int* );
HRESULT		( *p_Altaircam_put_Size )( HToupCam, int, int );
HRESULT		( *p_Altaircam_get_eSize )( HToupCam, unsigned* );
HRESULT		( *p_Altaircam_put_eSize )( HToupCam, unsigned );
HRESULT		( *p_Altaircam_get_Resolution )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Altaircam_get_ResolutionNumber )( HToupCam );
HRESULT		( *p_Altaircam_get_ResolutionRatio )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Altaircam_get_RawFormat )( HToupCam, unsigned*,
		    unsigned* );
HRESULT		( *p_Altaircam_get_AutoExpoEnable )( HToupCam, int* );
HRESULT		( *p_Altaircam_get_AutoExpoTarget )( HToupCam,
		    unsigned short* );
HRESULT		( *p_Altaircam_put_AutoExpoEnable )( HToupCam, int );
HRESULT		( *p_Altaircam_put_AutoExpoTarget )( HToupCam, unsigned short );
HRESULT		( *p_Altaircam_get_ExpoTime )( HToupCam, unsigned* );
HRESULT		( *p_Altaircam_get_ExpTimeRange )( HToupCam, unsigned*,
		    unsigned*, unsigned* );
HRESULT		( *p_Altaircam_put_ExpoTime )( HToupCam, unsigned );
HRESULT		( *p_Altaircam_put_MaxAutoExpoTimeAGain )( HToupCam, unsigned,
		    unsigned short );
HRESULT		( *p_Altaircam_get_ExpoAGain )( HToupCam, unsigned short* );
HRESULT		( *p_Altaircam_put_ExpoAGain )( HToupCam, unsigned short );
HRESULT		( *p_Altaircam_get_ExpoAGainRange )( HToupCam, unsigned short*,
		    unsigned short*, unsigned short* );
HRESULT		( *p_Altaircam_AwbInit )( HToupCam,
		    PITOUPCAM_WHITEBALANCE_CALLBACK, void* );
HRESULT		( *p_Altaircam_AwbOnePush )( HToupCam,
		    PITOUPCAM_TEMPTINT_CALLBACK, void* );
HRESULT		( *p_Altaircam_get_TempTint )( HToupCam, int*, int* );
HRESULT		( *p_Altaircam_put_TempTint )( HToupCam, int, int );
HRESULT		( *p_Altaircam_get_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_Altaircam_put_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_Altaircam_get_Hue )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Hue )( HToupCam, int );
HRESULT		( *p_Altaircam_get_Saturation )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Saturation )( HToupCam, int );
HRESULT		( *p_Altaircam_get_Brightness )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Brightness )( HToupCam, int );
HRESULT		( *p_Altaircam_get_Contrast )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Contrast )( HToupCam, int );
HRESULT		( *p_Altaircam_get_Gamma )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Gamma )( HToupCam, int );
HRESULT		( *p_Altaircam_get_Chrome )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Chrome )( HToupCam, int );
HRESULT		( *p_Altaircam_get_VFlip )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_VFlip )( HToupCam, int );
HRESULT		( *p_Altaircam_get_HFlip )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_HFlip )( HToupCam, int );
HRESULT		( *p_Altaircam_get_Negative )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Negative )( HToupCam, int );
HRESULT		( *p_Altaircam_get_MaxSpeed )( HToupCam );
HRESULT		( *p_Altaircam_get_Speed )( HToupCam, unsigned short* );
HRESULT		( *p_Altaircam_put_Speed )( HToupCam, unsigned short );
HRESULT		( *p_Altaircam_get_MaxBitDepth )( HToupCam );
HRESULT		( *p_Altaircam_get_HZ )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_HZ )( HToupCam, int );
HRESULT		( *p_Altaircam_get_Mode )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_Mode )( HToupCam, int );
HRESULT		( *p_Altaircam_get_AWBAuxRect )( HToupCam, RECT* );
HRESULT		( *p_Altaircam_put_AWBAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_Altaircam_get_AEAuxRect )( HToupCam, RECT* );
HRESULT		( *p_Altaircam_put_AEAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_Altaircam_get_MonoMode )( HToupCam );
HRESULT		( *p_Altaircam_get_StillResolution )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Altaircam_get_StillResolutionNumber )( HToupCam );
HRESULT		( *p_Altaircam_get_RealTime )( HToupCam, int* );
HRESULT		( *p_Altaircam_put_RealTime )( HToupCam, int );
HRESULT		( *p_Altaircam_Flush )( HToupCam );
HRESULT		( *p_Altaircam_get_Temperature )( HToupCam, short* );
HRESULT		( *p_Altaircam_put_Temperature )( HToupCam, short );
HRESULT		( *p_Altaircam_get_SerialNumber )( HToupCam, char[32] );
HRESULT		( *p_Altaircam_get_FwVersion )( HToupCam, char[16] );
HRESULT		( *p_Altaircam_get_HwVersion )( HToupCam, char[16] );
HRESULT		( *p_Altaircam_get_ProductionDate )( HToupCam, char[10] );
HRESULT		( *p_Altaircam_get_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Altaircam_put_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Altaircam_put_ExpoCallback )( HToupCam,
		    PITOUPCAM_EXPOSURE_CALLBACK, void* );
HRESULT		( *p_Altaircam_put_ChromeCallback )( HToupCam,
		    PITOUPCAM_CHROME_CALLBACK, void* );
HRESULT		( *p_Altaircam_LevelRangeAuto )( HToupCam );
HRESULT		( *p_Altaircam_GetHistogram )( HToupCam,
		    PITOUPCAM_HISTOGRAM_CALLBACK, void* );
HRESULT		( *p_Altaircam_put_LEDState )( HToupCam, unsigned short,
		    unsigned short, unsigned short );
HRESULT		( *p_Altaircam_read_EEPROM )( HToupCam, unsigned,
		    unsigned char*, unsigned );
HRESULT		( *p_Altaircam_write_EEPROM )( HToupCam, unsigned,
		    const unsigned char*, unsigned );
HRESULT		( *p_Altaircam_get_Option )( HToupCam, unsigned, unsigned* );
HRESULT		( *p_Altaircam_put_Option )( HToupCam, unsigned, unsigned );
HRESULT		( *p_Altaircam_get_Roi )( HToupCam, unsigned*, unsigned* );
HRESULT		( *p_Altaircam_put_Roi )( HToupCam, unsigned, unsigned,
		    unsigned, unsigned );
HRESULT		( *p_Altaircam_ST4PlusGuide )( HToupCam, unsigned, unsigned );
HRESULT		( *p_Altaircam_ST4PlusGuideState )( HToupCam );
double		( *p_Altaircam_calc_ClarityFactor )( const void*, int,
		    unsigned, unsigned );
void		( *p_Altaircam_deBayer )( unsigned, int, int, const void*,
		    void*, unsigned char );
void		( *p_Altaircam_HotPlug )( PTOUPCAM_HOTPLUG, void* );

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
#if 0
static void		_patchLibrary ( void* );
#endif

#define ALTAIR_PREFIX	1
#define TOUPCAM_PREFIX	2

/**
 * Cycle through the list of cameras returned by the altaircam library
 */

int
oaAltairGetCameras ( CAMERA_LIST* deviceList, int flags )
{
  ToupcamInst		devList[ TOUPCAM_MAX ];
  unsigned int		numCameras;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int                   ret, prefix;
#if 0
  int                   oalib;
#endif
  static void*		libHandle = 0;

  // On Linux, the only place we're going to look for this library is
  // in the default installation directory for the AltairCapture application
  // which is /usr/local/AltairCapture.
  // 
  // On MacOS we just try /Applications/AltairCapture.app/Contents/MacOS

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*		libName = "/Applications/AltairCapture.app/Contents/"
                            "MacOS/libaltaircam.dylib";
#else
  const char*		libName = "/usr/local/AltairCapture/libaltaircam.so";
#endif

  if ( !libHandle ) {
    if (!( libHandle = dlopen ( libName, RTLD_LAZY ))) {
      // fprintf ( stderr, "can't load %s\n", libraryPath );
      return 0;
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
  if (!( *( void** )( &p_Altaircam_AwbInit ) = _getDLSym ( libHandle,
      "AwbInit", prefix ))) {
    prefix = TOUPCAM_PREFIX;
    if (!( *( void** )( &p_Altaircam_AwbInit ) = _getDLSym ( libHandle,
        "AwbInit", prefix ))) {
      return 0;
    }
  }

  if (!( *( void** )( &p_Altaircam_AwbOnePush ) = _getDLSym ( libHandle,
      "AwbOnePush", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_calc_ClarityFactor ) = _getDLSym ( libHandle,
      "calc_ClarityFactor", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Close ) = _getDLSym ( libHandle,
      "Close", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_deBayer ) = _getDLSym ( libHandle,
      "deBayer", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Enum ) = _getDLSym ( libHandle,
      "Enum", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Flush ) = _getDLSym ( libHandle,
      "Flush", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AEAuxRect ) = _getDLSym ( libHandle,
      "get_AEAuxRect", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AutoExpoEnable ) = _getDLSym ( libHandle,
      "get_AutoExpoEnable", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AutoExpoTarget ) = _getDLSym ( libHandle,
      "get_AutoExpoTarget", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AWBAuxRect ) = _getDLSym ( libHandle,
      "get_AWBAuxRect", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Brightness ) = _getDLSym ( libHandle,
      "get_Brightness", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Chrome ) = _getDLSym ( libHandle,
      "get_Chrome", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Contrast ) = _getDLSym ( libHandle,
      "get_Contrast", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_eSize ) = _getDLSym ( libHandle,
      "get_eSize", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpoAGain ) = _getDLSym ( libHandle,
      "get_ExpoAGain", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpoAGainRange ) = _getDLSym ( libHandle,
      "get_ExpoAGainRange", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpoTime ) = _getDLSym ( libHandle,
      "get_ExpoTime", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpTimeRange ) = _getDLSym ( libHandle,
      "get_ExpTimeRange", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_FanMaxSpeed ) = _getDLSym ( libHandle,
      "get_FanMaxSpeed", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Field ) = _getDLSym ( libHandle,
      "get_Field", prefix ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_FwVersion ) = _getDLSym ( libHandle,
      "get_FwVersion", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Gamma ) = _getDLSym ( libHandle,
      "get_Gamma", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_HFlip ) = _getDLSym ( libHandle,
      "get_HFlip", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_GetHistogram ) = _getDLSym ( libHandle,
      "GetHistogram", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Hue ) = _getDLSym ( libHandle,
      "get_Hue", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_HwVersion ) = _getDLSym ( libHandle,
      "get_HwVersion", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_HZ ) = _getDLSym ( libHandle,
      "get_HZ", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_LevelRange ) = _getDLSym ( libHandle,
      "get_LevelRange", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_MaxBitDepth ) = _getDLSym ( libHandle,
      "get_MaxBitDepth", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_MaxSpeed ) = _getDLSym ( libHandle,
      "get_MaxSpeed", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Mode ) = _getDLSym ( libHandle,
      "get_Mode", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_MonoMode ) = _getDLSym ( libHandle,
      "get_MonoMode", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Negative ) = _getDLSym ( libHandle,
      "get_Negative", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Option ) = _getDLSym ( libHandle,
      "get_Option", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_PixelSize ) = _getDLSym ( libHandle,
      "get_PixelSize", prefix ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_ProductionDate ) = _getDLSym ( libHandle,
      "get_ProductionDate", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_RawFormat ) = _getDLSym ( libHandle,
      "get_RawFormat", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_RealTime ) = _getDLSym ( libHandle,
      "get_RealTime", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Resolution ) = _getDLSym ( libHandle,
      "get_Resolution", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ResolutionNumber ) = _getDLSym (
      libHandle, "get_ResolutionNumber", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ResolutionRatio ) = _getDLSym (
      libHandle, "get_ResolutionRatio", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Roi ) = _getDLSym ( libHandle,
      "get_Roi", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_RoiMode ) = _getDLSym ( libHandle,
      "get_RoiMode", prefix ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_Saturation ) = _getDLSym ( libHandle,
      "get_Saturation", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_SerialNumber ) = _getDLSym ( libHandle,
      "get_SerialNumber", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Size ) = _getDLSym ( libHandle,
      "get_Size", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Speed ) = _getDLSym ( libHandle,
      "get_Speed", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_StillResolution ) = _getDLSym (
      libHandle, "get_StillResolution", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_StillResolutionNumber ) = _getDLSym (
      libHandle, "get_StillResolutionNumber", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Temperature ) = _getDLSym ( libHandle,
      "get_Temperature", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_TempTint ) = _getDLSym ( libHandle,
      "get_TempTint", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_VFlip ) = _getDLSym ( libHandle,
      "get_VFlip", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_VignetAmountInt ) = _getDLSym (
      libHandle, "get_VignetAmountInt", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_VignetEnable ) = _getDLSym ( libHandle,
      "get_VignetEnable", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_VignetMidPointInt ) = _getDLSym (
      libHandle, "get_VignetMidPointInt", prefix ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_WhiteBalanceGain ) = _getDLSym (
      libHandle, "get_WhiteBalanceGain", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_HotPlug ) = _getDLSym ( libHandle,
      "HotPlug", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_LevelRangeAuto ) = _getDLSym ( libHandle,
      "LevelRangeAuto", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Open ) = _getDLSym ( libHandle,
      "Open", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_OpenByIndex ) = _getDLSym ( libHandle,
      "OpenByIndex", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Pause ) = _getDLSym ( libHandle,
      "Pause", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_PullImage ) = _getDLSym ( libHandle,
      "PullImage", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_PullStillImage ) = _getDLSym ( libHandle,
      "PullStillImage", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AEAuxRect ) = _getDLSym ( libHandle,
      "put_AEAuxRect", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AutoExpoEnable ) = _getDLSym ( libHandle,
      "put_AutoExpoEnable", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AutoExpoTarget ) = _getDLSym ( libHandle,
      "put_AutoExpoTarget", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AWBAuxRect ) = _getDLSym ( libHandle,
      "put_AWBAuxRect", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Brightness ) = _getDLSym ( libHandle,
      "put_Brightness", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Chrome ) = _getDLSym ( libHandle,
      "put_Chrome", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ChromeCallback ) = _getDLSym ( libHandle,
      "put_ChromeCallback", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Contrast ) = _getDLSym ( libHandle,
      "put_Contrast", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_eSize ) = _getDLSym ( libHandle,
      "put_eSize", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ExpoAGain ) = _getDLSym ( libHandle,
      "put_ExpoAGain", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ExpoCallback ) = _getDLSym ( libHandle,
      "put_ExpoCallback", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ExpoTime ) = _getDLSym ( libHandle,
      "put_ExpoTime", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Gamma ) = _getDLSym ( libHandle,
      "put_Gamma", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_HFlip ) = _getDLSym ( libHandle,
      "put_HFlip", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Hue ) = _getDLSym ( libHandle,
      "put_Hue", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_HZ ) = _getDLSym ( libHandle,
      "put_HZ", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_LEDState ) = _getDLSym ( libHandle,
      "put_LEDState", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_LevelRange ) = _getDLSym ( libHandle,
      "put_LevelRange", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_MaxAutoExpoTimeAGain ) = _getDLSym (
      libHandle, "put_MaxAutoExpoTimeAGain", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Mode ) = _getDLSym ( libHandle,
      "put_Mode", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Negative ) = _getDLSym ( libHandle,
      "put_Negative", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Option ) = _getDLSym ( libHandle,
      "put_Option", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_RealTime ) = _getDLSym ( libHandle,
      "put_RealTime", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Roi ) = _getDLSym ( libHandle,
      "put_Roi", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_put_RoiMode ) = _getDLSym ( libHandle,
      "put_RoiMode", prefix ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_put_Saturation ) = _getDLSym ( libHandle,
      "put_Saturation", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Size ) = _getDLSym ( libHandle,
      "put_Size", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Speed ) = _getDLSym ( libHandle,
      "put_Speed", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Temperature ) = _getDLSym ( libHandle,
      "put_Temperature", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_TempTint ) = _getDLSym ( libHandle,
      "put_TempTint", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_VFlip ) = _getDLSym ( libHandle,
      "put_VFlip", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_put_VignetAmountInt ) = _getDLSym (
      libHandle, "put_VignetAmountInt", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_VignetEnable ) = _getDLSym ( libHandle,
      "put_VignetEnable", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_VignetMidPointInt ) = _getDLSym (
      libHandle, "put_VignetMidPointInt", prefix ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_put_WhiteBalanceGain ) = _getDLSym (
      libHandle, "put_WhiteBalanceGain", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_read_EEPROM ) = _getDLSym ( libHandle,
      "read_EEPROM", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_read_UART ) = _getDLSym ( libHandle,
      "read_UART", prefix ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_Snap ) = _getDLSym ( libHandle,
      "Snap", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_ST4PlusGuide ) = _getDLSym ( libHandle,
      "ST4PlusGuide", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_ST4PlusGuideState ) = _getDLSym ( libHandle,
      "ST4PlusGuideState", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_StartPullModeWithCallback ) = _getDLSym (
      libHandle, "StartPullModeWithCallback", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_StartPushMode ) = _getDLSym ( libHandle,
      "StartPushMode", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Stop ) = _getDLSym ( libHandle,
      "Stop", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Trigger ) = _getDLSym ( libHandle,
      "Trigger", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Version ) = _getDLSym ( libHandle,
      "Version", prefix ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_write_EEPROM ) = _getDLSym ( libHandle,
      "write_EEPROM", prefix ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_write_UART ) = _getDLSym ( libHandle,
      "write_UART", prefix ))) {
    return 0;
  }
   */

#if 0
  oalib = !strcmp ( "1.8.7036.20160321", p_Altaircam_Version());

  if ( oalib ) {
    // Now comes the really ugly bit.  Patch the data section of the loaded
    // Altair library to match the new USB product IDs.  Actually, this
    // probably even gives "ugly" a bad name.

    _patchLibrary ( p_Altaircam_Enum );
  }
#endif

  numCameras = ( p_Altaircam_Enum )( devList );
  if ( numCameras < 1 ) {
    return 0;
  }

  for ( i = 0; i < numCameras; i++ ) {

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_ALTAIRCAM;
    ( void ) strncpy ( dev->deviceName, devList[i].displayname,
        OA_MAX_NAME_LEN+1 );
    _private->devIndex = i;
    ( void ) strcpy ( _private->toupcamId, devList[i].id );
    dev->_private = _private;
    dev->initCamera = oaAltairInitCamera;
    dev->hasLoadableFirmware = 0;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
  }

  return numCameras;
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
    fprintf ( stderr, "libaltaircam DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}


#if 0
#ifdef DYNLIB_EXT_DYLIB
void
_patchLibrary ( void* p )
{
  uint8_t*		enumFunction;
  uint8_t*		lea;
  uint8_t*		rip;
  uint8_t*		cmp;
  uint8_t*		pidTableStart = 0;
  uint8_t*		pidPos;
  int32_t		offset;
  int			entries = 0, entry;
  uint16_t		pid, newPid;

  enumFunction = (uint8_t*) p;

  lea = enumFunction + 0x64;
  rip = enumFunction + 0x6b;

  if ( *lea == 0x48 && *(lea+1) == 0x8d && *(lea+2) == 0x15 ) {
    lea += 3;
    offset = *lea++;
    offset |= ( *lea++ ) << 8;
    offset |= ( *lea++ ) << 16;
    offset |= ( *lea++ ) << 24;

    // fprintf ( stderr, "offset = %04x\n", offset );

    pidTableStart = rip + offset;
  }

  cmp = enumFunction + 0x0167;

  if ( *cmp == 0x49 && *(cmp+1) == 0x81 && *(cmp+2) == 0xfe ) {
    cmp += 3;
    entries = *cmp++;
    entries |= ( *cmp++ ) << 8;
    entries |= ( *cmp++ ) << 16;
    entries |= ( *cmp++ ) << 24;

    // fprintf ( stderr, "entries = %04x\n", entries );
  }

  if ( pidTableStart && entries ) {
    // fprintf ( stderr, "pid = %p to %p\n", pidTableStart, pidTableStart +
    //     entries );

    pidPos = pidTableStart;
    entry = 0;
    while ( entry < entries ) {
      pid = *pidPos;
      pid |= *( pidPos + 1 ) << 8;
      newPid = 0;
      switch ( pid ) {
        case 0xb135: // GCMOS01200KMA
          newPid = 0x0b2a; // GPCAMMT9M034M
          break;
        // From here on down it's all guesses
        case 0xb133: // GCMOS01200KPB
          newPid = 0x0b2b; // GPCAMAR0130C
          break;
        case 0x1005: // GPCMOS01200KPC
          newPid = 0x0b4f; // ALTAIRGP130C
          break;
        case 0xffff: // no idea what this one should be
          newPid = 0x0b8b; // ALTAIRGP130M
          break;
        case 0x1008: // G3CMOS02300KPB
          newPid = 0x0b2c; // AAGFIMX185C
          break;
        case 0xe317: // G3CMOS02300KPB (USB2)
          newPid = 0x0b2d; // AAGFIMX185C (USB2)
          break;
        case 0x0000: // no idea what this one should be
          newPid = 0x0b8c; // ALTAIRGP224C
          break;
        default:
          break;
      }
      if ( newPid ) {
        *pidPos = newPid & 0xff;
        *( pidPos + 1 ) = newPid >> 8;
      }
      pidPos += 0x20;
      entry++;
    }
/*
    i = 0;
    while ( i < ( 0x20 * entries )) {
      if ( i % 16 == 0 ) {
        fprintf ( stderr, "%04x  ", i );
      }
      fprintf ( stderr, "%02x ", *pidTableStart++ );
      i++;
      if ( i % 16 == 0 ) {
        fprintf ( stderr, "\n" );
      }
    }
*/
  }
}

#else
void
_patchLibrary ( void* p )
{
  uint8_t*		enumFunction;
  uint8_t*		lea;
  uint8_t*		rip;
  uint8_t*		pidTableStart = 0;
  uint8_t*		pidTableEnd = 0;
  uint8_t*		pidPos;
  int32_t		offset;
  uint16_t		pid, newPid;

  enumFunction = (uint8_t*) p;

  lea = enumFunction + 0x91;
  rip = enumFunction + 0x98;

  if ( *lea == 0x48 && *(lea+1) == 0x8d && *(lea+2) == 0x1d ) {
    lea += 3;
    offset = *lea++;
    offset |= ( *lea++ ) << 8;
    offset |= ( *lea++ ) << 16;
    offset |= ( *lea++ ) << 24;

    // fprintf ( stderr, "offset = %04x\n", offset );

    pidTableStart = rip + offset;
  }

  lea = enumFunction + 0x79;
  rip = enumFunction + 0x80;

  if ( *lea == 0x4c && *(lea+1) == 0x8d && *(lea+2) == 0x25 ) {
    lea += 3;
    offset = *lea++;
    offset |= ( *lea++ ) << 8;
    offset |= ( *lea++ ) << 16;
    offset |= ( *lea++ ) << 24;

    // fprintf ( stderr, "offset = %04x\n", offset );

    pidTableEnd = rip + offset;
  } else {
    lea = enumFunction + 0x79;
    // fprintf ( stderr, "lea = %02x %02x %02x\n", *lea, *(lea+1), *(lea+2));
  }

  if ( pidTableStart && pidTableEnd ) {
    // fprintf ( stderr, "pid = %p to %p\n", pidTableStart, pidTableEnd );

    pidPos = pidTableStart + 8;
    while ( pidPos < pidTableEnd ) {
      pid = *pidPos;
      pid |= *( pidPos + 1 ) << 8;
      newPid = 0;
      switch ( pid ) {
        case 0xb135: // GCMOS01200KMA
          newPid = 0x0b2a; // GPCAMMT9M034M
          break;
        // From here on down it's all guesses
        case 0xb133: // GCMOS01200KPB
          newPid = 0x0b2b; // GPCAMAR0130C
          break;
        case 0x1005: // GPCMOS01200KPC
          newPid = 0x0b4f; // ALTAIRGP130C
          break;
        case 0xffff: // no idea what this one should be
          newPid = 0x0b8b; // ALTAIRGP130M
          break;
        case 0x1008: // G3CMOS02300KPB
          newPid = 0x0b2c; // AAGFIMX185C
          break;
        case 0xe317: // G3CMOS02300KPB (USB2)
          newPid = 0x0b2d; // AAGFIMX185C (USB2)
          break;
        case 0x0000: // no idea what this one should be
          newPid = 0x0b8c; // ALTAIRGP224C
          break;
        default:
          break;
      }
      if ( newPid ) {
        *pidPos = newPid & 0xff;
        *( pidPos + 1 ) = newPid >> 8;
      }
      pidPos += 0x20;
    }

    /*
    i = 0;
    while ( pidTableStart < pidTableEnd ) {
      if ( i % 16 == 0 ) {
        fprintf ( stderr, "%04x  ", i );
      }
      fprintf ( stderr, "%02x ", *pidTableStart++ );
      i++;
      if ( i % 16 == 0 ) {
        fprintf ( stderr, "\n" );
      }
    }
     */
  }
}
#endif
#endif
