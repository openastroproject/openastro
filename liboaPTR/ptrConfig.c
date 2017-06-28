/*****************************************************************************
 *
 * ptrConfig.c -- Manage PTR device user config
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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
#include <openastro/ptr.h>

#include "oaptrprivate.h"
#include "ptr.h"


userDeviceConfig*	ptrConfig = 0;
int			ptrConfigEntries = 0;


void
oaPTRClearIDFilters ( void )
{
  if ( ptrConfig ) {
    free ( ptrConfig );
    ptrConfig = 0;
  }
  ptrConfigEntries = 0;
}


void
oaPTRAddIDFilter ( userDeviceConfig* config )
{
  userDeviceConfig*	newEntry;

  if ( !ptrConfig ) {
    if (!( ptrConfig = ( userDeviceConfig* ) malloc ( sizeof (
        userDeviceConfig ) * 3 ))) {
      fprintf ( stderr, "malloc failed in %s\n", __FUNCTION__ );
      return;
    }
  } else {
    if ( ptrConfigEntries % 3 == 0 ) {
      if (!( ptrConfig = ( userDeviceConfig* ) realloc ( ptrConfig,
          sizeof ( userDeviceConfig ) * ( ptrConfigEntries / 3 + 1 )))) {
        fprintf ( stderr, "realloc failed in %s\n", __FUNCTION__ );
        return;
      }
    }
  }

  newEntry = ptrConfig + ptrConfigEntries * sizeof ( userDeviceConfig );
  memcpy ( newEntry, config, sizeof ( userDeviceConfig ));
  ptrConfigEntries++;
  return;
}
