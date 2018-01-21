/*****************************************************************************
 *
 * PGEoacam.h -- header for Point Grey Gig-E camera API
 *
 * Copyright 2015,2016,2017,2018 James Fidell (james@openastroproject.org)
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

#ifndef OA_PGE_OACAM_H
#define OA_PGE_OACAM_H

extern int		oaPGEGetCameras ( CAMERA_LIST*, int );
extern oaCamera*	oaPGEInitCamera ( oaCameraDevice* );
extern int		oaPGECloseCamera ( oaCamera* );

extern int		oaPGECameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaPGECameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaPGECameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaPGECameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int		oaPGECameraGetControlDiscreteSet ( oaCamera*, int,
				int32_t*, int64_t** );

extern int		oaPGECameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int), void* );
extern int		oaPGECameraStopStreaming ( oaCamera* );
extern int		oaPGECameraIsStreaming ( oaCamera* );

extern int		oaPGECameraSetResolution ( oaCamera*, int, int );
extern int		oaPGECameraSetROI ( oaCamera*, int, int );
extern int		oaPGECameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamPGEcontroller ( void* );
extern void*		oacamPGEcallbackHandler ( void* );

extern const FRAMESIZES* oaPGECameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaPGECameraGetFrameRates ( oaCamera*, int, int );
extern int		oaPGECameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaPGECameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaPGECameraGetMenuString ( oaCamera*, int, int );

struct pgeCtrl {
  fc2PropertyType	pgeControl;
  int			oaControl;
  int			oaAutoControl;
};

extern struct pgeCtrl pgeControls[];
extern unsigned int numPGEControls;

struct pgeFrameRate {
  fc2FrameRate		pgeFrameRate;
  int			numerator;
  int			denominator;
};

extern struct pgeFrameRate pgeFrameRates[];
extern unsigned int numPGEFrameRates;


#define		FC2_REG_DATA_DEPTH		0x0630
#define		FC2_REG_IMAGE_DATA_FORMAT	0x1048

extern fc2Error       ( *p_fc2Connect )( fc2Context, fc2PGRGuid* );
extern fc2Error       ( *p_fc2CreateGigEContext )( fc2Context* );
extern fc2Error       ( *p_fc2DestroyContext )( fc2Context );
extern fc2Error       ( *p_fc2DiscoverGigECameras )( fc2Context, fc2CameraInfo*,
                          unsigned int* );
extern fc2Error       ( *p_fc2GetCameraFromIndex )( fc2Context, unsigned int,
                          fc2PGRGuid* );
extern fc2Error       ( *p_fc2GetCameraInfo )( fc2Context, fc2CameraInfo* );
extern fc2Error       ( *p_fc2GetGigEImageBinningSettings )( fc2Context,
                          unsigned int*, unsigned int* );
extern fc2Error       ( *p_fc2GetGigEImageSettings )( fc2Context,
                          fc2GigEImageSettings* );
extern fc2Error       ( *p_fc2GetGigEImageSettingsInfo )( fc2Context,
                          fc2GigEImageSettingsInfo* );
extern fc2Error       ( *p_fc2GetInterfaceTypeFromGuid )( fc2Context,
                          fc2PGRGuid*, fc2InterfaceType* );
extern fc2Error       ( *p_fc2GetNumOfCameras )( fc2Context, unsigned int* );
extern fc2Error       ( *p_fc2GetProperty )( fc2Context, fc2Property* );
extern fc2Error       ( *p_fc2GetPropertyInfo )( fc2Context, fc2PropertyInfo* );
extern fc2Error       ( *p_fc2GetStrobe )( fc2Context, fc2StrobeControl* );
extern fc2Error       ( *p_fc2GetStrobeInfo )( fc2Context, fc2StrobeInfo* );
extern fc2Error       ( *p_fc2GetTriggerDelay )( fc2Context, fc2TriggerDelay* );
extern fc2Error       ( *p_fc2GetTriggerDelayInfo )( fc2Context,
                          fc2TriggerDelayInfo* );
extern fc2Error       ( *p_fc2GetTriggerMode )( fc2Context, fc2TriggerMode* );
extern fc2Error       ( *p_fc2GetTriggerModeInfo )( fc2Context,
                          fc2TriggerModeInfo* );
extern fc2Error       ( *p_fc2QueryGigEImagingMode )( fc2Context, fc2Mode,
                          BOOL* );
extern fc2Error       ( *p_fc2ReadRegister )( fc2Context, unsigned int,
                          unsigned int* );
extern fc2Error       ( *p_fc2SetGigEImageBinningSettings )( fc2Context,
                          unsigned int, unsigned int );
extern fc2Error       ( *p_fc2SetGigEImageSettings )( fc2Context,
                          const fc2GigEImageSettings* );
extern fc2Error       ( *p_fc2SetGigEImagingMode )( fc2Context, fc2Mode );
extern fc2Error       ( *p_fc2SetProperty )( fc2Context, fc2Property* );
extern fc2Error       ( *p_fc2SetStrobe )( fc2Context, fc2StrobeControl* );
extern fc2Error       ( *p_fc2SetTriggerDelay )( fc2Context, fc2TriggerDelay* );
extern fc2Error       ( *p_fc2SetTriggerMode )( fc2Context, fc2TriggerMode* );
extern fc2Error       ( *p_fc2StartCaptureCallback )( fc2Context,
                          fc2ImageEventCallback, void* );
extern fc2Error       ( *p_fc2StopCapture )( fc2Context );


#endif	/* OA_PGE_OACAM_H */
