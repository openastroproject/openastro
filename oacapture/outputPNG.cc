/*****************************************************************************
 *
 * outputPNG.cc -- PNG output class
 *
 * Copyright 2016,2017 James Fidell (james@openastroproject.org)
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
};

#include "outputHandler.h"
#include "outputPNG.h"
#include "configuration.h"
#include "state.h"


OutputPNG::OutputPNG ( int x, int y, int n, int d, int fmt ) :
    OutputHandler ( x, y, n, d )
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
  int            i;
  FILE*          handle;
  void*          buffer = frame;
  unsigned char* s;
  unsigned char* t;
  unsigned int   pngTransforms;

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
  state.captureIndex++;
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
