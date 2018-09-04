/*****************************************************************************
 *
 * oaptr.c -- main PTR library entrypoint
 *
 * Copyright 2016,2017 James Fidell (james@openastroproject.org)
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

#include <openastro/timer.h>
#include <openastro/util.h>
#include <openastro/userConfig.h>

#include "oaptrversion.h"
#include "oaptrprivate.h"

#include "ptr.h"

PTR_LIST		list;

int
oaGetPTRDevices ( oaPTRDevice*** deviceList )
{
  int			err;

  list.ptrList = 0;
  list.numPTRDevices = list.maxPTRDevices = 0;

  // Not the right way to do this, but it will work for the time being
  // as I have no idea how to talk to the PTR on OSX for the time being
#if HAVE_LIBUDEV
  if (( err = oaPTREnumerate ( &list )) < 0 ) {
    _oaFreePTRDeviceList ( &list );
    list.ptrList = 0;
    list.numPTRDevices = 0;
    return err;
  }
#else
  err = 0;
#endif

  *deviceList = list.ptrList;
  return list.numPTRDevices;
}


void
oaReleasePTRDevices ( oaPTRDevice** deviceList )
{
  // This is a bit cack-handed because we don't know from the data
  // passed in how many cameras were found last time so we have to
  // consult a static global instead.

  _oaFreePTRDeviceList ( &list );
  list.ptrList = 0;
  list.numPTRDevices = 0;
  return;
}


unsigned int
oaGetPTRConfigFlags ( void )
{
  return OA_UDC_FLAG_USB_ALL;
}


unsigned int
oaGetPTRAPIVersion ( void )
{
  unsigned int v;

  v = ( OAPTR_MAJOR_VERSION << 16 ) + ( OAPTR_MINOR_VERSION << 8 ) +
      OAPTR_REVISION;
  return v;
}


const char*
oaGetPTRAPIVersionStr ( void )
{
  static char vs[ 40 ];

  snprintf ( vs, 40, "%d.%d.%d", OAPTR_MAJOR_VERSION,
      OAPTR_MINOR_VERSION, OAPTR_REVISION );
  return vs;
}


void
oaSetPTRDebugLevel ( int v )
{
  oaptrSetDebugLevel ( v );
}


void
oaClearPTRIDFilters ( void )
{
  oaPTRClearIDFilters();
}


void
oaAddPTRIDFilter ( userDeviceConfig* config )
{
  oaPTRAddIDFilter ( config );
}
