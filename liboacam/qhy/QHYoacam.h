/*****************************************************************************
 *
 * QHYoacam.h -- header for QHY camera API
 *
 * Copyright 2013,2014,2015,2016,2017,2019
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

#ifndef OA_QHY_OACAM_H
#define OA_QHY_OACAM_H

extern int		oaQHYGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaQHYInitCamera ( oaCameraDevice* );
extern int              oaQHYCloseCamera ( oaCamera* );

extern int		oaQHYCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaQHYCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaQHYCameraGetControlRange ( oaCamera*, int, int64_t*,
				 int64_t*, int64_t*, int64_t* );

extern int		oaQHYCameraSetResolution ( oaCamera*, int, int );
extern int		oaQHYCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaQHYCameraIsStreaming ( oaCamera* );
extern int		oaQHYCameraStopStreaming ( oaCamera* );

extern void*		oacamQHYcallbackHandler ( void* );

struct qhycam {
  unsigned int	vendorId;
  unsigned int	productId;
  const char*	name;
  unsigned int	devType;
  unsigned int	hasFirmware;
  unsigned int	firmwareLoaded;
  unsigned int	supported;
  int		wIndex;
  int		subtype;
  int		colour;
};

extern struct qhycam cameraList[];

#define QHY5_DEFAULT_EXPOSURE	100

#endif	/* OA_QHY_OACAM_H */
