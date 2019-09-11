/*****************************************************************************
 *
 * SXoacam.h -- header for Starlight Xpress camera API
 *
 * Copyright 2014,2015,2016,2017,2018,2019
 *   James Fidell (james@openastroproject.org)
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

#ifndef OA_SX_OACAM_H
#define OA_SX_OACAM_H

extern int		oaSXGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaSXInitCamera ( oaCameraDevice* );
extern int              oaSXCloseCamera ( oaCamera* );

extern int		oaSXCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaSXCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaSXCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaSXCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern int		oaSXCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int              oaSXCameraStopStreaming ( oaCamera* );
extern int              oaSXCameraIsStreaming ( oaCamera* );

extern int		oaSXCameraSetResolution ( oaCamera*, int, int );

extern void*		oacamSXcontroller ( void* );
extern void*		oacamSXcallbackHandler ( void* );

extern const FRAMESIZES* oaSXCameraGetFrameSizes ( oaCamera* );
extern int		oaSXCameraGetFramePixelFormat ( oaCamera* );
extern int		oaSXCameraTestROISize ( oaCamera*, unsigned int,
									unsigned int, unsigned int*, unsigned int* );

extern int		_SXsetTimer ( SX_STATE*, unsigned int );


struct sxcam {
  unsigned int	vendorId;
  unsigned int	productId;
  const char*	name;
  unsigned int	devType;
};

extern struct sxcam SXCameraList[];

#define	SX_DEFAULT_EXPOSURE	100

#endif	/* OA_SX_OACAM_H */
