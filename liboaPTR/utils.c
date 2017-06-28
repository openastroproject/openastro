/*****************************************************************************
 *
 * utils.c -- random support functions for PTR
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


void
_oaFreePTRDeviceList ( PTR_LIST* deviceList )
{
  unsigned int		i;

  for ( i = 0; i < deviceList->numPTRDevices; i++ ) {
    free (( void* ) deviceList->ptrList[i] );
  }
  free (( void* ) deviceList->ptrList );
  deviceList->ptrList = 0;
  deviceList->maxPTRDevices = deviceList->numPTRDevices = 0;
}


int
_oaCheckPTRArraySize ( PTR_LIST* deviceList )
{
  oaPTRDevice**		newList;
  int			newNum;

  if ( deviceList->maxPTRDevices > deviceList->numPTRDevices ) {
    return OA_ERR_NONE;
  }

  newNum = ( deviceList->maxPTRDevices + 1 ) * 8;
  if (!( newList = realloc ( deviceList->ptrList, newNum * sizeof (
      oaPTRDevice* )))) {
    return -OA_ERR_MEM_ALLOC;
  }

  deviceList->ptrList = newList;
  deviceList->maxPTRDevices = newNum;
  return OA_ERR_NONE;
}
