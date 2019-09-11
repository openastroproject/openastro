/*****************************************************************************
 *
 * ZWASIoacam.h -- header for ZW ASI camera API
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

#ifndef ZWASI_OACAM_H
#define ZWASI_OACAM_H

extern int		oaZWASIGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaZWASIInitCamera ( oaCameraDevice* );
extern int              oaZWASICloseCamera ( oaCamera* );

extern int		oaZWASICameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaZWASICameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaZWASICameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaZWASICameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern int              oaZWASICameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int              oaZWASICameraStopStreaming ( oaCamera* );
extern int              oaZWASICameraIsStreaming ( oaCamera* );

extern int		oaZWASICameraSetResolution ( oaCamera*, int, int );
extern int		oaZWASICameraSetROI ( oaCamera*, int, int );

extern void*		oacamZWASIcontroller ( void* );
extern void*		oacamZWASIcallbackHandler ( void* );

extern const FRAMESIZES* oaZWASICameraGetFrameSizes ( oaCamera* );
extern int		oaZWASICameraGetFramePixelFormat ( oaCamera* );
extern int		oaZWASICameraTestROISize ( oaCamera*, unsigned int,
				unsigned int, unsigned int*, unsigned int* );

#endif	/* ZWASI_OACAM_H */
