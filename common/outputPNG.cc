/*****************************************************************************
 *
 * outputPNG.cc -- PNG output class
 *
 * Copyright 2016,2017,2018,2019
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
#include <png.h>

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/video/formats.h>
};

#include "commonState.h"
#include "targets.h"
#include "trampoline.h"
#include "outputHandler.h"
#include "outputPNG.h"
#include "fitsSettings.h"


OutputPNG::OutputPNG ( int x, int y, int n, int d, int fmt,
		const char* appName, const char* appVer, QString fileTemplate,
		trampolineFuncs* trampolines ) :
    OutputHandler ( x, y, n, d, fileTemplate, trampolines ),
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
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGB48BE:
      colour = 1;
      pixelDepth = 16;
      break;

    case OA_PIX_FMT_BGR48LE:
      swapRedBlue = 1;
			/* FALLTHROUGH */
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
    int64_t expTime, const char* commentStr, FRAME_METADATA* metadata )
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
  if ( fitsConf.observer != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.observer.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "OBJECT";
  stringBuffs[ numComments ][0] = 0;
  int currentTargetId = trampolines->getCurrentTargetId();
  if ( currentTargetId > 0 && currentTargetId != TGT_UNKNOWN ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
				targetName ( currentTargetId ).toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  } else {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.object.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
  }
  if ( stringBuffs[ numComments ][0]) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "TELESCOP";
  if ( fitsConf.telescope != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.telescope.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "INSTRUME";
  if ( fitsConf.instrument != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.instrument.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "COMMENT1";
  if ( fitsConf.comment != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.comment.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
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
  if ( fitsConf.focalLength != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.focalLength.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "APTDIA";
  if ( fitsConf.apertureDia != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.apertureDia.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  } 

  pngComments[ numComments ].key = ( char* ) "APTAREA";
  if ( fitsConf.apertureArea != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.apertureArea.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "XPIXSZ";
  stringBuffs[ numComments ][0] = 0;
  if ( fitsConf.pixelSizeX != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.pixelSizeX.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  } else {
    int binMultiplier = 1;
    if ( commonState.binningValid ) {
      binMultiplier = OA_BIN_MODE_MULTIPLIER ( commonState.binModeX );
    }
    ( void ) snprintf ( stringBuffs[ numComments ],
        PNG_KEYWORD_MAX_LENGTH, "%f", commonState.camera->pixelSizeX() *
        binMultiplier / 1000.0 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "YPIXSZ";
  stringBuffs[ numComments ][0] = 0;
  if ( fitsConf.pixelSizeY != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.pixelSizeY.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  } else {
    int binMultiplier = 1;
    if ( commonState.binningValid ) {
      binMultiplier = OA_BIN_MODE_MULTIPLIER ( commonState.binModeY );
    }
    ( void ) snprintf ( stringBuffs[ numComments ],
        PNG_KEYWORD_MAX_LENGTH, "%f", commonState.camera->pixelSizeY() *
        binMultiplier / 1000.0 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  pngComments[ numComments ].key = ( char* ) "XORGSUBF";
  if ( fitsConf.subframeOriginX != "" ) {
    xorg = fitsConf.subframeOriginX.toInt();
  } else {
    if ( commonState.cropMode ) {
      xorg = ( commonState.sensorSizeX  - commonState.cropSizeX ) / 2;
    } else {
      xorg = ( commonState.sensorSizeX  - xSize ) / 2;
    }
  }
  ( void ) snprintf ( stringBuffs[ numComments ], PNG_KEYWORD_MAX_LENGTH,
      "%d", xorg );
  pngComments[ numComments ].text = stringBuffs[ numComments ];
  numComments++;

  pngComments[ numComments ].key = ( char* ) "YORGSUBF";
  if ( fitsConf.subframeOriginY != "" ) {
    yorg = fitsConf.subframeOriginY.toInt();
  } else {
    if ( commonState.cropMode ) {
      yorg = ( commonState.sensorSizeY  - commonState.cropSizeY ) / 2;
    } else {
      yorg = ( commonState.sensorSizeY  - ySize ) / 2;
    }
  }
  ( void ) snprintf ( stringBuffs[ numComments ], PNG_KEYWORD_MAX_LENGTH,
      "%d", yorg );
  pngComments[ numComments ].text = stringBuffs[ numComments ];
  numComments++;

  pngComments[ numComments ].key = ( char* ) "FILTER";
  QString currentFilter = trampolines->getCurrentFilterName();
  if ( fitsConf.filter != "" ) {
    currentFilter = fitsConf.filter;
  }
  if ( currentFilter != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        currentFilter.toStdString().c_str(), PNG_KEYWORD_MAX_LENGTH+1 );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  stringBuffs[ numComments ][0] = 0;
  pngComments[ numComments ].key = ( char* ) "SITELAT";
  if ( commonState.gpsValid ) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
		commonState.latitude );
  }
  if ( !stringBuffs[ numComments ][0] && fitsConf.siteLatitude != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.siteLatitude.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

  stringBuffs[ numComments ][0] = 0;
  pngComments[ numComments ].key = ( char* ) "SITELONG";
  if ( commonState.gpsValid ) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
				commonState.longitude );
  }
  if ( !stringBuffs[ numComments ][0] && fitsConf.siteLongitude != "" ) {
    ( void ) strncpy ( stringBuffs[ numComments ],
        fitsConf.siteLatitude.toStdString().c_str(),
        PNG_KEYWORD_MAX_LENGTH+1 );
  }
  if ( stringBuffs[ numComments ][0] ) {
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

	stringBuffs[ numComments ][0] = 0;
  pngComments[ numComments ].key = ( char* ) "SITEELEV";
  if ( commonState.gpsValid ) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
		commonState.altitude );
    ( void ) sprintf ( stringBuffs[ numComments + 1 ], "%g",
				commonState.altitude );
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
  if ( commonState.cameraTempValid ) {
    ( void ) sprintf ( stringBuffs[ numComments ], "%g",
				commonState.cameraTemp );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }


  if ( commonState.binningValid ) {
    pngComments[ numComments ].key = ( char* ) "XBINNING";
    ( void ) sprintf ( stringBuffs[ numComments ], "%d",
				commonState.binModeX );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
    pngComments[ numComments ].key = ( char* ) "YBINNING";
    ( void ) sprintf ( stringBuffs[ numComments ], "%d",
				commonState.binModeY );
    pngComments[ numComments ].text = stringBuffs[ numComments ];
    numComments++;
  }

	if ( metadata && metadata->frameCounterValid ) {
		pngComments[ numComments ].key = ( char* ) "FRAMESEQ";
		( void ) sprintf ( stringBuffs[ numComments ], "%d",
				metadata->frameCounter );
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
  commonState.captureIndex++;
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
