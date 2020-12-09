/*****************************************************************************
 *
 * SVBoacam.h -- header for SVBony camera
 *
 * Copyright 2020 James Fidell (james@openastroproject.org)
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

#ifndef SVB_OACAM_H
#define SVB_OACAM_H

extern int		oaSVBGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaSVBInitCamera ( oaCameraDevice* );
extern int              oaSVBCloseCamera ( oaCamera* );

extern int		oaSVBCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaSVBCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern void*		oacamSVBcontroller ( void* );

extern int		oaSVBCameraGetFramePixelFormat ( oaCamera* );
extern int		oaSVBCameraSetResolution ( oaCamera*, int, int );
extern const FRAMESIZES*	oaSVBCameraGetFrameSizes ( oaCamera* );
extern int		oaSVBCameraTestROISize ( oaCamera*, unsigned int, unsigned int,
									unsigned int*, unsigned int* );

extern void*	oacamSVBcallbackHandler ( void* );

#endif	/* SVB_OACAM_H */
