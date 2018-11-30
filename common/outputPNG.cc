/*****************************************************************************
 *
 * outputPNG.cc -- PNG output class
 *
 * Copyright 2016,2017,2018 James Fidell (james@openastroproject.org)
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
#include <png.h>

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/video/formats.h>
};

#include "targets.h"
#include "trampoline.h"
#include "fitsSettings.h"
#include "outputHandler.h"
#include "outputPNG.h"


OutputPNG::OutputPNG ( int x, int y, int n, int d, int fmt,
		const char* appName, const char* appVer, QString fileTemplate,
		unsigned long long* pcounter, fitsConfig* pConf,
		trampolineFuncs* trampolines ) :
    OutputHandler ( x, y, n, d, fileTemplate, pcounter, pConf, trampolines ),
		applicationName ( appName ), applicationVersion ( appVer ),
		imageFormat ( fmt )
{
  int pixelSize;

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
  rowPointers = 0;

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
      reverseByteOrder = 1;
      break;

    case OA_PIX_FMT_GREY16BE:
      pixelDepth = 16;
      break;

    case OA_PIX_FMT_BGR48BE:
      swapRedBlue = 1;
    case OA_PIX_FMT_RGB48BE:
      colour = 1;
      pixelDepth = 16;
      break;

    case OA_PIX_FMT_BGR48LE:
      swapRedBlue = 1;
    case OA_PIX_FMT_RGB48LE:
      colour = 1;
      pixelDepth = 16;
      reverseByteOrder = 1;
      break;

    default:
      validFileType = 0;
      break;
  }

  pixelSize = pixelDepth / 8 * ( colour ? 3 : 1 );
  if ( validFileType ) {
    frameSize = xSize * ySize * pixelSize;
    rowLength = xSize * pixelSize;
  }
}


OutputPNG::~OutputPNG()
{
  // Probably nothing to do here for PNG files
}


int
OutputPNG::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".png";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputPNG::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".png";
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
OutputPNG::openOutput ( void )
{
  if ( validFileType ) {
    if (!( writeBuffer = ( unsigned char* ) malloc ( frameSize ))) {
      qWarning() << "write buffer allocation failed";
      return -1;
    }

    if (!( rowPointers = ( png_bytep* ) calloc ( ySize,
        sizeof ( png_bytep )))) {
      free ( writeBuffer );
      writeBuffer = 0;
      qWarning() << "row pointers allocation failed";
      return -1;
    }
  }
  return !validFileType;
}


int
OutputPNG::addFrame ( void* frame, const char* timestampStr,
    int64_t expTime, const char* commentStr )
{
  int			i;
  FILE*			handle;
  void*			buffer = frame;
  unsigned char*	s;
  unsigned char*	t;
  unsigned int		pngTransforms;
  png_text		pngComments[ 30 ];
  int			numComments = 0;
  char			stringBuffs[30][ PNG_KEYWORD_MAX_LENGTH + 1 ];
  int       xorg, yorg;

  filenameRoot = getNewFilename();
  fullSaveFilePath = filenameRoot + ".png";

  if (!( handle = fopen ( fullSaveFilePath.toStdString().c_str(), "wb" ))) {
    qWarning() << "open of " << fullSaveFilePath << " failed";
    // Need this or we'll never stop
    frameCount++;
    return -1;
  }

  pngPtr = png_create_write_struct ( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
  if ( !pngPtr ) {
    qWarning() << "create of write struct for " << fullSaveFilePath <<
        " failed";
    // Need this or we'll never stop
    frameCount++;
    return -1;
  }

  infoPtr = png_create_info_struct ( pngPtr );
  if ( !infoPtr ) {
    png_destroy_write_struct ( &pngPtr, 0 );
    qWarning() << "create of write struct for " << fullSaveFilePath <<
        " failed";
    // Need this or we'll never stop
    frameCount++;
    return -1;
  }

  png_init_io ( pngPtr, handle );

  png_set_IHDR ( pngPtr, infoPtr, xSize, ySize, pixelDepth, colour ?
      PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );

  pngTransforms = PNG_TRANSFORM_IDENTITY;
  if ( pixelDepth > 8 && reverseByteOrder ) {
    pngTransforms |= PNG_TRANSFORM_SWAP_ENDIAN;
  }

  // FIX ME -- perhaps the majority of these comments should be set up in
  // the constructor and just the ones that can change per frame handled
  // here?

  pngComments[ numComments ].key = ( char* ) "DATE-OBS";
  if ( timestampStr ) {
    pngComments[ numComments ].text = ( char* ) timestampStr;
    numComments++;
  } else {
    QDateTime now = QDateTime::currentDateTimeUtc();
    // QString dateStr = now.toString ( Qt::ISODate );
    QString dateStr = now.toString ( "yyyy-MM-ddThh:mm:ss.zzz" );
    ( void ) strncpy ( stringBuffs[ numComments ],
        dateStr.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "OBSERVER";
  if ( pConfig->observer != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->observer.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "OBJECT";
  stringBuffs[ numComments ][0] = 0;
  int currentTargetId = trampolines->getCurrentTargetId();
  if ( currentTargetId > 0 && currentTargetId != TGT_UNKNOWN ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        targetList[ currentTargetId ], PNG_KEYWORD_MAX_LENGTH+1 );
  } else {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->object.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
  }
  if ( stringBuffs[ numComments ][0]) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "TELESCOP";
  if ( pConfig->telescope != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->telescope.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "INSTRUME";
  if ( pConfig->instrument != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->instrument.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "COMMENT1";
  if ( pConfig->comment != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->comment.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "COMMENT2";
  if ( commentStr && *commentStr ) {
    ( void ) strncpy ( stringBuffs[ numComments ], commentStr,
        PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "FORMATIN";
  snprintf ( stringBuffs[ numComments ], PNG_KEYWORD_MAX_LENGTH+1, "%s (%s)",
      oaFrameFormats[ imageFormat ].name,
      oaFrameFormats[ imageFormat ].simpleName );
  pngComments[ numComments ].text = stringBuffs[ numComments ];
  numComments++;

  pngComments[ numComments ].key = ( char* ) "FOCALLEN";
  if ( pConfig->focalLength != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->focalLength.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "APTDIA";
  if ( pConfig->apertureDia != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->apertureDia.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  } 

  pngComments[ numComments ].key = ( char* ) "APTAREA";
  if ( pConfig->apertureArea != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->apertureArea.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "XPIXSZ";
  stringBuffs[ numComments ][0] = 0;
  if ( pConfig->pixelSizeX != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->pixelSizeX.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  } else {
    int binMultiplier = 1;
    if ( trampolines->isBinningValid()) {
      binMultiplier = OA_BIN_MODE_MULTIPLIER ( trampolines->binModeX());
    }
    ( void ) snprintf ( stringBuffs[ numComments ],
        PNG_KEYWORD_MAX_LENGTH, "%f", trampolines->pixelSizeX() *
        binMultiplier / 1000.0 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "YPIXSZ";
  stringBuffs[ numComments ][0] = 0;
  if ( pConfig->pixelSizeY != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->pixelSizeY.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  } else {
    int binMultiplier = 1;
    if ( trampolines->isBinningValid()) {
      binMultiplier = OA_BIN_MODE_MULTIPLIER ( trampolines->binModeY());
    }
    ( void ) snprintf ( stringBuffs[ numComments ],
        PNG_KEYWORD_MAX_LENGTH, "%f", trampolines->pixelSizeY() *
        binMultiplier / 1000.0 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "XORGSUBF";
  if ( pConfig->subframeOriginX != "" ) {
    xorg = pConfig->subframeOriginX.toInt();
  } else {
    if ( trampolines->isCropMode()) {
      xorg = ( trampolines->sensorSizeX() - trampolines->cropSizeX()) / 2;
    } else {
      xorg = ( trampolines->sensorSizeX() - xSize ) / 2;
    }
  }
  ( void ) snprintf ( stringBuffs[ numComments ], PNG_KEYWORD_MAX_LENGTH,
      "%d", xorg );
  pngComments[ numComments ].text = stringBuffs[ numComments ];
  numComments++;

  pngComments[ numComments ].key = ( char* ) "YORGSUBF";
  if ( pConfig->subframeOriginY != "" ) {
    yorg = pConfig->subframeOriginY.toInt();
  } else {
    if ( trampolines->isCropMode()) {
      yorg = ( trampolines->sensorSizeY() - trampolines->cropSizeY()) / 2;
    } else {
      yorg = ( trampolines->sensorSizeY() - ySize ) / 2;
    }
  }
  ( void ) snprintf ( stringBuffs[ numComments ], PNG_KEYWORD_MAX_LENGTH,
      "%d", yorg );
  pngComments[ numComments ].text = stringBuffs[ numComments ];
  numComments++;

  pngComments[ numComments ].key = ( char* ) "FILTER";
  QString currentFilter = trampolines->getCurrentFilterName();
  if ( pConfig->filter != "" ) {
    currentFilter = pConfig->filter;
  }
  if ( currentFilter != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        currentFilter.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  stringBuffs[ numComments ][0] = 0;
  pngComments[ numComments ].key = ( char* ) "SITELAT";
  if ( trampolines->isGPSValid()) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
		trampolines->latitude());
  }
  if ( !stringBuffs[ numComments ][0] && pConfig->siteLatitude != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->siteLatitude.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  stringBuffs[ numComments ][0] = 0;
  pngComments[ numComments ].key = ( char* ) "SITELONG";
  if ( trampolines->isGPSValid()) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
				trampolines->longitude());
  }
  if ( !stringBuffs[ numComments ][0] && pConfig->siteLongitude != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        pConfig->siteLatitude.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

	stringBuffs[ numComments ][0] = 0;
  pngComments[ numComments ].key = ( char* ) "SITEELEV";
  if ( trampolines->isGPSValid()) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
		trampolines->altitude());
    ( void ) sprintf ( stringBuffs[ numComments + 1 ], "%g",
				trampolines->altitude());
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
    pngComments[ numComments ].key = ( char* ) "ELEVATIO";
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  snprintf ( stringBuffs[ numComments ], PNG_KEYWORD_MAX_LENGTH+1, "%s %s",
			applicationName, applicationVersion );
  pngComments[ numComments ].key = ( char* ) "SWCREATE";
  pngComments[ numComments ].text = stringBuffs[ numComments ];
  numComments++;

  pngComments[ numComments ].key = ( char* ) "EXPTIME";
  ( void ) sprintf ( stringBuffs[ numComments ], "%g", expTime / 1000000.0 );
  pngComments[ numComments ].text = stringBuffs[ numComments ];
  numComments++;

  if ( oaFrameFormats[ imageFormat ].rawColour ) {
    char* xoff;
    char* yoff;
    // "Bayer" format is GRBG, so all the other formats are offset in some
    // manner from that
    switch ( imageFormat ) {
      case OA_DEMOSAIC_BGGR:
        xoff = ( char* ) "0";
        yoff = ( char* ) "1";
        break;
      case OA_DEMOSAIC_RGGB:
        xoff = ( char* ) "1";
        yoff = ( char* ) "0";
        break;
      case OA_DEMOSAIC_GBRG:
        xoff = ( char* ) "1";
        yoff = ( char* ) "1";
        break;
      case OA_DEMOSAIC_GRBG:
        xoff = ( char* ) "0";
        yoff = ( char* ) "0";
        break;
      default: // clearly this shouldn't ever happen
        xoff = ( char* ) "0";
        yoff = ( char* ) "0";
        break;
    }

    pngComments[ numComments ].key = ( char* ) "BAYERPAT";
    pngComments[ numComments ].text = ( char* ) "TRUE";
    numComments++;
    pngComments[ numComments ].key = ( char* ) "XBAYROFF";
    pngComments[ numComments ].text = xoff;
    numComments++;
    pngComments[ numComments ].key = ( char* ) "YBAYROFF";
    pngComments[ numComments ].text = yoff;
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "CCD-TEMP";
  if ( trampolines->isCameraTempValid()) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
				trampolines->cameraTemp());
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }


  if ( trampolines->isBinningValid()) {
    pngComments[ numComments ].key = ( char* ) "XBINNING";
    ( void ) sprintf ( stringBuffs[ numComments ], "%d",
				trampolines->binModeX());
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
    pngComments[ numComments ].key = ( char* ) "YBINNING";
    ( void ) sprintf ( stringBuffs[ numComments ], "%d",
				trampolines->binModeY());
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  for ( i = 0; i < numComments; i++ ) {
    pngComments[i].compression = PNG_TEXT_COMPRESSION_NONE;
  }

  png_set_text ( pngPtr, infoPtr, pngComments, numComments );

  // swap R and B if we need to
  // I've done this in for separate loops to avoid tests inside the loops

  s = ( unsigned char* ) frame;
  t = writeBuffer;

  if ( 16 == pixelDepth ) {
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

  t = ( unsigned char* ) buffer;
  for ( i = 0; i < ySize; i++ ) {
    rowPointers[i] = t;
    t += rowLength;
  }

  // zlib barfs if compression is enabled
  png_set_compression_level ( pngPtr, 0 );
  png_set_rows ( pngPtr, infoPtr, rowPointers );
  png_write_png ( pngPtr, infoPtr, pngTransforms, 0 );
  png_destroy_write_struct ( &pngPtr, &infoPtr );

  fclose ( handle );
  frameCount++;
  *pCaptureIndex++;
  return OA_ERR_NONE;
}

void
OutputPNG::closeOutput ( void )
{
  if ( writeBuffer ) {
    ( void ) free ( writeBuffer );
  }
  if ( rowPointers ) {
    ( void ) free ( rowPointers );
  }
  writeBuffer = 0;
  rowPointers = 0;
}
