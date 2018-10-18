/*****************************************************************************
 *
 * unimplemented.c -- catch-all for unimplemented PTR functions
 *
 * Copyright 2015, 2017, 2018 James Fidell (james@openastroproject.org)
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

#include <openastro/timer.h>
#include <openastro/util.h>

#include "oaptrprivate.h"
#include "unimplemented.h"


static oaPTR*
_initPTR ( oaPTRDevice* device )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return 0;
}


static int
_closePTR ( oaPTR* device )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_readControl ( oaPTR* device, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_setControl ( oaPTR* device, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_testControl ( oaPTR* device, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_readGPS ( oaPTR* device, double* buffer )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_readCachedGPS ( oaPTR* device, double* buffer )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


void
_oaInitPTRDeviceFunctionPointers ( oaPTRDevice* device )
{
  device->init = _initPTR;
}


void
_oaInitPTRFunctionPointers ( oaPTR* device )
{
  device->funcs.init = _initPTR;
  device->funcs.close = _closePTR;
  device->funcs.readControl = _readControl;
  device->funcs.testControl = _testControl;
  device->funcs.setControl = _setControl;
  device->funcs.readGPS = _readGPS;
  device->funcs.readCachedGPS = _readCachedGPS;
}
