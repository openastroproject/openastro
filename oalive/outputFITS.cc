/*****************************************************************************
 *
 * outputFITS.cc -- FITS output class
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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
}

#include "outputHandler.h"
#include "outputFITS.h"
#include "configuration.h"
#include "state.h"

/*
 * FITS RGB -- naxes = 3, 1 = R, 2 = G, 3 = B
 */

OutputFITS::OutputFITS ( int x, int y, int n, int d, int fmt,
    QString fileTemplate ) : OutputHandler ( x, y, n, d, fileTemplate )
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
  splitPlanes = 1;
  writeBuffer = 0;
  elements = 0;

  switch ( fmt ) {

    case OA_PIX_FMT_GREY8:
      bitpix = BYTE_IMG;
      tableType = TBYTE;
      nAxes = 2;
      planeDepth = 1;
      break;

#ifdef RGB_FITS
    case OA_PIX_FMT_BGR24:
      swapRedBlue = 1;
    case OA_PIX_FMT_RGB24:
      bitpix = BYTE_IMG;
      tableType = TBYTE;
      nAxes = 3;
      splitPlanes = 1;
      planeDepth = 1;
      break;
#endif

    case OA_PIX_FMT_GREY16LE:
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
      bitpix = USHORT_IMG;
      nAxes = 2;
      if ( *firstByte == 0x34 ) {
        reverseByteOrder = 1;
      }
      tableType = TUSHORT;
      bytesPerPixel = 2;
      planeDepth = 2;
      break;

#ifdef RGB_FITS
    case OA_PIX_FMT_BGR48BE:
      swapRedBlue = 1;
    case OA_PIX_FMT_RGB48BE:
      bitpix = LONG_IMG;
      nAxes = 3;
      if ( *firstByte == 0x34 ) {
        reverseByteOrder = 1;
      }
      tableType = TULONG;
      bytesPerPixel = 4;
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
    frameSize = xSize * ySize * bytesPerPixel;
    rowLength = xSize * planeDepth;
    totalRows = ySize;
    if ( nAxes == 3 ) {
      totalRows *= 3;
    }

    fitsAxes[0] = xSize;
    fitsAxes[1] = ySize;
    if ( nAxes == 3 ) {
      fitsAxes[2] = 3;
    }
  }

qWarning() << "valid file type" << validFileType;
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

  if (( slashPos = testPath.indexOf ( "/", -1 )) == -1 ) {
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
    if ( reverseByteOrder || swapRedBlue ) {
      if (!( writeBuffer = ( unsigned char* ) malloc ( frameSize ))) {;
        qWarning() << "write buffer allocation failed";
        return -1;
      }
    }
  }

  return !validFileType;
}


int
OutputFITS::addFrame ( void* frame, const char* constTimestampStr )
{
  unsigned char* s;
  unsigned char* t;
  int i, status = 0;
  fitsfile* fptr;
  // Hack to get around older versions of library using const* rather
  // than const char*
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
  const char* timestampStr = constTimestampStr;
  const char* tempStr;
#else
  char stampStr[FLEN_VALUE+1];
  char *timestampStr;
  char tempStr[FLEN_VALUE+1];

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
#ifdef RGB_FITS
      if ( swapRedBlue ) {
        for ( i = 0; i < frameSize; i += 6, s += 6 ) {
          *t++ = *( s + 5 );
          *t++ = *( s + 4 );
          *t++ = *( s + 3 );
          *t++ = *( s + 2 );
          *t++ = *( s + 1 );
          *t++ = *s;
        }
      } else {
#endif
        for ( i = 0; i < frameSize; i += 2, s += 2 ) {
          *t++ = *( s + 1 );
          *t++ = *s;
        }
#ifdef RGB_FITS
      }
    } else {
      if ( swapRedBlue ) {
        for ( i = 0; i < frameSize; i += 6, s += 6 ) {
          *t++ = *( s + 4 );
          *t++ = *( s + 5 );
          *t++ = *( s + 2 );
          *t++ = *( s + 3 );
          *t++ = *s;
          *t++ = *( s + 1 );
        }
      }
#endif
    }
  }

#ifdef RGB_FITS
  if ( 1 == bytesPerPixel ) {
    if ( swapRedBlue ) {
      for ( i = 0; i < frameSize; i += 3, s += 3 ) {
        *t++ = *( s + 2 );
        *t++ = *( s + 1 );
        *t++ = *s;
      }
    }
  }
#endif

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
    QDateTime now = QDateTime::currentDateTime();
    QString dateStr = now.toString ( Qt::ISODate );
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
    tempStr = dateStr.toStdString().c_str();
#else
    strncpy ( tempStr, dateStr.toStdString().c_str(), FLEN_VALUE + 1 );
#endif
    fits_write_key_str ( fptr, "DATE-OBS", tempStr, "", &status );
  }

  fits_write_date ( fptr, &status );

  if ( config.fitsObserver != "" ) {
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
    tempStr = config.fitsObserver.toStdString().c_str();
#else
    strncpy ( tempStr, config.fitsObserver.toStdString().c_str(),
        FLEN_VALUE + 1 );
#endif
    fits_write_key_str ( fptr, "OBSERVER", tempStr, "", &status );
  }
  if ( config.fitsObject != "" ) {
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
    tempStr = config.fitsObject.toStdString().c_str();
#else
    strncpy ( tempStr, config.fitsObject.toStdString().c_str(),
        FLEN_VALUE + 1 );
#endif
    fits_write_key_str ( fptr, "OBJECT", tempStr, "", &status );
  }
  if ( config.fitsTelescope != "" ) {
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
    tempStr = config.fitsTelescope.toStdString().c_str();
#else
    strncpy ( tempStr, config.fitsTelescope.toStdString().c_str(),
        FLEN_VALUE + 1 );
#endif
    fits_write_key_str ( fptr, "TELESCOP", tempStr, "", &status );
  }
  if ( config.fitsInstrument != "" ) {
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
    tempStr = config.fitsInstrument.toStdString().c_str();
#else
    strncpy ( tempStr, config.fitsInstrument.toStdString().c_str(),
        FLEN_VALUE + 1 );
#endif
    fits_write_key_str ( fptr, "INSTRUME", tempStr, "", &status );
  }
  if ( config.fitsComment != "" ) {
#if CFITSIO_MAJOR > 3 || ( CFITSIO_MAJOR == 3 && CFITSIO_MINOR > 30 )
    tempStr = config.fitsComment.toStdString().c_str();
#else
    strncpy ( tempStr, config.fitsComment.toStdString().c_str(),
        FLEN_VALUE + 1 );
#endif
    fits_write_comment ( fptr, tempStr, &status );
  }

  if ( fits_write_img ( fptr, tableType, 1, elements, frame, &status )) {
    if ( status ) {
      fits_report_error ( stderr, status );
    }
    frameCount++;
    return -1;
  }

  // Write additional keywords

  if ( fits_close_file ( fptr, &status )) {
    if ( status ) {
      fits_report_error ( stderr, status );
    }
    frameCount++;
    return -1;
  }

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
