/*****************************************************************************
 *
 * outputFFMPEG.cc -- FFMPEG output class
 *
 * Copyright 2013,2014,2015,2016,2017 James Fidell (james@openastroproject.org)
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


extern "C" {
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

#include "outputHandler.h"
#include "outputFFMPEG.h"
#include "state.h"


OutputFFMPEG::OutputFFMPEG ( int x, int y, int n, int d, int fmt ) :
    OutputHandler ( x, y, n, d )
{
  if ( !state.libavStarted ) {
    av_register_all();
    av_log_set_level ( AV_LOG_QUIET );
    // av_log_set_level ( AV_LOG_INFO );
    state.libavStarted = 1;
  }

  writesDiscreteFiles = 0;
  outputFormat = 0;
  formatContext = 0;
  videoStream = 0;
  frameCount = 0;
  xSize = x;
  ySize = y;
  fpsNumerator = n;
  fpsDenominator = d;
  storedPixelFormat = actualPixelFormat = AV_PIX_FMT_RGB24;
  videoCodec = AV_CODEC_ID_UTVIDEO;
  bpp = 1;
  switch ( fmt ) {
    case OA_PIX_FMT_BGR24:
      actualPixelFormat = AV_PIX_FMT_BGR24;
      break;
    case OA_PIX_FMT_GREY8:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_GRAY8;
      break;
    case OA_PIX_FMT_GREY16LE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_GRAY16LE;
      bpp = 2;
      break;
    case OA_PIX_FMT_GREY16BE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_GRAY16BE;
      bpp = 2;
      break;
    case OA_PIX_FMT_BGGR8:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_BGGR8;
      break;
    case OA_PIX_FMT_RGGB8:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_RGGB8;
      break;
    case OA_PIX_FMT_GRBG8:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_GRBG8;
      break;
    case OA_PIX_FMT_GBRG8:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_GBRG8;
      break;
    case OA_PIX_FMT_BGGR16LE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_BGGR16LE;
      bpp = 2;
      break;
    case OA_PIX_FMT_BGGR16BE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_BGGR16BE;
      bpp = 2;
      break;
    case OA_PIX_FMT_RGGB16LE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_RGGB16LE;
      bpp = 2;
      break;
    case OA_PIX_FMT_RGGB16BE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_RGGB16BE;
      bpp = 2;
      break;
    case OA_PIX_FMT_GBRG16LE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_GBRG16LE;
      bpp = 2;
      break;
    case OA_PIX_FMT_GBRG16BE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_GBRG16BE;
      bpp = 2;
      break;
    case OA_PIX_FMT_GRBG16LE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_GRBG16LE;
      bpp = 2;
      break;
    case OA_PIX_FMT_GRBG16BE:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_BAYER_GRBG16BE;
      bpp = 2;
      break;
    case OA_PIX_FMT_YUYV:
      actualPixelFormat = storedPixelFormat = AV_PIX_FMT_YUYV422;
      bpp = 2;
      break;
  }

  fullSaveFilePath = "";
  fileExtension = "avi";
}


OutputFFMPEG::~OutputFFMPEG()
{
}


int
OutputFFMPEG::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + "." + fileExtension;
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputFFMPEG::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + "." + fileExtension;
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), W_OK )) ? 0 : 1;
}


int
OutputFFMPEG::openOutput ( void )
{
  int		e;
  char		errbuf[100];

  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + "." + fileExtension;
  }

  if (!( outputFormat = av_guess_format ( fileExtension, 0, 0 ))) {
    qWarning() << "av_guess_format failed";
    return -1;
  }

  if (!( formatContext = avformat_alloc_context())) {
    qWarning() << "avformat_alloc_context failed";
    return -1;
  }

  formatContext->oformat = outputFormat;
  snprintf ( formatContext->filename, sizeof ( formatContext->filename ),
    "%s", fullSaveFilePath.toStdString().c_str());

  if (!( videoStream = addVideoStream ( formatContext, videoCodec ))) {
    qWarning() << "add video stream failed";
    return -1;
  }

  av_dump_format ( formatContext, 0,
      fullSaveFilePath.toStdString().c_str(), 1 );

  if ( openVideo ( videoStream )) {
    qWarning() << "open video stream failed";
    return -1;
  }

  if (( e = avio_open ( &formatContext->pb,
      fullSaveFilePath.toStdString().c_str(), AVIO_FLAG_WRITE )) < 0 ) {
    av_strerror( e, errbuf, sizeof(errbuf));
    qWarning() << "open of " << fullSaveFilePath << " failed, error = " <<
        errbuf;
    return -1;
  }

  avformat_write_header ( formatContext, 0 );

  // av_log_set_level ( AV_LOG_DEBUG );

  return 0;
}


int
OutputFFMPEG::addFrame ( void* frame, const char* timestampStr,
    int64_t expTime, const char* commentStr )
{
  int64_t lastPTS;

  if ( actualPixelFormat != storedPixelFormat ) {
    // the second here is for quicktime
    if ( AV_PIX_FMT_BGR24 == actualPixelFormat || ( actualPixelFormat ==
        AV_PIX_FMT_RGB24 && storedPixelFormat == AV_PIX_FMT_BGR24 )) {
      // Quick hack to swap the R and B bytes...
      uint8_t* t = picture->data[0];
      uint8_t* s = ( uint8_t*) frame;
      int l = 0;
      while ( l < frameSize ) {
        *t++ = *( s + 2 );
        *t++ = *( s + 1 );
        *t++ = *s;
        s += 3;
        l += 3;
      }
    } else {
      if ( actualPixelFormat == AV_PIX_FMT_GRAY8 &&
          storedPixelFormat == AV_PIX_FMT_GRAY16BE ) {
        // Really this is just for quicktime
        uint8_t* t = picture->data[0];
        uint8_t* s = ( uint8_t*) frame;
        int l = 0;
        while ( l < frameSize ) {
          *t++ = *s++;
          *t++ = 0;
          l += 2;
        }
      } else {
        if ( actualPixelFormat == AV_PIX_FMT_GRAY16LE &&
            storedPixelFormat == AV_PIX_FMT_GRAY16BE ) {
          // again, just for quicktime
          uint8_t* t = picture->data[0];
          uint8_t* s = ( uint8_t*) frame;
          int l = 0;
          while ( l < frameSize ) {
            *(t+1) = *s++;
            *t = *s++;
            t += 2;
            l += 2;
          }
        } else {
          qWarning() << __FUNCTION__ << "unsupported pixel format" <<
              actualPixelFormat << "to" << storedPixelFormat;
        }
      }
    }
  } else {
    memcpy ( picture->data[0], ( uint8_t* ) frame, frameSize );
  }

  lastPTS = picture->pts;
  picture->pts = frameCount * fpsNumerator / fpsDenominator;
  if ( picture->pts <= lastPTS ) {
    picture->pts++;
  }

  AVPacket packet;
  AVCodecContext* codecContext = videoStream->codec;
  av_init_packet ( &packet );
  packet.data = videoOutputBuffer;
  packet.size = videoOutputBufferSize;
  packet.dts = AV_NOPTS_VALUE;
  packet.pts = AV_NOPTS_VALUE;
  int gotPacket, ret;
  if (!( ret = avcodec_encode_video2 ( codecContext, &packet, picture,
      &gotPacket ))) {
    /*
    if ( packet.pts != AV_NOPTS_VALUE ) {
      packet.pts = av_rescale_q ( packet.pts,
          codecContext->time_base, videoStream->time_base );
    }
     */
    // This is just a hack to make the error "Application provided invalid,
    // non monotonically increasing dts to muxer" go away.  I should fix it
    // properly somehow
    packet.dts = videoStream->cur_dts;
    packet.dts++;
    packet.pts = packet.dts;
    if ( gotPacket ) {
      ret = av_write_frame ( formatContext, &packet );
      av_free_packet ( &packet );
      if ( ret ) {
        qWarning() << "av_write_frame failed, error" << ret;
      }
    }
  } else {
    av_free_packet ( &packet );
    qWarning() << "avcodec_encode_video2 failed, error" << ret;
  }

  frameCount++;
  return ret;
}


