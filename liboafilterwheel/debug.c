/*****************************************************************************
 *
 * debug.c -- debug message handling
 *
 * Copyright 2014,2015,2020 James Fidell (james@openastroproject.org)
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

#if STDC_HEADERS
#include <stdarg.h>
#endif
#include <libusb-1.0/libusb.h>

#include <openastro/filterwheel.h>
#include <openastro/util.h>
#include <openastro/debug.h>

#include "oafwprivate.h"

static int _debugLevel = 0;

void
oafwSetDebugLevel ( int v )
{
  _debugLevel = v;
}


void
oafwClearDebugLevel ( int v )
{
  _debugLevel &= ~v;
}


void
oafwAddDebugLevel ( int v )
{
  _debugLevel |= v;
}


void
oafwDebugMsg ( int type, const char* msg, ... )
{
  va_list args;

  if ( type & _debugLevel ) {
    va_start ( args, msg );
    vfprintf ( stderr, msg, args );
    va_end ( args );
  }
}
