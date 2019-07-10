/*****************************************************************************
 *
 * outputFITS.cc -- FITS output class
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

#include <oa_common.h>

extern "C" {
#ifdef HAVE_FITSIO_H
#include "fitsio.h"
#else
#ifdef HAVE_CFITSIO_FITSIO_H
#include "cfitsio/fitsio.h"
#endif
#endif

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/video/formats.h>
}

#include "commonState.h"
#include "targets.h"
#include "trampoline.h"
#include "outputHandler.h"
#include "outputFITS.h"
#include "fitsSettings.h"


OutputFITS::OutputFITS ( int x, int y, int n, int d, int fmt,
		const char *appName, const char* appVer, QString fileTemplate,
		trampolineFuncs* trampolines ) :
		OutputHandler ( x, y, n, d, fileTemplate, trampolines ),
		applicationName ( appName ), applicationVersion ( appVer ),
		imageFormat ( fmt )
{
  uint16_t byteOrderTest = 0x1234;
  uint8_t* firstByte;

  firstByte = ( uint8_t* ) &byteOrderTest;

  writesDiscreteFiles = 1;
  frameCount = 0;
  xSize = x;
  ySize = y;
  fullSaveFilePath = "";
  validFileType = 1;
  reverseByteOrder = 0;
  nAxes = 3;
  bitpix = 0;
  tableType = 0;
  swapRedBlue = 0;
  bytesPerPixel = 1;
  splitPlanes = 0;
  writeBuffer = 0;
  elements = 0;

  switch ( fmt ) {

    case OA_PIX_FMT_GREY8:
    case OA_PIX_FMT_BGGR8:
    case OA_PIX_FMT_RGGB8:
    case OA_PIX_FMT_GBRG8:
    case OA_PIX_FMT_GRBG8:
      bitpix = BYTE_IMG;
      tableType = TBYTE;
      nAxes = 2;
      planeDepth = 1;
      break;

    case OA_PIX_FMT_BGR24:
      swapRedBlue = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGB24:
      bitpix = BYTE_IMG;
      tableType = TBYTE;
      nAxes = 3;
      splitPlanes = 1;
      planeDepth = 1;
      bytesPerPixel = 3;
      break;

    case OA_PIX_FMT_GREY16LE:
    case OA_PIX_FMT_BGGR16LE:
    case OA_PIX_FMT_RGGB16LE:
    case OA_PIX_FMT_GBRG16LE:
    case OA_PIX_FMT_GRBG16LE:
      bitpix = USHORT_IMG;
      nAxes = 2;
      if ( *firstByte == 0x12 ) {
        reverseByteOrder = 1;
      }
      tableType = TUSHORT;
      bytesPerPixel = 2;
      planeDepth = 2;
      break;

    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_BGGR16BE:
    case OA_PIX_FMT_RGGB16BE:
    case OA_PIX_FMT_GBRG16BE:
    case OA_PIX_FMT_GRBG16BE:
      bitpix = USHORT_IMG;
      nAxes = 2;
      if ( *firstByte == 0x34 ) {
        reverseByteOrder = 1;
      }
      tableType = TUSHORT;
      bytesPerPixel = 2;
      planeDepth = 2;
      break;

#ifdef RGB48_FITS
    case OA_PIX_FMT_BGR48BE:
      swapRedBlue = 1;
    case OA_PIX_FMT_RGB48BE:
      bitpix = LONG_IMG;
      nAxes = 3;
      if ( *firstByte == 0x34 ) {
        reverseByteOrder = 1;
      }
      tableType = TULONG;
      bytesPerPixel = 6;
      splitPlanes = 1;
      planeDepth = 4;
      break;

    case OA_PIX_FMT_BGR48LE:
      swapRedBlue = 1;
    case OA_PIX_FMT_RGB48LE:
      bitpix = LONG_IMG;
      nAxes = 3;
      if ( *firstByte == 0x12 ) {
        reverseByteOrder = 1;
      }
      tableType = TULONG;
      bytesPerPixel = 4;
      splitPlanes = 1;
      planeDepth = 4;
      break;
#endif

    default:
      validFileType = 0;
      break;
  }

  if ( validFileType ) {
    elements = xSize * ySize;
    frameSize = elements * bytesPerPixel;
    planeSize = fitsSize = elements * planeDepth;

    fitsAxes[0] = xSize;
    fitsAxes[1] = ySize;
    if ( nAxes == 3 ) {
      fitsAxes[2] = 3;
      fitsSize *= 3;
    }
  }
}


OutputFITS::~OutputFITS()
{
  // Probably nothing to do here for FITS
}


int
OutputFITS::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".fits";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputFITS::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".fits";
  }

  QString testPath = fullSaveFilePath;
  int slashPos;

  if (( slashPos = testPath.lastIndexOf ( "/", -1 )) == -1 ) {
    testPath = ".";
  } else {
    testPath = testPath.left ( slashPos + 1 );
  }
 
  // FIX ME -- what if this returns an error?
  return ( access ( testPath.toStdString().c_str(), W_OK )) ? 0 : 1;
}


int
OutputFITS::openOutput ( void )
{
  if ( validFileType ) {
    if ( reverseByteOrder || swapRedBlue || nAxes == 3 ) {
      if (!( writeBuffer = ( unsigned char* ) malloc ( fitsSize ))) {;
        qWarning() << "write buffer allocation failed";
        return -1;
      }
    }
  }

  return !validFileType;
}


int
OutputFITS::addFrame ( void* frame, const char* constTimestampStr,
    int64_t expTime, const char* commentStr, FRAME_METADATA* metadata )
{
  unsigned char* s;
  unsigned char* t;
  int i, status = 0;
  fitsfile* fptr;
  void* outputBuffer = frame;
  char stringBuff[FLEN_VALUE+1];
  float pixelSize = 0;
  int xorg, yorg;
  // Hack to get around older versions of library using char* rather
  // than const char*
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
  const char* timestampStr = constTimestampStr;
  const char* cString = stringBuff;
#else
  char stampStr[FLEN_VALUE+1];
  char *timestampStr;
  char *cString = stringBuff;

  if ( constTimestampStr ) {
    ( void ) strcpy ( stampStr, constTimestampStr );
    timestampStr = stampStr;
  } else {
    timestampStr = 0;
  }
#endif

  filenameRoot = getNewFilename();
  fullSaveFilePath = filenameRoot + ".fits";

  // swap byte orders if we need to
  // swap R and B if we need to
  // I've done this in for separate loops to avoid tests inside the loops
  // FIX ME -- This code is also in outputTIFF.cc.  I should refactor it
  // somewhere

  s = ( unsigned char* ) frame;
  t = writeBuffer;

  if ( 2 == bytesPerPixel ) {
    if ( reverseByteOrder ) {
      for ( i = 0; i < frameSize; i += 2, s += 2 ) {
        *t++ = *( s + 1 );
        *t++ = *s;
      }
			outputBuffer = writeBuffer;
    }
  }

  if ( 3 == bytesPerPixel ) { // RGB or BGR
    unsigned char* redPlane;
    unsigned char* greenPlane;
    unsigned char* bluePlane;

    redPlane = t;
    greenPlane = redPlane + planeSize;
    bluePlane = greenPlane + planeSize;
    if ( swapRedBlue ) { // BGR
      for ( i = 0; i < frameSize; i += 3 ) {
        *bluePlane++ = *s++;
        *greenPlane++ = *s++;
        *redPlane++ = *s++;
      }
    } else { // RGB
      for ( i = 0; i < frameSize; i += 3 ) {
        *redPlane++ = *s++;
        *greenPlane++ = *s++;
        *bluePlane++ = *s++;
      }
    }
    outputBuffer = writeBuffer;
  }

  if ( fits_create_file ( &fptr, fullSaveFilePath.toStdString().c_str(),
      &status )) {
    if ( status ) {
      fits_report_error ( stderr, status );
    }
    frameCount++;
    return -1;
  }

  if ( fits_create_img ( fptr, bitpix, nAxes, fitsAxes, &status )) {
    if ( status ) {
      fits_report_error ( stderr, status );
    }
    frameCount++;
    return -1;
  }

  // FIX ME -- should check the status of all these fits_write_key calls

  /*
   * KEYWORD:   DATE-OBS
   * DEFINITION: The date of the observation, in the format specified in the
   * FITS Standard.  The old date format was 'yy/mm/dd' and may be used only
   * for dates from 1900 through 1999.  The new Y2K compliant date format is
   * 'yyyy-mm-dd' or 'yyyy-mm-ddTHH:MM:SS[.sss]'.
   */

  if ( timestampStr ) {
    fits_write_key_str ( fptr, "DATE-OBS", timestampStr, "", &status );
  } else {
    QDateTime now = QDateTime::currentDateTimeUtc();
    // QString dateStr = now.toString ( Qt::ISODate );
    QString dateStr = now.toString ( "yyyy-MM-ddThh:mm:ss.zzz" );
    ( void ) strncpy ( stringBuff,
        dateStr.toStdString().c_str(), FLEN_VALUE+1 );
    fits_write_key_str ( fptr, "DATE-OBS", cString, "UTC", &status );
  }

  fits_write_date ( fptr, &status );

  if ( fitsConf.observer != "" ) {
    ( void ) strncpy ( stringBuff,
        fitsConf.observer.toStdString().c_str(), FLEN_VALUE+1 );
    fits_write_key_str ( fptr, "OBSERVER", cString, "", &status );
  }

  stringBuff[0] = 0;
  int currentTargetId = trampolines->getCurrentTargetId();
  if ( currentTargetId > 0 && currentTargetId != TGT_UNKNOWN ) {
    ( void ) strncpy ( stringBuff,
				targetName ( currentTargetId ).toStdString().c_str(),
        FLEN_VALUE+1 );
  } else {
    ( void ) strncpy ( stringBuff,
        fitsConf.object.toStdString().c_str(), FLEN_VALUE+1 );
  }
  if ( stringBuff[0]) {
    fits_write_key_str ( fptr, "OBJECT", cString, "", &status );
  }

  if ( fitsConf.telescope != "" ) {
    ( void ) strncpy ( stringBuff,
        fitsConf.telescope.toStdString().c_str(), FLEN_VALUE+1 );
    fits_write_key_str ( fptr, "TELESCOP", cString, "", &status );
  }

  if ( fitsConf.instrument != "" ) {
    ( void ) strncpy ( stringBuff,
        fitsConf.instrument.toStdString().c_str(), FLEN_VALUE+1 );
    fits_write_key_str ( fptr, "INSTRUME", cString, "", &status );
  }

  if ( fitsConf.comment != "" ) {
    ( void ) strncpy ( stringBuff,
        fitsConf.comment.toStdString().c_str(), FLEN_VALUE+1 );
    fits_write_comment ( fptr, cString, &status );
  }

  if ( commentStr && *commentStr ) {
    fits_write_comment ( fptr, commentStr, &status );
  }

  snprintf ( stringBuff, FLEN_VALUE, "Input Frame Format: %s (%s)",
      oaFrameFormats[ imageFormat ].name,
      oaFrameFormats[ imageFormat ].simpleName );
  fits_write_comment ( fptr, stringBuff, &status );

  if ( fitsConf.focalLength != "" ) {
    fits_write_key_lng ( fptr, "FOCALLEN", fitsConf.focalLength.toInt(),
        "", &status );
  }

  if ( fitsConf.apertureDia != "" ) {
    fits_write_key_lng ( fptr, "APTDIA", fitsConf.apertureDia.toInt(),
        "", &status );
  }

  if ( fitsConf.apertureArea != "" ) {
    fits_write_key_lng ( fptr, "APTAREA", fitsConf.apertureArea.toInt(),
        "", &status );
  }

  /*
   * XPIXSZ and YPIXSZ are supposed to take account of binning.  If the
   * defaults are set then we'll assume the user knows what they're doing,
   * otherwise we deal with it ourselves
   */
  
  if ( fitsConf.pixelSizeX != "" ) {
    pixelSize = fitsConf.pixelSizeX.toFloat();
  } else {
    int binMultiplier = 1;
    if ( commonState.binningValid ) {
      binMultiplier = OA_BIN_MODE_MULTIPLIER ( commonState.binModeX );
    }
    pixelSize = commonState.camera->pixelSizeX() * binMultiplier / 1000.0;
  }
  if ( pixelSize ) {
    fits_write_key_dbl ( fptr, "XPIXSZ", pixelSize, -5, "", &status );
  }

  if ( fitsConf.pixelSizeY != "" ) {
    pixelSize = fitsConf.pixelSizeY.toFloat();
  } else {
    int binMultiplier = 1;
    if ( commonState.binningValid ) {
      binMultiplier = OA_BIN_MODE_MULTIPLIER ( commonState.binModeY );
    }
    pixelSize = commonState.camera->pixelSizeY() * binMultiplier / 1000.0;
  }
  if ( pixelSize ) {
    fits_write_key_dbl ( fptr, "YPIXSZ", pixelSize, -5, "", &status );
  }


  if ( fitsConf.subframeOriginX != "" ) {
    xorg = fitsConf.subframeOriginX.toInt();
  } else {
    if ( commonState.cropMode ) {
      xorg = ( commonState.sensorSizeX - commonState.cropSizeX ) / 2;
    } else {
      xorg = ( commonState.sensorSizeX - xSize ) / 2;
    }
  }
  fits_write_key_lng ( fptr, "XORGSUBF", xorg, "", &status );

  if ( fitsConf.subframeOriginY != "" ) {
    yorg = fitsConf.subframeOriginY.toInt();
  } else {
    if ( commonState.cropMode ) {
      yorg = ( commonState.sensorSizeY - commonState.cropSizeY ) / 2;
    } else {
      yorg = ( commonState.sensorSizeY - ySize ) / 2;
    }
  }
  fits_write_key_lng ( fptr, "YORGSUBF", yorg, "", &status );

  QString currentFilter = trampolines->getCurrentFilterName();
  if ( fitsConf.filter != "" ) {
    currentFilter = fitsConf.filter;
  }
  if ( currentFilter != "" ) {
    ( void ) strncpy ( stringBuff,
        currentFilter.toStdString().c_str(), FLEN_VALUE+1 );
    fits_write_key_str ( fptr, "FILTER", cString, "", &status );
  }

  stringBuff[0] = 0;
  if ( commonState.gpsValid ) {
    ( void ) sprintf ( stringBuff, "%+8.6e", commonState.latitude );
  }
  if ( !stringBuff[0] && fitsConf.siteLatitude != "" ) {
    ( void ) strncpy ( stringBuff,
        fitsConf.siteLatitude.toStdString().c_str(), FLEN_VALUE+1 );
  }
  if ( stringBuff[0] ) {
    fits_write_key_str ( fptr, "SITELAT", cString, "", &status );
  }

  stringBuff[0] = 0;
  if ( commonState.gpsValid ) {
    ( void ) sprintf ( stringBuff, "%+8.6e", commonState.longitude );
  }
  if ( !stringBuff[0] && fitsConf.siteLongitude != "" ) {
    ( void ) strncpy ( stringBuff,
        fitsConf.siteLongitude.toStdString().c_str(), FLEN_VALUE+1 );
  }
  if ( stringBuff[0] ) {
    fits_write_key_str ( fptr, "SITELONG", cString, "", &status );
  }

	stringBuff[0] = 0;
  if ( commonState.gpsValid ) {
    ( void ) sprintf ( stringBuff, "%+8.6e", commonState.altitude );
    fits_write_key_str ( fptr, "SITEELEV", cString, "", &status );
    ( void ) sprintf ( stringBuff, "%g", commonState.altitude );
    fits_write_key_str ( fptr, "ELEVATIO", cString, "", &status );
  }

	( void ) snprintf ( stringBuff, FLEN_VALUE, "%s %s", applicationName,
			applicationVersion );
  fits_write_key_str ( fptr, "SWCREATE", cString, "", &status );

  // fits_write_key_dbl ( fptr, "BSCALE", 1.0, -5, "", &status );
  // fits_write_key_dbl ( fptr, "BZERO", 0.0, -5, "", &status );

  fits_write_key_dbl ( fptr, "EXPTIME", expTime / 1000000.0, -10, "", &status );

  if ( oaFrameFormats[ imageFormat ].rawColour ) {
    long xoff = 0, yoff = 0;
    // "Bayer" format is GRBG, so all the other formats are offset in some
    // manner from that
    switch ( oaFrameFormats[ imageFormat ].cfaPattern ) {
      case OA_DEMOSAIC_BGGR:
        xoff = 0;
        yoff = 1;
        break;
      case OA_DEMOSAIC_RGGB:
        xoff = 1;
        yoff = 0;
        break;
      case OA_DEMOSAIC_GBRG:
        xoff = 1;
        yoff = 1;
        break;
    }
    fits_write_key_str ( fptr, "BAYERPAT", "TRUE", "", &status );
    fits_write_key_lng ( fptr, "XBAYROFF", xoff, "", &status );
    fits_write_key_lng ( fptr, "YBAYROFF", yoff, "", &status );
  }

  if ( commonState.cameraTempValid ) {
    fits_write_key_dbl ( fptr, "CCD-TEMP", commonState.cameraTemp, -5, "",
				&status );
  }

  if ( commonState.binningValid ) {
    fits_write_key_lng ( fptr, "XBINNING", commonState.binModeX, "",
				&status );
    fits_write_key_lng ( fptr, "YBINNING", commonState.binModeY, "",
				&status );
  }

	if ( metadata && metadata->frameCounterValid ) {
		fits_write_key_lng ( fptr, "FRAMESEQ", metadata->frameCounter, "",
				&status );
	}

  if ( fits_write_img ( fptr, tableType, 1, elements * ( nAxes == 3 ? 3 : 1 ),
      outputBuffer, &status )) {
    if ( status ) {
      fits_report_error ( stderr, status );
    }
    frameCount++;
    return -1;
  }

  if ( fits_close_file ( fptr, &status )) {
    if ( status ) {
      fits_report_error ( stderr, status );
    }
    frameCount++;
    return -1;
  }

  commonState.captureIndex++;
  frameCount++;
  return 0;
}


void
OutputFITS::closeOutput ( void )
{
  if ( writeBuffer ) {
    ( void ) free ( writeBuffer );
  }
}
