/*****************************************************************************
 *
 * UVCoacam.h -- header for UVC camera API
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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

#ifndef OA_UVC_OACAM_H
#define OA_UVC_OACAM_H

#include <libuvc/libuvc.h>
#include <openastro/camera.h>

#include "oacamprivate.h"

extern int		oaUVCGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaUVCInitCamera ( oaCameraDevice* );
extern int              oaUVCCloseCamera ( oaCamera* );

extern int		oaUVCCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaUVCCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaUVCCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaUVCCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int		oaUVCCameraGetControlDiscreteSet ( oaCamera*, int,
				int32_t*, int64_t** );

extern int		oaUVCCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int              oaUVCCameraStopStreaming ( oaCamera* );
extern int              oaUVCCameraIsStreaming ( oaCamera* );

extern int		oaUVCCameraSetResolution ( oaCamera*, int, int );

extern void*		oacamUVCcontroller ( void* );
extern void*		oacamUVCcallbackHandler ( void* );

extern const FRAMESIZES* oaUVCCameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaUVCCameraGetFrameRates ( oaCamera*, int, int );
extern int		oaUVCCameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaUVCCameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaUVCCameraGetMenuString ( oaCamera*, int, int );

extern int		getUVCControl ( uvc_device_handle_t*, uint8_t, uint8_t,
			    int, enum uvc_req_code );
extern int		setUVCControl ( uvc_device_handle_t*, uint8_t, uint8_t,
			    int, int );
extern void		frameCallback ( uvc_frame_t*, void* );

#endif	/* OA_UVC_OACAM_H */
