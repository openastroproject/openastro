/*****************************************************************************
 *
 * xagylConfig.c -- Manage Xagyl filter wheel user config
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

#include <hidapi.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "xagylfw.h"


userDeviceConfig*	xagylConfig = 0;
int			xagylConfigEntries = 0;


void
oaXagylClearIDFilters ( void )
{
  if ( xagylConfig ) {
    free ( xagylConfig );
    xagylConfig = 0;
  }
  xagylConfigEntries = 0;
}


void
oaXagylAddIDFilter ( userDeviceConfig* config )
{
  userDeviceConfig*	newEntry;

  if ( !xagylConfig ) {
    if (!( xagylConfig = ( userDeviceConfig* ) malloc ( sizeof (
        userDeviceConfig ) * 3 ))) {
      fprintf ( stderr, "malloc failed in %s\n", __FUNCTION__ );
      return;
    }
  } else {
    if ( xagylConfigEntries % 3 == 0 ) {
      if (!( xagylConfig = ( userDeviceConfig* ) realloc ( xagylConfig,
          sizeof ( userDeviceConfig ) * ( xagylConfigEntries / 3 + 1 )))) {
        fprintf ( stderr, "realloc failed in %s\n", __FUNCTION__ );
        return;
      }
    }
  }

  newEntry = xagylConfig + xagylConfigEntries * sizeof ( userDeviceConfig );
  memcpy ( newEntry, config, sizeof ( userDeviceConfig ));
  xagylConfigEntries++;
  return;
}
