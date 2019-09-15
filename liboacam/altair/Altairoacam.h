/*****************************************************************************
 *
 * Altairoacam.h -- header for Altair camera API
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

#ifndef OA_ALTAIRCAM_OACAM_H
#define OA_ALTAIRCAM_OACAM_H

extern int		oaAltairGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaAltairInitCamera ( oaCameraDevice* );

extern int		oaAltairCloseCamera ( oaCamera* );

extern int		oaAltairCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaAltairCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaAltairCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaAltairCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int              oaAltairCameraGetControlDiscreteSet ( oaCamera*, int,
                                int32_t*, int64_t** );

extern int		oaAltairCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaAltairCameraStopStreaming ( oaCamera* );
extern int		oaAltairCameraIsStreaming ( oaCamera* );

extern int		oaAltairCameraSetResolution ( oaCamera*, int, int );
extern int		oaAltairCameraSetROI ( oaCamera*, int, int );
extern int		oaAltairCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamAltaircontroller ( void* );
extern void*		oacamAltaircallbackHandler ( void* );

extern const FRAMESIZES* oaAltairCameraGetFrameSizes ( oaCamera* );
extern int		oaAltairCameraGetFramePixelFormat ( oaCamera* );

extern const char*      oaAltairCameraGetMenuString ( oaCamera*, int, int );

extern int	oaAltairCameraStartExposure ( oaCamera*, time_t,
								void* (*)(void*, void*, int, void* ), void* );
extern int	oaAltairCameraAbortExposure ( oaCamera* );
#endif	/* OA_ALTAIRCAM_OACAM_H */
