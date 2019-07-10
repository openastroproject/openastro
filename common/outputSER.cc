/*****************************************************************************
 *
 * outputSER.cc -- SER output class
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
#include <openastro/camera.h>
#include <openastro/SER.h>
#include <openastro/video/formats.h>
}

#include "commonState.h"
#include "outputHandler.h"
#include "outputSER.h"
#include "trampoline.h"
#include "fitsSettings.h"


OutputSER::OutputSER ( int x, int y, int n, int d, int fmt,
		QString fileTemplate, trampolineFuncs* trampolines ) :
    OutputHandler ( x, y, n, d, fileTemplate, trampolines )
{
  // FIX ME -- I should move a load of this into liboaSER

  writesDiscreteFiles = 0;
  frameCount = 0;
  xSize = x;
  ySize = y;
  colourId = OA_SER_MONO;
  littleEndian = 0;
  pixelDepth = 8;
  switch ( fmt ) {

    case OA_PIX_FMT_GREY8:
      colourId = 0;
      break;

    case OA_PIX_FMT_GREY16LE:
    case OA_PIX_FMT_GREY10_16LE:
    case OA_PIX_FMT_GREY12_16LE:
    case OA_PIX_FMT_GREY14_16LE:
      colourId = 0;
      littleEndian = 1;
      pixelDepth = 16;
      break;

    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_GREY10_16BE:
    case OA_PIX_FMT_GREY12_16BE:
    case OA_PIX_FMT_GREY14_16BE:
      colourId = 0;
      pixelDepth = 16;
      break;

    case OA_PIX_FMT_BGGR16LE:
		case OA_PIX_FMT_BGGR10_16LE:
		case OA_PIX_FMT_BGGR12_16LE:
		case OA_PIX_FMT_BGGR14_16LE:
      littleEndian = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_BGGR16BE:
		case OA_PIX_FMT_BGGR10_16BE:
		case OA_PIX_FMT_BGGR12_16BE:
		case OA_PIX_FMT_BGGR14_16BE:
      pixelDepth = 16;
			/* FALLTHROUGH */
    case OA_PIX_FMT_BGGR8:
      colourId = OA_SER_BAYER_BGGR;
      break;

    case OA_PIX_FMT_RGGB16LE:
		case OA_PIX_FMT_RGGB10_16LE:
		case OA_PIX_FMT_RGGB12_16LE:
		case OA_PIX_FMT_RGGB14_16LE:
      littleEndian = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGGB16BE:
		case OA_PIX_FMT_RGGB10_16BE:
		case OA_PIX_FMT_RGGB12_16BE:
		case OA_PIX_FMT_RGGB14_16BE:
      pixelDepth = 16;
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGGB8:
      colourId = OA_SER_BAYER_RGGB;
      break;

    case OA_PIX_FMT_GRBG16LE:
		case OA_PIX_FMT_GRBG10_16LE:
		case OA_PIX_FMT_GRBG12_16LE:
		case OA_PIX_FMT_GRBG14_16LE:
      littleEndian = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_GRBG16BE:
		case OA_PIX_FMT_GRBG10_16BE:
		case OA_PIX_FMT_GRBG12_16BE:
		case OA_PIX_FMT_GRBG14_16BE:
      pixelDepth = 16;
			/* FALLTHROUGH */
    case OA_PIX_FMT_GRBG8:
      colourId = OA_SER_BAYER_GRBG;
      break;

    case OA_PIX_FMT_GBRG16LE:
		case OA_PIX_FMT_GBRG10_16LE:
		case OA_PIX_FMT_GBRG12_16LE:
		case OA_PIX_FMT_GBRG14_16LE:
      littleEndian = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_GBRG16BE:
		case OA_PIX_FMT_GBRG10_16BE:
		case OA_PIX_FMT_GBRG12_16BE:
		case OA_PIX_FMT_GBRG14_16BE:
      pixelDepth = 16;
			/* FALLTHROUGH */
    case OA_PIX_FMT_GBRG8:
      colourId = OA_SER_BAYER_GBRG;
      break;

    case OA_PIX_FMT_RGB48LE:
      littleEndian = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGB48BE:
      pixelDepth = 16;
			/* FALLTHROUGH */
    case OA_PIX_FMT_RGB24:
      colourId = OA_SER_RGB;
      break;

    case OA_PIX_FMT_BGR48LE:
      littleEndian = 1;
			/* FALLTHROUGH */
    case OA_PIX_FMT_BGR48BE:
      pixelDepth = 16;
			/* FALLTHROUGH */
    case OA_PIX_FMT_BGR24:
      colourId = OA_SER_BGR;
      break;
  }
  fullSaveFilePath = "";
}


OutputSER::~OutputSER()
{
  // Probably nothing to do here for SER files
}


int
OutputSER::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".ser";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputSER::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".ser";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), W_OK )) ? 0 : 1;
}


int
OutputSER::openOutput ( void )
{
  oaSERHeader	header;

  memset ( &header, 0, sizeof ( header ));
  header.LuID = 0;
  header.ColorID = colourId;
  header.LittleEndian = littleEndian;
  header.ImageWidth = xSize;
  header.ImageHeight = ySize;
  header.PixelDepth = pixelDepth;
  header.FrameCount = 0;

  ( void ) strncpy ( header.Observer,
      fitsConf.observer.toStdString().c_str(), 40 );
  ( void ) strncpy ( header.Instrument,
      fitsConf.instrument.toStdString().c_str(), 40 );
  ( void ) strncpy ( header.Telescope,
      fitsConf.telescope.toStdString().c_str(), 40 );

  int		e;

  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".ser";
  }

  if (( e = oaSEROpen ( fullSaveFilePath.toStdString().c_str(),
      &SERContext ))) {
    qWarning() << "open of " << fullSaveFilePath << " failed";
    return -1;
  }

  oaSERWriteHeader ( &SERContext, &header );
  return 0;
}


int
OutputSER::addFrame ( void* frame, const char* timestampStr,
		int64_t expTime __attribute__((unused)),
    const char* commentStr __attribute__((unused)),
		FRAME_METADATA* metadata __attribute__((unused)))
{
  int ret;

  ret = oaSERWriteFrame ( &SERContext, frame, timestampStr );
  if ( ret ) {
    qWarning() << "oaSERWriteFrame failed";
  }
  frameCount++;
  return ret;
}


void
OutputSER::closeOutput ( void )
{
  oaSERWriteTrailer ( &SERContext );
  oaSERClose ( &SERContext );
  commonState.captureIndex++;
}
