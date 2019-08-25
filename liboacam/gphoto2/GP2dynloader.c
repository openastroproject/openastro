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
void				( *p_gp_context_unref )( GPContext* );

int					( *p_gp_list_new )( CameraList** );
int					( *p_gp_list_reset )( CameraList* );
int					( *p_gp_list_free )( CameraList* );
int					( *p_gp_list_unref )( CameraList* );
int					( *p_gp_list_count )( CameraList* );
int					( *p_gp_list_get_name )( CameraList*, int, const char** );
int					( *p_gp_list_get_value )( CameraList*, int, const char** );

int					( *p_gp_camera_autodetect )( CameraList*, GPContext* );
int					( *p_gp_camera_new )( Camera** );
int					( *p_gp_camera_set_abilities )( Camera*, CameraAbilities );
int					( *p_gp_camera_set_port_info )( Camera*, GPPortInfo );
int					( *p_gp_camera_unref )( Camera* );
int					( *p_gp_camera_init )( Camera*, GPContext* );
int					( *p_gp_camera_exit )( Camera*, GPContext* );
int					( *p_gp_camera_get_config )( Camera*, CameraWidget**, GPContext* );
int					( *p_gp_camera_set_config )( Camera*, CameraWidget*, GPContext* );
int					( *p_gp_camera_trigger_capture )( Camera*, GPContext* );
int					( *p_gp_camera_wait_for_event )( Camera*, int, CameraEventType*,
								void**, GPContext* );
int					( *p_gp_camera_file_get )( Camera*, const char*, const char*,
								CameraFileType, CameraFile*, GPContext* );

void				( *p_gp_context_set_error_func )( GPContext*, GPContextErrorFunc,
								void* );
void				( *p_gp_context_set_status_func )( GPContext*, GPContextStatusFunc,
								void* );
void				( *p_gp_context_set_cancel_func )( GPContext*, GPContextCancelFunc,
								void* );
void				( *p_gp_context_set_message_func )( GPContext*,
								GPContextMessageFunc, void* );

int					( *p_gp_abilities_list_get_abilities )( CameraAbilitiesList*,
								int, CameraAbilities* );
int					( *p_gp_abilities_list_load )( CameraAbilitiesList*, GPContext* );
int					( *p_gp_abilities_list_lookup_model )( CameraAbilitiesList*,
								const char* );
int					( *p_gp_abilities_list_new )( CameraAbilitiesList** );

int					( *p_gp_widget_get_child_by_name )( CameraWidget*, const char*,
								CameraWidget** );
int					( *p_gp_widget_get_child_by_label )( CameraWidget*, const char*,
								CameraWidget** );
int					( *p_gp_widget_get_name )( CameraWidget*, const char** );
int					( *p_gp_widget_get_type )( CameraWidget*, CameraWidgetType* );
int					( *p_gp_widget_get_value )( CameraWidget*, void* );
int					( *p_gp_widget_count_choices )( CameraWidget* );
int					( *p_gp_widget_get_choice )( CameraWidget*, int, const char** );
int					( *p_gp_widget_set_value )( CameraWidget*, const void* );

int					( *p_gp_port_info_list_count )( GPPortInfoList* );
int					( *p_gp_port_info_list_free )( GPPortInfoList* );
int					( *p_gp_port_info_list_get_info )( GPPortInfoList*, int,
								GPPortInfo* );
int					( *p_gp_port_info_list_load )( GPPortInfoList* );
int					( *p_gp_port_info_list_lookup_path )( GPPortInfoList*,
								const char* );
int					( *p_gp_port_info_list_new )( GPPortInfoList** );

int					( *p_gp_log_add_func )( GPLogLevel, GPLogFunc, void* );

int					( *p_gp_file_new )( CameraFile** );
int					( *p_gp_file_free )( CameraFile* );
int					( *p_gp_file_get_data_and_size )( CameraFile*, const char**,
								unsigned long* );
