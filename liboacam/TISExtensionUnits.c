/*****************************************************************************
 *
 * TISExtensionUnits.c -- Process TIS Extension Units
 *
 * Copyright 2017 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBUVC

#include <openastro/camera.h>

#include "oacamprivate.h"
#include "UVCExtnUnits.h"
#include "TISExtensionUnits.h"


void
processTISExtnUnitUSB ( oaCamera* camera, COMMON_INFO* commonInfo,
    uint64_t flags )
{
  fprintf ( stderr, "TIS USB XU 0aba49de-5c0b-49d5-8f71-0be40f94a67a not "
      "currently handled\n" );
}


void
processTISExtnUnitUSB3 ( oaCamera* camera, COMMON_INFO* commonInfo,
    uint64_t flags )
{
  fprintf ( stderr, "TIS USB3 XU de49ba0a-0b5c-d549-8f71-0be40f94a67a not "
      "currently handled\n" );
}

#endif /* HAVE_LIBUVC */
