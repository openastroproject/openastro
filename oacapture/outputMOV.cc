/*****************************************************************************
 *
 * outputMOV.cc -- MOV output class
 *
 * Copyright 2013,2014,2015 James Fidell (james@openastroproject.org)
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

#include "outputHandler.h"
#include "outputFFMPEG.h"
#include "outputMOV.h"
#include "state.h"


OutputMOV::OutputMOV ( int x, int y, int n, int d, int fmt ) :
    OutputFFMPEG ( x, y, n, d, fmt )
{
  videoCodec = AV_CODEC_ID_RAWVIDEO;
  fileExtension = "mov";

  if ( fmt == OA_PIX_FMT_GREY8 ) {
    actualPixelFormat = AV_PIX_FMT_GRAY8;
    storedPixelFormat = AV_PIX_FMT_GRAY16BE;
  }
  if ( fmt == OA_PIX_FMT_RGB24 ) {
    actualPixelFormat = AV_PIX_FMT_RGB24;
    storedPixelFormat = AV_PIX_FMT_BGR24;
  }
  if ( fmt == OA_PIX_FMT_BGR24 ) {
    actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BGR24;
  }
}
