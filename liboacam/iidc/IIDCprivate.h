/*****************************************************************************
 *
 * IIDCprivate.h -- internal header for IIDC camera interface
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

#ifndef OA_IIDC_PRIVATE_H
#define OA_IIDC_PRIVATE_H

extern int							_iidcInitLibraryFunctionPointers ( void );

extern dc1394error_t		( *p_dc1394_camera_enumerate )( dc1394_t*,
														dc1394camera_list_t ** );
extern void							( *p_dc1394_camera_free )( dc1394camera_t* );
extern void							( *p_dc1394_camera_free_list )( dc1394camera_list_t* );
extern dc1394error_t		( *p_dc1394_camera_get_broadcast )( dc1394camera_t*,
														dc1394bool_t* );
extern dc1394camera_t*	( *p_dc1394_camera_new_unit )( dc1394_t*, uint64_t,
														int );
extern dc1394error_t		( *p_dc1394_camera_reset )( dc1394camera_t* );
extern dc1394error_t		( *p_dc1394_capture_dequeue )( dc1394camera_t*,
														dc1394capture_policy_t, dc1394video_frame_t** );
extern dc1394error_t		( *p_dc1394_capture_enqueue )( dc1394camera_t*,
														dc1394video_frame_t* );
extern dc1394error_t		( *p_dc1394_capture_setup )( dc1394camera_t*, uint32_t,
														uint32_t );
extern dc1394error_t		( *p_dc1394_capture_stop )( dc1394camera_t* );
extern dc1394error_t		( *p_dc1394_external_trigger_set_mode )(
														dc1394camera_t*, dc1394trigger_mode_t );
extern dc1394error_t		( *p_dc1394_external_trigger_set_polarity )(
														dc1394camera_t*, dc1394trigger_polarity_t );
extern dc1394error_t		( *p_dc1394_external_trigger_set_power )(
														dc1394camera_t*, dc1394switch_t );
extern dc1394error_t		( *p_dc1394_feature_get_absolute_value )(
														dc1394camera_t*, dc1394feature_t, float* );
extern dc1394error_t		( *p_dc1394_feature_get_all )( dc1394camera_t*,
														dc1394featureset_t* );
extern dc1394error_t		( *p_dc1394_feature_get_mode )( dc1394camera_t*,
														dc1394feature_t, dc1394feature_mode_t* );
extern dc1394error_t		( *p_dc1394_feature_get_value )( dc1394camera_t*,
														dc1394feature_t, uint32_t* );
extern dc1394error_t		( *p_dc1394_feature_set_absolute_control )(
														dc1394camera_t*, dc1394feature_t, dc1394switch_t );
extern dc1394error_t		( *p_dc1394_feature_set_absolute_value )(
														dc1394camera_t*, dc1394feature_t, float );
extern dc1394error_t		( *p_dc1394_feature_set_mode )( dc1394camera_t*,
														dc1394feature_t, dc1394feature_mode_t );
extern dc1394error_t		( *p_dc1394_feature_set_power )( dc1394camera_t*,
														dc1394feature_t, dc1394switch_t );
extern dc1394error_t		( *p_dc1394_feature_set_value )( dc1394camera_t*,
														dc1394feature_t, uint32_t );
extern dc1394error_t		( *p_dc1394_feature_temperature_get_value )(
														dc1394camera_t*, uint32_t*, uint32_t* );
extern dc1394error_t		( *p_dc1394_feature_temperature_set_value )(
														dc1394camera_t*, uint32_t );
extern dc1394error_t		( *p_dc1394_feature_whitebalance_get_value )(
														dc1394camera_t*, uint32_t*, uint32_t* );
extern dc1394error_t		( *p_dc1394_feature_whitebalance_set_value )(
														dc1394camera_t*, uint32_t, uint32_t );
extern dc1394error_t		( *p_dc1394_format7_get_modeset )( dc1394camera_t*,
														dc1394format7modeset_t* );
extern dc1394error_t		( *p_dc1394_format7_set_roi )( dc1394camera_t*,
														dc1394video_mode_t, dc1394color_coding_t, int32_t,
														int32_t, int32_t, int32_t, int32_t );
extern void							( *p_dc1394_free  )( dc1394_t* );
extern dc1394error_t		( *p_dc1394_get_color_coding_from_video_mode )(
														dc1394camera_t*, dc1394video_mode_t,
														dc1394color_coding_t* );
extern dc1394error_t		( *p_dc1394_get_image_size_from_video_mode )(
														dc1394camera_t*, dc1394video_mode_t, uint32_t*,
														uint32_t* );
extern dc1394_t*				( *p_dc1394_new )( void );
extern dc1394error_t		( *p_dc1394_video_get_supported_framerates )(
														dc1394camera_t*, dc1394video_mode_t,
														dc1394framerates_t* );
extern dc1394error_t		( *p_dc1394_video_get_supported_modes )(
														dc1394camera_t*, dc1394video_modes_t* );
extern dc1394error_t		( *p_dc1394_video_set_framerate )( dc1394camera_t*,
														dc1394framerate_t );
extern dc1394error_t		( *p_dc1394_video_set_iso_speed )( dc1394camera_t*,
														dc1394speed_t );
extern dc1394error_t		( *p_dc1394_video_set_mode )( dc1394camera_t*,
														dc1394video_mode_t );
extern dc1394error_t		( *p_dc1394_video_set_operation_mode )( dc1394camera_t*,
														dc1394operation_mode_t );
extern dc1394error_t		( *p_dc1394_video_set_transmission )( dc1394camera_t*,
														dc1394switch_t );

#endif	/* OA_IIDC_PRIVATE_H */
