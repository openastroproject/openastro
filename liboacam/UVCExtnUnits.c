/*****************************************************************************
 *
 * UVCExtnUnits.c -- List of UVC extension units
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


struct UVCExtension UVCExtensionMap[] = {
  {
    .guid = { 0x0a, 0xba, 0x49, 0xde, 0x5c, 0x0b, 0x49, 0xd5,
              0x8f, 0x71, 0x0b, 0xe4, 0x0f, 0x94, 0xa6, 0x7a },
    .handler = processTISExtnUnitUSB
  },
  {
    .guid = { 0xde, 0x49, 0xba, 0x0a, 0x0b, 0x5c, 0xd5, 0x49,
              0x8f, 0x71, 0x0b, 0xe4, 0x0f, 0x94, 0xa6, 0x7a },
    .handler = processTISExtnUnitUSB3
  },
  { .handler = 0
  }
};

#endif
