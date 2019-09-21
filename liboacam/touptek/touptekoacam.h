/*****************************************************************************
 *
 * oacam.h -- header for Touptek camera API
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

#ifndef OA_TOUPTEK_OACAM_H
#define OA_TOUPTEK_OACAM_H

extern int		TT_FUNC( oa, GetCameras )( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	TT_FUNC( oa, InitCamera )( oaCameraDevice* );

extern int		TT_FUNC( oa, CloseCamera )( oaCamera* );

extern int		TT_FUNC( oa, CameraTestControl )( oaCamera*, int,
				oaControlValue* );
extern int		TT_FUNC( oa, CameraSetControl )( oaCamera*, int,
				oaControlValue*, int );
extern int		TT_FUNC( oa, CameraReadControl )( oaCamera*, int,
				oaControlValue* );
extern int		TT_FUNC( oa, CameraGetControlRange )( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int              TT_FUNC( oa, CameraGetControlDiscreteSet )( oaCamera*, int,
                                int32_t*, int64_t** );

extern int		TT_FUNC( oa, CameraStartStreaming )( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		TT_FUNC( oa, CameraStopStreaming )( oaCamera* );
extern int		TT_FUNC( oa, CameraIsStreaming )( oaCamera* );

extern int		TT_FUNC( oa, CameraSetResolution )( oaCamera*, int, int );
extern int		TT_FUNC( oa, CameraSetROI )( oaCamera*, int, int );
extern int		TT_FUNC( oa, CameraTestROISize )( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		TT_FUNC( oacam, controller )( void* );
extern void*		TT_FUNC( oacam, callbackHandler )( void* );

extern const FRAMESIZES* TT_FUNC( oa, CameraGetFrameSizes )( oaCamera* );
extern int		TT_FUNC( oa, CameraGetFramePixelFormat )( oaCamera* );

extern const char*      TT_FUNC( oa, CameraGetMenuString )( oaCamera*, int, int );

extern int	TT_FUNC( oa, CameraStartExposure )( oaCamera*, time_t,
								void* (*)(void*, void*, int, void* ), void* );
extern int	TT_FUNC( oa, CameraAbortExposure )( oaCamera* );

#endif	/* OA_TOUPTEK_OACAM_H */
