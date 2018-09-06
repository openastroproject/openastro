/*****************************************************************************
 *
 * sxConfig.c -- Manage Starlight Xpress filter wheels user configuration
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
#include "sxfw.h"


userDeviceConfig*	sxConfig = 0;
static int              sxConfigEntries = 0;


void
oaSXClearIDFilters ( void )
{
  if ( sxConfig ) {
    free ( sxConfig );
    sxConfig = 0;
  }
  sxConfigEntries = 0;
}


void
oaSXAddIDFilter ( userDeviceConfig* config )
{
  userDeviceConfig*     newEntry;

  if ( !sxConfig ) {
    if (!( sxConfig = ( userDeviceConfig* ) malloc ( sizeof (
        userDeviceConfig ) * 3 ))) {
      fprintf ( stderr, "malloc failed in %s\n", __FUNCTION__ );
      return;
    }
  } else {
    if ( sxConfigEntries % 3 == 0 ) {
      if (!( sxConfig = ( userDeviceConfig* ) realloc ( sxConfig,
          sizeof ( userDeviceConfig ) * ( sxConfigEntries / 3 + 1 )))) {
        fprintf ( stderr, "realloc failed in %s\n", __FUNCTION__ );
        return;
      }
    }
  }

  newEntry = sxConfig + sxConfigEntries * sizeof ( userDeviceConfig );
  memcpy ( newEntry, config, sizeof ( userDeviceConfig ));
  sxConfigEntries++;
  return;
}
