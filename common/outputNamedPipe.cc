/*****************************************************************************
 *
 * outputNamedPipe.cc -- Named pipe output class
 *
 * Copyright 2019
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

#include <cerrno>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/video/formats.h>
};

#include "commonState.h"
#include "targets.h"
#include "trampoline.h"
#include "outputHandler.h"
#include "outputNamedPipe.h"


OutputNamedPipe::OutputNamedPipe ( int x, int y, int n, int d, int fmt,
		const char* appName, const char* appVer, QString fileTemplate,
		trampolineFuncs* trampolines ) :
    OutputHandler ( x, y, n, d, fileTemplate, trampolines ),
		applicationName ( appName ), applicationVersion ( appVer ),
		imageFormat ( fmt )
{
  int pixelSize;

  writesDiscreteFiles = 0;
  frameCount = 0;
  xSize = x;
  ySize = y;
  fullSaveFilePath = "";
  validFileType = 1;
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

    default:
      validFileType = 0;
      break;
  }

  pixelSize = ( colour ? 3 : 1 );
  if ( validFileType ) {
    frameSize = xSize * ySize * pixelSize;
		( void ) snprintf ( pnmHeader, 32, "P%d\n%d %d\n%d\n", colour ? 6 : 5,
				xSize, ySize, 255 );
		pnmHeaderLength = strlen ( pnmHeader );
  }
}


OutputNamedPipe::~OutputNamedPipe()
{
}


int
OutputNamedPipe::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot;
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputNamedPipe::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot;
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
OutputNamedPipe::openOutput ( void )
{

  if ( validFileType ) {
		if ( swapRedBlue ) {
			if (!( writeBuffer = ( unsigned char* ) malloc ( frameSize ))) {
				qWarning() << "write buffer allocation failed";
				return -1;
			}
    }

		if (( pipeFD = open ( fullSaveFilePath.toStdString().c_str(),
					O_WRONLY )) < 0 ) {
			qWarning() << "Unable to open" << fullSaveFilePath << "for append." <<
					"Error" << strerror ( errno );
			( void ) free (( void* ) writeBuffer );
			writeBuffer = 0;
			return -1;
		}
	}

  return !validFileType;
}


int
OutputNamedPipe::addFrame ( void* frame,
		const char* timestampStr __attribute__((unused)),
    int64_t expTime __attribute__((unused)),
		const char* commentStr __attribute__((unused)),
		FRAME_METADATA* metadata __attribute__((unused)))
{
	int								i;
	unsigned char*		s;
	unsigned char*		t;
	unsigned char*		buffer = ( unsigned char* ) frame;

	if ( swapRedBlue ) {
		s = ( unsigned char* ) frame;
		t = writeBuffer;
		for ( i = 0; i < frameSize; i += 3, s += 3 ) {
			*t++ = *( s + 2 );
			*t++ = *( s + 1 );
			*t++ = *s;
		}
		buffer = writeBuffer;
	}

	if ( write ( pipeFD, pnmHeader, pnmHeaderLength ) != pnmHeaderLength ) {
		qWarning() << "failed to write PNM header.  Error" << strerror ( errno ) <<
			"\ndata:\n" << pnmHeader;
		return OA_ERR_SYSTEM_ERROR;
	}
	if ( write ( pipeFD, buffer, frameSize ) != frameSize ) {
		qWarning() << "failed to write PNM data.  Error" << strerror ( errno );
		return OA_ERR_SYSTEM_ERROR;
	}

  frameCount++;
  commonState.captureIndex++;
  return OA_ERR_NONE;
}


void
OutputNamedPipe::closeOutput ( void )
{
	close ( pipeFD );
  if ( writeBuffer ) {
    ( void ) free ( writeBuffer );
  }
  writeBuffer = 0;
}
