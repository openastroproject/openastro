/*****************************************************************************
 *
 * showControls.c
 *
 * example program to list the available controls for connected cameras
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
	int									numCameras, controlType;
	int									c, i;
	oaCamera*						cameraCtx = NULL;
	const char*					typeString;

	// Get list of connected cameras.  Don't filter on any specific feature
	numCameras = oaGetCameras ( &cameraDevs, OA_CAM_FEATURE_NONE );

	if ( numCameras > 0 ) {
		for ( i = 0; i < numCameras; i++ ) {
			printf ( "found camera %s via %s interface\n", cameraDevs[i]->deviceName,
					oaCameraInterfaces[ cameraDevs[i]->interface ].name );
			cameraCtx = cameraDevs[i]->initCamera ( cameraDevs[i] );
			if ( cameraCtx ) {
				printf ( "controls:\n" );
				for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
					controlType = cameraCtx->OA_CAM_CTRL_TYPE(c);
					if ( controlType ) {
						switch ( controlType ) {
							case OA_CTRL_TYPE_INT32:
								typeString = "32-bit int";
								break;
							case OA_CTRL_TYPE_BOOLEAN:
								typeString = "64-bit int";
								break;
							case OA_CTRL_TYPE_MENU:
								typeString = "menu";
								break;
							case OA_CTRL_TYPE_BUTTON:
								typeString = "button";
								break;
							case OA_CTRL_TYPE_INT64:
								typeString = "64-bit int";
								break;
							case OA_CTRL_TYPE_STRING:
								typeString = "string";
								break;
							case OA_CTRL_TYPE_DISCRETE:
								typeString = "discrete values";
								break;
							case OA_CTRL_TYPE_DISC_MENU:
								typeString = "menu of discrete values";
								break;
							case OA_CTRL_TYPE_READONLY:
								typeString = "read-only 32-bit int";
								break;
							default:
								typeString = "unknown";
								break;
						}
						printf ( "  %s (%s)", oaCameraControlLabel[c], typeString );
						if ( controlType == OA_CTRL_TYPE_INT32 ||
								controlType == OA_CTRL_TYPE_INT64 ||
								controlType == OA_CTRL_TYPE_BOOLEAN ||
								controlType == OA_CTRL_TYPE_BUTTON ||
								controlType == OA_CTRL_TYPE_MENU ) {
							int64_t min, max, step, def;
							cameraCtx->funcs.getControlRange ( cameraCtx, c, &min, &max,
									&step, &def );
							printf (
									":\n    min = %ld, max = %ld, step = %ld, default = %ld\n",
									(long) min, (long) max, (long) step, (long) def );
						} else {
							printf ( "\n" );
						}
					}
				}
			} else {
				printf ( "camera initialisation failed\n" );
			}
			if ( cameraCtx != NULL ) {
				cameraCtx->funcs.closeCamera ( cameraCtx );
			} else {
				printf ( "failed to initialise camera\n" );
			}
			printf ( "\n\n" );
		}
	}

	// Release camera list
	oaReleaseCameras ( cameraDevs );

	return 0;
}