int					( *p_gp_file_get_mime_type )( CameraFile*, const char** );

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

	  if (!( *( void** )( &p_gp_list_unref ) = _getDLSym ( libHandle,
	      "gp_list_unref" ))) {
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

	  if (!( *( void** )( &p_gp_camera_new ) = _getDLSym ( libHandle,
	      "gp_camera_new" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_set_abilities ) = _getDLSym ( libHandle,
	      "gp_camera_set_abilities" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_set_port_info ) = _getDLSym ( libHandle,
	      "gp_camera_set_port_info" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_unref ) = _getDLSym ( libHandle,
	      "gp_camera_unref" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_init ) = _getDLSym ( libHandle,
	      "gp_camera_init" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_exit ) = _getDLSym ( libHandle,
	      "gp_camera_exit" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_get_config ) = _getDLSym ( libHandle,
	      "gp_camera_get_config" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_set_config ) = _getDLSym ( libHandle,
	      "gp_camera_set_config" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }
	  if (!( *( void** )( &p_gp_camera_trigger_capture ) = _getDLSym ( libHandle,
	      "gp_camera_trigger_capture" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_wait_for_event ) = _getDLSym ( libHandle,
	      "gp_camera_wait_for_event" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_camera_file_get ) = _getDLSym ( libHandle,
	      "gp_camera_file_get" ))) {
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

	  if (!( *( void** )( &p_gp_context_set_message_func ) =
				_getDLSym ( libHandle, "gp_context_set_message_func" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_abilities_list_get_abilities ) =
				_getDLSym ( libHandle, "gp_abilities_list_get_abilities" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_abilities_list_load ) = _getDLSym ( libHandle,
	      "gp_abilities_list_load" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_abilities_list_lookup_model ) =
				_getDLSym ( libHandle, "gp_abilities_list_lookup_model" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_abilities_list_new ) = _getDLSym ( libHandle,
	      "gp_abilities_list_new" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_get_child_by_name ) =
				_getDLSym ( libHandle, "gp_widget_get_child_by_name" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_get_child_by_label ) =
				_getDLSym ( libHandle, "gp_widget_get_child_by_label" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_get_name ) = _getDLSym ( libHandle,
	      "gp_widget_get_name" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_get_type ) = _getDLSym ( libHandle,
	      "gp_widget_get_type" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_get_value ) = _getDLSym ( libHandle,
	      "gp_widget_get_value" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_count_choices ) = _getDLSym ( libHandle,
	      "gp_widget_count_choices" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_get_choice ) = _getDLSym ( libHandle,
	      "gp_widget_get_choice" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_widget_set_value ) = _getDLSym ( libHandle,
	      "gp_widget_set_value" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_port_info_list_count ) = _getDLSym ( libHandle,
	      "gp_port_info_list_count" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_port_info_list_free ) = _getDLSym ( libHandle,
	      "gp_port_info_list_free" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_port_info_list_get_info ) = _getDLSym ( libHandle,
	      "gp_port_info_list_get_info" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_port_info_list_load ) = _getDLSym ( libHandle,
	      "gp_port_info_list_load" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_port_info_list_lookup_path ) =
				_getDLSym ( libHandle, "gp_port_info_list_lookup_path" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_port_info_list_new ) = _getDLSym ( libHandle,
	      "gp_port_info_list_new" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_log_add_func ) = _getDLSym ( libHandle,
	      "gp_log_add_func" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_file_new ) = _getDLSym ( libHandle,
	      "gp_file_new" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_file_free ) = _getDLSym ( libHandle,
	      "gp_file_free" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_file_get_data_and_size ) = _getDLSym ( libHandle,
	      "gp_file_get_data_and_size" ))) {
			dlclose ( libHandle );
			libHandle = 0;
	    return OA_ERR_SYMBOL_NOT_FOUND;
	  }

	  if (!( *( void** )( &p_gp_file_get_mime_type ) = _getDLSym ( libHandle,
	      "gp_file_get_mime_type" ))) {
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
	p_gp_list_unref = gp_list_unref;
	p_gp_list_count = gp_list_count;
	p_gp_list_get_name = gp_list_get_name;
	p_gp_list_get_value = gp_list_get_value;

	p_gp_camera_autodetect = gp_camera_autodetect;
	p_gp_camera_new = gp_camera_new;
	p_gp_camera_set_abilities = gp_camera_set_abilities;
	p_gp_camera_set_port_info = gp_camera_set_port_info;
	p_gp_camera_unref = gp_camera_unref;
	p_gp_camera_init = gp_camera_init;
	p_gp_camera_exit = gp_camera_exit;
	p_gp_camera_get_config = gp_camera_get_config;
	p_gp_camera_set_config = gp_camera_set_config;
	p_gp_camera_trigger_capture = gp_camera_trigger_capture;
	p_gp_camera_wait_for_event = gp_camera_wait_for_event;
	p_gp_camera_file_get = gp_camera_file_get;

	p_gp_context_set_error_func = gp_context_set_error_func;
	p_gp_context_set_status_func = gp_context_set_status_func;
	p_gp_context_set_cancel_func = gp_context_set_cancel_func;
	p_gp_context_set_message_func = gp_context_set_message_func;

	p_gp_abilities_list_get_abilities = gp_abilities_list_get_abilities;
	p_gp_abilities_list_load = gp_abilities_list_load;
	p_gp_abilities_list_lookup_model = gp_abilities_list_lookup_model;
	p_gp_abilities_list_new = gp_abilities_list_new;

	p_gp_widget_get_child_by_name = gp_widget_get_child_by_name;
	p_gp_widget_get_child_by_label = gp_widget_get_child_by_label;
	p_gp_widget_get_name = gp_widget_get_name;
	p_gp_widget_get_type = gp_widget_get_type;
	p_gp_widget_get_value = gp_widget_get_value;
	p_gp_widget_count_choices = gp_widget_count_choices;
	p_gp_widget_get_choice = gp_widget_get_choice;
	p_gp_widget_set_value = gp_widget_set_value;

	p_gp_port_info_list_count = gp_port_info_list_count;
	p_gp_port_info_list_free = gp_port_info_list_free;
	p_gp_port_info_list_get_info = gp_port_info_list_get_info;
	p_gp_port_info_list_load = gp_port_info_list_load;
	p_gp_port_info_list_lookup_path = gp_port_info_list_lookup_path;
	p_gp_port_info_list_new = gp_port_info_list_new;

	p_gp_log_add_func = gp_log_add_func;

	p_gp_file_new = gp_file_new;
	p_gp_file_free = gp_file_free;
	p_gp_file_get_data_and_size = gp_file_get_data_and_size;
	p_gp_file_get_data_and_size = gp_file_get_mime_type;

#else
	return OA_ERR_LIBRARY_NOT_FOUND;
#endif	/* HAVE_STATIC_LIBGPHOTO2 */
#endif	/* HAVE_LIBDL && !HAVE_STATIC_LIBGPHOTO2 */
	return OA_ERR_NONE;
}


#if HAVE_LIBDL && !HAVE_STATIC_LIBGPHOTO2
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
#endif /* HAVE_LIBDL && !HAVE_STATIC_LIBGPHOTO2 */
