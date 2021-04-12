/*****************************************************************************
 *
 * aravisoacam.h -- header for Aravis camera API
 *
 * Copyright 2021 James Fidell (james@openastroproject.org)
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

#ifndef OA_ARAVIS_OACAM_H
#define OA_ARAVIS_OACAM_H

extern int				oaAravisGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaAravisInitCamera ( oaCameraDevice* );

extern int				oaAravisCloseCamera ( oaCamera* );

extern int				oaAravisCameraTestControl ( oaCamera*, int, oaControlValue* );
extern int				oaAravisCameraGetControlRange ( oaCamera*, int, int64_t*,
											int64_t*, int64_t*, int64_t* );
extern int        oaAravisCameraGetControlDiscreteSet ( oaCamera*, int,
                      int32_t*, int64_t** );

extern int				oaAravisCameraSetROI ( oaCamera*, int, int );
extern int				oaAravisCameraTestROISize ( oaCamera*, unsigned int,
											unsigned int, unsigned int*, unsigned int* );

extern void*			oacamAravisController ( void* );
extern void*			oacamAraviscallbackHandler ( void* );

extern const FRAMESIZES* oaAravisCameraGetFrameSizes ( oaCamera* );
extern int				oaAravisCameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaAravisCameraGetMenuString ( oaCamera*, int, int );

#endif	/* OA_ARAVIS_OACAM_H */
