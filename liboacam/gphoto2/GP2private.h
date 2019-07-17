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

extern GPContext*		( *p_gp_context_new )( void );
extern int					( *p_gp_context_unref )( GPContext* );

extern int					( *p_gp_list_new )( CameraList** );
extern int					( *p_gp_list_reset )( CameraList* );
extern int					( *p_gp_list_free )( CameraList* );
extern int					( *p_gp_list_count )( CameraList* );
extern int					( *p_gp_list_get_name )( CameraList*, int, const char** );
extern int					( *p_gp_list_get_value )( CameraList*, int, const char** );

extern int					( *p_gp_camera_autodetect )( CameraList*, GPContext* );

extern int					( *p_gp_context_set_error_func )( GPContext*, void*,
												void* );
extern int					( *p_gp_context_set_status_func )( GPContext*, void*,
												void* );
extern int					( *p_gp_context_set_cancel_func )( GPContext*, void*,
												void* );
extern int					( *p_gp_context_set_message_func )( GPContext*, void*,
												void* );

#endif	/* OA_GP2_PRIVATE_H */
