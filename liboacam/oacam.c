/*****************************************************************************
 *
 * oacam.c -- main camera library entrypoint
 *
 * Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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
#include "V4L2oacam.h"
#endif
#if HAVE_LIBDC1394
#include "IIDCoacam.h"
#endif
#include "PWCoacam.h"
#if HAVE_LIBASI
#include "ZWASIoacam.h"
#endif
#if HAVE_LIBASI2
#include "ZWASI2oacam.h"
#endif
#include "QHYoacam.h"
#if HAVE_LIBUVC
#include "UVCoacam.h"
#endif
#include "SX.h"
#include "SXstate.h"
#include "SXoacam.h"
#include "EUVC.h"
#include "EUVCstate.h"
#include "EUVCoacam.h"
#if HAVE_LIBUDEV || HAVE_LIBFTDI
#include "atikSerialoacam.h"
#endif
#if HAVE_LIBFLYCAPTURE2
#include "PGEoacam.h"
#endif
#if HAVE_LIBTOUPCAM
#include "Touptekoacam.h"
#endif
#if HAVE_LIBMALLINCAM
#include "Mallincamoacam.h"
#endif
#if HAVE_LIBALTAIRCAM
#include "Altairoacam.h"
#endif


oaInterface	oaCameraInterfaces[] = {
  { 0, "", "", 0, OA_UDC_FLAG_NONE },
#if HAVE_LIBV4L2
  { OA_CAM_IF_V4L2, "Linux V4L2", "V4L2", oaV4L2GetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#ifdef	OA_PWC_DRIVER_SUPPORT
  { OA_CAM_IF_PWC, "Philips", "PWC", oaPWCGetCameras, 0, OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBASI
  { OA_CAM_IF_ZWASI, "ZW Optical ASI", "ZWASI", oaZWASIGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  { OA_CAM_IF_QHY, "QHYCCD", "QHY", oaQHYGetCameras, 0, OA_UDC_FLAG_NONE },
#if HAVE_LIBDC1394
  { OA_CAM_IF_IIDC, "IIDC/DCAM", "IIDC", oaIIDCGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBUVC
  { OA_CAM_IF_UVC, "Userspace UVC", "UVC", oaUVCGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  { OA_CAM_IF_SX, "Starlight Xpress", "SX", oaSXGetCameras, 0,
      OA_UDC_FLAG_NONE },
#if HAVE_LIBUDEV || HAVE_LIBFTDI
  { OA_CAM_IF_ATIK_SERIAL, "Atik Serial", "Atik", oaAtikSerialGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBASI2
  { OA_CAM_IF_ZWASI2, "ZW Optical ASI v2", "ZWASI2", oaZWASI2GetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  { OA_CAM_IF_EUVC, "TIS EUVC", "EUVC", oaEUVCGetCameras, 0, OA_UDC_FLAG_NONE },
#if HAVE_LIBFLYCAPTURE2
  { OA_CAM_IF_PGE, "Point Grey Gig-E", "PGE", oaPGEGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBTOUPCAM
  { OA_CAM_IF_TOUPCAM, "Touptek", "TTEK", oaTouptekGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBMALLINCAM
  { OA_CAM_IF_MALLINCAM, "Mallincam", "MCAM", oaMallincamGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
#if HAVE_LIBALTAIRCAM
  { OA_CAM_IF_ALTAIRCAM, "Altair", "AACAM", oaAltairGetCameras, 0,
      OA_UDC_FLAG_NONE },
#else
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE },
#endif
  { 0, "", "", 0, 0, OA_UDC_FLAG_NONE }
};


char*	installPathRoot = 0;

int
oaGetCameras( oaCameraDevice*** deviceList )
{
  int           i, err;
  CAMERA_LIST	list;

  list.cameraList = 0;
  list.numCameras = list.maxCameras = 0;

  for ( i = 0; i < OA_CAM_IF_COUNT; i++ ) {
    if ( oaCameraInterfaces[i].interfaceType ) {
      if (( err = oaCameraInterfaces[i].enumerate ( &list,
          oaCameraInterfaces[i].flags )) < 0 ) {
        return err;
      }
    }
  }
  
  *deviceList = list.cameraList;
  return list.numCameras;
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
