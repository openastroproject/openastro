/*****************************************************************************
 *
 * Touptekoacam.h -- header for Touptek camera API
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

#ifndef OA_TOUPTEK_OACAM_H
#define OA_TOUPTEK_OACAM_H

extern int		oaTouptekGetCameras ( CAMERA_LIST*, int );
extern oaCamera*	oaTouptekInitCamera ( oaCameraDevice* );

extern int		oaTouptekCloseCamera ( oaCamera* );

extern int		oaTouptekCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaTouptekCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaTouptekCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaTouptekCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int              oaTouptekCameraGetControlDiscreteSet ( oaCamera*, int,
                                int32_t*, int64_t** );

extern int		oaTouptekCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaTouptekCameraStopStreaming ( oaCamera* );
extern int		oaTouptekCameraIsStreaming ( oaCamera* );

extern int		oaTouptekCameraSetResolution ( oaCamera*, int, int );
extern int		oaTouptekCameraSetROI ( oaCamera*, int, int );
extern int		oaTouptekCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamTouptekcontroller ( void* );
extern void*		oacamTouptekcallbackHandler ( void* );

extern const FRAMESIZES* oaTouptekCameraGetFrameSizes ( oaCamera* );
extern int		oaTouptekCameraGetFramePixelFormat ( oaCamera* );

extern const char*      oaTouptekCameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_TOUPTEK_OACAM_H */
