/*****************************************************************************
 *
 * zwoDynloader.c -- handle dynamic loading of libEFWFilter
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBZWOFW

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif

#include <openastro/errno.h>
#include <openastro/util.h>
#include <openastro/filterwheel.h>
#include <EFW_filter.h>

#include "oafwprivate.h"


int							( *p_EFWGetNum )( void );
EFW_ERROR_CODE	( *p_EFWGetID )( int, int* );
EFW_ERROR_CODE	( *p_EFWOpen )( int );
EFW_ERROR_CODE	( *p_EFWGetProperty )( int, EFW_INFO* );
EFW_ERROR_CODE	( *p_EFWClose )( int );
EFW_ERROR_CODE	( *p_EFWSetPosition )( int, int );


#if HAVE_LIBDL && !HAVE_STATIC_LIBEFWFILTER
static void*    _getDLSym ( void*, const char* );
#endif

int
_zwofwInitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL && !HAVE_STATIC_LIBEFWFILTER
	static void*		libHandle = 0;

	if ( !libHandle ) {
		if (!( libHandle = dlopen( "libEFWFilter.so", RTLD_LAZY ))) {
			return OA_ERR_LIBRARY_NOT_FOUND;
		}
	}

	dlerror();

  if (!( *( void** )( &p_EFWOpen ) = _getDLSym ( libHandle, "EFWOpen" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_EFWGetID ) = _getDLSym ( libHandle, "EFWGetID" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_EFWGetNum ) = _getDLSym ( libHandle, "EFWGetNum" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_EFWGetProperty ) = _getDLSym ( libHandle,
      "EFWGetProperty" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_EFWSetPosition ) =
			_getDLSym ( libHandle, "EFWSetPosition" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_EFWClose ) = _getDLSym ( libHandle, "EFWClose" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

#else
#if HAVE_STATIC_LIBEFWFILTER

	p_EFWOpen = EFWOpen;
	p_EFWGetID = EFWGetID;
	p_EFWGetNum = EFWGetNum;
	p_EFWGetProperty = EFWGetProperty;
	p_EFWSetPosition = EFWSetPosition;
	p_EFWClose = EFWClose;

#else
	return OA_ERR_LIBRARY_NOT_FOUND;
#endif	/* HAVE_STATIC_LIBEFWFILTER */
#endif	/* HAVE_LIBDL && !HAVE_STATIC_LIBEFWFILTER */
	return OA_ERR_NONE;
}


#if HAVE_LIBDL && !HAVE_STATIC_LIBEFWFILTER
static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libEFWFilter DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif
#endif	/* HAVE_LIBZWOFW */
