/*****************************************************************************
 *
 * outputDIB.cc -- Windows DIB AVI output class
 *
 * Copyright 2015,2016,2017,2018,2019
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

#include <QtGui>

extern "C" {
#include <openastro/camera.h>
}

#include "pipp_avi_write_dib.h"
#include "commonState.h"
#include "outputHandler.h"
#include "outputDIB.h"


OutputDIB::OutputDIB ( int x, int y, int n, int d,
		int fmt __attribute((unused)), QString fileTemplate,
		trampolineFuncs* trampolines ) :
    OutputHandler ( x, y, n, d, fileTemplate, trampolines )
{
  writesDiscreteFiles = 0;
  frameCount = 0;
  xSize = x;
  ySize = y;
  fpsNumerator = n;
  fpsDenominator = d;
  fullSaveFilePath = "";
  fileExtension = "avi";
  colour = 0;
  bpp = 1;
}


OutputDIB::~OutputDIB()
{
}


int
OutputDIB::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + "." + fileExtension;
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputDIB::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + "." + fileExtension;
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), W_OK )) ? 0 : 1;
}


int
OutputDIB::openOutput ( void )
{
  outputFile = new c_pipp_avi_write_dib();
  outputFile->create ( fullSaveFilePath.toStdString().c_str(), xSize,
      ySize, colour, fpsDenominator, fpsNumerator, 0, 0, 0 );

  return 0;
}


int
OutputDIB::addFrame ( void* frame,
		const char* timestampStr __attribute__((unused)),
		int64_t expTime __attribute__((unused)),
    const char* commentStr __attribute((unused)),
		FRAME_METADATA* metadata __attribute__((unused)))
{
  outputFile->write_frame (( uint8_t* ) frame, 0, bpp, 0 );
  frameCount++;
  return 0;
}


void
OutputDIB::closeOutput ( void )
{
  outputFile->close();
  delete outputFile;
  outputFile = 0;
  commonState.captureIndex++;
}
