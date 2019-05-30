/*****************************************************************************
 *
 * oafw.c -- main filter wheel library entrypoint
 *
 * Copyright 2014,2015,2018 James Fidell (james@openastroproject.org)
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

#include <hidapi.h>

#include <openastro/filterwheel.h>
#include <openastro/util.h>
#include <openastro/userConfig.h>

#include "oafwversion.h"
#include "oafwprivate.h"

#include "xagylfw.h"
#include "sxfw.h"
#include "zwofw.h"
#include "brightstarfw.h"

oaInterface	oaFilterWheelInterfaces[] = {
  { 0, "", "", 0, OA_UDC_FLAG_NONE },
#if defined(HAVE_LIBUDEV) || defined(HAVE_LIBFTDI)
  { OA_FW_IF_XAGYL, "Xagyl", "Xagyl", oaXagylGetFilterWheels, 0,
      OA_UDC_FLAG_USB_ALL },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  { OA_FW_IF_SX, "Starlight Xpress", "SX", oaSXGetFilterWheels, 0,
      OA_UDC_FLAG_NONE },
#ifdef HAVE_LIBZWOFW
  { OA_FW_IF_ZWO, "ZW Optical", "ZWO", oaZWOGetFilterWheels, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if defined(HAVE_LIBUDEV)
  { OA_FW_IF_BRIGHTSTAR, "Brightstar", "Brightstar",
			oaBrightstarGetFilterWheels, 0, OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE }
};
  

static FILTERWHEEL_LIST	list;

int
oaGetFilterWheels( oaFilterWheelDevice*** deviceList )
{
  int			i, err;

  list.wheelList = 0;
  list.numFilterWheels = list.maxFilterWheels = 0;

  for ( i = 0; i < OA_FW_IF_COUNT; i++ ) {
    if ( oaFilterWheelInterfaces[i].interfaceType ) {
      if (( err = oaFilterWheelInterfaces[i].enumerate ( &list )) < 0 ) {
        _oaFreeFilterWheelDeviceList ( &list );
				if ( err != OA_ERR_LIBRARY_NOT_FOUND && err !=
						OA_ERR_SYMBOL_NOT_FOUND ) {
					list.numFilterWheels = 0;
					list.wheelList = 0;
					return err;
				}
      }
    }
  }

  *deviceList = list.wheelList;
  return list.numFilterWheels;
}


void
oaReleaseFilterWheels ( oaFilterWheelDevice** deviceList )
{
  // This is a bit cack-handed because we don't know from the data
  // passed in how many cameras were found last time so we have to
  // consult a static global instead.

  _oaFreeFilterWheelDeviceList ( &list );
  list.numFilterWheels = 0;
  list.wheelList = 0;
  return;
}


unsigned int
oaGetFilterWheelConfigFlags ( int interfaceType )
{
  if ( interfaceType > 0 && interfaceType < OA_FW_IF_COUNT ) {
    return oaFilterWheelInterfaces[interfaceType].userConfigFlags;
  }
  return OA_UDC_FLAG_NONE;
}


unsigned int
oaGetFilterWheelAPIVersion ( void )
{
  unsigned int v;

  v = ( OAFWHEEL_MAJOR_VERSION << 16 ) + ( OAFWHEEL_MINOR_VERSION << 8 ) +
      OAFWHEEL_REVISION;
  return v;
}


const char*
oaGetFilterWheelAPIVersionStr ( void )
{
  static char vs[ 40 ];

  snprintf ( vs, 40, "%d.%d.%d", OAFWHEEL_MAJOR_VERSION,
      OAFWHEEL_MINOR_VERSION, OAFWHEEL_REVISION );
  return vs;
}


void
oaSetFilterWheelDebugLevel ( int v )
{
  oafwSetDebugLevel ( v );
}


void
oaClearFilterWheelIDFilters ( int interfaceType )
{
  switch ( interfaceType ) {
#if defined(HAVE_LIBUDEV) || defined(HAVE_LIBFTDI)
    case OA_FW_IF_XAGYL:
      oaXagylClearIDFilters();
      break;
#endif
    case OA_FW_IF_SX:
      oaSXClearIDFilters();
      break;
  }
}


void
oaAddFilterWheelIDFilter ( int interfaceType, userDeviceConfig* config )
{
  switch ( interfaceType ) {
#if defined(HAVE_LIBUDEV) || defined(HAVE_LIBFTDI)
    case OA_FW_IF_XAGYL:
      oaXagylAddIDFilter ( config );
      break;
#endif
    case OA_FW_IF_SX:
      oaSXAddIDFilter ( config );
      break;
  }
}
