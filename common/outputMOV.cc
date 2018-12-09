/*****************************************************************************
 *
 * outputMOV.cc -- MOV output class
 *
 * Copyright 2013,2014,2015,2018 James Fidell (james@openastroproject.org)
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
#include <openastro/video/formats.h>
}

#include "commonState.h"
#include "outputHandler.h"
#include "outputFFMPEG.h"
#include "outputMOV.h"
#include "trampoline.h"


OutputMOV::OutputMOV ( int x, int y, int n, int d, int fmt,
		QString fileTemplate, trampolineFuncs* trampolines ) :
    OutputFFMPEG ( x, y, n, d, fmt, fileTemplate, trampolines )
{
  //videoCodec = AV_CODEC_ID_RAWVIDEO;
  videoCodec = AV_CODEC_ID_QTRLE;
  fileExtension = "mov";

  switch ( fmt ) {
    case OA_PIX_FMT_GREY8:
      actualPixelFormat = AV_PIX_FMT_GRAY8;
      // storedPixelFormat = AV_PIX_FMT_GRAY16BE;
      storedPixelFormat = AV_PIX_FMT_GRAY8;
      break;

    /*
    case OA_PIX_FMT_GREY16LE:
      actualPixelFormat = AV_PIX_FMT_GRAY16LE;
      storedPixelFormat = AV_PIX_FMT_GRAY16BE;
      break;

    case OA_PIX_FMT_GREY16BE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_GRAY16BE;
      break;
    */

    case OA_PIX_FMT_RGB24:
      actualPixelFormat = AV_PIX_FMT_RGB24;
      //storedPixelFormat = AV_PIX_FMT_BGR24;
      storedPixelFormat = AV_PIX_FMT_RGB24;
      break;

    case OA_PIX_FMT_BGR24:
      //actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BGR24;
      actualPixelFormat = AV_PIX_FMT_BGR24;
      storedPixelFormat = AV_PIX_FMT_RGB24;
      break;
  }
}
