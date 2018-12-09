/*****************************************************************************
 *
 * outputAVI.cc -- AVI output class
 *
 * Copyright 2013,2014,2018 James Fidell (james@openastroproject.org)
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
#include "outputAVI.h"
#include "trampoline.h"


OutputAVI::OutputAVI ( int x, int y, int n, int d, int fmt,
		QString fileTemplate, trampolineFuncs* trampolines) :
    OutputFFMPEG ( x, y, n, d, fmt, fileTemplate, trampolines )
{
  videoCodec = AV_CODEC_ID_UTVIDEO;

  switch ( fmt ) {
    case OA_PIX_FMT_GREY8:
    case OA_PIX_FMT_GREY16LE:
    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_BGGR8:
    case OA_PIX_FMT_RGGB8:
    case OA_PIX_FMT_GRBG8:
    case OA_PIX_FMT_GBRG8:
    case OA_PIX_FMT_BGGR16LE:
    case OA_PIX_FMT_BGGR16BE:
    case OA_PIX_FMT_RGGB16LE:
    case OA_PIX_FMT_RGGB16BE:
    case OA_PIX_FMT_GBRG16LE:
    case OA_PIX_FMT_GBRG16BE:
    case OA_PIX_FMT_GRBG16LE:
    case OA_PIX_FMT_GRBG16BE:
    case OA_PIX_FMT_YUV444P:
    case OA_PIX_FMT_YUV422P:
    case OA_PIX_FMT_YUV420P:
    case OA_PIX_FMT_YUV411P:
    case OA_PIX_FMT_YUV410P:
    case OA_PIX_FMT_YUYV:
    case OA_PIX_FMT_UYVY:
      videoCodec = AV_CODEC_ID_RAWVIDEO;
      break;
  }
}
