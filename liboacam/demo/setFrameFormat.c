/*****************************************************************************
 *
 * setFrameFormat.c
 *
 * example program to check and change the frame format from a camera
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
	int									numCameras, format, newFormat, i;
	oaCamera*						cameraCtx = NULL;
	oaControlValue			val;

	// Get list of connected cameras.  Don't filter on any specific feature
	numCameras = oaGetCameras ( &cameraDevs, OA_CAM_FEATURE_STREAMING );

	if ( numCameras > 0 ) {
		cameraCtx = cameraDevs[0]->initCamera ( cameraDevs[0] );
		format = cameraCtx->funcs.getFramePixelFormat ( cameraCtx );
		printf ( "Current frame format is: %s\n",
				oaFrameFormats[ format ].name );
		newFormat = 0;
		printf ( "Supported frame formats:\n" );
		for ( i = 0; i < OA_PIX_FMT_LAST_P1; i++ ) {
			if ( cameraCtx->frameFormats[i] ) {
				printf ( "  %s (%s)\n", oaFrameFormats[i].name,
						oaFrameFormats[i].simpleName );
				printf ( "    %f bytes per pixel, %d bits per pixel\n",
						oaFrameFormats[i].bytesPerPixel, oaFrameFormats[i].bitsPerPixel );
				if ( oaFrameFormats[i].monochrome ) {
					printf ( "    monochrome\n" );
				}
				if ( oaFrameFormats[i].rawColour ) {
					printf ( "    raw colour\n" );
				}
				if ( oaFrameFormats[i].fullColour ) {
					printf ( "    full colour\n" );
				}
				if ( !newFormat && format != i ) {
					newFormat = i;
				}
			}
		}
		if ( cameraCtx->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) ==
				OA_CTRL_TYPE_DISCRETE ) {
			if ( newFormat ) {
				val.valueType = OA_CTRL_TYPE_DISCRETE;
				val.discrete = newFormat;
				cameraCtx->funcs.setControl ( cameraCtx, OA_CAM_CTRL_FRAME_FORMAT,
						&val, 0 );
				val.int32 = 0;
				format = cameraCtx->funcs.getFramePixelFormat ( cameraCtx );
				printf ( "new frame format is: %s\n",
						oaFrameFormats[ format ].name );
			} else {
				printf ( "No recognised format for change\n" );
			}
		} else {
			printf ( "Camera doesn't appear to support changing frame format\n" );
		}
		cameraCtx->funcs.closeCamera ( cameraCtx );
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
