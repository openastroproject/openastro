/*****************************************************************************
 *
 * iidcDynloader.c -- handle dynamic loading of libdc1394
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

#if HAVE_LIBDC1394

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif

#include <openastro/errno.h>
#include <dc1394/dc1394.h>

#include "IIDCprivate.h"


dc1394error_t		( *p_dc1394_camera_enumerate )( dc1394_t*,
										dc1394camera_list_t ** );
void						( *p_dc1394_camera_free )( dc1394camera_t* );
void						( *p_dc1394_camera_free_list )( dc1394camera_list_t* );
dc1394error_t		( *p_dc1394_camera_get_broadcast )( dc1394camera_t*,
										dc1394bool_t* );
dc1394camera_t*	( *p_dc1394_camera_new_unit )( dc1394_t*, uint64_t, int );
dc1394error_t		( *p_dc1394_camera_reset )( dc1394camera_t* );
dc1394error_t		( *p_dc1394_capture_dequeue )( dc1394camera_t*,
										dc1394capture_policy_t, dc1394video_frame_t** );
dc1394error_t		( *p_dc1394_capture_enqueue )( dc1394camera_t*,
										dc1394video_frame_t* );
dc1394error_t		( *p_dc1394_capture_setup )( dc1394camera_t*, uint32_t,
										uint32_t );
dc1394error_t		( *p_dc1394_capture_stop )( dc1394camera_t* );
dc1394error_t		( *p_dc1394_external_trigger_set_mode )( dc1394camera_t*,
										dc1394trigger_mode_t );
dc1394error_t		( *p_dc1394_external_trigger_set_polarity )( dc1394camera_t*,
										dc1394trigger_polarity_t );
dc1394error_t		( *p_dc1394_external_trigger_set_power )( dc1394camera_t*,
										dc1394switch_t );
dc1394error_t		( *p_dc1394_feature_get_absolute_value )( dc1394camera_t*,
										dc1394feature_t, float* );
dc1394error_t		( *p_dc1394_feature_get_all )( dc1394camera_t*,
											dc1394featureset_t* );
dc1394error_t		( *p_dc1394_feature_get_mode )( dc1394camera_t*,
											dc1394feature_t, dc1394feature_mode_t* );
dc1394error_t		( *p_dc1394_feature_get_value )( dc1394camera_t*,
											dc1394feature_t, uint32_t* );
dc1394error_t		( *p_dc1394_feature_set_absolute_control )( dc1394camera_t*,
											dc1394feature_t, dc1394switch_t );
dc1394error_t		( *p_dc1394_feature_set_absolute_value )( dc1394camera_t*,
											dc1394feature_t, float );
dc1394error_t		( *p_dc1394_feature_set_mode )( dc1394camera_t*,
											dc1394feature_t, dc1394feature_mode_t );
dc1394error_t		( *p_dc1394_feature_set_power )( dc1394camera_t*,
											dc1394feature_t, dc1394switch_t );
dc1394error_t		( *p_dc1394_feature_set_value )( dc1394camera_t*,
											dc1394feature_t, uint32_t );
dc1394error_t		( *p_dc1394_feature_temperature_get_value )( dc1394camera_t*,
											uint32_t*, uint32_t* );
dc1394error_t		( *p_dc1394_feature_temperature_set_value )( dc1394camera_t*,
											uint32_t );
dc1394error_t		( *p_dc1394_feature_whitebalance_get_value )( dc1394camera_t*,
											uint32_t*, uint32_t* );
dc1394error_t		( *p_dc1394_feature_whitebalance_set_value )( dc1394camera_t*,
											uint32_t, uint32_t );
dc1394error_t		( *p_dc1394_format7_get_modeset )( dc1394camera_t*,
											dc1394format7modeset_t* );
dc1394error_t		( *p_dc1394_format7_set_roi )( dc1394camera_t*,
											dc1394video_mode_t, dc1394color_coding_t, int32_t,
											int32_t, int32_t, int32_t, int32_t );
void						( *p_dc1394_free  )( dc1394_t* );
dc1394error_t		( *p_dc1394_get_color_coding_from_video_mode )(
											dc1394camera_t*, dc1394video_mode_t,
											dc1394color_coding_t* );
dc1394error_t		( *p_dc1394_get_image_size_from_video_mode )( dc1394camera_t*,
											dc1394video_mode_t, uint32_t*, uint32_t* );
dc1394_t*				( *p_dc1394_new )( void );
dc1394error_t		( *p_dc1394_video_get_supported_framerates )( dc1394camera_t*,
											dc1394video_mode_t, dc1394framerates_t* );
dc1394error_t		( *p_dc1394_video_get_supported_modes )( dc1394camera_t*,
											dc1394video_modes_t* );
dc1394error_t		( *p_dc1394_video_set_framerate )( dc1394camera_t*,
											dc1394framerate_t );
dc1394error_t		( *p_dc1394_video_set_iso_speed )( dc1394camera_t*,
											dc1394speed_t );
dc1394error_t		( *p_dc1394_video_set_mode )( dc1394camera_t*,
											dc1394video_mode_t );
dc1394error_t		( *p_dc1394_video_set_operation_mode )( dc1394camera_t*,
											dc1394operation_mode_t );
dc1394error_t		( *p_dc1394_video_set_transmission )( dc1394camera_t*,
											dc1394switch_t );

#if HAVE_LIBDL && !HAVE_STATIC_LIBDC1394
static void*    _getDLSym ( void*, const char* );
#endif

int
_iidcInitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL && !HAVE_STATIC_LIBDC1394
	static void*		libHandle = 0;

	if ( !libHandle ) {
		if (!( libHandle = dlopen( "libdc1394.so.22", RTLD_LAZY ))) {
			return OA_ERR_LIBRARY_NOT_FOUND;
		}
	}

	dlerror();

  if (!( *( void** )( &p_dc1394_camera_enumerate ) = _getDLSym ( libHandle,
      "dc1394_camera_enumerate" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_camera_free ) = _getDLSym ( libHandle,
      "dc1394_camera_free" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_camera_free_list ) = _getDLSym ( libHandle,
      "dc1394_camera_free_list" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_camera_get_broadcast ) = _getDLSym ( libHandle,
      "dc1394_camera_get_broadcast" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_camera_new_unit ) = _getDLSym ( libHandle,
      "dc1394_camera_new_unit" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_camera_reset ) = _getDLSym ( libHandle,
      "dc1394_camera_reset" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_capture_dequeue ) = _getDLSym ( libHandle,
      "dc1394_capture_dequeue" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_capture_enqueue ) = _getDLSym ( libHandle,
      "dc1394_capture_enqueue" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_capture_setup ) = _getDLSym ( libHandle,
      "dc1394_capture_setup" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_capture_stop ) = _getDLSym ( libHandle,
      "dc1394_capture_stop" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_external_trigger_set_mode ) =
			_getDLSym ( libHandle, "dc1394_external_trigger_set_mode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_external_trigger_set_polarity ) =
			_getDLSym ( libHandle, "dc1394_external_trigger_set_polarity" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_external_trigger_set_power ) =
			_getDLSym ( libHandle, "dc1394_external_trigger_set_power" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_get_absolute_value ) =
			_getDLSym ( libHandle, "dc1394_feature_get_absolute_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_get_all ) = _getDLSym ( libHandle,
      "dc1394_feature_get_all" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_get_mode ) = _getDLSym ( libHandle,
      "dc1394_feature_get_mode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_get_value ) = _getDLSym ( libHandle,
      "dc1394_feature_get_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_set_absolute_control ) =
			_getDLSym ( libHandle, "dc1394_feature_set_absolute_control" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_set_absolute_value ) =
			_getDLSym ( libHandle, "dc1394_feature_set_absolute_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_set_mode ) = _getDLSym ( libHandle,
      "dc1394_feature_set_mode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_set_power ) = _getDLSym ( libHandle,
      "dc1394_feature_set_power" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_set_value ) = _getDLSym ( libHandle,
      "dc1394_feature_set_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_temperature_get_value ) =
			_getDLSym ( libHandle, "dc1394_feature_temperature_get_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_temperature_set_value ) =
			_getDLSym ( libHandle, "dc1394_feature_temperature_set_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_whitebalance_get_value ) =
			_getDLSym ( libHandle, "dc1394_feature_whitebalance_get_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_feature_whitebalance_set_value ) =
			_getDLSym ( libHandle, "dc1394_feature_whitebalance_set_value" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_format7_get_modeset ) = _getDLSym ( libHandle,
      "dc1394_format7_get_modeset" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_format7_set_roi ) = _getDLSym ( libHandle,
      "dc1394_format7_set_roi" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_free ) = _getDLSym ( libHandle,
      "dc1394_free" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_get_color_coding_from_video_mode ) =
			_getDLSym ( libHandle, "dc1394_get_color_coding_from_video_mode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_get_image_size_from_video_mode ) =
			_getDLSym ( libHandle, "dc1394_get_image_size_from_video_mode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_new ) = _getDLSym ( libHandle,
      "dc1394_new" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_video_get_supported_framerates ) =
			_getDLSym ( libHandle, "dc1394_video_get_supported_framerates" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_video_get_supported_modes ) =
			_getDLSym ( libHandle, "dc1394_video_get_supported_modes" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_video_set_framerate ) = _getDLSym ( libHandle,
      "dc1394_video_set_framerate" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_video_set_iso_speed ) = _getDLSym ( libHandle,
      "dc1394_video_set_iso_speed" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_video_set_mode ) = _getDLSym ( libHandle,
      "dc1394_video_set_mode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_video_set_operation_mode ) =
			_getDLSym ( libHandle, "dc1394_video_set_operation_mode" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_dc1394_video_set_transmission ) =
			_getDLSym ( libHandle, "dc1394_video_set_transmission" ))) {
    dlclose ( libHandle );
    libHandle = 0;
    return OA_ERR_SYMBOL_NOT_FOUND;
  }


#else
#if HAVE_STATIC_LIBDC1394

	p_dc1394_camera_enumerate = dc1394_camera_enumerate;
	p_dc1394_camera_free = dc1394_camera_free;
	p_dc1394_camera_free_list = dc1394_camera_free_list;
	p_dc1394_camera_get_broadcast = dc1394_camera_get_broadcast;
	p_dc1394_camera_new_unit = dc1394_camera_new_unit;
	p_dc1394_camera_reset = dc1394_camera_reset;
	p_dc1394_capture_dequeue = dc1394_capture_dequeue;
	p_dc1394_capture_enqueue = dc1394_capture_enqueue;
	p_dc1394_capture_setup = dc1394_capture_setup;
	p_dc1394_capture_stop = dc1394_capture_stop;
	p_dc1394_external_trigger_set_mode = dc1394_external_trigger_set_mode;
	p_dc1394_external_trigger_set_polarity =
			dc1394_external_trigger_set_polarity;
	p_dc1394_external_trigger_set_power = dc1394_external_trigger_set_power;
	p_dc1394_feature_get_absolute_value = dc1394_feature_get_absolute_value;
	p_dc1394_feature_get_all = dc1394_feature_get_all;
	p_dc1394_feature_get_mode = dc1394_feature_get_mode;
	p_dc1394_feature_get_value = dc1394_feature_get_value;
	p_dc1394_feature_set_absolute_control = dc1394_feature_set_absolute_control;
	p_dc1394_feature_set_absolute_value = dc1394_feature_set_absolute_value;
	p_dc1394_feature_set_mode = dc1394_feature_set_mode;
	p_dc1394_feature_set_power = dc1394_feature_set_power;
	p_dc1394_feature_set_value = dc1394_feature_set_value;
	p_dc1394_feature_temperature_get_value =
			dc1394_feature_temperature_get_value;
	p_dc1394_feature_temperature_set_value =
			dc1394_feature_temperature_set_value;
	p_dc1394_feature_whitebalance_get_value =
			dc1394_feature_whitebalance_get_value;
	p_dc1394_feature_whitebalance_set_value =
			dc1394_feature_whitebalance_set_value;
	p_dc1394_format7_get_modeset = dc1394_format7_get_modeset;
	p_dc1394_format7_set_roi = dc1394_format7_set_roi;
	p_dc1394_free = dc1394_free;
	p_dc1394_get_color_coding_from_video_mode =
			dc1394_get_color_coding_from_video_mode;
	p_dc1394_get_image_size_from_video_mode =
			dc1394_get_image_size_from_video_mode;
	p_dc1394_new = dc1394_new;
	p_dc1394_video_get_supported_framerates =
			dc1394_video_get_supported_framerates;
	p_dc1394_video_get_supported_modes = dc1394_video_get_supported_modes;
	p_dc1394_video_set_framerate = dc1394_video_set_framerate;
	p_dc1394_video_set_iso_speed = dc1394_video_set_iso_speed;
	p_dc1394_video_set_mode = dc1394_video_set_mode;
	p_dc1394_video_set_operation_mode = dc1394_video_set_operation_mode;
	p_dc1394_video_set_transmission = dc1394_video_set_transmission;

#else
	return OA_ERR_LIBRARY_NOT_FOUND;
#endif	/* HAVE_STATIC_LIBDC1394 */
#endif	/* HAVE_LIBDL && !HAVE_STATIC_LIBDC1394 */
	return OA_ERR_NONE;
}

#if HAVE_LIBDL && !HAVE_STATIC_LIBDC1394
static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libdc1394 DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif
#endif	/* HAVE_LIBDC1394 */
