/*****************************************************************************
 *
 * camera.h -- camera API header
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

#ifndef OPENASTRO_CAMERA_H
#define OPENASTRO_CAMERA_H

#include <stdint.h>
#include <openastro/openastro.h>
#include <openastro/camera/controls.h>
#include <openastro/camera/features.h>
#include <openastro/video/formats.h>

enum oaCameraInterfaceType {
  OA_CAM_IF_V4L2			= 1,
  OA_CAM_IF_PWC				= 2,
  OA_CAM_IF_ZWASI			= 3,
  OA_CAM_IF_QHYCCD			= 4,
  OA_CAM_IF_QHY				= 5,
  OA_CAM_IF_IIDC			= 6,
  OA_CAM_IF_UVC				= 7,
  OA_CAM_IF_SX				= 8,
  OA_CAM_IF_ATIK_SERIAL			= 9,
  OA_CAM_IF_ZWASI2			= 10,
  OA_CAM_IF_EUVC			= 11,
  OA_CAM_IF_FC2				= 12,
  OA_CAM_IF_TOUPCAM			= 13,
  OA_CAM_IF_MALLINCAM			= 14,
  OA_CAM_IF_ALTAIRCAM			= 15,
  OA_CAM_IF_ALTAIRCAM_LEGACY			= 16,
	OA_CAM_IF_STARSHOOTG		= 17,
	OA_CAM_IF_RISINGCAM		= 18,
  OA_CAM_IF_SPINNAKER			= 19,
  OA_CAM_IF_GPHOTO2			= 20,
  OA_CAM_IF_DUMMY			= 21,
  OA_CAM_IF_COUNT			= 22
};

extern oaInterface	oaCameraInterfaces[ OA_CAM_IF_COUNT + 1 ];

typedef struct FRAMESIZE {
  unsigned int		x;
  unsigned int		y;
} FRAMESIZE;

typedef struct FRAMESIZES {
  unsigned int		numSizes;
  FRAMESIZE*		sizes;
} FRAMESIZES;

typedef struct FRAMERATE {
  int			numerator;
  int			denominator;
} FRAMERATE;

typedef struct FRAMERATES {
  unsigned int		numRates;
  FRAMERATE*		rates;
} FRAMERATES;

typedef struct FRAME_METADATA {
	unsigned int		frameCounterValid : 1;
	unsigned int		frameCounter;
} FRAME_METADATA;

struct oaCamera;
struct oaCameraDevice;

typedef struct oaCameraFuncs {
  struct oaCamera* ( *initCamera )( struct oaCameraDevice* );
  int              ( *closeCamera )( struct oaCamera* );

//int              ( *resetCamera )( struct oaCamera* );

  int              ( *readControl )( struct oaCamera*, int, oaControlValue* );
  int              ( *setControl )( struct oaCamera*, int, oaControlValue*,
                       int );
  int              ( *testControl )( struct oaCamera*, int, oaControlValue* );
  int              ( *getControlRange )( struct oaCamera*, int, int64_t*,
                       int64_t*, int64_t*, int64_t* );
  int              ( *getControlDiscreteSet )( struct oaCamera*, int, int32_t*,
                       int64_t** );

  int              ( *startStreaming )( struct oaCamera*,
                       void* (*)(void*, void*, int, void* ), void* );
  int              ( *stopStreaming )( struct oaCamera* );
  int              ( *isStreaming )( struct oaCamera* );

//int              ( *hasLoadableFirmware )( struct oaCamera* );
//int              ( *isFirmwareLoaded )( struct oaCamera* );
//int              ( *loadFirmware )( struct oaCamera* );

  int              ( *setResolution )( struct oaCamera*, int, int );
  int              ( *setROI )( struct oaCamera*, int, int );
  int              ( *setFrameInterval )( struct oaCamera*, int, int );

  const FRAMESIZES* ( *enumerateFrameSizes )( struct oaCamera* );
  const FRAMERATES* ( *enumerateFrameRates )( struct oaCamera*, int, int );
  int              ( *getFramePixelFormat )( struct oaCamera* );
  int              ( *testROISize )( struct oaCamera*, unsigned int,
                       unsigned int, unsigned int*, unsigned int* );

  const char*      ( *getMenuString )( struct oaCamera*, int, int );
  int              ( *getAutoWBManualSetting )( struct oaCamera* );
  int              ( *hasAuto )( struct oaCamera*, int );
  int              ( *isAuto )( struct oaCamera*, int );

	int								( *startExposure )( struct oaCamera*, time_t,
                       void* (*)(void*, void*, int, void* ), void* );
	int								( *abortExposure )( struct oaCamera* );
} oaCameraFuncs;

typedef struct oaCamera {
  enum oaCameraInterfaceType	interface;
  unsigned char			cameraInterface;
  unsigned long			cameraInterfaceInfo;
  char				deviceName[OA_MAX_NAME_LEN+1];
  oaCameraFuncs			funcs;
  uint8_t			controlType[OA_CAM_CTRL_MODIFIERS_P1][OA_CAM_CTRL_LAST_P1];
  uint8_t			frameFormats[ OA_PIX_FMT_LAST_P1 ];
  oaCameraFeatures		features;
  void*				_common;
  void*				_private;
} oaCamera;

typedef struct oaCameraDevice {
  enum oaCameraInterfaceType	interface;
  char				deviceName[OA_MAX_NAME_LEN+1];
  oaCamera*			( *initCamera )( struct oaCameraDevice* );
  int           		( *loadFirmware )( struct oaCameraDevice* );
  unsigned int			hasLoadableFirmware;
  unsigned int			firmwareLoaded;
  unsigned char			cameraInterface;
  unsigned long			cameraInterfaceInfo;
  void*				_private;
} oaCameraDevice;

extern int		oaGetCameras ( oaCameraDevice***, unsigned long );
extern void		oaReleaseCameras ( oaCameraDevice** );
extern unsigned		oaGetCameraAPIVersion ( void );
extern const char*	oaGetCameraAPIVersionStr ( void );
extern void		oaSetCameraDebugLevel ( int );
extern int		oaGetAutoForControl ( int );
extern int		oaGetControlForAuto ( int );
extern int		oaIsAuto ( int );
extern void		oaSetRootPath ( const char* );

// FIX ME -- These may no longer be required

#define OA_FRAMESIZES_DISCRETE		1
#define OA_FRAMESIZES_CONTINUOUS	2
#define OA_FRAMESIZES_STEPWISE		3

#define OA_FRAMERATES_DISCRETE		1
#define OA_FRAMERATES_CONTINUOUS	2
#define OA_FRAMERATES_STEPWISE		3

#define	OA_BIN_MODE_NONE		1
#define	OA_BIN_MODE_2x2			2
#define	OA_BIN_MODE_3x3			3
#define	OA_BIN_MODE_4x4			4
#define OA_BIN_MODE_MULTIPLIER(x)	(x)

// Values for cameraInterface

#define OA_IF_UNKNOWN           0
#define OA_IF_USB               1
#define OA_IF_FIREWIRE          2
#define OA_IF_GIGE              3

// Values used in cameraInterfaceInfo

#define OA_IF_INFO_USB_UNKNOWN  0
#define OA_IF_INFO_USB1         1
#define OA_IF_INFO_USB2         2
#define OA_IF_INFO_USB3         3

#define OA_IF_USBSPEED_UNKNOWN  0
#define OA_IF_USBSPEED_LOW      1
#define OA_IF_USBSPEED_FULL     2
#define OA_IF_USBSPEED_HIGH     3
#define OA_IF_USBSPEED_SUPER    4

#define OA_MAX_BINNING		4

#endif	/* OPENASTRO_CAMERA_H */
