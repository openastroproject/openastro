/*****************************************************************************
 *
 * GP2common.c -- private functions for libgphoto2 Cameras
 *
 * Copyright 2019
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

#include <openastro/camera.h>
#include <gphoto2/gphoto2-camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "GP2oacam.h"
#include "GP2private.h"


CameraAbilitiesList		*_gp2Abilities = 0;
GPPortInfoList				*_gp2PortInfoList = 0;
int										numPorts = 0;


int
_gp2OpenCamera ( Camera** camera, const char* name, const char* port,
		GPContext* ctx )
{
	CameraAbilities			abilities;
	GPPortInfo					portInfo;
	int									modelIndex, portIndex;

	if ( p_gp_camera_new ( camera ) != GP_OK ) {
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( !_gp2Abilities ) {
		if ( p_gp_abilities_list_new ( &_gp2Abilities ) != GP_OK ) {
			p_gp_camera_unref ( *camera );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( p_gp_abilities_list_load ( _gp2Abilities, ctx ) != GP_OK ) {
			p_gp_camera_unref ( *camera );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

	if ( !_gp2PortInfoList ) {
		if ( p_gp_port_info_list_new ( &_gp2PortInfoList ) != GP_OK ) {
			p_gp_camera_unref ( *camera );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( p_gp_port_info_list_load ( _gp2PortInfoList ) != GP_OK ) {
			p_gp_port_info_list_free ( _gp2PortInfoList );
			p_gp_camera_unref ( *camera );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if (( numPorts = p_gp_port_info_list_count ( _gp2PortInfoList )) < 0 ) {
			p_gp_port_info_list_free ( _gp2PortInfoList );
			p_gp_camera_unref ( *camera );
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

	if (( modelIndex = p_gp_abilities_list_lookup_model ( _gp2Abilities,
			name )) < GP_OK ) {
		p_gp_port_info_list_free ( _gp2PortInfoList );
		p_gp_camera_unref ( *camera );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( p_gp_abilities_list_get_abilities ( _gp2Abilities, modelIndex,
			&abilities ) != GP_OK ) {
		p_gp_port_info_list_free ( _gp2PortInfoList );
		p_gp_camera_unref ( *camera );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( p_gp_camera_set_abilities ( *camera, abilities ) != GP_OK ) {
		p_gp_port_info_list_free ( _gp2PortInfoList );
		p_gp_camera_unref ( *camera );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if (( portIndex = p_gp_port_info_list_lookup_path ( _gp2PortInfoList,
			port )) < 0 ) {
		if ( portIndex == GP_ERROR_UNKNOWN_PORT ) {
			fprintf ( stderr, "Unrecognised port '%s'\n", port );
		}
		p_gp_port_info_list_free ( _gp2PortInfoList );
		p_gp_camera_unref ( *camera );
		return -OA_ERR_SYSTEM_ERROR;
	}

  if ( p_gp_port_info_list_get_info ( _gp2PortInfoList, portIndex,
			&portInfo ) != GP_OK ) {
		p_gp_port_info_list_free ( _gp2PortInfoList );
		p_gp_camera_unref ( *camera );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( p_gp_camera_set_port_info ( *camera, portInfo ) != GP_OK ) {
		p_gp_port_info_list_free ( _gp2PortInfoList );
		p_gp_camera_unref ( *camera );
		return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}


int
_gp2CloseCamera ( Camera* camera, GPContext* ctx )
{
	return ( p_gp_camera_exit ( camera, ctx ) == GP_OK ) ? OA_ERR_NONE :
		-OA_ERR_SYSTEM_ERROR;
}


int
_gp2GetConfig ( Camera* camera, CameraWidget** widget, GPContext* ctx )
{
	return ( p_gp_camera_get_config ( camera, widget, ctx ) == GP_OK ) ?
		OA_ERR_NONE : -OA_ERR_SYSTEM_ERROR;
}


int
_gp2FindWidget ( CameraWidget* root, const char* name, CameraWidget** result )
{
	int		ret;

	if (( ret = p_gp_widget_get_child_by_name ( root, name, result )) != GP_OK ) {
		( void ) p_gp_widget_get_child_by_label ( root, name, result );
	}

	return ( ret == GP_OK ) ? OA_ERR_NONE : -OA_ERR_SYSTEM_ERROR;
}


int
_gp2GetWidgetType ( CameraWidget* widget, CameraWidgetType* type )
{
	return ( p_gp_widget_get_type ( widget, type ) == GP_OK ) ? OA_ERR_NONE :
			-OA_ERR_SYSTEM_ERROR;
}
