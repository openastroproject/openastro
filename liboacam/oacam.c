/*****************************************************************************
 *
 * oacam.c -- main camera library entrypoint
 *
 * Copyright 2013,2014,2015,2016,2018,2019
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

#include <oa_common.h>

#if HAVE_LIMITS_H
#include <limits.h> 
#endif
#if HAVE_LIBFLYCAPTURE2
#include <flycapture/C/FlyCapture2_C.h>
#endif


#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/userConfig.h>

#include "oacamversion.h"
#include "oacamprivate.h"

#if HAVE_LIBV4L2
#include "v4l2/V4L2oacam.h"
#endif
#if HAVE_LIBDC1394
#include "iidc/IIDCoacam.h"
#endif
#include "pwc/PWCoacam.h"
#if HAVE_LIBASI2
#include "zwo/ZWASI2oacam.h"
#endif
#include "qhy/QHYoacam.h"
#if HAVE_LIBUVC
#include "uvc/UVCoacam.h"
#endif
#include "sx/SX.h"
#include "sx/SXstate.h"
#include "sx/SXoacam.h"
#include "euvc/EUVC.h"
#include "euvc/EUVCstate.h"
#include "euvc/EUVCoacam.h"
#if HAVE_LIBUDEV || HAVE_LIBFTDI
#include "atik/atikSerialoacam.h"
#endif
#if HAVE_LIBFLYCAPTURE2
#include "flycap2/FC2oacam.h"
#endif
#if HAVE_LIBTOUPCAM
#include "toupcam/oacam.h"
#endif
#if HAVE_LIBMALLINCAM
#include "mallincam/Mallincamoacam.h"
#endif
#if HAVE_LIBALTAIRCAM
#include "altair/Altairoacam.h"
#endif
#if HAVE_LIBALTAIRCAM_LEGACY
#include "altair-legacy/LegacyAltairoacam.h"
#endif
#if HAVE_LIBSTARSHOOTG
#include "starshootg/oacam.h"
#endif
#if HAVE_LIBNNCAM
#include "risingcam/oacam.h"
#endif
#if HAVE_LIBSPINNAKER
#include "spinnaker/Spinoacam.h"
#endif
#if HAVE_LIBQHYCCD
#include "qhyccd/qhyccdoacam.h"
#endif
#if HAVE_LIBGPHOTO2
#include "gphoto2/GP2oacam.h"
#endif


oaInterface	oaCameraInterfaces[] = {
  { 0, "", "", 0, OA_UDC_FLAG_NONE },
#if HAVE_LIBV4L2
  {
    OA_CAM_IF_V4L2,
    "Linux V4L2",
    "V4L2",
    oaV4L2GetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#ifdef	OA_PWC_DRIVER_SUPPORT
  {
    OA_CAM_IF_PWC,
    "Philips",
    "PWC",
    oaPWCGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
	// This used to be for the original ZWO ASI SDK
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#if HAVE_LIBQHYCCD
  {
    OA_CAM_IF_QHYCCD,
    "libqhyccd",
    "QHYCCD",
    oaQHYCCDGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  {
    OA_CAM_IF_QHY,
    "QHYCCD",
    "QHY",
    oaQHYGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#if HAVE_LIBDC1394
  {
    OA_CAM_IF_IIDC,
    "IIDC/DCAM",
    "IIDC",
    oaIIDCGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBUVC
  {
    OA_CAM_IF_UVC,
    "Userspace UVC",
    "UVC",
    oaUVCGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  {
    OA_CAM_IF_SX,
    "Starlight Xpress",
    "SX",
    oaSXGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#if HAVE_LIBUDEV || HAVE_LIBFTDI
  {
    OA_CAM_IF_ATIK_SERIAL,
    "Atik Serial",
    "Atik",
    oaAtikSerialGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBASI2
  {
    OA_CAM_IF_ZWASI2,
    "ZW Optical ASI v2",
    "ZWASI2",
    oaZWASI2GetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  {
    OA_CAM_IF_EUVC,
    "TIS EUVC",
    "EUVC",
    oaEUVCGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#if HAVE_LIBFLYCAPTURE2
  {
    OA_CAM_IF_FC2,
    "Point Grey Flycapture2",
    "FC2",
    oaFC2GetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBTOUPCAM
  {
    OA_CAM_IF_TOUPCAM,
    "Touptek",
    "TTEK",
    oaToupcamGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBMALLINCAM
  {
    OA_CAM_IF_MALLINCAM,
    "Mallincam",
    "MCAM",
    oaMallincamGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBALTAIRCAM
  {
    OA_CAM_IF_ALTAIRCAM,
    "Altair",
    "AACAM",
    oaAltairGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBALTAIRCAM_LEGACY
  {
    OA_CAM_IF_ALTAIRCAM_LEGACY,
    "Legacy Altair",
    "AALEG",
    oaAltairLegacyGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBSTARSHOOTG
  {
    OA_CAM_IF_STARSHOOTG,
    "Starshoot G",
    "SSG",
    oaStarshootGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBNNCAM
  {
    OA_CAM_IF_RISINGCAM,
    "Risingcam",
    "RCam",
    oaNncamGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBSPINNAKER
  {
    OA_CAM_IF_SPINNAKER,
    "Spinnaker",
    "SPIN",
    oaSpinGetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBGPHOTO2
  {
    OA_CAM_IF_GPHOTO2,
    "gphoto2",
    "GP2",
    oaGP2GetCameras,
    0,
    OA_UDC_FLAG_NONE
  },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  {
		0,
		"Dummy Camera",
		"",
		0,
		0,
		OA_UDC_FLAG_NONE
	},
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE }
};


char*               installPathRoot = 0;
static CAMERA_LIST	list;

int
oaGetCameras( oaCameraDevice*** deviceList, unsigned long featureFlags )
{
  int           i, err;

  list.cameraList = 0;
  list.numCameras = list.maxCameras = 0;

  for ( i = 0; i < OA_CAM_IF_COUNT; i++ ) {
    if ( oaCameraInterfaces[i].interfaceType ) {
      if (( err = oaCameraInterfaces[i].enumerate ( &list, featureFlags,
          oaCameraInterfaces[i].flags )) < 0 ) {
				if ( err != OA_ERR_LIBRARY_NOT_FOUND && err !=
						OA_ERR_SYMBOL_NOT_FOUND ) {
					_oaFreeCameraDeviceList ( &list );
					list.numCameras = 0;
					list.cameraList = 0;
					return err;
				}
      }
    }
  }
  
  *deviceList = list.cameraList;
  return list.numCameras;
}


void
oaReleaseCameras ( oaCameraDevice** deviceList )
{
  // This is a bit cack-handed because we don't know from the data
  // passed in how many cameras were found last time so we have to
  // consult a static global instead.

  _oaFreeCameraDeviceList ( &list );
  list.numCameras = 0;
  list.cameraList = 0;
  return;
}


unsigned int
oaGetCameraAPIVersion ( void )
{
  unsigned int v;

  v = ( OACAM_MAJOR_VERSION << 16 ) + ( OACAM_MINOR_VERSION << 8 ) +
      OACAM_REVISION;
  return v;
}


const char*
oaGetCameraAPIVersionStr ( void )
{
  static char vs[ 40 ];

  snprintf ( vs, 40, "%d.%d.%d", OACAM_MAJOR_VERSION, OACAM_MINOR_VERSION,
      OACAM_REVISION );
  return vs;
}


void
oaSetCameraDebugLevel ( int v )
{
  oacamSetDebugLevel ( v );
}
