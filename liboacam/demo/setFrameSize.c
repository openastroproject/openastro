/*****************************************************************************
 *
 * setFrameFormat.c
 *
 * example program to set the frame size
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
	oaCamera*						cameraCtx = NULL;
	int									i;
	unsigned int				reqX, reqY, useX, useY;
	const FRAMESIZES*		frameSizes;

	// Get list of connected cameras.  Don't filter on any specific feature
	numCameras = oaGetCameras ( &cameraDevs, OA_CAM_FEATURE_NONE );

	if ( numCameras > 0 ) {
		cameraCtx = cameraDevs[0]->initCamera ( cameraDevs[0] );
		frameSizes = cameraCtx->funcs.enumerateFrameSizes ( cameraCtx );
		printf ( "Supported frame sizes:\n" );
		for ( i = 0; i < frameSizes->numSizes; i++ ) {
			printf ( "  %dx%d\n", frameSizes->sizes[i].x, frameSizes->sizes[i].y );
		}

		if ( cameraCtx->features.flags & OA_CAM_FEATURE_ROI ) {
			printf ( "Camera supports region of interest\n" );
			if ( cameraCtx->features.flags & OA_CAM_FEATURE_FIXED_FRAME_SIZES ) {
				if ( frameSizes->numSizes > 1 ) {
					printf ( "Setting frame size to %dx%d\n", frameSizes->sizes[1].x,
							frameSizes->sizes[1].y );
					cameraCtx->funcs.setROI ( cameraCtx, frameSizes->sizes[1].x,
							frameSizes->sizes[1].y );
				} else {
					printf ( "Camera only has one frame size\n" );
				}
			} else {
				printf ( "ROI sizes are not fixed\n" );
				// Try to set a size of 768x588 (chosen at random)
				reqX = 768;
				reqY = 588;
				if ( cameraCtx->funcs.testROISize ( cameraCtx, reqX, reqY, &useX,
						&useY ) == OA_ERR_NONE ) {
					printf ( "%dx%d is available\n", reqX, reqY );
					useX = reqX;
					useY = reqY;
				} else {
					printf ( "%dx%d is not available, trying %dx%d instead\n", reqX, reqY,
							useX, useY );
				}
				printf ( "setting ROI size to %dx%d\n", useX, useY );
				cameraCtx->funcs.setROI ( cameraCtx, useX, useY );
			}
		} else {
			printf ( "Camera does not support ROI\n" );
		}
		cameraCtx->funcs.closeCamera ( cameraCtx );
	}

	// Release camera list
	oaReleaseCameras ( cameraDevs );

	return 0;
}