void
OutputFFMPEG::closeOutput ( void )
{
#ifdef WINDOWS_RAW
  int fd;
  char buf[4] = { 'D', 'I', 'B', ' ' };
#endif

  av_write_trailer ( formatContext );

  for ( unsigned int i = 0; i < formatContext->nb_streams; i++ ) {
    avcodec_close ( formatContext->streams[i]->codec );
    // av_freep ( &formatContext->streams[i]->codec );
    // av_freep ( &formatContext->streams[i] );
  }

  closeVideo();
  avio_closep ( &formatContext->pb );
  avformat_free_context ( formatContext );

  formatContext = 0;
  outputFormat = 0;
  videoStream = 0;

  state.captureIndex++;

#ifdef WINDOWS_RAW
  // This does not work
  if ( config.useWindowsRaw ) {
    if (( fd = open ( fullSaveFilePath.toStdString().c_str(), O_WRONLY ))) {
      lseek ( fd, 112, SEEK_SET );
      write ( fd, buf, 4 );
      close ( fd );
    }
  }
#endif
}


AVStream*
OutputFFMPEG::addVideoStream ( AVFormatContext* formatContext,
    enum AVCodecID codecId )
{
  AVCodecContext*	codecContext;
  AVStream*		stream;

  if (!( stream = avformat_new_stream ( formatContext, 0 ))) {
    qWarning() << "avformat_new_stream failed";
    return 0;
  }

