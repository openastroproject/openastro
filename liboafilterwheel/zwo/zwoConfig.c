/*****************************************************************************
 *
 * zwoConfig.c -- Manage ZWO filter wheels user configuration
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "zwofw.h"


userDeviceConfig*	zwoConfig = 0;
static int              zwoConfigEntries = 0;


void
oaZWOClearIDFilters ( void )
{
  if ( zwoConfig ) {
    free ( zwoConfig );
    zwoConfig = 0;
  }
  zwoConfigEntries = 0;
}


void
oaZWOAddIDFilter ( userDeviceConfig* config )
{
  userDeviceConfig*     newEntry;

  if ( !zwoConfig ) {
    if (!( zwoConfig = ( userDeviceConfig* ) malloc ( sizeof (
        userDeviceConfig ) * 3 ))) {
      fprintf ( stderr, "malloc failed in %s\n", __FUNCTION__ );
      return;
    }
  } else {
    if ( zwoConfigEntries % 3 == 0 ) {
      if (!( zwoConfig = ( userDeviceConfig* ) realloc ( zwoConfig,
          sizeof ( userDeviceConfig ) * ( zwoConfigEntries / 3 + 1 )))) {
        fprintf ( stderr, "realloc failed in %s\n", __FUNCTION__ );
        return;
      }
    }
  }

  newEntry = zwoConfig + zwoConfigEntries * sizeof ( userDeviceConfig );
  memcpy ( newEntry, config, sizeof ( userDeviceConfig ));
  zwoConfigEntries++;
  return;
}
