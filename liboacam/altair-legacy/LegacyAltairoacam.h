/*****************************************************************************
 *
 * LegacyAltairoacam.h -- header for Altair camera API
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

#ifndef OA_ALTAIRCAM_LEGACY_OACAM_H
#define OA_ALTAIRCAM_LEGACY_OACAM_H

extern int		oaAltairLegacyGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaAltairLegacyInitCamera ( oaCameraDevice* );

extern int		oaAltairLegacyCloseCamera ( oaCamera* );

extern int		oaAltairLegacyCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaAltairLegacyCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaAltairLegacyCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaAltairLegacyCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int    oaAltairLegacyCameraGetControlDiscreteSet ( oaCamera*, int,
                                int32_t*, int64_t** );

extern int		oaAltairLegacyCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaAltairLegacyCameraStopStreaming ( oaCamera* );
extern int		oaAltairLegacyCameraIsStreaming ( oaCamera* );

extern int		oaAltairLegacyCameraSetResolution ( oaCamera*, int, int );
extern int		oaAltairLegacyCameraSetROI ( oaCamera*, int, int );
extern int		oaAltairLegacyCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamAltairLegacycontroller ( void* );
extern void*		oacamAltairLegacycallbackHandler ( void* );

extern const FRAMESIZES* oaAltairLegacyCameraGetFrameSizes ( oaCamera* );
extern int		oaAltairLegacyCameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaAltairLegacyCameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_ALTAIRCAM_LEGACY_OACAM_H */
