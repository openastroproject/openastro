/*****************************************************************************
 *
 * FC2oacam.h -- header for Point Grey Gig-E camera API
 *
 * Copyright 2015,2016,2017,2018,2019
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

#ifndef OA_FC2_OACAM_H
#define OA_FC2_OACAM_H

extern int		oaFC2GetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaFC2InitCamera ( oaCameraDevice* );
extern int		oaFC2CloseCamera ( oaCamera* );

extern int		oaFC2CameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaFC2CameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaFC2CameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaFC2CameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int		oaFC2CameraGetControlDiscreteSet ( oaCamera*, int,
				int32_t*, int64_t** );

extern int		oaFC2CameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaFC2CameraStopStreaming ( oaCamera* );
extern int		oaFC2CameraIsStreaming ( oaCamera* );

extern int		oaFC2CameraSetResolution ( oaCamera*, int, int );
extern int		oaFC2CameraSetROI ( oaCamera*, int, int );
extern int		oaFC2CameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamFC2controller ( void* );
extern void*		oacamFC2callbackHandler ( void* );

extern const FRAMESIZES* oaFC2CameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaFC2CameraGetFrameRates ( oaCamera*, int, int );
extern int		oaFC2CameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaFC2CameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaFC2CameraGetMenuString ( oaCamera*, int, int );

struct pgeCtrl {
  fc2PropertyType	pgeControl;
  int			oaControl;
  int			oaAutoControl;
};

extern struct pgeCtrl pgeControls[];
extern unsigned int numFC2Controls;

struct pgeFrameRate {
  fc2FrameRate		pgeFrameRate;
  int			numerator;
  int			denominator;
};

extern struct pgeFrameRate pgeFrameRates[];
extern unsigned int numFC2FrameRates;


#define		FC2_REG_DATA_DEPTH		0x0630
#define		FC2_REG_IMAGE_DATA_FORMAT	0x1048

#endif	/* OA_FC2_OACAM_H */
