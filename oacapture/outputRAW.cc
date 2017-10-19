/*****************************************************************************
 *
 * outputRAW.cc -- RAW output class
 *
 * Copyright 2016 James Fidell (james@openastroproject.org)
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

#include "outputHandler.h"
#include "outputRAW.h"
#include "state.h"


OutputRAW::OutputRAW ( int x, int y, int n, int d, int fmt ) :
    OutputHandler ( x, y, n, d )
{
    frameSize = x * y * OA_BYTES_PER_PIXEL( fmt );
}


OutputRAW::~OutputRAW()
{
  // Probably nothing to do here for PNG files
}


int
OutputRAW::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".raw";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputRAW::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".raw";
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
OutputRAW::openOutput ( void )
{
  if (!( writeBuffer = ( unsigned char* ) malloc ( frameSize ))) {
      qWarning() << "write buffer allocation failed";
      return -1;
  }
  return 0;
}


int
OutputRAW::addFrame ( void* frame, const char* timestampStr,
    int64_t expTime )
{
  FILE*          handle;

  filenameRoot = getNewFilename();
  fullSaveFilePath = filenameRoot + ".raw";

  if (!( handle = fopen ( fullSaveFilePath.toStdString().c_str(), "wb" ))) {
    qWarning() << "open of " << fullSaveFilePath << " failed";
    // Need this or we'll never stop
    frameCount++;
    return -1;
  }

  if ( 1 != fwrite( frame, frameSize, 1, handle ) ) {
    qWarning() << "write to " << fullSaveFilePath << " failed";
  }

  fclose ( handle );
  frameCount++;
  state.captureIndex++;
  return OA_ERR_NONE;
}

void
OutputRAW::closeOutput ( void )
{
  if ( writeBuffer ) {
    ( void ) free ( writeBuffer );
  }
  writeBuffer = 0;
}
