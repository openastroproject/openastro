/*****************************************************************************
 *
 * GP2dynloader.c -- handle dynamic loading of libgphoto2
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif
#include <openastro/errno.h>
#include <gphoto2/gphoto2-camera.h>

#include "oacamprivate.h"
#include "GP2private.h"


// Pointers to libgphoto2 functions so we can use them via libdl.

GPContext*	( *p_gp_context_new )( void );
int					( *p_gp_context_unref )( GPContext* );

int					( *p_gp_list_new )( CameraList** );
int					( *p_gp_list_reset )( CameraList* );
int					( *p_gp_list_free )( CameraList* );
int					( *p_gp_list_count )( CameraList* );
int					( *p_gp_list_get_name )( CameraList*, int, const char** );
int					( *p_gp_list_get_value )( CameraList*, int, const char** );

int					( *p_gp_camera_autodetect )( CameraList*, GPContext* );

int					( *p_gp_context_set_error_func )( GPContext*, void*, void* );
int					( *p_gp_context_set_status_func )( GPContext*, void*, void* );
int					( *p_gp_context_set_cancel_func )( GPContext*, void*, void* );
int					( *p_gp_context_set_message_func )( GPContext*, void*, void* );

#if HAVE_LIBDL && !HAVE_STATIC_LIBGPHOTO2
static void*		_getDLSym ( void*, const char* );
#endif


int
_gp2InitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL && !HAVE_STATIC_LIBGPHOTO2
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*		libName = "libgphoto2.dylib";
#else
  const char*		libName = "libgphoto2.so.6";
#endif

	*libPath = 0;
  if ( !libHandle ) {
		if ( installPathRoot ) {
			( void ) strncpy ( libPath, installPathRoot, PATH_MAX );
		}
#ifdef SHLIB_PATH
		( void ) strncat ( libPath, SHLIB_PATH, PATH_MAX );
#endif
		( void ) strncat ( libPath, libName, PATH_MAX );

    if (!( libHandle = dlopen ( libPath, RTLD_LAZY ))) {
      // fprintf ( stderr, "can't load %s:\n%s\n", libPath, dlerror());
      return OA_ERR_LIBRARY_NOT_FOUND;
    }

	  dlerror();

	  if (!( *( void** )( &p_gp_context_new ) = _getDLSym ( libHandle,
	      "gp_context_new" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_context_unref ) = _getDLSym ( libHandle,
	      "gp_context_unref" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_list_new ) = _getDLSym ( libHandle,
	      "gp_list_new" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_list_reset ) = _getDLSym ( libHandle,
	      "gp_list_reset" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_list_free ) = _getDLSym ( libHandle,
	      "gp_list_free" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_list_count ) = _getDLSym ( libHandle,
	      "gp_list_count" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_list_get_name ) = _getDLSym ( libHandle,
	      "gp_list_get_name" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_list_get_value ) = _getDLSym ( libHandle,
	      "gp_list_get_value" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_autodetect ) = _getDLSym ( libHandle,
	      "gp_camera_autodetect" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_context_set_error_func ) = _getDLSym ( libHandle,
	      "gp_context_set_error_func" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_context_set_status_func ) = _getDLSym ( libHandle,
	      "gp_context_set_status_func" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_context_set_cancel_func ) = _getDLSym ( libHandle,
	      "gp_context_set_cancel_func" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_context_set_message_func ) = _getDLSym ( libHandle,
	      "gp_context_set_message_func" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	}
#else
#if HAVE_STATIC_LIBGPHOTO2

	p_gp_context_new = gp_context_new;
	p_gp_context_unref = gp_context_unref;

	p_gp_list_new = gp_list_new;
	p_gp_list_reset = gp_list_reset;
	p_gp_list_free = gp_list_free;
	p_gp_list_count = gp_list_count;
	p_gp_list_get_name = gp_list_get_name;
	p_gp_list_get_value = gp_list_get_value;

	p_gp_camera_autodetect = gp_camera_autodetect;

	p_gp_context_set_error_func = gp_context_set_error_func;
	p_gp_context_set_status_func = gp_context_set_status_func;
	p_gp_context_set_cancel_func = gp_context_set_cancel_func;
	p_gp_context_set_message_func = gp_context_set_message_func;

#else
	return OA_ERR_LIBRARY_NOT_FOUND;
#endif	/* HAVE_STATIC_LIBGPHOTO2 */
#endif	/* HAVE_LIBDL && !HAVE_STATIC_LIBGPHOTO2 */
	return OA_ERR_NONE;
}


static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libgphoto2 DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
