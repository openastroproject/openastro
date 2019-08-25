/*****************************************************************************
 *
 * GP2private.h -- private header for libgphoto2 camera API
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

#ifndef OA_GP2_PRIVATE_H
#define OA_GP2_PRIVATE_H


extern int					_gp2InitLibraryFunctionPointers ( void );

extern int					_gp2OpenCamera ( Camera**, const char*, const char*,
												GPContext* );
extern int					_gp2CloseCamera ( Camera*, GPContext* );

extern int					_gp2GetConfig ( Camera*, CameraWidget**, GPContext* );
extern int					_gp2FindWidget ( CameraWidget*, const char*,
												CameraWidget** );
extern int					_gp2GetWidgetType ( CameraWidget*, CameraWidgetType* );
extern void					_gp2ConfigureCallbacks ( GPContext* );

extern GPContext*		( *p_gp_context_new )( void );
extern void					( *p_gp_context_unref )( GPContext* );

extern int					( *p_gp_list_new )( CameraList** );
extern int					( *p_gp_list_reset )( CameraList* );
extern int					( *p_gp_list_free )( CameraList* );
extern int					( *p_gp_list_unref )( CameraList* );
extern int					( *p_gp_list_count )( CameraList* );
extern int					( *p_gp_list_get_name )( CameraList*, int, const char** );
extern int					( *p_gp_list_get_value )( CameraList*, int, const char** );

extern int					( *p_gp_camera_autodetect )( CameraList*, GPContext* );
extern int					( *p_gp_camera_new )( Camera** );
extern int					( *p_gp_camera_set_abilities )( Camera*, CameraAbilities );
extern int					( *p_gp_camera_set_port_info )( Camera*, GPPortInfo );
extern int					( *p_gp_camera_unref )( Camera* );
extern int					( *p_gp_camera_init )( Camera*, GPContext* );
extern int					( *p_gp_camera_exit )( Camera*, GPContext* );
extern int					( *p_gp_camera_get_config )( Camera*, CameraWidget**,
												GPContext* );
extern int					( *p_gp_camera_set_config )( Camera*, CameraWidget*,
												GPContext* );
extern int					( *p_gp_camera_trigger_capture )( Camera*, GPContext* );
extern int					( *p_gp_camera_wait_for_event )( Camera*, int,
												CameraEventType*, void**, GPContext* );
extern int					( *p_gp_camera_file_get )( Camera*, const char*,
												const char*, CameraFileType, CameraFile*, GPContext* );

extern void					( *p_gp_context_set_error_func )( GPContext*,
												GPContextErrorFunc, void* );
extern void					( *p_gp_context_set_status_func )( GPContext*,
												GPContextStatusFunc, void* );
extern void					( *p_gp_context_set_cancel_func )( GPContext*,
												GPContextCancelFunc, void* );
extern void					( *p_gp_context_set_message_func )( GPContext*,
												GPContextMessageFunc, void* );

extern int					( *p_gp_abilities_list_get_abilities )(
												CameraAbilitiesList*, int, CameraAbilities* );
extern int					( *p_gp_abilities_list_load )( CameraAbilitiesList*,
												GPContext* );
extern int					( *p_gp_abilities_list_lookup_model )( CameraAbilitiesList*,
												const char* );
extern int					( *p_gp_abilities_list_new )( CameraAbilitiesList** );

extern int					( *p_gp_widget_get_child_by_name )( CameraWidget*,
												const char*, CameraWidget** );
extern int					( *p_gp_widget_get_child_by_label )( CameraWidget*,
												const char*, CameraWidget** );
extern int					( *p_gp_widget_get_name )( CameraWidget*, const char** );
extern int					( *p_gp_widget_get_type )( CameraWidget*,
												CameraWidgetType* );
extern int					( *p_gp_widget_get_value )( CameraWidget*, void* );
extern int					( *p_gp_widget_count_choices )( CameraWidget* );
extern int					( *p_gp_widget_get_choice )( CameraWidget*, int,
												const char** );
extern int					( *p_gp_widget_set_value )( CameraWidget*, const void* );

extern int					( *p_gp_port_info_list_count )( GPPortInfoList* );
extern int					( *p_gp_port_info_list_free )( GPPortInfoList* );
extern int					( *p_gp_port_info_list_get_info )( GPPortInfoList*, int,
												GPPortInfo* );
extern int					( *p_gp_port_info_list_load )( GPPortInfoList* );
extern int					( *p_gp_port_info_list_lookup_path )( GPPortInfoList*,
												const char* );
extern int					( *p_gp_port_info_list_new )( GPPortInfoList** );

extern int					( *p_gp_log_add_func )( GPLogLevel, GPLogFunc, void* );

extern int					( *p_gp_file_new )( CameraFile** );
extern int					( *p_gp_file_free )( CameraFile* );
extern int					( *p_gp_file_get_data_and_size )( CameraFile*,
												const char**, unsigned long* );
extern int					( *p_gp_file_get_mime_type )( CameraFile*, const char** );

#define	CAMERA_MANUF_UNKNOWN		0
#define	CAMERA_MANUF_CANON			1
#define	CAMERA_MANUF_NIKON			2
#define	CAMERA_MANUF_SONY				3

#endif	/* OA_GP2_PRIVATE_H */
