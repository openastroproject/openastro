/*****************************************************************************
 *
 * UVCprivate.h -- private header for UVC camera API
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

#ifndef OA_UVC_PRIVATE_H
#define OA_UVC_PRIVATE_H

#include <libuvc/libuvc.h>

extern int					_uvcInitLibraryFunctionPointers ( void );

extern uvc_error_t	( *p_uvc_init )( uvc_context_t**, struct libusb_context * );
extern void				( *p_uvc_exit )( uvc_context_t* );
extern uvc_error_t ( *p_uvc_get_device_list )( uvc_context_t*, uvc_device_t*** );
extern void				( *p_uvc_free_device_list )( uvc_device_t **, uint8_t );
extern uvc_error_t ( *p_uvc_get_device_descriptor )( uvc_device_t*, uvc_device_descriptor_t** );
extern void				( *p_uvc_free_device_descriptor )( uvc_device_descriptor_t* );
extern uint8_t			( *p_uvc_get_bus_number )( uvc_device_t* );
extern uint8_t			( *p_uvc_get_device_address )( uvc_device_t* );
extern uvc_error_t	( *p_uvc_find_device )( uvc_context_t*, uvc_device_t**, int, int, const char* );
extern uvc_error_t	( *p_uvc_find_devices )( uvc_context_t*, uvc_device_t***, int, int, const char* );
extern uvc_error_t	( *p_uvc_open )( uvc_device_t*, uvc_device_handle_t** );
extern void				( *p_uvc_close )( uvc_device_handle_t* );
extern uvc_device_t* ( *p_uvc_get_device )( uvc_device_handle_t* );
extern struct libusb_device_handle* ( *p_uvc_get_libusb_handle )(uvc_device_handle_t* );
extern void				( *p_uvc_ref_device )( uvc_device_t* );
extern void				( *p_uvc_unref_device )(uvc_device_t* );
extern void				( *p_uvc_set_status_callback )( uvc_device_handle_t*, uvc_status_callback_t, void* );
extern void				( *p_uvc_set_button_callback )( uvc_device_handle_t*, uvc_button_callback_t, void* );
extern const uvc_input_terminal_t* ( *p_uvc_get_camera_terminal )( uvc_device_handle_t* );
extern const uvc_input_terminal_t* ( *p_uvc_get_input_terminals )( uvc_device_handle_t* );
extern const uvc_output_terminal_t* ( *p_uvc_get_output_terminals )( uvc_device_handle_t* );
extern const uvc_selector_unit_t* ( *p_uvc_get_selector_units )( uvc_device_handle_t* );
extern const uvc_processing_unit_t* ( *p_uvc_get_processing_units )( uvc_device_handle_t* );
extern const uvc_extension_unit_t* ( *p_uvc_get_extension_units )( uvc_device_handle_t* );
extern uvc_error_t	( *p_uvc_get_stream_ctrl_format_size )( uvc_device_handle_t*, uvc_stream_ctrl_t*, enum uvc_frame_format, int, int, int );
extern const uvc_format_desc_t* ( *p_uvc_get_format_descs )( uvc_device_handle_t* );
extern uvc_error_t	( *p_uvc_probe_stream_ctrl )( uvc_device_handle_t*, uvc_stream_ctrl_t* );
extern uvc_error_t	( *p_uvc_start_streaming )( uvc_device_handle_t*, uvc_stream_ctrl_t*, uvc_frame_callback_t*, void*, uint8_t );
extern uvc_error_t ( *p_uvc_start_iso_streaming )( uvc_device_handle_t*, uvc_stream_ctrl_t*, uvc_frame_callback_t*, void* );
extern void				( *p_uvc_stop_streaming )( uvc_device_handle_t* );
extern uvc_error_t	( *p_uvc_stream_open_ctrl )( uvc_device_handle_t*, uvc_stream_handle_t**, uvc_stream_ctrl_t* );
extern uvc_error_t	( *p_uvc_stream_ctrl )( uvc_stream_handle_t*, uvc_stream_ctrl_t* );
extern uvc_error_t	( *p_uvc_stream_start )( uvc_stream_handle_t*, uvc_frame_callback_t*, void*, uint8_t );
extern uvc_error_t	( *p_uvc_stream_start_iso )( uvc_stream_handle_t*, uvc_frame_callback_t*, void* );
extern uvc_error_t	( *p_uvc_stream_get_frame )( uvc_stream_handle_t*, uvc_frame_t **, int32_t  );
extern uvc_error_t ( *p_uvc_stream_stop )( uvc_stream_handle_t* );
extern void				( *p_uvc_stream_close )( uvc_stream_handle_t* );
extern int					( *p_uvc_get_ctrl_len )( uvc_device_handle_t*, uint8_t, uint8_t );
extern int					( *p_uvc_get_ctrl )( uvc_device_handle_t*, uint8_t, uint8_t, void*, int, enum uvc_req_code );
extern int					( *p_uvc_set_ctrl )( uvc_device_handle_t*, uint8_t, uint8_t, void*, int );
extern uvc_error_t	( *p_uvc_get_power_mode )( uvc_device_handle_t*, enum uvc_device_power_mode*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_power_mode )( uvc_device_handle_t*, enum uvc_device_power_mode );
extern uvc_error_t	( *p_uvc_get_scanning_mode )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_scanning_mode )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_ae_mode )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_ae_mode )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_ae_priority )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_ae_priority )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_exposure_abs )( uvc_device_handle_t*, uint32_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_exposure_abs )( uvc_device_handle_t*, uint32_t );
extern uvc_error_t	( *p_uvc_get_exposure_rel )( uvc_device_handle_t*, int8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_exposure_rel )( uvc_device_handle_t*, int8_t );
extern uvc_error_t	( *p_uvc_get_focus_abs )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_focus_abs )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_focus_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_focus_rel )( uvc_device_handle_t*, int8_t, uint8_t );
extern uvc_error_t	( *p_uvc_get_focus_simple_range )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_focus_simple_range )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_focus_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_focus_auto )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_iris_abs )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_iris_abs )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_iris_rel )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_iris_rel )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_zoom_abs )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_zoom_abs )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_zoom_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_zoom_rel )( uvc_device_handle_t*, int8_t, uint8_t, uint8_t );
extern uvc_error_t	( *p_uvc_get_pantilt_abs )( uvc_device_handle_t*, int32_t*, int32_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_pantilt_abs )( uvc_device_handle_t*, int32_t, int32_t );
extern uvc_error_t	( *p_uvc_get_pantilt_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, int8_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_pantilt_rel )( uvc_device_handle_t*, int8_t, uint8_t, int8_t, uint8_t );
extern uvc_error_t	( *p_uvc_get_roll_abs )( uvc_device_handle_t*, int16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_roll_abs )( uvc_device_handle_t*, int16_t );
extern uvc_error_t	( *p_uvc_get_roll_rel )( uvc_device_handle_t*, int8_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_roll_rel )( uvc_device_handle_t*, int8_t, uint8_t );
extern uvc_error_t	( *p_uvc_get_privacy )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_privacy )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_digital_window )( uvc_device_handle_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t ( *p_uvc_set_digital_window )( uvc_device_handle_t*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t );
extern uvc_error_t	( *p_uvc_get_digital_roi )( uvc_device_handle_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_digital_roi )( uvc_device_handle_t*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t );
extern uvc_error_t	( *p_uvc_get_backlight_compensation )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_backlight_compensation )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_brightness )( uvc_device_handle_t*, int16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_brightness )( uvc_device_handle_t*, int16_t );
extern uvc_error_t	( *p_uvc_get_contrast )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_contrast )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_contrast_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_contrast_auto )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_gain )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_gain )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_power_line_frequency )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_power_line_frequency )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_hue )( uvc_device_handle_t*, int16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_hue )( uvc_device_handle_t*, int16_t );
extern uvc_error_t	( *p_uvc_get_hue_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_hue_auto )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_saturation )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_saturation )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_sharpness )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_sharpness )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_gamma )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_gamma )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_white_balance_temperature )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_white_balance_temperature )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_white_balance_temperature_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_white_balance_temperature_auto )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_white_balance_component )( uvc_device_handle_t*, uint16_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_white_balance_component )( uvc_device_handle_t*, uint16_t, uint16_t );
extern uvc_error_t	( *p_uvc_get_white_balance_component_auto )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_white_balance_component_auto )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_digital_multiplier )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_digital_multiplier )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_digital_multiplier_limit )( uvc_device_handle_t*, uint16_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_digital_multiplier_limit )( uvc_device_handle_t*, uint16_t );
extern uvc_error_t	( *p_uvc_get_analog_video_standard )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_analog_video_standard )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_analog_video_lock_status )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_analog_video_lock_status )( uvc_device_handle_t*, uint8_t );
extern uvc_error_t	( *p_uvc_get_input_select )( uvc_device_handle_t*, uint8_t*, enum uvc_req_code );
extern uvc_error_t	( *p_uvc_set_input_select )( uvc_device_handle_t*, uint8_t );
extern void				( *p_uvc_perror )( uvc_error_t, const char* );
extern const char*	( *p_uvc_strerror )( uvc_error_t );
extern void				( *p_uvc_print_diag )( uvc_device_handle_t*, FILE* );
extern void				( *p_uvc_print_stream_ctrl )( uvc_stream_ctrl_t *, FILE* );
extern uvc_frame_t* ( *p_uvc_allocate_frame )( size_t );
extern void				( *p_uvc_free_frame )( uvc_frame_t* );


#endif	/* OA_UVC_PRIVATE_H */
