/*****************************************************************************
 *
 * control.c -- interface for camera control functions
 *
 * Copyright 2013,2014,2015,2016,2017,2019
 *   James Fidell (james@openastroproject.org)
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
  "White Balance",
  "Blue Balance",
  "Red Balance",
  "Gamma",
  "Exposure",
  "Gain",			// 10
  "Horizontal Flip",
  "Vertical Flip",
  "Power Line Freq.",
  "White Balance Temp.",
  "Sharpness",
  "Backlight Compensation",
  "Chroma AGC",
  "Colour Killer",
  "Colour FX",
  "Band Stop Filter",		// 20
  "Rotate",
  "Background Colour",
  "Chroma Gain",
  "Min Buffers for Capture",
  "Alpha Component",
  "Colour FX CBCR",
  "Absolute Exposure",
  "Pan Relative",
  "Tilt Relative",
  "Pan Reset",			// 30
  "Tilt Reset",
  "Pan Absolute",
  "Tilt Absolute",
  "Zoom Absolute",
  "Backlight",
  "Black Level",
  "2x Gain",
  "Gain Boost",
  "HDR",
  "HPC",			// 40
  "High Speed",
  "Low Noise",
  "Pixel Clock",
  "Colour Mode",
  "Rolling Shutter",
  "Shutter",
  "Signal Boost",
  "Subs Voltage",
  "Temperature Setpoint",
  "USB Traffic",		// 50
  "Bit Depth",
  "Binning",
  "Temperature",
  "Green Balance",
  "Contour",
  "Noise Reduction",
  "Save User Settings",
  "Restore User Settings",
  "Restore Factory Settings",
  "Dropped Frames",		// 60
  "Reset Dropped Frames",
  "Overclock",
  "Cooler",
  "Cooler Power",
  "Enable Trigger Input",
  "Trigger Mode",
  "Trigger Source",
  "Trigger Polarity",
  "Enable Trigger Delay",
  "Trigger Delay",		// 70
  "Enable Strobe Output",
  "Strobe Polarity",
  "Strobe Source",
  "Strobe Delay",
  "Strobe Duration",
  "Speed",
  "Fan",
  "Pattern Adjust",
  "Mono Bin Colour",
  "Dew Heater",			// 80
  "Auto White Balance Speed",
  "Auto White Balance Delay",
  "Auto White Balance Temp",
  "Max Auto Exposure",
  "Max Auto Gain",
  "Interlace Enable",
  "Focus",
  "Iris",
  "Roll",
  "Privacy Enable",		// 90
  "Simple Focus",
  "Relative Focus",
  "Relative Focus Speed",
  "Relative Iris",
  "Relative Zoom",
  "Relative Zoom Speed",
  "Digital Zoom Enable",
  "Pan Relative Speed",
  "Tilt Relative Speed",
  "Relative Roll",		// 100
  "Relative Roll Speed",
  "White Shading",
  "LED State",
  "LED Period",
  "Auto Exposure Priority",
  "Exposure Value",
  "White Balance Preset",
  "Digital Gain",
  "Digital Gain (Red)",
  "Digital Gain (Green)",	// 110
  "Digital Gain (Blue)",
  "Frame Format",
  "ISO",
	"Shutter Speed",
	"Mirror Lockup",
	"Power Source",
	"Battery Level"
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

const char* oaCameraControlModifierPrefix[ OA_CAM_CTRL_MODIFIERS_P1 ] = {
  "",
  "Auto ",
  "Enable "
};


int
oaGetAutoForControl ( int control )
{
  // There's probably no reason not to return OA_CAM_CTRL_MODE_AUTO(control)
  // here, but in the name of finding out about potential issues I wasn't
  // aware of, I'll do this the long way round.

  switch ( control ) {

    case OA_CAM_CTRL_WHITE_BALANCE:
    case OA_CAM_CTRL_GAIN:
    case OA_CAM_CTRL_HUE:
    case OA_CAM_CTRL_BRIGHTNESS:
    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    case OA_CAM_CTRL_GAMMA:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_USBTRAFFIC:
    case OA_CAM_CTRL_GREEN_BALANCE:
    case OA_CAM_CTRL_CONTOUR:
    case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
    case OA_CAM_CTRL_CONTRAST:
    case OA_CAM_CTRL_OVERCLOCK:
    case OA_CAM_CTRL_EXPOSURE_VALUE:
    case OA_CAM_CTRL_SHARPNESS:
      return OA_CAM_CTRL_MODE_AUTO(control);
      break;
  }
  return -OA_ERR_INVALID_CONTROL;
}


int
oaGetControlForAuto ( int control )
{
  // Same comment applies as for oaGetAutoForControl()

  switch ( control ) {
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GREEN_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTOUR ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE_TEMP ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_OVERCLOCK ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_VALUE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_SHARPNESS ):
      return OA_CAM_CTRL_MODE_NONAUTO ( control );
      break;
  }
  return -OA_ERR_INVALID_CONTROL;
}


int
oacamHasAuto ( oaCamera* camera, int control )
{
  // And yet again, the same comment applies as for oaGetAutoForControl()

  switch ( control ) {

    case OA_CAM_CTRL_WHITE_BALANCE:
    case OA_CAM_CTRL_GAIN:
    case OA_CAM_CTRL_HUE:
    case OA_CAM_CTRL_BRIGHTNESS:
    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    case OA_CAM_CTRL_GAMMA:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_USBTRAFFIC:
    case OA_CAM_CTRL_GREEN_BALANCE:
    case OA_CAM_CTRL_CONTOUR:
    case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
    case OA_CAM_CTRL_CONTRAST:
    case OA_CAM_CTRL_OVERCLOCK:
    case OA_CAM_CTRL_EXPOSURE_VALUE:
    case OA_CAM_CTRL_SHARPNESS:
      return camera->OA_CAM_CTRL_AUTO_TYPE( control ) ?
          OA_CAM_CTRL_MODE_AUTO(control) : 0;
      break;
  }
  return -OA_ERR_INVALID_CONTROL;
}


int
oaIsAuto ( int control )
{
  // FIX ME -- change for control being out of range?
  return OA_CAM_CTRL_IS_AUTO(control) ? 1 : 0;
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
