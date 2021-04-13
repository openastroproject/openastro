/*****************************************************************************
 *
 * startStreaming.c
 *
 * example program to stream images from a camera
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


void* handleFrame ( void*, void*, int, void* );

int
main()
{
	oaCameraDevice**		cameraDevs;
	int									numCameras;
	oaCamera*						cameraCtx = NULL;
	void*								userData = ( void* ) 0x12345678;

	// Get list of connected cameras.  Don't filter on any specific feature
	numCameras = oaGetCameras ( &cameraDevs, OA_CAM_FEATURE_STREAMING );

	if ( numCameras > 0 ) {
		cameraCtx = cameraDevs[0]->initCamera ( cameraDevs[0] );
		printf ( "starting streaming on %s\n", cameraDevs[0]->deviceName );
		cameraCtx->funcs.startStreaming ( cameraCtx, handleFrame, userData );
		sleep ( 5 );
		cameraCtx->funcs.stopStreaming ( cameraCtx );
	} else {
		printf ( "no cameras supporting streaming are available\n" );
	}

	// Release camera list
	oaReleaseCameras ( cameraDevs );

	return 0;
}


void*
handleFrame ( void* args, void* imageData, int length, void* imageMetadata )
{
	printf ( "received frame at %p with length %d, args = %p, metadata = %p\n",
			imageData, length, args, imageMetadata );
	return 0;
}