  codecContext = stream->codec;
  codecContext->codec_id = codecId;
  codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
  codecContext->width = xSize;
  codecContext->height = ySize;
  codecContext->time_base.num = fpsNumerator;
  codecContext->time_base.den = fpsDenominator;
  codecContext->gop_size = 0;
  codecContext->pix_fmt = storedPixelFormat;

  // Need to add this because setting it in the context is now deprecated
  stream->time_base.num = fpsNumerator;
  stream->time_base.den = fpsDenominator;

  if ( formatContext->flags & AVFMT_GLOBALHEADER ) {
    codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
  }

  return stream;
}


int
OutputFFMPEG::openVideo ( AVStream* stream )
{
  AVCodecContext* codecContext = stream->codec;
  AVCodec* codec = avcodec_find_encoder ( codecContext->codec_id );
  if ( !codec ) {
    qWarning() << "codec not found" << codecContext->codec_id;
    return -1;
  }

  if ( avcodec_open2 ( codecContext, codec, 0 ) < 0 ) {
    qWarning() << "couldn't open codec";
    return -1;
  }

  videoOutputBufferSize = 5 * xSize * ySize * bpp;
  videoOutputBuffer = ( uint8_t* ) av_malloc ( videoOutputBufferSize );

  if (!( picture = allocatePicture ( codecContext->pix_fmt,
      codecContext->width, codecContext->height ))) {
    return -1;
  }

  return 0;
}


void
OutputFFMPEG::closeVideo ( void )
{
  if ( videoOutputBuffer ) {
    av_free ( videoOutputBuffer );
    videoOutputBuffer = 0;
  }

  if ( picture ) {
    av_free ( picture->data[0] );
    av_free ( picture );
    picture = 0;
  }
}


AVFrame*
OutputFFMPEG::allocatePicture ( enum AVPixelFormat format, int width,
    int height )
{
  AVFrame* picture = av_frame_alloc();
  if ( !picture ) {
    qWarning() << "av_frame_alloc failed";
    return 0;
  }
  frameSize = avpicture_get_size ( format, width, height );
  uint8_t* pictureBuffer = ( uint8_t* ) av_malloc ( frameSize );
  if ( !pictureBuffer ) {
    qWarning() << "pictureBuffer av_malloc failed";
    av_free ( picture );
    return 0;
  }

  avpicture_fill (( AVPicture* ) picture, pictureBuffer, format, width,
    height );
  picture->format = format;
  picture->width = width;
  picture->height = height;
  return picture;
}
