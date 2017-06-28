/*****************************************************************************
 *
 * unimplemented.c -- catch-all for unimplemented filter wheel functions
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

#include <openastro/filterwheel.h>
#include <openastro/util.h>

#include "oafwprivate.h"
#include "unimplemented.h"


static oaFilterWheel*
_initFilterWheel ( oaFilterWheelDevice* device )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      device->deviceName );
  return 0;
}


static int
_closeFilterWheel ( oaFilterWheel* wheel )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      wheel->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_readControl ( oaFilterWheel* wheel, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      wheel->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_setControl ( oaFilterWheel* wheel, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      wheel->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


static int
_testControl ( oaFilterWheel* wheel, int c, oaControlValue* v )
{
  fprintf ( stderr, "%s not implemented for %s\n", __FUNCTION__,
      wheel->deviceName );
  return -OA_ERR_UNIMPLEMENTED;
}


void
_oaInitFilterWheelDeviceFunctionPointers ( oaFilterWheelDevice* device )
{
  device->initFilterWheel = _initFilterWheel;
}


void
_oaInitFilterWheelFunctionPointers ( oaFilterWheel* wheel )
{
  wheel->funcs.initWheel = _initFilterWheel;
  wheel->funcs.closeWheel = _closeFilterWheel;
  wheel->funcs.readControl = _readControl;
  wheel->funcs.testControl = _testControl;
  wheel->funcs.setControl = _setControl;
}
