/*****************************************************************************
 *
 * Spinoacam.h -- header for Point Grey Spinnaker camera API
 *
 * Copyright 2018,2019,2021
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

#ifndef OA_SPINNAKER_OACAM_H
#define OA_SPINNAKER_OACAM_H

extern int		oaSpinGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaSpinInitCamera ( oaCameraDevice* );
extern int		oaSpinCloseCamera ( oaCamera* );

extern int		oaSpinCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaSpinCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int		oaSpinCameraGetControlDiscreteSet ( oaCamera*, int,
				int32_t*, int64_t** );

extern int		oaSpinCameraSetROI ( oaCamera*, int, int );
extern int		oaSpinCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamSpinController ( void* );
extern void*		oacamSpinCallbackHandler ( void* );

extern const FRAMESIZES* oaSpinCameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaSpinCameraGetFrameRates ( oaCamera*, int, int );
extern int		oaSpinCameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaSpinCameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaSpinCameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_SPINNAKER_OACAM_H */
