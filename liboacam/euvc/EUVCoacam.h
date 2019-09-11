/*****************************************************************************
 *
 * EUVCoacam.h -- header for EUVC camera API
 *
 * Copyright 2015,2016,2017,2018,2019
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

#ifndef OA_EUVC_OACAM_H
#define OA_EUVC_OACAM_H

extern int		oaEUVCGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaEUVCInitCamera ( oaCameraDevice* );
extern int              oaEUVCCloseCamera ( oaCamera* );

extern int		oaEUVCCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaEUVCCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaEUVCCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaEUVCCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern int		oaEUVCCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int              oaEUVCCameraStopStreaming ( oaCamera* );
extern int              oaEUVCCameraIsStreaming ( oaCamera* );

extern int		oaEUVCCameraSetResolution ( oaCamera*, int, int );
extern int		oaEUVCCameraSetROI ( oaCamera*, int, int );
extern int		oaEUVCCameraTestROISize ( oaCamera*, unsigned int,
                            unsigned int, unsigned int*, unsigned int* );

extern void*		oacamEUVCcontroller ( void* );
extern void*		oacamEUVCcallbackHandler ( void* );

extern const FRAMESIZES* oaEUVCCameraGetFrameSizes ( oaCamera* );
extern int		oaEUVCCameraGetFramePixelFormat ( oaCamera* );
extern const FRAMERATES* oaEUVCCameraGetFrameRates ( oaCamera*, int, int );
extern int              oaEUVCCameraSetFrameInterval ( oaCamera*, int, int );

extern const char*	oaEUVCCameraGetMenuString ( oaCamera*, int, int );

int			getEUVCControl ( EUVC_STATE*, uint8_t, int, int );
int			getEUVCTermControl ( EUVC_STATE*, uint8_t, void*,
			    int, int );
int			setEUVCTermControl ( EUVC_STATE*, uint8_t, void*,
			    int, int );


#define EUVC_DEFAULT_EXPOSURE	33
#define EUVC_DEFAULT_GAIN	48

#endif	/* OA_EUVC_OACAM_H */
