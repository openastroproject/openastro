/*****************************************************************************
 *
 * showFeatures.c
 *
 * example program to list the available features for connected cameras
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
	int									numCameras;
	int									i;
	oaCamera*						cameraCtx = NULL;

	// Get list of connected cameras.  Don't filter on any specific feature
	numCameras = oaGetCameras ( &cameraDevs, OA_CAM_FEATURE_NONE );

	if ( numCameras > 0 ) {
		for ( i = 0; i < numCameras; i++ ) {
			printf ( "found camera %s via %s interface\n", cameraDevs[i]->deviceName,
					oaCameraInterfaces[ cameraDevs[i]->interface ].name );
			cameraCtx = cameraDevs[i]->initCamera ( cameraDevs[i] );
			if ( cameraCtx ) {
				printf ( "  features:\n" );
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_RAW_MODE ) {
					printf ( "    raw colour mode\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_DEMOSAIC_MODE ) {
					printf ( "    demosaic (RGB) colour mode\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_BINNING ) {
					printf ( "    binning support\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_FRAME_RATES ) {
					printf ( "    frame rate support\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_ROI ) {
					printf ( "    region of interest\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_RESET ) {
					printf ( "    reset camera\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_EXTERNAL_TRIGGER ) {
					printf ( "    external trigger of exposures\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_STROBE_OUTPUT ) {
					printf ( "    strobe output for exposures\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_FIXED_FRAME_SIZES ) {
					printf ( "    has fixed set of frame sizes\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_READABLE_CONTROLS ) {
					printf ( "    allows control values to be read\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_FIXED_READ_NOISE ) {
					printf ( "    umm....\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_STREAMING ) {
					printf ( "    supports streaming of frames\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_SINGLE_SHOT ) {
					printf ( "    supports single shot capture\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_FRAME_SIZE_UNKNOWN ) {
					printf ( "    umm....\n" );
				}
				if ( cameraCtx->features.flags & OA_CAM_FEATURE_GPS ) {
					printf ( "    has GPS\n" );
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
