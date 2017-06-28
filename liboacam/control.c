/*****************************************************************************
 *
 * control.c -- interface for camera control functions
 *
 * Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#include <openastro/camera.h>

#include "oacamprivate.h"


const char* oaCameraControlLabel[ OA_CAM_CTRL_LAST_P1 ] = {
  "",				// 0
  "Brightness",
  "Contrast",
  "Saturation",
  "Hue",
  "Auto White Balance",
  "White Balance",
  "Blue Balance",
  "Red Balance",
  "Gamma",
  "Exposure",			// 10
  "Auto Gain",
  "Gain",
  "Horizontal Flip",
  "Vertical Flip",
  "Power Line Freq.",
  "Auto Hue",
  "White Balance Temp.",
  "Sharpness",
  "Backlight Compensation",
  "Chroma Auto Gain Control",	// 20
  "Colour Killer",
  "Colour FX",
  "Auto Brightness",
  "Band Stop Filter",
  "Rotate",
  "Background Colour",
  "Chroma Gain",
  "Min Buffers for Capture",
  "Alpha Component",
  "Colour FX CBCR",		// 30
  "Auto Exposure",
  "Absolute Exposure",
  "Pan Relative",
  "Tilt Relative",
  "Pan Reset",
  "Tilt Reset",
  "Pan Absolute",
  "Tilt Absolute",
  "Zoom Absolute",
  "Backlight",			// 40
  "Black Level",
  "2x Gain",
  "Gain Boost",
  "HDR",
  "HPC",
  "High Speed",
  "Low Noise",
  "Pixel Clock",
  "Colour Mode",
  "Rolling Shutter",		// 50
  "Shutter",
  "Signal Boost",
  "Subs Voltage",
  "Temperature Setpoint",
  "USB Traffic",
  "Bit Depth",
  "Auto Gamma",
  "Auto Red Balance",
  "Auto Blue Balance",
  "Binning",			// 60
  "Auto USB Traffic",
  "Temperature",
  "Green Balance",
  "Auto Green Balance",
  "Contour",
  "Auto Contour",
  "Noise Reduction",
  "Save User Settings",
  "Restore User Settings",
  "Restore Factory Settings",	// 70
  "Auto White Balance Speed",
  "Auto White Balance Delay",
  "Dropped Frames",
  "Reset Dropped Frames",
  "Auto White Balance Temp",
  "Auto Contrast",
  "Overclock",
  "Auto Overclock",
  "Cooler",
  "Cooler Power",		// 80
  "Enable Trigger Input",
  "Trigger Mode",
  "Trigger Source",
  "Trigger Polarity",
  "Enable Trigger Delay",
  "Trigger Delay",
  "Enable Strobe Output",
  "Strobe Polarity",
  "Strobe Source",
  "Strobe Delay",		// 90
  "Strobe Duration",
  "Speed",
  "Fan",
  "Mono Bin Colour",
  "Pattern Adjust"
};

const char* oaCameraPresetAWBLabel[ OA_AWB_PRESET_LAST_P1 ] = {
  "Manual",
  "Auto",
  "Incandescent",
  "Fluorescent",
  "Fluorescent H",
  "Horizon",
  "Daylight",
  "Flash",
  "Cloudy"
};

const char* oaCameraAutoExposureLabel[ OA_EXPOSURE_TYPE_LAST_P1 ] = {
  "Auto",
  "Manual",
  "Shutter Priority",
  "Aperture Priority"
};


int
oaGetAutoForControl ( int control )
{
  switch ( control ) {

    case OA_CAM_CTRL_WHITE_BALANCE:
      return OA_CAM_CTRL_AUTO_WHITE_BALANCE;
      break;
    
    case OA_CAM_CTRL_GAIN:
      return OA_CAM_CTRL_AUTO_GAIN;
      break;

    case OA_CAM_CTRL_HUE:
      return OA_CAM_CTRL_HUE_AUTO;
      break;

    case OA_CAM_CTRL_BRIGHTNESS:
      return OA_CAM_CTRL_AUTO_BRIGHTNESS;
      break;

    case OA_CAM_CTRL_EXPOSURE:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      return OA_CAM_CTRL_AUTO_EXPOSURE;
      break;

    case OA_CAM_CTRL_GAMMA:
      return OA_CAM_CTRL_AUTO_GAMMA;
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      return OA_CAM_CTRL_AUTO_BLUE_BALANCE;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      return OA_CAM_CTRL_AUTO_RED_BALANCE;
      break;

    case OA_CAM_CTRL_USBTRAFFIC:
      return OA_CAM_CTRL_AUTO_USBTRAFFIC;
      break;

    case OA_CAM_CTRL_GREEN_BALANCE:
      return OA_CAM_CTRL_AUTO_GREEN_BALANCE;
      break;

    case OA_CAM_CTRL_CONTOUR:
      return OA_CAM_CTRL_AUTO_CONTOUR;
      break;

    case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
      return OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP;
      break;

    case OA_CAM_CTRL_CONTRAST:
      return OA_CAM_CTRL_AUTO_CONTRAST;
      break;

    case OA_CAM_CTRL_OVERCLOCK:
      return OA_CAM_CTRL_AUTO_OVERCLOCK;
      break;
  }

  return -OA_ERR_INVALID_CONTROL;
}


int
oaGetControlForAuto ( int control )
{
  switch ( control ) {

    case OA_CAM_CTRL_AUTO_WHITE_BALANCE:
      return OA_CAM_CTRL_WHITE_BALANCE;
      break;
    
    case OA_CAM_CTRL_AUTO_GAIN:
      return OA_CAM_CTRL_GAIN;
      break;

    case OA_CAM_CTRL_HUE_AUTO:
      return OA_CAM_CTRL_HUE;
      break;

    case OA_CAM_CTRL_AUTO_BRIGHTNESS:
      return OA_CAM_CTRL_BRIGHTNESS;
      break;

    // FIX ME -- this could be either
    case OA_CAM_CTRL_AUTO_EXPOSURE:
      return OA_CAM_CTRL_EXPOSURE;
      // return OA_CAM_CTRL_EXPOSURE_ABSOLUTE;
      break;

    case OA_CAM_CTRL_AUTO_GAMMA:
      return OA_CAM_CTRL_GAMMA;
      break;

    case OA_CAM_CTRL_AUTO_BLUE_BALANCE:
      return OA_CAM_CTRL_BLUE_BALANCE;
      break;

    case OA_CAM_CTRL_AUTO_RED_BALANCE:
      return OA_CAM_CTRL_RED_BALANCE;
      break;

    case OA_CAM_CTRL_AUTO_USBTRAFFIC:
      return OA_CAM_CTRL_USBTRAFFIC;
      break;

    case OA_CAM_CTRL_AUTO_GREEN_BALANCE:
      return OA_CAM_CTRL_GREEN_BALANCE;
      break;

    case OA_CAM_CTRL_AUTO_CONTOUR:
      return OA_CAM_CTRL_CONTOUR;
      break;

    case OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP:
      return OA_CAM_CTRL_WHITE_BALANCE_TEMP;
      break;

    case OA_CAM_CTRL_AUTO_CONTRAST:
      return OA_CAM_CTRL_CONTRAST;
      break;

    case OA_CAM_CTRL_AUTO_OVERCLOCK:
      return OA_CAM_CTRL_OVERCLOCK;
      break;
  }

  return -OA_ERR_INVALID_CONTROL;
}


int
oacamHasAuto ( oaCamera* camera, int control )
{
  switch ( control ) {

    case OA_CAM_CTRL_WHITE_BALANCE:
      return camera->controls[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] ?
          OA_CAM_CTRL_AUTO_WHITE_BALANCE : 0;
      break;
    
    case OA_CAM_CTRL_GAIN:
      return camera->controls[ OA_CAM_CTRL_AUTO_GAIN ] ?
          OA_CAM_CTRL_AUTO_GAIN : 0;
      break;

    case OA_CAM_CTRL_HUE:
      return camera->controls[ OA_CAM_CTRL_HUE_AUTO ] ?
          OA_CAM_CTRL_HUE_AUTO : 0;
      break;

    case OA_CAM_CTRL_BRIGHTNESS:
      return camera->controls[ OA_CAM_CTRL_AUTO_BRIGHTNESS ] ?
          OA_CAM_CTRL_AUTO_BRIGHTNESS : 0;
      break;

    case OA_CAM_CTRL_EXPOSURE:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      return camera->controls[ OA_CAM_CTRL_AUTO_EXPOSURE ] ?
          OA_CAM_CTRL_AUTO_EXPOSURE : 0;
      break;

    case OA_CAM_CTRL_GAMMA:
      return camera->controls[ OA_CAM_CTRL_AUTO_GAMMA ] ?
          OA_CAM_CTRL_AUTO_GAMMA : 0;
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      return camera->controls[ OA_CAM_CTRL_AUTO_BLUE_BALANCE ] ?
          OA_CAM_CTRL_AUTO_BLUE_BALANCE : 0;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      return camera->controls[ OA_CAM_CTRL_AUTO_RED_BALANCE ] ?
          OA_CAM_CTRL_AUTO_RED_BALANCE : 0;
      break;

    case OA_CAM_CTRL_USBTRAFFIC:
      return camera->controls[ OA_CAM_CTRL_AUTO_USBTRAFFIC ] ?
          OA_CAM_CTRL_AUTO_USBTRAFFIC : 0;
      break;

    case OA_CAM_CTRL_GREEN_BALANCE:
      return camera->controls[ OA_CAM_CTRL_AUTO_GREEN_BALANCE ] ?
          OA_CAM_CTRL_AUTO_GREEN_BALANCE : 0;
      break;

    case OA_CAM_CTRL_CONTOUR:
      return camera->controls[ OA_CAM_CTRL_AUTO_CONTOUR ] ?
          OA_CAM_CTRL_AUTO_CONTOUR : 0;
      break;

    case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
      return camera->controls[ OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP ] ?
          OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP : 0;

    case OA_CAM_CTRL_CONTRAST:
      return camera->controls[ OA_CAM_CTRL_AUTO_CONTRAST ] ?
          OA_CAM_CTRL_AUTO_CONTRAST : 0;
      break;

    case OA_CAM_CTRL_OVERCLOCK:
      return camera->controls[ OA_CAM_CTRL_AUTO_OVERCLOCK ] ?
          OA_CAM_CTRL_AUTO_OVERCLOCK : 0;
      break;
  }

  return -OA_ERR_INVALID_CONTROL;
}


int
oaIsAuto ( int control )
{
  switch ( control ) {
    case OA_CAM_CTRL_AUTO_WHITE_BALANCE:
    case OA_CAM_CTRL_AUTO_GAIN:
    case OA_CAM_CTRL_HUE_AUTO:
    case OA_CAM_CTRL_AUTO_BRIGHTNESS:
    case OA_CAM_CTRL_AUTO_EXPOSURE:
    case OA_CAM_CTRL_AUTO_GAMMA:
    case OA_CAM_CTRL_AUTO_RED_BALANCE:
    case OA_CAM_CTRL_AUTO_BLUE_BALANCE:
    case OA_CAM_CTRL_AUTO_USBTRAFFIC:
    case OA_CAM_CTRL_AUTO_GREEN_BALANCE:
    case OA_CAM_CTRL_AUTO_CONTOUR:
    case OA_CAM_CTRL_AUTO_WB_SPEED:
    case OA_CAM_CTRL_AUTO_WB_DELAY:
    case OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP:
    case OA_CAM_CTRL_AUTO_CONTRAST:
    case OA_CAM_CTRL_AUTO_OVERCLOCK:
      return 1;
      break;
  }
  return 0;
}


void
oaSetRootPath ( const char* path )
{
  if ( installPathRoot ) {
    free ( installPathRoot );
  }
  installPathRoot = strdup ( path );
}


int64_t
oacamGetControlValue ( oaControlValue* v )
{
  uint64_t ret = 0;

  switch ( v->valueType ) {
    case OA_CTRL_TYPE_INT32:
      ret = v->int32;
      break;
    case OA_CTRL_TYPE_BOOLEAN:
      ret = v->boolean;
      break;
    case OA_CTRL_TYPE_MENU:
      ret = v->menu;
      break;
    case OA_CTRL_TYPE_INT64:
      ret = v->int64;
      break;
    case OA_CTRL_TYPE_DISCRETE:
      ret = v->discrete;
      break;
    case OA_CTRL_TYPE_READONLY:
      ret = v->readonly;
      break;
    default:
      fprintf ( stderr, "%s: Unexpected valueType %d\n", __FUNCTION__,
          v->valueType );
      break;
  }

  return ret;
}
