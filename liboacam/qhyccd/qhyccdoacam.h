/*****************************************************************************
 *
 * qhyccdoacam.h -- header for QHYCCD camera API
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_QHYCCD_OACAM_H
#define OA_QHYCCD_OACAM_H

extern int		oaQHYCCDGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaQHYCCDInitCamera ( oaCameraDevice* );

extern int		oaQHYCCDCloseCamera ( oaCamera* );

extern int		oaQHYCCDCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaQHYCCDCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaQHYCCDCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaQHYCCDCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int              oaQHYCCDCameraGetControlDiscreteSet ( oaCamera*, int,
                                int32_t*, int64_t** );

extern int		oaQHYCCDCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaQHYCCDCameraStopStreaming ( oaCamera* );
extern int		oaQHYCCDCameraIsStreaming ( oaCamera* );

extern int		oaQHYCCDCameraSetResolution ( oaCamera*, int, int );
extern int		oaQHYCCDCameraSetROI ( oaCamera*, int, int );
extern int		oaQHYCCDCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamQHYCCDcontroller ( void* );
extern void*		oacamQHYCCDcallbackHandler ( void* );

extern const FRAMESIZES* oaQHYCCDCameraGetFrameSizes ( oaCamera* );
extern int		oaQHYCCDCameraGetFramePixelFormat ( oaCamera* );

#endif	/* OA_QHYCCD_OACAM_H */
