/*****************************************************************************
 *
 * V4L2oacam.h -- header for V4L2 camera API
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

#include <oa_common.h>

#if HAVE_LIBV4L2

#ifndef OA_V4L2_OACAM_H
#define OA_V4L2_OACAM_H

#define	SYS_V4L_PATH		"/sys/class/video4linux"

extern int		oaV4L2GetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaV4L2InitCamera ( oaCameraDevice* );
extern int              oaV4L2CloseCamera ( oaCamera* );

extern int		oaV4L2CameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaV4L2CameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaV4L2CameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaV4L2CameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int		oaV4L2CameraGetControlDiscreteSet ( oaCamera*, int,
				int32_t*, int64_t** );

extern int		oaV4L2CameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaV4L2CameraStopStreaming ( oaCamera* );
extern int		oaV4L2CameraIsStreaming ( oaCamera* );

extern int		oaV4L2CameraSetResolution ( oaCamera*, int, int );

extern void*		oacamV4L2controller ( void* );
extern void*		oacamV4L2callbackHandler ( void* );

extern const FRAMESIZES* oaV4L2CameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaV4L2CameraGetFrameRates ( oaCamera*, int, int );
extern int		oaV4L2CameraSetFrameInterval ( oaCamera*, int, int );
extern int              oaV4L2CameraGetFramePixelFormat ( oaCamera* );

extern int              oaV4L2CameraGetAutoWBManualSetting ( oaCamera* );

extern int		oaV4L2CameraHasFixedFrameRates ( oaCamera*, int, int );
extern const char*	oaV4L2CameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_V4L2_OACAM_H */

#endif /* HAVE_LIBV4L2 */
