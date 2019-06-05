/*****************************************************************************
 *
 * Mallincamoacam.h -- header for Mallincam camera API
 *
 * Copyright 2016,2017,2018,2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_MALLINCAM_OACAM_H
#define OA_MALLINCAM_OACAM_H

extern int		oaMallincamGetCameras ( CAMERA_LIST*, int );
extern oaCamera*	oaMallincamInitCamera ( oaCameraDevice* );

extern int		oaMallincamCloseCamera ( oaCamera* );

extern int		oaMallincamCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaMallincamCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaMallincamCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaMallincamCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int              oaMallincamCameraGetControlDiscreteSet ( oaCamera*, int,
                                int32_t*, int64_t** );

extern int		oaMallincamCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaMallincamCameraStopStreaming ( oaCamera* );
extern int		oaMallincamCameraIsStreaming ( oaCamera* );

extern int		oaMallincamCameraSetResolution ( oaCamera*, int, int );
extern int		oaMallincamCameraSetROI ( oaCamera*, int, int );
extern int		oaMallincamCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamMallincamcontroller ( void* );
extern void*		oacamMallincamcallbackHandler ( void* );

extern const FRAMESIZES* oaMallincamCameraGetFrameSizes ( oaCamera* );
extern int		oaMallincamCameraGetFramePixelFormat ( oaCamera* );

extern const char*      oaMallincamCameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_MALLINCAM_OACAM_H */
