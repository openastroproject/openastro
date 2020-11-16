/*****************************************************************************
 *
 * oacam.h -- header for Basler Pylon camera API
 *
 * Copyright 2020
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

#ifndef OA_PYLON_OACAM_H
#define OA_PYLON_OACAM_H

extern int				oaPylonGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaPylonInitCamera ( oaCameraDevice* );

extern int		oaPylonCloseCamera ( oaCamera* );

extern int		oaPylonCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaPylonCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
/*
extern int		oaPylonCameraGetControlDiscreteSet ( oaCamera*, int,
				int32_t*, int64_t** );
*/

extern int		oaPylonCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamPylonController ( void* );
extern void*		oacamPylonCallbackHandler ( void* );

extern const FRAMESIZES* oaPylonCameraGetFrameSizes ( oaCamera* );
//extern const FRAMERATES* oaPylonCameraGetFrameRates ( oaCamera*, int, int );
extern int		oaPylonCameraGetFramePixelFormat ( oaCamera* );

//extern const char*	oaPylonCameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_PYLON_OACAM_H */
