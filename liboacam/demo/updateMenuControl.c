/*****************************************************************************
 *
 * updateMenuControl.c
 *
 * example program to change the value of a MENU control
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
	int									onOffControl, autoControl, baseControl;
	int									onOffType, autoType;
	int									i, ctrl;
	oaControlValue			val;
	uint64_t						currentSetting, newSetting, min, max, step, def;
	const char*					itemStr;

	/*
	 * The functions called below generally return error codes and they should
	 * be checked, but this has been left out for simplicity.
	 *
	 * Where a function returns an error code, the return value of the function
	 * will be negative. A return value of zero (or OA_ERR_NONE) indicates
	 * success.
	 */

	// Get list of connected cameras.  Pick one with controls we can read
	numCameras = oaGetCameras ( &cameraDevs, OA_CAM_FEATURE_READABLE_CONTROLS );

	if ( numCameras > 0 ) {
		cameraCtx = cameraDevs[0]->initCamera ( cameraDevs[0] );

		baseControl = 0;
		for ( ctrl = 1; !baseControl && ctrl < OA_CAM_CTRL_LAST_P1; ctrl++ ) {
			if ( cameraCtx->OA_CAM_CTRL_TYPE ( ctrl ) == OA_CTRL_TYPE_MENU ) {
				baseControl = ctrl;
			}
		}

		if ( baseControl ) {

			onOffControl = OA_CAM_CTRL_MODE_ON_OFF ( baseControl );
			autoControl = OA_CAM_CTRL_MODE_AUTO ( baseControl );

			// Get the types of the related controls
			onOffType = cameraCtx->OA_CAM_CTRL_TYPE ( onOffControl );
			autoType = cameraCtx->OA_CAM_CTRL_TYPE ( autoControl );

			printf ( "%s is a menu control\n", oaCameraControlLabel[ baseControl ]);

			// If on/off is a boolean control, turn the control on if it is off
			if ( onOffType == OA_CTRL_TYPE_BOOLEAN ) {
				printf ( "on/off control is boolean\n" );
				cameraCtx->funcs.readControl ( cameraCtx, onOffControl, &val );
				if ( val.boolean ) {
					printf ( "  control is on\n" );
				} else {
					// set the control on
					printf ( "  control is off, setting to on\n" );
					val.boolean = 1;
					cameraCtx->funcs.setControl ( cameraCtx, onOffControl, &val, 0 );
				}
			} else {
				if ( onOffType ) {
					printf ( "on/off control is available, but is not boolean\n" );
				} else {
					printf ( "on/off control is not present\n" );
				}
			}

			// If auto mode is a boolean control, turn it off if it is on
			if ( autoType == OA_CTRL_TYPE_BOOLEAN ) {
				printf ( "auto control is boolean\n" );
				cameraCtx->funcs.readControl ( cameraCtx, autoControl, &val );
				if ( val.boolean ) {
					printf ( "  auto mode is on, turning it off\n" );
					val.boolean = 0;
					cameraCtx->funcs.setControl ( cameraCtx, autoControl, &val, 0 );
				} else {
					// set the control on
					printf ( "  auto mode is off\n" );
				}
			} else {
				if ( autoType ) {
					printf ( "auto control is available, but is not boolean\n" );
				} else {
					printf ( "auto control is not present\n" );
				}
			}

			cameraCtx->funcs.readControl ( cameraCtx, baseControl, &val );
			currentSetting = val.menu;
			printf ( "  current value is %ld\n", currentSetting );
			cameraCtx->funcs.getControlRange ( cameraCtx, baseControl, &min, &max,
					&step, &def );
			printf ( "  control range: min = %ld, max = %ld, step = %ld, "
					"default = %ld\n", min, max, step, def );
			for ( i = min; i <= max; i += step ) {
				itemStr = cameraCtx->funcs.getMenuString ( cameraCtx, baseControl, i );
				printf ( "menu string for %d is '%s'\n", i, itemStr );
			}

			newSetting = min;
			if ( currentSetting == newSetting ) {
				newSetting += step;
			}
			printf ( "  setting new value to %ld\n", newSetting );
			val.menu = newSetting;
			cameraCtx->funcs.setControl ( cameraCtx, baseControl, &val, 0 );
			val.menu = 0;
			cameraCtx->funcs.readControl ( cameraCtx, baseControl, &val );
			currentSetting = val.menu;
			printf ( "  now current value is %ld\n", currentSetting );
		} else {
			printf ( "No menu type controls were found for camera '%s'\n",
					cameraDevs[0]->deviceName );
		}
	} else {
		printf ( "no cameras with readable controls are available\n" );
	}

	// Release camera list
	oaReleaseCameras ( cameraDevs );

	return 0;
}
