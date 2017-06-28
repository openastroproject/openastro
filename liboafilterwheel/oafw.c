/*****************************************************************************
 *
 * oafw.c -- main filter wheel library entrypoint
 *
 * Copyright 2014,2015 James Fidell (james@openastroproject.org)
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

oaInterface	oaFilterWheelInterfaces[] = {
  { 0, "", "", 0, OA_UDC_FLAG_NONE },
#if defined(HAVE_LIBUDEV) || defined(HAVE_LIBFTDI)
  { OA_FW_IF_XAGYL, "Xagyl", "Xagyl", oaXagylGetFilterWheels,
      OA_UDC_FLAG_USB_ALL },
#else
  { 0, "", "", 0, OA_UDC_FLAG_NONE },
#endif
  { OA_FW_IF_SX, "Starlight Xpress", "SX", oaSXGetFilterWheels,
      OA_UDC_FLAG_NONE },
  { 0, "", "", 0, OA_UDC_FLAG_NONE }
};
  

int
oaGetFilterWheels( oaFilterWheelDevice*** deviceList )
{
  int			i, err;
  FILTERWHEEL_LIST	list;

  list.wheelList = 0;
  list.numFilterWheels = list.maxFilterWheels = 0;

  for ( i = 0; i < OA_FW_IF_COUNT; i++ ) {
    if ( oaFilterWheelInterfaces[i].interfaceType ) {
      if (( err = oaFilterWheelInterfaces[i].enumerate ( &list )) < 0 ) {
        return err;
      }
    }
  }

  *deviceList = list.wheelList;
  return list.numFilterWheels;
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
