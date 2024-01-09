/*****************************************************************************
 *
 * aravisoacam.c -- main entrypoint for Aravis interface
 *
 * Copyright 2021,2023
 *   James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif
#ifdef ARAVIS_V06
#include <aravis-0.6/arv.h>
#define ARAVIS_NAME "libaravis-0.6.so.0"
#endif
#ifdef ARAVIS_V08
#include <aravis-0.8/arv.h>
#define ARAVIS_NAME "libaravis-0.8.so.0"
#endif

#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/demosaic.h>

#include "oacamprivate.h"
#include "unimplemented.h"

// Pointers to Aravis functions so we can use them via libdl.

#if HAVE_LIBDL
//static void*		_getDLSym ( void*, const char* );
#endif

/**
 * Cycle through the list of cameras returned by the Aravis library
 */

int
oaAravisGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
#if HAVE_LIBDL
  static void*		libHandle = 0;

  if ( !libHandle ) {
    if (!( libHandle = dlopen( ARAVIS_NAME, RTLD_LAZY ))) {
      return 0;
    }
  }

  dlerror();

#else /* HAVE_LIBDL */

#endif /* HAVE_LIBDL */

	return 0;
}
