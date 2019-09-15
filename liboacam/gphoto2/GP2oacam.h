/*****************************************************************************
 *
 * GP2oacam.h -- header for libgphoto2 camera API
 *
 * Copyright 2019
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

#ifndef OA_GP2_OACAM_H
#define OA_GP2_OACAM_H

#include <openastro/camera.h>

#include "oacamprivate.h"

extern int				oaGP2GetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaGP2InitCamera ( oaCameraDevice* );
extern int        oaGP2CloseCamera ( oaCamera* );

extern int				oaGP2CameraTestControl ( oaCamera*, int, oaControlValue* );
extern int				oaGP2CameraSetControl ( oaCamera*, int,
											oaControlValue*, int );
extern int				oaGP2CameraReadControl ( oaCamera*, int, oaControlValue* );
extern int				oaGP2CameraGetControlRange ( oaCamera*, int, int64_t*,
											int64_t*, int64_t*, int64_t* );
extern int				oaGP2CameraGetControlDiscreteSet ( oaCamera*, int, int32_t*,
											int64_t** );

extern int				oaGP2CameraStartStreaming ( oaCamera*,
											void* (*)(void*, void*, int, void* ), void* );
extern int        oaGP2CameraStopStreaming ( oaCamera* );
extern int        oaGP2CameraIsStreaming ( oaCamera* );

extern int				oaGP2CameraSetResolution ( oaCamera*, int, int );

extern int				oaGP2CameraStartExposure ( oaCamera*, time_t,
											void* (*)(void*, void*, int, void* ), void* );
extern int				oaGP2CameraAbortExposure ( oaCamera* );

extern void*			oacamGP2controller ( void* );
extern void*			oacamGP2callbackHandler ( void* );

extern const FRAMESIZES* oaGP2CameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaGP2CameraGetFrameRates ( oaCamera*, int, int );
extern int				oaGP2CameraSetFrameInterval ( oaCamera*, int, int );
extern int				oaGP2CameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaGP2CameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_GP2_OACAM_H */
