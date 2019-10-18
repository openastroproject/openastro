/*****************************************************************************
 *
 * IIDCoacam.h -- header for IEE1394/IIDC camera API
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

#ifndef OA_IIDC_OACAM_H
#define OA_IIDC_OACAM_H

extern int		oaIIDCGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaIIDCInitCamera ( oaCameraDevice* );
extern int		oaIIDCCloseCamera ( oaCamera* );

extern int		oaIIDCCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaIIDCCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern void*		oacamIIDCcontroller ( void* );
extern void*		oacamIIDCcallbackHandler ( void* );

extern const FRAMESIZES* oaIIDCCameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaIIDCCameraGetFrameRates ( oaCamera*, int, int );
extern int		oaIIDCCameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaIIDCCameraGetMenuString ( oaCamera*, int, int );


struct iidcCtrl {
  int  iidcControl;
  int  oaControl;
};

extern struct iidcCtrl dc1394Controls[];
extern unsigned int numIIDCControls;

struct iidcFrameRate {
  int iidcFrameRate;
  int numerator;
  int denominator;
};

extern struct iidcFrameRate dc1394FrameRates[];
extern unsigned int numIIDCFrameRates;

#endif	/* OA_IIDC_OACAM_H */
