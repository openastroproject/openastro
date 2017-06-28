/*****************************************************************************
 *
 * utils.c -- random support functions for filter wheels
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

#include <hidapi.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"


void
_oaFreeFilterWheelDeviceList ( FILTERWHEEL_LIST* deviceList )
{
  unsigned int		i;

  for ( i = 0; i < deviceList->numFilterWheels; i++ ) {
    free (( void* ) deviceList->wheelList[i] );
  }
  free (( void* ) deviceList->wheelList );
  deviceList->wheelList = 0;
  deviceList->maxFilterWheels = deviceList->numFilterWheels = 0;
}


int
_oaCheckFilterWheelArraySize ( FILTERWHEEL_LIST* deviceList )
{
  oaFilterWheelDevice**	newList;
  int			newNum;

  if ( deviceList->maxFilterWheels > deviceList->numFilterWheels ) {
    return OA_ERR_NONE;
  }

  newNum = ( deviceList->maxFilterWheels + 1 ) * 8;
  if (!( newList = realloc ( deviceList->wheelList, newNum * sizeof (
      oaFilterWheelDevice* )))) {
    return -OA_ERR_MEM_ALLOC;
  }

  deviceList->wheelList = newList;
  deviceList->maxFilterWheels = newNum;
  return OA_ERR_NONE;
}
