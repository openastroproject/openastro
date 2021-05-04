/*****************************************************************************
 *
 * updateIntControl.c
 *
 * example program to change the value of an INT32/INT64 control
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
	int									onOffType, autoType, baseType;
	oaControlValue			val;
	int64_t							currentSetting, newSetting, min, max, step, def, rem;

	baseControl = OA_CAM_CTRL_GAIN;
	onOffControl = OA_CAM_CTRL_MODE_ON_OFF ( baseControl );
	autoControl = OA_CAM_CTRL_MODE_AUTO ( baseControl );

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

		// Get the types of the related controls
		onOffType = cameraCtx->OA_CAM_CTRL_TYPE ( onOffControl );
		autoType = cameraCtx->OA_CAM_CTRL_TYPE ( autoControl );
		baseType = cameraCtx->OA_CAM_CTRL_TYPE ( baseControl );

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

		// If the base control is INT32 or INT64, set it to somewhere in the
		// middle of the range of possible values

		if ( baseType == OA_CTRL_TYPE_INT32 || baseType == OA_CTRL_TYPE_INT64 ) {
			printf ( "control is int32/int64\n" );
			cameraCtx->funcs.readControl ( cameraCtx, baseControl, &val );
			if ( baseType == OA_CTRL_TYPE_INT32 ) {
				currentSetting = val.int32;
			} else {
				currentSetting = val.int64;
			}
			printf ( "  current value is %ld\n", ( long ) currentSetting );
			cameraCtx->funcs.getControlRange ( cameraCtx, baseControl, &min, &max,
					&step, &def );
			printf ( "  control range: min = %ld, max = %ld, step = %ld, "
					"default = %ld\n", (long) min, (long) max, (long) step, (long) def );
			newSetting = ( min + max ) / 2;
			// Need to make sure the new setting is consistent with the step value
			if ( step != 1 ) {
				if (( rem = ( newSetting - min ) % step ) != 0 ) {
					newSetting -= rem;
				}
			}
			if ( baseType == OA_CTRL_TYPE_INT32 ) {
				val.int32 = newSetting;
			} else {
				val.int64 = newSetting;
			}
			cameraCtx->funcs.setControl ( cameraCtx, baseControl, &val, 0 );
			// Clear the value set
			val.int32 = 0;
			val.int64 = 0;
			cameraCtx->funcs.readControl ( cameraCtx, baseControl, &val );
			if ( baseType == OA_CTRL_TYPE_INT32 ) {
				currentSetting = val.int32;
			} else {
				currentSetting = val.int64;
			}
			printf ( "  now current value is %ld\n", ( long ) currentSetting );
		} else {
			if ( baseType ) {
				printf ( "control is available, but is not INT32/INT64\n" );
			} else {
				printf ( "control is not present\n" );
			}
		}
	} else {
		printf ( "no cameras with readable controls are available\n" );
	}

	// Release camera list
	oaReleaseCameras ( cameraDevs );

	return 0;
}
