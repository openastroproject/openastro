/*****************************************************************************
 *
 * Altairoacam.c -- main entrypoint for Altair cameras
 *
 * Copyright 2016,2017,2018,2019 James Fidell (james@openastroproject.org)
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
#include <altaircam.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Altairoacam.h"

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
// Altaircam_EnumV2(AltaircamInstV2*)
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

/**
 * Cycle through the list of cameras returned by the altaircam library
 */

int
oaAltairGetCameras ( CAMERA_LIST* deviceList, int flags )
{
  AltaircamInstV2		devList[ ALTAIRCAM_MAX ];
  unsigned int		numCameras;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int                   ret;
  static void*		libHandle = 0;

  // On MacOS we just try /Applications/AltairCapture.app/Contents/MacOS

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*		libName = "/Applications/AltairCapture.app/Contents/"
                            "MacOS/libaltaircam.dylib";
#else
  const char*		libName = "libaltaircam.so.1";
#endif

  if ( !libHandle ) {
    if (!( libHandle = dlopen ( libName, RTLD_LAZY ))) {
      fprintf ( stderr, "can't load %s\n", libName );
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

  if (!( *( void** )( &p_Altaircam_AwbInit ) = _getDLSym ( libHandle,
      "Altaircam_AwbInit" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_AwbOnePush ) = _getDLSym ( libHandle,
      "Altaircam_AwbOnePush" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_calc_ClarityFactor ) = _getDLSym ( libHandle,
      "Altaircam_calc_ClarityFactor" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Close ) = _getDLSym ( libHandle,
      "Altaircam_Close" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_deBayerV2 ) = _getDLSym ( libHandle,
      "Altaircam_deBayerV2" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_EnumV2 ) = _getDLSym ( libHandle,
      "Altaircam_EnumV2" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Flush ) = _getDLSym ( libHandle,
      "Altaircam_Flush" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AEAuxRect ) = _getDLSym ( libHandle,
      "Altaircam_get_AEAuxRect" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AutoExpoEnable ) = _getDLSym ( libHandle,
      "Altaircam_get_AutoExpoEnable" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AutoExpoTarget ) = _getDLSym ( libHandle,
      "Altaircam_get_AutoExpoTarget" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_AWBAuxRect ) = _getDLSym ( libHandle,
      "Altaircam_get_AWBAuxRect" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Brightness ) = _getDLSym ( libHandle,
      "Altaircam_get_Brightness" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Chrome ) = _getDLSym ( libHandle,
      "Altaircam_get_Chrome" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Contrast ) = _getDLSym ( libHandle,
      "Altaircam_get_Contrast" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_eSize ) = _getDLSym ( libHandle,
      "Altaircam_get_eSize" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpoAGain ) = _getDLSym ( libHandle,
      "Altaircam_get_ExpoAGain" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpoAGainRange ) = _getDLSym ( libHandle,
      "Altaircam_get_ExpoAGainRange" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpoTime ) = _getDLSym ( libHandle,
      "Altaircam_get_ExpoTime" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ExpTimeRange ) = _getDLSym ( libHandle,
      "Altaircam_get_ExpTimeRange" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_FanMaxSpeed ) = _getDLSym ( libHandle,
      "Altaircam_get_FanMaxSpeed" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Field ) = _getDLSym ( libHandle,
      "Altaircam_get_Field" ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_FwVersion ) = _getDLSym ( libHandle,
      "Altaircam_get_FwVersion" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Gamma ) = _getDLSym ( libHandle,
      "Altaircam_get_Gamma" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_HFlip ) = _getDLSym ( libHandle,
      "Altaircam_get_HFlip" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_GetHistogram ) = _getDLSym ( libHandle,
      "Altaircam_GetHistogram" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Hue ) = _getDLSym ( libHandle,
      "Altaircam_get_Hue" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_HwVersion ) = _getDLSym ( libHandle,
      "Altaircam_get_HwVersion" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_HZ ) = _getDLSym ( libHandle,
      "Altaircam_get_HZ" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_LevelRange ) = _getDLSym ( libHandle,
      "Altaircam_get_LevelRange" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_MaxBitDepth ) = _getDLSym ( libHandle,
      "Altaircam_get_MaxBitDepth" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_MaxSpeed ) = _getDLSym ( libHandle,
      "Altaircam_get_MaxSpeed" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Mode ) = _getDLSym ( libHandle,
      "Altaircam_get_Mode" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_MonoMode ) = _getDLSym ( libHandle,
      "Altaircam_get_MonoMode" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Negative ) = _getDLSym ( libHandle,
      "Altaircam_get_Negative" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Option ) = _getDLSym ( libHandle,
      "Altaircam_get_Option" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_PixelSize ) = _getDLSym ( libHandle,
      "Altaircam_get_PixelSize" ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_ProductionDate ) = _getDLSym ( libHandle,
      "Altaircam_get_ProductionDate" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_RawFormat ) = _getDLSym ( libHandle,
      "Altaircam_get_RawFormat" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_RealTime ) = _getDLSym ( libHandle,
      "Altaircam_get_RealTime" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Resolution ) = _getDLSym ( libHandle,
      "Altaircam_get_Resolution" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ResolutionNumber ) = _getDLSym (
      libHandle, "Altaircam_get_ResolutionNumber" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_ResolutionRatio ) = _getDLSym (
      libHandle, "Altaircam_get_ResolutionRatio" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Roi ) = _getDLSym ( libHandle,
      "Altaircam_get_Roi" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_RoiMode ) = _getDLSym ( libHandle,
      "Altaircam_get_RoiMode" ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_Saturation ) = _getDLSym ( libHandle,
      "Altaircam_get_Saturation" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_SerialNumber ) = _getDLSym ( libHandle,
      "Altaircam_get_SerialNumber" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Size ) = _getDLSym ( libHandle,
      "Altaircam_get_Size" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Speed ) = _getDLSym ( libHandle,
      "Altaircam_get_Speed" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_StillResolution ) = _getDLSym (
      libHandle, "Altaircam_get_StillResolution" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_StillResolutionNumber ) = _getDLSym (
      libHandle, "Altaircam_get_StillResolutionNumber" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_Temperature ) = _getDLSym ( libHandle,
      "Altaircam_get_Temperature" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_TempTint ) = _getDLSym ( libHandle,
      "Altaircam_get_TempTint" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_VFlip ) = _getDLSym ( libHandle,
      "Altaircam_get_VFlip" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_get_VignetAmountInt ) = _getDLSym (
      libHandle, "Altaircam_get_VignetAmountInt" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_VignetEnable ) = _getDLSym ( libHandle,
      "Altaircam_get_VignetEnable" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_get_VignetMidPointInt ) = _getDLSym (
      libHandle, "Altaircam_get_VignetMidPointInt" ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_get_WhiteBalanceGain ) = _getDLSym (
      libHandle, "Altaircam_get_WhiteBalanceGain" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_HotPlug ) = _getDLSym ( libHandle,
      "Altaircam_HotPlug" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_LevelRangeAuto ) = _getDLSym ( libHandle,
      "Altaircam_LevelRangeAuto" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Open ) = _getDLSym ( libHandle,
      "Altaircam_Open" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_OpenByIndex ) = _getDLSym ( libHandle,
      "Altaircam_OpenByIndex" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Pause ) = _getDLSym ( libHandle,
      "Altaircam_Pause" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_PullImageV2 ) = _getDLSym ( libHandle,
      "Altaircam_PullImageV2" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_PullStillImageV2 ) = _getDLSym ( libHandle,
      "Altaircam_PullStillImageV2" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AEAuxRect ) = _getDLSym ( libHandle,
      "Altaircam_put_AEAuxRect" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AutoExpoEnable ) = _getDLSym ( libHandle,
      "Altaircam_put_AutoExpoEnable" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AutoExpoTarget ) = _getDLSym ( libHandle,
      "Altaircam_put_AutoExpoTarget" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_AWBAuxRect ) = _getDLSym ( libHandle,
      "Altaircam_put_AWBAuxRect" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Brightness ) = _getDLSym ( libHandle,
      "Altaircam_put_Brightness" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Chrome ) = _getDLSym ( libHandle,
      "Altaircam_put_Chrome" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ChromeCallback ) = _getDLSym ( libHandle,
      "Altaircam_put_ChromeCallback" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Contrast ) = _getDLSym ( libHandle,
      "Altaircam_put_Contrast" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_eSize ) = _getDLSym ( libHandle,
      "Altaircam_put_eSize" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ExpoAGain ) = _getDLSym ( libHandle,
      "Altaircam_put_ExpoAGain" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ExpoCallback ) = _getDLSym ( libHandle,
      "Altaircam_put_ExpoCallback" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_ExpoTime ) = _getDLSym ( libHandle,
      "Altaircam_put_ExpoTime" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Gamma ) = _getDLSym ( libHandle,
      "Altaircam_put_Gamma" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_HFlip ) = _getDLSym ( libHandle,
      "Altaircam_put_HFlip" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Hue ) = _getDLSym ( libHandle,
      "Altaircam_put_Hue" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_HZ ) = _getDLSym ( libHandle,
      "Altaircam_put_HZ" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_LEDState ) = _getDLSym ( libHandle,
      "Altaircam_put_LEDState" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_LevelRange ) = _getDLSym ( libHandle,
      "Altaircam_put_LevelRange" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_MaxAutoExpoTimeAGain ) = _getDLSym (
      libHandle, "Altaircam_put_MaxAutoExpoTimeAGain" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Mode ) = _getDLSym ( libHandle,
      "Altaircam_put_Mode" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Negative ) = _getDLSym ( libHandle,
      "Altaircam_put_Negative" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Option ) = _getDLSym ( libHandle,
      "Altaircam_put_Option" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_RealTime ) = _getDLSym ( libHandle,
      "Altaircam_put_RealTime" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Roi ) = _getDLSym ( libHandle,
      "Altaircam_put_Roi" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_put_RoiMode ) = _getDLSym ( libHandle,
      "Altaircam_put_RoiMode" ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_put_Saturation ) = _getDLSym ( libHandle,
      "Altaircam_put_Saturation" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Size ) = _getDLSym ( libHandle,
      "Altaircam_put_Size" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Speed ) = _getDLSym ( libHandle,
      "Altaircam_put_Speed" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_Temperature ) = _getDLSym ( libHandle,
      "Altaircam_put_Temperature" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_TempTint ) = _getDLSym ( libHandle,
      "Altaircam_put_TempTint" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_VFlip ) = _getDLSym ( libHandle,
      "Altaircam_put_VFlip" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_put_VignetAmountInt ) = _getDLSym (
      libHandle, "Altaircam_put_VignetAmountInt" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_VignetEnable ) = _getDLSym ( libHandle,
      "Altaircam_put_VignetEnable" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_put_VignetMidPointInt ) = _getDLSym (
      libHandle, "Altaircam_put_VignetMidPointInt" ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_put_WhiteBalanceGain ) = _getDLSym (
      libHandle, "Altaircam_put_WhiteBalanceGain" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_read_EEPROM ) = _getDLSym ( libHandle,
      "Altaircam_read_EEPROM" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_read_UART ) = _getDLSym ( libHandle,
      "Altaircam_read_UART" ))) {
    return 0;
  }
   */

  if (!( *( void** )( &p_Altaircam_Snap ) = _getDLSym ( libHandle,
      "Altaircam_Snap" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_ST4PlusGuide ) = _getDLSym ( libHandle,
      "Altaircam_ST4PlusGuide" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_ST4PlusGuideState ) = _getDLSym ( libHandle,
      "Altaircam_ST4PlusGuideState" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_StartPullModeWithCallback ) = _getDLSym (
      libHandle, "Altaircam_StartPullModeWithCallback" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_StartPushModeV2 ) = _getDLSym ( libHandle,
      "Altaircam_StartPushModeV2" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Stop ) = _getDLSym ( libHandle,
      "Altaircam_Stop" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Trigger ) = _getDLSym ( libHandle,
      "Altaircam_Trigger" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_Version ) = _getDLSym ( libHandle,
      "Altaircam_Version" ))) {
    return 0;
  }

  if (!( *( void** )( &p_Altaircam_write_EEPROM ) = _getDLSym ( libHandle,
      "Altaircam_write_EEPROM" ))) {
    return 0;
  }

  /*
  if (!( *( void** )( &p_Altaircam_write_UART ) = _getDLSym ( libHandle,
      "Altaircam_write_UART" ))) {
    return 0;
  }
   */

  numCameras = ( p_Altaircam_EnumV2 )( devList );
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
