/*****************************************************************************
 *
 * UVCdynloader.c -- handle dynamic loading of libuvc
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

#if HAVE_LIBUVC

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif

#include <openastro/errno.h>
#include <libuvc/libuvc.h>

#include "UVCprivate.h"


uvc_error_t	( *p_uvc_init )( uvc_context_t**, struct libusb_context * );
void				( *p_uvc_exit )( uvc_context_t* );
uvc_error_t ( *p_uvc_get_device_list )( uvc_context_t*, uvc_device_t*** );
void				( *p_uvc_free_device_list )( uvc_device_t **, uint8_t );
uvc_error_t ( *p_uvc_get_device_descriptor )( uvc_device_t*, uvc_device_descriptor_t** );
void				( *p_uvc_free_device_descriptor )( uvc_device_descriptor_t* );
uint8_t			( *p_uvc_get_bus_number )( uvc_device_t* );
uint8_t			( *p_uvc_get_device_address )( uvc_device_t* );
uvc_error_t	( *p_uvc_find_device )( uvc_context_t*, uvc_device_t**, int, int, const char* );
uvc_error_t	( *p_uvc_find_devices )( uvc_context_t*, uvc_device_t***, int, int, const char* );
uvc_error_t	( *p_uvc_open )( uvc_device_t*, uvc_device_handle_t** );
void				( *p_uvc_close )( uvc_device_handle_t* );
uvc_device_t* ( *p_uvc_get_device )( uvc_device_handle_t* );
struct libusb_device_handle* ( *p_uvc_get_libusb_handle )(uvc_device_handle_t* );
void				( *p_uvc_ref_device )( uvc_device_t* );
void				( *p_uvc_unref_device )(uvc_device_t* );
void				( *p_uvc_set_status_callback )( uvc_device_handle_t*, uvc_status_callback_t, void* );
void				( *p_uvc_set_button_callback )( uvc_device_handle_t*, uvc_button_callback_t, void* );
const uvc_input_terminal_t* ( *p_uvc_get_camera_terminal )( uvc_device_handle_t* );
const uvc_input_terminal_t* ( *p_uvc_get_input_terminals )( uvc_device_handle_t* );
const uvc_output_terminal_t* ( *p_uvc_get_output_terminals )( uvc_device_handle_t* );
const uvc_selector_unit_t* ( *p_uvc_get_selector_units )( uvc_device_handle_t* );
const uvc_processing_unit_t* ( *p_uvc_get_processing_units )( uvc_device_handle_t* );
const uvc_extension_unit_t* ( *p_uvc_get_extension_units )( uvc_device_handle_t* );
uvc_error_t	( *p_uvc_get_stream_ctrl_format_size )( uvc_device_handle_t*, uvc_stream_ctrl_t*, enum uvc_frame_format, int, int, int );
const uvc_format_desc_t* ( *p_uvc_get_format_descs )( uvc_device_handle_t* );
uvc_error_t	( *p_uvc_probe_stream_ctrl )( uvc_device_handle_t*, uvc_stream_ctrl_t* );
uvc_error_t	( *p_uvc_start_streaming )( uvc_device_handle_t*, uvc_stream_ctrl_t*, uvc_frame_callback_t*, void*, uint8_t );
uvc_error_t ( *p_uvc_start_iso_streaming )( uvc_device_handle_t*, uvc_stream_ctrl_t*, uvc_frame_callback_t*, void* );
void				( *p_uvc_stop_streaming )( uvc_device_handle_t* );
uvc_error_t	( *p_uvc_stream_open_ctrl )( uvc_device_handle_t*, uvc_stream_handle_t**, uvc_stream_ctrl_t* );
uvc_error_t	( *p_uvc_stream_ctrl )( uvc_stream_handle_t*, uvc_stream_ctrl_t* );
uvc_error_t	( *p_uvc_stream_start )( uvc_stream_handle_t*, uvc_frame_callback_t*, void*, uint8_t );
uvc_error_t	( *p_uvc_stream_start_iso )( uvc_stream_handle_t*, uvc_frame_callback_t*, void* );
uvc_error_t	( *p_uvc_stream_get_frame )( uvc_stream_handle_t*, uvc_frame_t **, int32_t  );
uvc_error_t ( *p_uvc_stream_stop )( uvc_stream_handle_t* );
void				( *p_uvc_stream_close )( uvc_stream_handle_t* );
int					( *p_uvc_get_ctrl_len )( uvc_device_handle_t*, uint8_t, uint8_t );
int					( *p_uvc_get_ctrl )( uvc_device_handle_t*, uint8_t, uint8_t, void*, int, enum uvc_req_code );
int					( *p_uvc_set_ctrl )( uvc_device_handle_t*, uint8_t, uint8_t, void*, int );
uvc_error_t	( *p_uvc_get_power_mode )( uvc_device_handle_t*, enum uvc_device_power_mode*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_power_mode )( uvc_device_handle_t*, enum uvc_device_power_mode );
uvc_error_t	( *p_uvc_get_scanning_mode )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_scanning_mode )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_ae_mode )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_ae_mode )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_ae_priority )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_ae_priority )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_exposure_abs )( uvc_device_handle_t*, uint32_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_exposure_abs )( uvc_device_handle_t*, uint32_t );
uvc_error_t	( *p_uvc_get_exposure_rel )( uvc_device_handle_t*, int8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_exposure_rel )( uvc_device_handle_t*, int8_t );
uvc_error_t	( *p_uvc_get_focus_abs )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_focus_abs )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_focus_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_focus_rel )( uvc_device_handle_t*, int8_t, uint8_t );
uvc_error_t	( *p_uvc_get_focus_simple_range )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_focus_simple_range )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_focus_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_focus_auto )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_iris_abs )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_iris_abs )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_iris_rel )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_iris_rel )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_zoom_abs )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_zoom_abs )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_zoom_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_zoom_rel )( uvc_device_handle_t*, int8_t, uint8_t, uint8_t );
uvc_error_t	( *p_uvc_get_pantilt_abs )( uvc_device_handle_t*, int32_t*, int32_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_pantilt_abs )( uvc_device_handle_t*, int32_t, int32_t );
uvc_error_t	( *p_uvc_get_pantilt_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, int8_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_pantilt_rel )( uvc_device_handle_t*, int8_t, uint8_t, int8_t, uint8_t );
uvc_error_t	( *p_uvc_get_roll_abs )( uvc_device_handle_t*, int16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_roll_abs )( uvc_device_handle_t*, int16_t );
uvc_error_t	( *p_uvc_get_roll_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_roll_rel )( uvc_device_handle_t*, int8_t, uint8_t );
uvc_error_t	( *p_uvc_get_privacy )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_privacy )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_digital_window )( uvc_device_handle_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, enum uvc_req_code );
uvc_error_t ( *p_uvc_set_digital_window )( uvc_device_handle_t*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t );
uvc_error_t	( *p_uvc_get_digital_roi )( uvc_device_handle_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_digital_roi )( uvc_device_handle_t*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t );
uvc_error_t	( *p_uvc_get_backlight_compensation )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_backlight_compensation )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_brightness )( uvc_device_handle_t*, int16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_brightness )( uvc_device_handle_t*, int16_t );
uvc_error_t	( *p_uvc_get_contrast )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_contrast )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_contrast_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_contrast_auto )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_gain )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_gain )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_power_line_frequency )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_power_line_frequency )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_hue )( uvc_device_handle_t*, int16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_hue )( uvc_device_handle_t*, int16_t );
uvc_error_t	( *p_uvc_get_hue_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_hue_auto )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_saturation )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_saturation )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_sharpness )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_sharpness )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_gamma )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_gamma )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_white_balance_temperature )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_white_balance_temperature )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_white_balance_temperature_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_white_balance_temperature_auto )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_white_balance_component )( uvc_device_handle_t*, uint16_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_white_balance_component )( uvc_device_handle_t*, uint16_t, uint16_t );
uvc_error_t	( *p_uvc_get_white_balance_component_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_white_balance_component_auto )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_digital_multiplier )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_digital_multiplier )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_digital_multiplier_limit )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_digital_multiplier_limit )( uvc_device_handle_t*, uint16_t );
uvc_error_t	( *p_uvc_get_analog_video_standard )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_analog_video_standard )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_analog_video_lock_status )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_analog_video_lock_status )( uvc_device_handle_t*, uint8_t );
uvc_error_t	( *p_uvc_get_input_select )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
uvc_error_t	( *p_uvc_set_input_select )( uvc_device_handle_t*, uint8_t );
void				( *p_uvc_perror )( uvc_error_t, const char* );
const char*	( *p_uvc_strerror )( uvc_error_t );
void				( *p_uvc_print_diag )( uvc_device_handle_t*, FILE* );
void				( *p_uvc_print_stream_ctrl )( uvc_stream_ctrl_t *, FILE* );
uvc_frame_t* ( *p_uvc_allocate_frame )( size_t );
void				( *p_uvc_free_frame )( uvc_frame_t* );

#if HAVE_LIBDL && !HAVE_STATIC_LIBUVC
static void*    _getDLSym ( void*, const char* );
#endif

int
_uvcInitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL && !HAVE_STATIC_LIBUVC
	static void*		libHandle = 0;

	if ( !libHandle ) {
		if (!( libHandle = dlopen( "libuvc.so.0", RTLD_LAZY ))) {
			return OA_ERR_LIBRARY_NOT_FOUND;
		}
	}

	dlerror();

	if (!( *( void** )( &p_uvc_init ) = _getDLSym ( libHandle,
			"uvc_init" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_exit ) = _getDLSym ( libHandle,
			"uvc_exit" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_device_list ) = _getDLSym ( libHandle,
			"uvc_get_device_list" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_free_device_list ) = _getDLSym ( libHandle,
			"uvc_free_device_list" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_device_descriptor ) = _getDLSym ( libHandle,
			"uvc_get_device_descriptor" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_free_device_descriptor ) = _getDLSym ( libHandle,
			"uvc_free_device_descriptor" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_bus_number ) = _getDLSym ( libHandle,
			"uvc_get_bus_number" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_device_address ) = _getDLSym ( libHandle,
			"uvc_get_device_address" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_find_device ) = _getDLSym ( libHandle,
			"uvc_find_device" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_find_devices ) = _getDLSym ( libHandle,
			"uvc_find_devices" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_open ) = _getDLSym ( libHandle,
			"uvc_open" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_close ) = _getDLSym ( libHandle,
			"uvc_close" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_device ) = _getDLSym ( libHandle,
			"uvc_get_device" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_libusb_handle ) = _getDLSym ( libHandle,
			"uvc_get_libusb_handle" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_ref_device ) = _getDLSym ( libHandle,
			"uvc_ref_device" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_unref_device ) = _getDLSym ( libHandle,
			"uvc_unref_device" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_status_callback ) = _getDLSym ( libHandle,
			"uvc_set_status_callback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_button_callback ) = _getDLSym ( libHandle,
			"uvc_set_button_callback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_camera_terminal ) = _getDLSym ( libHandle,
			"uvc_get_camera_terminal" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_input_terminals ) = _getDLSym ( libHandle,
			"uvc_get_input_terminals" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_output_terminals ) = _getDLSym ( libHandle,
			"uvc_get_output_terminals" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_selector_units ) = _getDLSym ( libHandle,
			"uvc_get_selector_units" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_processing_units ) = _getDLSym ( libHandle,
			"uvc_get_processing_units" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_extension_units ) = _getDLSym ( libHandle,
			"uvc_get_extension_units" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_stream_ctrl_format_size ) =
			_getDLSym ( libHandle, "uvc_get_stream_ctrl_format_size" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_format_descs ) = _getDLSym ( libHandle,
			"uvc_get_format_descs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_probe_stream_ctrl ) = _getDLSym ( libHandle,
			"uvc_probe_stream_ctrl" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_start_streaming ) = _getDLSym ( libHandle,
			"uvc_start_streaming" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_start_iso_streaming ) = _getDLSym ( libHandle,
			"uvc_start_iso_streaming" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stop_streaming ) = _getDLSym ( libHandle,
			"uvc_stop_streaming" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stream_open_ctrl ) = _getDLSym ( libHandle,
			"uvc_stream_open_ctrl" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stream_ctrl ) = _getDLSym ( libHandle,
			"uvc_stream_ctrl" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stream_start ) = _getDLSym ( libHandle,
			"uvc_stream_start" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stream_start_iso ) = _getDLSym ( libHandle,
			"uvc_stream_start_iso" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stream_get_frame ) = _getDLSym ( libHandle,
			"uvc_stream_get_frame" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stream_stop ) = _getDLSym ( libHandle,
			"uvc_stream_stop" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_stream_close ) = _getDLSym ( libHandle,
			"uvc_stream_close" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_ctrl_len ) = _getDLSym ( libHandle,
			"uvc_get_ctrl_len" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_ctrl ) = _getDLSym ( libHandle,
			"uvc_get_ctrl" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_ctrl ) = _getDLSym ( libHandle,
			"uvc_set_ctrl" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_power_mode ) = _getDLSym ( libHandle,
			"uvc_get_power_mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_power_mode ) = _getDLSym ( libHandle,
			"uvc_set_power_mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_scanning_mode ) = _getDLSym ( libHandle,
			"uvc_get_scanning_mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_scanning_mode ) = _getDLSym ( libHandle,
			"uvc_set_scanning_mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_ae_mode ) = _getDLSym ( libHandle,
			"uvc_get_ae_mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_ae_mode ) = _getDLSym ( libHandle,
			"uvc_set_ae_mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_ae_priority ) = _getDLSym ( libHandle,
			"uvc_get_ae_priority" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_ae_priority ) = _getDLSym ( libHandle,
			"uvc_set_ae_priority" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_exposure_abs ) = _getDLSym ( libHandle,
			"uvc_get_exposure_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_exposure_abs ) = _getDLSym ( libHandle,
			"uvc_set_exposure_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_exposure_rel ) = _getDLSym ( libHandle,
			"uvc_get_exposure_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_exposure_rel ) = _getDLSym ( libHandle,
			"uvc_set_exposure_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_focus_abs ) = _getDLSym ( libHandle,
			"uvc_get_focus_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_focus_abs ) = _getDLSym ( libHandle,
			"uvc_set_focus_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_focus_rel ) = _getDLSym ( libHandle,
			"uvc_get_focus_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_focus_rel ) = _getDLSym ( libHandle,
			"uvc_set_focus_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_focus_simple_range ) = _getDLSym ( libHandle,
			"uvc_get_focus_simple_range" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_focus_simple_range ) = _getDLSym ( libHandle,
			"uvc_set_focus_simple_range" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_focus_auto ) = _getDLSym ( libHandle,
			"uvc_get_focus_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_focus_auto ) = _getDLSym ( libHandle,
			"uvc_set_focus_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_iris_abs ) = _getDLSym ( libHandle,
			"uvc_get_iris_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_iris_abs ) = _getDLSym ( libHandle,
			"uvc_set_iris_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_iris_rel ) = _getDLSym ( libHandle,
			"uvc_get_iris_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_iris_rel ) = _getDLSym ( libHandle,
			"uvc_set_iris_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_zoom_abs ) = _getDLSym ( libHandle,
			"uvc_get_zoom_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_zoom_abs ) = _getDLSym ( libHandle,
			"uvc_set_zoom_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_zoom_rel ) = _getDLSym ( libHandle,
			"uvc_get_zoom_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_zoom_rel ) = _getDLSym ( libHandle,
			"uvc_set_zoom_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_pantilt_abs ) = _getDLSym ( libHandle,
			"uvc_get_pantilt_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_pantilt_abs ) = _getDLSym ( libHandle,
			"uvc_set_pantilt_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_pantilt_rel ) = _getDLSym ( libHandle,
			"uvc_get_pantilt_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_pantilt_rel ) = _getDLSym ( libHandle,
			"uvc_set_pantilt_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_roll_abs ) = _getDLSym ( libHandle,
			"uvc_get_roll_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_roll_abs ) = _getDLSym ( libHandle,
			"uvc_set_roll_abs" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_roll_rel ) = _getDLSym ( libHandle,
			"uvc_get_roll_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_roll_rel ) = _getDLSym ( libHandle,
			"uvc_set_roll_rel" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_privacy ) = _getDLSym ( libHandle,
			"uvc_get_privacy" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_privacy ) = _getDLSym ( libHandle,
			"uvc_set_privacy" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_digital_window ) = _getDLSym ( libHandle,
			"uvc_get_digital_window" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_digital_window ) = _getDLSym ( libHandle,
			"uvc_set_digital_window" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_digital_roi ) = _getDLSym ( libHandle,
			"uvc_get_digital_roi" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_digital_roi ) = _getDLSym ( libHandle,
			"uvc_set_digital_roi" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_backlight_compensation ) =
			_getDLSym ( libHandle, "uvc_get_backlight_compensation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_backlight_compensation ) =
			_getDLSym ( libHandle, "uvc_set_backlight_compensation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_brightness ) = _getDLSym ( libHandle,
			"uvc_get_brightness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_brightness ) = _getDLSym ( libHandle,
			"uvc_set_brightness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_contrast ) = _getDLSym ( libHandle,
			"uvc_get_contrast" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_contrast ) = _getDLSym ( libHandle,
			"uvc_set_contrast" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_contrast_auto ) = _getDLSym ( libHandle,
			"uvc_get_contrast_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_contrast_auto ) = _getDLSym ( libHandle,
			"uvc_set_contrast_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_gain ) = _getDLSym ( libHandle,
			"uvc_get_gain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_gain ) = _getDLSym ( libHandle,
			"uvc_set_gain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_power_line_frequency ) =
			_getDLSym ( libHandle, "uvc_get_power_line_frequency" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_power_line_frequency ) =
			_getDLSym ( libHandle, "uvc_set_power_line_frequency" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_hue ) = _getDLSym ( libHandle,
			"uvc_get_hue" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_hue ) = _getDLSym ( libHandle,
			"uvc_set_hue" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_hue_auto ) = _getDLSym ( libHandle,
			"uvc_get_hue_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_hue_auto ) = _getDLSym ( libHandle,
			"uvc_set_hue_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_saturation ) = _getDLSym ( libHandle,
			"uvc_get_saturation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_saturation ) = _getDLSym ( libHandle,
			"uvc_set_saturation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_sharpness ) = _getDLSym ( libHandle,
			"uvc_get_sharpness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_sharpness ) = _getDLSym ( libHandle,
			"uvc_set_sharpness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_gamma ) = _getDLSym ( libHandle,
			"uvc_get_gamma" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_gamma ) = _getDLSym ( libHandle,
			"uvc_set_gamma" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_white_balance_temperature ) =
			_getDLSym ( libHandle, "uvc_get_white_balance_temperature" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_white_balance_temperature ) =
			_getDLSym ( libHandle, "uvc_set_white_balance_temperature" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_white_balance_temperature_auto ) =
			_getDLSym ( libHandle, "uvc_get_white_balance_temperature_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_white_balance_temperature_auto ) =
			_getDLSym ( libHandle, "uvc_set_white_balance_temperature_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_white_balance_component ) =
			_getDLSym ( libHandle, "uvc_get_white_balance_component" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_white_balance_component ) =
			_getDLSym ( libHandle, "uvc_set_white_balance_component" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_white_balance_component_auto ) =
			_getDLSym ( libHandle, "uvc_get_white_balance_component_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_white_balance_component_auto ) =
			_getDLSym ( libHandle, "uvc_set_white_balance_component_auto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_digital_multiplier ) =
			_getDLSym ( libHandle, "uvc_get_digital_multiplier" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_digital_multiplier ) =
			_getDLSym ( libHandle, "uvc_set_digital_multiplier" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_digital_multiplier_limit ) =
			_getDLSym ( libHandle, "uvc_get_digital_multiplier_limit" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_digital_multiplier_limit ) =
			_getDLSym ( libHandle, "uvc_set_digital_multiplier_limit" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_analog_video_standard ) =
			_getDLSym ( libHandle, "uvc_get_analog_video_standard" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_analog_video_standard ) =
			_getDLSym ( libHandle, "uvc_set_analog_video_standard" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_analog_video_lock_status ) =
			_getDLSym ( libHandle, "uvc_get_analog_video_lock_status" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_analog_video_lock_status ) =
			_getDLSym ( libHandle, "uvc_set_analog_video_lock_status" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_get_input_select ) = _getDLSym ( libHandle,
			"uvc_get_input_select" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_set_input_select ) = _getDLSym ( libHandle,
			"uvc_set_input_select" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_perror ) = _getDLSym ( libHandle,
			"uvc_perror" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_strerror ) = _getDLSym ( libHandle,
			"uvc_strerror" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_print_diag ) = _getDLSym ( libHandle,
			"uvc_print_diag" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_print_stream_ctrl ) = _getDLSym ( libHandle,
			"uvc_print_stream_ctrl" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_allocate_frame ) = _getDLSym ( libHandle,
			"uvc_allocate_frame" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}

	if (!( *( void** )( &p_uvc_free_frame ) = _getDLSym ( libHandle,
			"uvc_free_frame" ))) {
		dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
	}


#else
#if HAVE_STATIC_LIBUVC

	p_uvc_init = uvc_init;
	p_uvc_exit = uvc_exit;
	p_uvc_get_device_list = uvc_get_device_list;
	p_uvc_free_device_list = uvc_free_device_list;
	p_uvc_get_device_descriptor = uvc_get_device_descriptor;
	p_uvc_free_device_descriptor = uvc_free_device_descriptor;
	p_uvc_get_bus_number = uvc_get_bus_number;
	p_uvc_get_device_address = uvc_get_device_address;
	p_uvc_find_device = uvc_find_device;
	p_uvc_find_devices = uvc_find_devices;
	p_uvc_open = uvc_open;
	p_uvc_close = uvc_close;
	p_uvc_get_device = uvc_get_device;
	p_uvc_get_libusb_handle = uvc_get_libusb_handle;
	p_uvc_ref_device = uvc_ref_device;
	p_uvc_unref_device = uvc_unref_device;
	p_uvc_set_status_callback = uvc_set_status_callback;
	p_uvc_set_button_callback = uvc_set_button_callback;
	p_uvc_get_camera_terminal = uvc_get_camera_terminal;
	p_uvc_get_input_terminals = uvc_get_input_terminals;
	p_uvc_get_output_terminals = uvc_get_output_terminals;
	p_uvc_get_selector_units = uvc_get_selector_units;
	p_uvc_get_processing_units = uvc_get_processing_units;
	p_uvc_get_extension_units = uvc_get_extension_units;
	p_uvc_get_stream_ctrl_format_size = uvc_get_stream_ctrl_format_size;
	p_uvc_get_format_descs = uvc_get_format_descs;
	p_uvc_probe_stream_ctrl = uvc_probe_stream_ctrl;
	p_uvc_start_streaming = uvc_start_streaming;
	p_uvc_start_iso_streaming = uvc_start_iso_streaming;
	p_uvc_stop_streaming = uvc_stop_streaming;
	p_uvc_stream_open_ctrl = uvc_stream_open_ctrl;
	p_uvc_stream_ctrl = uvc_stream_ctrl;
	p_uvc_stream_start = uvc_stream_start;
	p_uvc_stream_start_iso = uvc_stream_start_iso;
	p_uvc_stream_get_frame = uvc_stream_get_frame;
	p_uvc_stream_stop = uvc_stream_stop;
	p_uvc_stream_close = uvc_stream_close;
	p_uvc_get_ctrl_len = uvc_get_ctrl_len;
	p_uvc_get_ctrl = uvc_get_ctrl;
	p_uvc_set_ctrl = uvc_set_ctrl;
	p_uvc_get_power_mode = uvc_get_power_mode;
	p_uvc_set_power_mode = uvc_set_power_mode;
	p_uvc_get_scanning_mode = uvc_get_scanning_mode;
	p_uvc_set_scanning_mode = uvc_set_scanning_mode;
	p_uvc_get_ae_mode = uvc_get_ae_mode;
	p_uvc_set_ae_mode = uvc_set_ae_mode;
	p_uvc_get_ae_priority = uvc_get_ae_priority;
	p_uvc_set_ae_priority = uvc_set_ae_priority;
	p_uvc_get_exposure_abs = uvc_get_exposure_abs;
	p_uvc_set_exposure_abs = uvc_set_exposure_abs;
	p_uvc_get_exposure_rel = uvc_get_exposure_rel;
	p_uvc_set_exposure_rel = uvc_set_exposure_rel;
	p_uvc_get_focus_abs = uvc_get_focus_abs;
	p_uvc_set_focus_abs = uvc_set_focus_abs;
	p_uvc_get_focus_rel = uvc_get_focus_rel;
	p_uvc_set_focus_rel = uvc_set_focus_rel;
	p_uvc_get_focus_simple_range = uvc_get_focus_simple_range;
	p_uvc_set_focus_simple_range = uvc_set_focus_simple_range;
	p_uvc_get_focus_auto = uvc_get_focus_auto;
	p_uvc_set_focus_auto = uvc_set_focus_auto;
	p_uvc_get_iris_abs = uvc_get_iris_abs;
	p_uvc_set_iris_abs = uvc_set_iris_abs;
	p_uvc_get_iris_rel = uvc_get_iris_rel;
	p_uvc_set_iris_rel = uvc_set_iris_rel;
	p_uvc_get_zoom_abs = uvc_get_zoom_abs;
	p_uvc_set_zoom_abs = uvc_set_zoom_abs;
	p_uvc_get_zoom_rel = uvc_get_zoom_rel;
	p_uvc_set_zoom_rel = uvc_set_zoom_rel;
	p_uvc_get_pantilt_abs = uvc_get_pantilt_abs;
	p_uvc_set_pantilt_abs = uvc_set_pantilt_abs;
	p_uvc_get_pantilt_rel = uvc_get_pantilt_rel;
	p_uvc_set_pantilt_rel = uvc_set_pantilt_rel;
	p_uvc_get_roll_abs = uvc_get_roll_abs;
	p_uvc_set_roll_abs = uvc_set_roll_abs;
	p_uvc_get_roll_rel = uvc_get_roll_rel;
	p_uvc_set_roll_rel = uvc_set_roll_rel;
	p_uvc_get_privacy = uvc_get_privacy;
	p_uvc_set_privacy = uvc_set_privacy;
	p_uvc_get_digital_window = uvc_get_digital_window;
	p_uvc_set_digital_window = uvc_set_digital_window;
	p_uvc_get_digital_roi = uvc_get_digital_roi;
	p_uvc_set_digital_roi = uvc_set_digital_roi;
	p_uvc_get_backlight_compensation = uvc_get_backlight_compensation;
	p_uvc_set_backlight_compensation = uvc_set_backlight_compensation;
	p_uvc_get_brightness = uvc_get_brightness;
	p_uvc_set_brightness = uvc_set_brightness;
	p_uvc_get_contrast = uvc_get_contrast;
	p_uvc_set_contrast = uvc_set_contrast;
	p_uvc_get_contrast_auto = uvc_get_contrast_auto;
	p_uvc_set_contrast_auto = uvc_set_contrast_auto;
	p_uvc_get_gain = uvc_get_gain;
	p_uvc_set_gain = uvc_set_gain;
	p_uvc_get_power_line_frequency = uvc_get_power_line_frequency;
	p_uvc_set_power_line_frequency = uvc_set_power_line_frequency;
	p_uvc_get_hue = uvc_get_hue;
	p_uvc_set_hue = uvc_set_hue;
	p_uvc_get_hue_auto = uvc_get_hue_auto;
	p_uvc_set_hue_auto = uvc_set_hue_auto;
	p_uvc_get_saturation = uvc_get_saturation;
	p_uvc_set_saturation = uvc_set_saturation;
	p_uvc_get_sharpness = uvc_get_sharpness;
	p_uvc_set_sharpness = uvc_set_sharpness;
	p_uvc_get_gamma = uvc_get_gamma;
	p_uvc_set_gamma = uvc_set_gamma;
	p_uvc_get_white_balance_temperature = uvc_get_white_balance_temperature;
	p_uvc_set_white_balance_temperature = uvc_set_white_balance_temperature;
	p_uvc_get_white_balance_temperature_auto =
			uvc_get_white_balance_temperature_auto;
	p_uvc_set_white_balance_temperature_auto =
			uvc_set_white_balance_temperature_auto;
	p_uvc_get_white_balance_component = uvc_get_white_balance_component;
	p_uvc_set_white_balance_component = uvc_set_white_balance_component;
	p_uvc_get_white_balance_component_auto = uvc_get_white_balance_component_auto;
	p_uvc_set_white_balance_component_auto = uvc_set_white_balance_component_auto;
	p_uvc_get_digital_multiplier = uvc_get_digital_multiplier;
	p_uvc_set_digital_multiplier = uvc_set_digital_multiplier;
	p_uvc_get_digital_multiplier_limit = uvc_get_digital_multiplier_limit;
	p_uvc_set_digital_multiplier_limit = uvc_set_digital_multiplier_limit;
	p_uvc_get_analog_video_standard = uvc_get_analog_video_standard;
	p_uvc_set_analog_video_standard = uvc_set_analog_video_standard;
	p_uvc_get_analog_video_lock_status = uvc_get_analog_video_lock_status;
	p_uvc_set_analog_video_lock_status = uvc_set_analog_video_lock_status;
	p_uvc_get_input_select = uvc_get_input_select;
	p_uvc_set_input_select = uvc_set_input_select;
	p_uvc_perror = uvc_perror;
	p_uvc_strerror = uvc_strerror;
	p_uvc_print_diag = uvc_print_diag;
	p_uvc_print_stream_ctrl = uvc_print_stream_ctrl;
	p_uvc_allocate_frame = uvc_allocate_frame;
	p_uvc_free_frame = uvc_free_frame;

#else
	return OA_ERR_LIBRARY_NOT_FOUND;
#endif	/* HAVE_STATIC_LIBUVC */
#endif	/* HAVE_LIBDL && !HAVE_STATIC_LIBUVC */
	return OA_ERR_NONE;
}

#if HAVE_LIBDL && !HAVE_STATIC_LIBUVC
static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libuvc DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif

#endif	/* HAVE_LIBUVC */
