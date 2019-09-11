/*****************************************************************************
 *
 * dummyoacam.h -- header for dummy camera
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

#ifndef DUMMY_OACAM_H
#define DUMMY_OACAM_H

extern int		oaDummyGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaDummyInitCamera ( oaCameraDevice* );
extern int              oaDummyCloseCamera ( oaCamera* );

extern int		oaDummyCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaDummyCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaDummyCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaDummyCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern int		oaDummyCameraStartStreaming ( oaCamera*,
								void* (*)(void*, void*, int, void* ), void* );
extern int		oaDummyCameraStopStreaming ( oaCamera* );
extern int		oaDummyCameraIsStreaming ( oaCamera* );

extern void*		oacamDummyController ( void* );
extern void*		oacamDummyCallbackHandler ( void* );

extern const FRAMESIZES*	oaDummyCameraGetFrameSizes ( oaCamera* );
extern int		oaDummyCameraGetFramePixelFormat ( oaCamera* );
extern int    oaDummyCameraTestROISize ( oaCamera*, unsigned int,
								unsigned int, unsigned int*, unsigned int* );

#endif	/* DUMMY_OACAM_H */
