/*****************************************************************************
 *
 * logging.c
 *
 * example program to demonstrate logging
 *
 * Copyright 2021
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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sys/types.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/errno.h>


int
main()
{
	oaCameraDevice**		cameraDevs;

	// see include/openastro/util.h for other logging levels
	oaSetLogLevel ( OA_LOG_INFO );

	// required for logging specific to cameras
	oaSetLogType ( OA_LOG_CAMERA );

	// Get list of connected cameras.  Don't filter on any specific feature
	( void ) oaGetCameras ( &cameraDevs, OA_CAM_FEATURE_NONE );

	// Release camera list
	oaReleaseCameras ( cameraDevs );

	return 0;
}
