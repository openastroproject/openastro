/*****************************************************************************
 *
 * outputTIFF.cc -- TIFF output class
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
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

extern "C" {
#include <tiffio.h>
#include <openastro/camera.h>
#include <openastro/video/formats.h>
};

#include "commonState.h"
#include "fitsSettings.h"
#include "outputHandler.h"
#include "outputTIFF.h"
#include "trampoline.h"


OutputTIFF::OutputTIFF ( int x, int y, int n, int d, int fmt,
		const char* appName, const char* appVer, QString fileTemplate,
		trampolineFuncs* trampolines ) :
    OutputHandler ( x, y, n, d, fileTemplate, trampolines ),
		applicationName ( appName ), applicationVersion ( appVer )
{
  uint16_t byteOrderTest = 0x1234;
  uint8_t* firstByte;

  firstByte = ( uint8_t* ) &byteOrderTest;

  writesDiscreteFiles = 1;
  frameCount = 0;
  xSize = x;
  ySize = y;
  pixelDepth = 8;
  fullSaveFilePath = "";
  validFileType = 1;
  reverseByteOrder = 0;
  swapRedBlue = 0;
  colour = 0;
  writeBuffer = 0;

  switch ( fmt ) {

    case OA_PIX_FMT_RGB24:
      colour = 1;
    case OA_PIX_FMT_GREY8:
      break;

    case OA_PIX_FMT_BGR24:
      colour = 1;
      swapRedBlue = 1;
      break;

    case OA_PIX_FMT_GREY16LE:
      pixelDepth = 16;
      if ( *firstByte == 0x12 ) {
        reverseByteOrder = 1;
      }
      break;

    case OA_PIX_FMT_GREY16BE:
      pixelDepth = 16;
      if ( *firstByte == 0x34 ) {
        reverseByteOrder = 1;
      }
      break;

    case OA_PIX_FMT_BGR48BE:
      swapRedBlue = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGB48BE:
      colour = 1;
      pixelDepth = 16;
      if ( *firstByte == 0x34 ) {
        reverseByteOrder = 1;
      }
      break;

    case OA_PIX_FMT_BGR48LE:
      swapRedBlue = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGB48LE:
      colour = 1;
      pixelDepth = 16;
      if ( *firstByte == 0x12 ) {
        reverseByteOrder = 1;
      }
      break;

    default:
      validFileType = 0;
      break;
  }

  if ( validFileType ) {
    frameSize = xSize * ySize * pixelDepth / 8 * ( colour ? 3 : 1 );
  }
}


OutputTIFF::~OutputTIFF()
{
  // Probably nothing to do here for TIFF files
}


int
OutputTIFF::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".tiff";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputTIFF::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".tiff";
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
OutputTIFF::openOutput ( void )
{
  if ( validFileType ) {
    if (!( writeBuffer = ( unsigned char* ) malloc ( frameSize ))) {;
      qWarning() << "write buffer allocation failed";
      return -1;
    }
  }
  return !validFileType;
}


int
OutputTIFF::addFrame ( void* frame, const char* timestampStr,
		int64_t expTime __attribute__((unused)), const char* commentStr,
		FRAME_METADATA* metadata __attribute__((unused)))
{
  int            ret, i;
  TIFF*          handle;
  void*          buffer = frame;
  unsigned char* s;
  unsigned char* t;
	char						tiffField[128];

  filenameRoot = getNewFilename();
  fullSaveFilePath = filenameRoot + ".tiff";

  if (!( handle = TIFFOpen ( fullSaveFilePath.toStdString().c_str(), "w" ))) {
    qWarning() << "open of " << fullSaveFilePath << " failed";
    // Need this or we'll never stop
    frameCount++;
    return -1;
  }

  TIFFSetField ( handle, TIFFTAG_IMAGEWIDTH, xSize );
  TIFFSetField ( handle, TIFFTAG_IMAGELENGTH, ySize );
  TIFFSetField ( handle, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
  TIFFSetField ( handle, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField ( handle, TIFFTAG_ROWSPERSTRIP, ySize );

  if ( colour ) {
    TIFFSetField ( handle, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
    TIFFSetField ( handle, TIFFTAG_SAMPLESPERPIXEL, 3 );
  } else {
    TIFFSetField ( handle, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );
    TIFFSetField ( handle, TIFFTAG_SAMPLESPERPIXEL, 1 );
  }
  TIFFSetField ( handle, TIFFTAG_BITSPERSAMPLE, pixelDepth );

	( void ) snprintf ( tiffField, 128, "%s %s", applicationName,
			applicationVersion );
  TIFFSetField ( handle, TIFFTAG_SOFTWARE, tiffField );
  if ( timestampStr && *timestampStr ) {
    TIFFSetField ( handle, TIFFTAG_DATETIME, timestampStr );
  }
  if ( commentStr && *commentStr ) {
    TIFFSetField ( handle, TIFFTAG_IMAGEDESCRIPTION, commentStr );
  }

  // swap byte orders if we need to
  // swap R and B if we need to
  // I've done this in for separate loops to avoid tests inside the loops

  s = ( unsigned char* ) frame;
  t = writeBuffer;

  if ( 16 == pixelDepth ) {
    if ( reverseByteOrder ) {
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
        for ( i = 0; i < frameSize; i += 2, s += 2 ) {
          *t++ = *( s + 1 );
          *t++ = *s;
        }
      }
      buffer = writeBuffer;
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
        buffer = writeBuffer;
      }
    }
  }
  if ( 8 == pixelDepth ) {
    if ( swapRedBlue ) {
      for ( i = 0; i < frameSize; i += 3, s += 3 ) {
        *t++ = *( s + 2 );
        *t++ = *( s + 1 );
        *t++ = *s;
      }
      buffer = writeBuffer;
    }
  }

  ret = TIFFWriteEncodedStrip ( handle, 0, buffer, frameSize );
  TIFFClose ( handle );
  frameCount++;
  commonState.captureIndex++;
  return ret;
}

void
OutputTIFF::closeOutput ( void )
{
  if ( writeBuffer ) {
    ( void ) free ( writeBuffer );
  }
}
