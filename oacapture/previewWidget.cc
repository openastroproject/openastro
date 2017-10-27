/*****************************************************************************
 *
 * previewWidget.cc -- class for the preview window in the UI (and more)
 *
 * Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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
#include <cstdlib>
#include <math.h>
#include <pthread.h>

extern "C" {
#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/video.h>
#include <openastro/imgproc.h>
#include <openastro/video/formats.h>
}

#include "configuration.h"
#include "previewWidget.h"
#include "outputHandler.h"
#include "histogramWidget.h"
#include "focusOverlay.h"
#include "state.h"


// FIX ME -- Lots of this stuff needs refactoring or placing elsewhere
// as it's really not anything to do with the actual preview window
// any more

PreviewWidget::PreviewWidget ( QWidget* parent ) : QFrame ( parent )
{
  currentZoom = 100;
  int zoomFactor = state.zoomWidget->getZoomFactor();
  // setAttribute( Qt::WA_NoSystemBackground, true );
  lastCapturedFramesUpdateTime = 0;
  capturedFramesDisplayInterval = 200;
  lastDisplayUpdateTime = 0;
  frameDisplayInterval = 1000/15; // display frames per second
  previewEnabled = 1;
  videoFramePixelFormat = OA_PIX_FMT_RGB24;
  framesInFpsCalcPeriod = fpsCalcPeriodStartTime = 0;
  secondForTemperature = secondForDropped = 0;
  flipX = flipY = 0;
  movingReticle = rotatingReticle = rotationAngle = 0;
  savedXSize = savedYSize = 0;
  recalculateDimensions ( zoomFactor );
  previewBufferLength = 0;
  previewImageBuffer[0] = writeImageBuffer[0] = 0;
  previewImageBuffer[1] = writeImageBuffer[1] = 0;
  expectedSize = config.imageSizeX * config.imageSizeY *
      OA_BYTES_PER_PIXEL( videoFramePixelFormat );
  demosaic = config.demosaic;

  int r = config.currentColouriseColour.red();
  int g = config.currentColouriseColour.green();
  int b = config.currentColouriseColour.blue();
  int cr, cg, cb;
  cr = cg = cb = 0;
  greyscaleColourTable.clear();
  falseColourTable.clear();
  for ( int i = 0; i < 256; i++ ) {
    greyscaleColourTable.append ( QColor ( i, i, i ).rgb());
    falseColourTable.append ( QColor ( cr / 256, cg / 256, cb / 256 ).rgb());
    cr += r;
    cg += g;
    cb += b;
  }

  pthread_mutex_init ( &imageMutex, 0 );
  recordingInProgress = 0;
  manualStop = 0;

  connect ( this, SIGNAL( updateDisplay ( void )),
      this, SLOT( update ( void )));
}


PreviewWidget::~PreviewWidget()
{
  if ( previewImageBuffer[0] ) {
    free ( previewImageBuffer[0] );
    free ( previewImageBuffer[1] );
  }
  if ( writeImageBuffer[0] ) {
    free ( writeImageBuffer[0] );
    free ( writeImageBuffer[1] );
  }
}


void
PreviewWidget::configure ( void )
{
  // setGeometry ( 0, 0, config.imageSizeX, config.imageSizeY );
}


void
PreviewWidget::updatePreviewSize ( void )
{
  int zoomFactor = state.zoomWidget->getZoomFactor();
  recalculateDimensions ( zoomFactor );
  expectedSize = config.imageSizeX * config.imageSizeY *
      OA_BYTES_PER_PIXEL( videoFramePixelFormat );
  int newBufferLength = config.imageSizeX * config.imageSizeY * 3;
  if ( newBufferLength > previewBufferLength ) {
    if (!( previewImageBuffer[0] = realloc ( previewImageBuffer[0],
        newBufferLength ))) {
      qWarning() << "preview image buffer realloc failed";
    }
    if (!( previewImageBuffer[1] = realloc ( previewImageBuffer[1],
        newBufferLength ))) {
      qWarning() << "preview image buffer realloc failed";
    }
    previewBufferLength = newBufferLength;
  }
  diagonalLength = sqrt ( config.imageSizeX * config.imageSizeX +
      config.imageSizeY * config.imageSizeY );
}


void
PreviewWidget::recalculateDimensions ( int zoomFactor )
{
  currentZoom = zoomFactor;
  currentZoomX = config.imageSizeX * zoomFactor / 100;
  currentZoomY = config.imageSizeY * zoomFactor / 100;
  if ( savedXSize && savedYSize ) {
    reticleCentreX = ( reticleCentreX * currentZoomX ) / savedXSize;
    reticleCentreY = ( reticleCentreY * currentZoomY ) / savedYSize;
  } else {
    reticleCentreX = currentZoomX / 2;
    reticleCentreY = currentZoomY / 2;
  }
  setGeometry ( 0, 0, currentZoomX, currentZoomY );
  savedXSize = currentZoomX;
  savedYSize = currentZoomY;
  if ( 0 == rotationAngle ) {
    rotationTransform.reset();
  } else {
    rotationTransform = QTransform().translate ( reticleCentreX,
        reticleCentreY ).rotate ( rotationAngle ).translate ( -reticleCentreX,
        -reticleCentreY );
  }
}


void
PreviewWidget::paintEvent ( QPaintEvent* event )
{
  Q_UNUSED( event );

  QPainter painter ( this );

  pthread_mutex_lock ( &imageMutex );
  painter.drawImage ( 0, 0, image );
  pthread_mutex_unlock ( &imageMutex );

  painter.setTransform ( rotationTransform );
  painter.setRenderHint ( QPainter::Antialiasing, true );
  painter.setPen ( QPen ( Qt::red, 2, Qt::SolidLine, Qt::FlatCap ));

  if ( config.showReticle ) {
    switch ( config.reticleStyle ) {

      case RETICLE_CIRCLE:
        painter.drawEllipse ( reticleCentreX - 50, reticleCentreY - 50,
          100, 100 );
        painter.drawEllipse ( reticleCentreX - 100, reticleCentreY - 100,
          200, 200 );
        painter.drawEllipse ( reticleCentreX - 200, reticleCentreY - 200,
          400, 400 );
        painter.drawLine ( reticleCentreX, reticleCentreY - 5,
          reticleCentreX, reticleCentreY + 5 );
        painter.drawLine ( reticleCentreX - 5, reticleCentreY,
          reticleCentreX + 5, reticleCentreY );
        break;

      case RETICLE_CROSS:
        // drawing lines at least the diagonal length long will mean
        // they always run to the edge of the displayed image.  The
        // widget will crop the lines as appropriate
        painter.drawLine ( reticleCentreX, reticleCentreY - diagonalLength,
            reticleCentreX, reticleCentreY - 10 );
        painter.drawLine ( reticleCentreX, reticleCentreY + diagonalLength,
            reticleCentreX, reticleCentreY + 10 );
        painter.drawLine ( reticleCentreX - diagonalLength, reticleCentreY,
            reticleCentreX - 10, reticleCentreY );
        painter.drawLine ( reticleCentreX + diagonalLength, reticleCentreY,
            reticleCentreX + 10, reticleCentreY );
        break;

      case RETICLE_TRAMLINES:
        // see comments for cross
        painter.drawLine ( reticleCentreX - 10,
            reticleCentreY - diagonalLength, reticleCentreX - 10,
            reticleCentreY + diagonalLength );
        painter.drawLine ( reticleCentreX + 10,
            reticleCentreY - diagonalLength, reticleCentreX + 10,
            reticleCentreY + diagonalLength );
        painter.drawLine ( reticleCentreX - diagonalLength,
            reticleCentreY - 10, reticleCentreX + diagonalLength,
            reticleCentreY - 10 );
        painter.drawLine ( reticleCentreX - diagonalLength,
            reticleCentreY + 10, reticleCentreX + diagonalLength,
            reticleCentreY + 10 );
        break;
    }
  }
}


void
PreviewWidget::mousePressEvent ( QMouseEvent* event )
{
  if ( event->button() == Qt::LeftButton ) {
    lastPointerX = event->x();
    lastPointerY = event->y();
    if (( abs ( lastPointerX - reticleCentreX ) < 20 ) &&
        ( abs ( lastPointerY - reticleCentreY ) < 20 )) {
      movingReticle = 1;
    }
  }
}


void
PreviewWidget::mouseMoveEvent ( QMouseEvent* event )
{
  int x = event->x();
  int y = event->y();
  if ( movingReticle ) {
    reticleCentreX += ( x - lastPointerX );
    reticleCentreY += ( y - lastPointerY );
  }
  lastPointerX = x;
  lastPointerY = y;
}

void
PreviewWidget::mouseReleaseEvent ( QMouseEvent* event )
{
  if ( event->button() == Qt::LeftButton ) {
    movingReticle = rotatingReticle = 0;
  }
}


void
PreviewWidget::wheelEvent ( QWheelEvent* event )
{
  int change = event->delta();
  int direction = ( change > 0 ) - ( change < 0 );
  if ( !direction ) {
    event->ignore();
  } else {
    int x = event->x();
    qreal scale = ( qreal ) abs ( x - reticleCentreX ) / 50;
    rotationAngle += direction * scale;
    if ( 0 == rotationAngle ) {
      rotationTransform.reset();
    } else {
      rotationTransform = QTransform().translate ( reticleCentreX,
          reticleCentreY ).rotate ( rotationAngle ).translate ( -reticleCentreX,
          -reticleCentreY );
    }
    event->accept();
  }
}


void
PreviewWidget::recentreReticle ( void )
{
  reticleCentreX = currentZoomX / 2;
  reticleCentreY = currentZoomY / 2;
}


void
PreviewWidget::derotateReticle ( void )
{
  rotationTransform.reset();
  rotationAngle = 0;
}


void
PreviewWidget::setCapturedFramesDisplayInterval ( int millisecs )
{
  capturedFramesDisplayInterval = millisecs;
}


void
PreviewWidget::setEnabled ( int state )
{
  previewEnabled = state;
}


void
PreviewWidget::setVideoFramePixelFormat ( int format )
{
  videoFramePixelFormat = format;
  expectedSize = config.imageSizeX * config.imageSizeY *
      OA_BYTES_PER_PIXEL( videoFramePixelFormat );
  state.mainWindow->setPixelFormatValue( format );
}


void
PreviewWidget::enableTempDisplay ( int state )
{
  hasTemp = state;
}


void
PreviewWidget::enableDroppedDisplay ( int state )
{
  hasDroppedFrames = state;
}


void
PreviewWidget::enableFlipX ( int state )
{
  flipX = state;
}


void
PreviewWidget::enableFlipY ( int state )
{
  flipY = state;
}


void
PreviewWidget::enableDemosaic ( int state )
{
  demosaic = state;
}


void
PreviewWidget::setDisplayFPS ( int fps )
{
  frameDisplayInterval = 1000 / fps;
}


// FIX ME -- could combine this with beginRecording() ?
void
PreviewWidget::setFirstFrameTime ( void )
{
  setNewFirstFrameTime = 1;
}


void
PreviewWidget::beginRecording ( void )
{
  recordingInProgress = 1;
}


void
PreviewWidget::forceRecordingStop ( void )
{
  manualStop = 1;
}


void
PreviewWidget::processFlip ( void* imageData, int length, int format )
{
  uint8_t* data = ( uint8_t* ) imageData;
  int assumedFormat = format;

  // fake up a format for mosaic frames here as properly flipping a
  // mosaicked frame would be quite hairy

  if ( OA_ISBAYER8 ( format )) {
    assumedFormat = OA_PIX_FMT_GREY8;
  } else {
    if ( OA_ISBAYER16 ( format )) {
      assumedFormat = OA_PIX_FMT_GREY16BE;
    }
  }

  switch ( assumedFormat ) {
    case OA_PIX_FMT_GREY8:
      processFlip8Bit ( data, length );
      break;
    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_GREY16LE:
      processFlip16Bit ( data, length );
      break;
    case OA_PIX_FMT_RGB24:
    case OA_PIX_FMT_BGR24:
      processFlip24BitColour ( data, length );
      break;
    default:
      qWarning() << __FUNCTION__ << " unable to flip format " <<
          OA_PIX_FMT_STRING(format);
      break;
  }
}


void
PreviewWidget::processFlip8Bit ( uint8_t* imageData, int length )
{
  if ( flipX && flipY ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 1;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2-- = s;
    }
  } else {
    if ( flipX ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      for ( unsigned int y = 0; y < config.imageSizeY; y++ ) {
        p1 = imageData + y * config.imageSizeX;
        p2 = p1 + config.imageSizeX - 1;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2-- = s;
        }
      }
    }
    if ( flipY ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      p1 = imageData;
      for ( unsigned int y = config.imageSizeY - 1; y >= config.imageSizeY / 2;
          y-- ) {
        p2 = imageData + y * config.imageSizeX;
        for ( unsigned int x = 0; x < config.imageSizeX; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


void
PreviewWidget::processFlip16Bit ( uint8_t* imageData, int length )
{
  if ( flipX && flipY ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 2;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2 = s;
      p2 -= 3;
    }
  } else {
    if ( flipX ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      for ( unsigned int y = 0; y < config.imageSizeY; y++ ) {
        p1 = imageData + y * config.imageSizeX * 2;
        p2 = p1 + ( config.imageSizeX - 1 ) * 2;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2 = s;
          p2 -= 3;
        }
      }
    }
    if ( flipY ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      p1 = imageData;
      for ( unsigned int y = config.imageSizeY - 1; y > config.imageSizeY / 2;
          y-- ) {
        p2 = imageData + y * config.imageSizeX * 2;
        for ( unsigned int x = 0; x < config.imageSizeX * 2; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


void
PreviewWidget::processFlip24BitColour ( uint8_t* imageData, int length )
{
  if ( flipX && flipY ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 3;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2 = s;
      p2 -= 5;
    }
  } else {
    if ( flipX ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      for ( unsigned int y = 0; y < config.imageSizeY; y++ ) {
        p1 = imageData + y * config.imageSizeX * 3;
        p2 = p1 + ( config.imageSizeX - 1 ) * 3;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2 = s;
          p2 -= 5;
        }
      }
    }
    if ( flipY ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      p1 = imageData;
      for ( unsigned int y = config.imageSizeY - 1; y > config.imageSizeY / 2;
          y-- ) {
        p2 = imageData + y * config.imageSizeX * 3;
        for ( unsigned int x = 0; x < config.imageSizeX * 3; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


void
PreviewWidget::convert16To8Bit ( void* imageData, int length, int format )
{
  uint8_t* t = ( uint8_t* ) imageData;
  uint8_t* s = ( uint8_t* ) imageData;

  if ( OA_ISLITTE_ENDIAN( format )) {
    s++;
  }
  for ( int i = 0; i < length; i += 2, s += 2 ) {
    *t++ = *s;
  }
}


void
PreviewWidget::setMonoPalette ( QColor colour )
{
  unsigned int r = colour.red();
  unsigned int g = colour.green();
  unsigned int b = colour.blue();
  unsigned int cr, cg, cb;
  cr = cg = cb = 0;
  QRgb rgb;
  for ( int i = 0; i < 256; i++ ) {
    // This looks odd, but the shifts make sense because we actually
    // want ( <colour-component> / 256 ) shifted into the right place
    rgb = 0xff000000 | ((  cr & 0xff00 ) << 8 ) | ( cg & 0xff00 ) | ( cb >> 8 );
    falseColourTable[i] = rgb;
    cr += r;
    cg += g;
    cb += b;
  } 
}


void*
PreviewWidget::updatePreview ( void* args, void* imageData, int length )
{
  STATE*		state = ( STATE* ) args;
  PreviewWidget*	self = state->previewWidget;
  struct timeval	t;
  int			doDisplay = 0;
  int			doHistogram = 0;
  int			previewPixelFormat, writePixelFormat;
  // write straight from the data if possible
  void*			previewBuffer = imageData;
  void*			writeBuffer = imageData;
  int			currentPreviewBuffer = -1;
  int			writeDemosaicPreviewBuffer = 0;
  int			previewIsDemosaicked = 0;
  int			maxLength;
  const char*		timestamp;

  // don't do anything if the length is not as expected
  if ( length != self->expectedSize ) {
    // qWarning() << "size mismatch.  have:" << length << " expected: "
    //    << self->expectedSize;
    return 0;
  }

  // assign the temporary buffers for image transforms if they
  // don't already exist or the existing ones are too small

  maxLength = config.imageSizeX * config.imageSizeY * 6;
  if ( !self->previewImageBuffer[0] ||
      self->previewBufferLength < maxLength ) {
    self->previewBufferLength = maxLength;
    if (!( self->previewImageBuffer[0] =
        realloc ( self->previewImageBuffer[0], self->previewBufferLength ))) {
      qWarning() << "preview image buffer 0 malloc failed";
      return 0;
    }
    if (!( self->previewImageBuffer[1] =
        realloc ( self->previewImageBuffer[1], self->previewBufferLength ))) {
      qWarning() << "preview image buffer 1 malloc failed";
      return 0;
    }
  }
  if ( !self->writeImageBuffer[0] ||
      self->writeBufferLength < maxLength ) {
    self->writeBufferLength = maxLength;
    if (!( self->writeImageBuffer[0] =
        realloc ( self->writeImageBuffer[0], self->writeBufferLength ))) {
      qWarning() << "write image buffer 0 malloc failed";
      return 0;
    }
    if (!( self->writeImageBuffer[1] =
        realloc ( self->writeImageBuffer[1], self->writeBufferLength ))) {
      qWarning() << "write image buffer 1 malloc failed";
      return 0;
    }
  }

  previewPixelFormat = writePixelFormat = self->videoFramePixelFormat;

  // if we have a luminance/chrominance video format then we need to
  // unpack that first

  if ( OA_IS_LUM_CHROM( self->videoFramePixelFormat )) {
    // this is going to make the flip quite ugly and means we need to
    // start using currentPreviewBuffer too
    currentPreviewBuffer = ( -1 == currentPreviewBuffer ) ? 0 :
        !currentPreviewBuffer;
    previewPixelFormat = OA_PIX_FMT_RGB24;
    ( void ) oaconvert ( previewBuffer,
        self->previewImageBuffer[ currentPreviewBuffer ], config.imageSizeX,
        config.imageSizeY, self->videoFramePixelFormat, previewPixelFormat );
    previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];

    // we can flip the preview image here if required, but not the
    // image that is going to be written out.
    // FIX ME -- work this out some time

    if ( self->flipX || self->flipY ) {
      self->processFlip ( previewBuffer, length, previewPixelFormat );
    }
  } else {
    // do a vertical/horizontal flip if required
    if ( self->flipX || self->flipY ) {
      // this is going to make a mess for data we intend to demosaic.
      // the user will have to deal with that
      ( void ) memcpy ( self->writeImageBuffer[0], writeBuffer, length );
      self->processFlip ( self->writeImageBuffer[0], length,
          writePixelFormat );
      // both preview and write will come from this buffer for the
      // time being.  This may change later on
      previewBuffer = self->writeImageBuffer[0];
      writeBuffer = self->writeImageBuffer[0];
    }
  }

  int reducedGreyscaleBitDepth = 0;
  if ( OA_PIX_FMT_GREY16BE == previewPixelFormat ||
      OA_PIX_FMT_GREY16LE == previewPixelFormat ||
      OA_ISBAYER16 ( previewPixelFormat )) {
    currentPreviewBuffer = ( -1 == currentPreviewBuffer ) ? 0 :
        !currentPreviewBuffer;
    ( void ) memcpy ( self->previewImageBuffer[ currentPreviewBuffer ],
        previewBuffer, length );
    self->convert16To8Bit ( self->previewImageBuffer[ currentPreviewBuffer ],
        length, previewPixelFormat );
    previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];
    if (  OA_PIX_FMT_GREY16BE == previewPixelFormat ||
        OA_PIX_FMT_GREY16LE == previewPixelFormat ) {
      reducedGreyscaleBitDepth = 1;
    }
  }

  ( void ) gettimeofday ( &t, 0 );
  unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
      ( unsigned long ) t.tv_usec / 1000;

  int cfaPattern = config.cfaPattern;
  if ( OA_ISBAYER ( previewPixelFormat )) {
    if ( OA_DEMOSAIC_AUTO == cfaPattern ) {
      cfaPattern = self->formatToCfaPattern ( previewPixelFormat );
    }
  }

  if ( self->previewEnabled ) {
    if (( self->lastDisplayUpdateTime + self->frameDisplayInterval ) < now ) {
      self->lastDisplayUpdateTime = now;
      doDisplay = 1;

      if ( self->demosaic && config.demosaicPreview ) {
        if ( OA_ISBAYER ( previewPixelFormat )) {
          currentPreviewBuffer = ( -1 == currentPreviewBuffer ) ? 0 :
              !currentPreviewBuffer;
          // Use the demosaicking to copy the data to the previewImageBuffer
          ( void ) oademosaic ( previewBuffer,
              self->previewImageBuffer[ currentPreviewBuffer ],
              config.imageSizeX, config.imageSizeY, 8, cfaPattern,
              config.demosaicMethod );
          if ( config.demosaicOutput && previewBuffer == writeBuffer ) {
            writeDemosaicPreviewBuffer = 1;
          }
          previewIsDemosaicked = 1;
          previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];
        }
      }

      if ( config.showFocusAid ) {
        int fmt = previewPixelFormat;

        if ( previewIsDemosaicked ) {
          fmt = OA_ISBAYER16 ( fmt )  ? OA_PIX_FMT_RGB48BE :
              OA_PIX_FMT_RGB24;
        }
        // This call should be thread-safe
        state->focusOverlay->addScore ( oaFocusScore ( previewBuffer,
            0, config.imageSizeX, config.imageSizeY, fmt ));
      }

      QImage* newImage;
      QImage* swappedImage = 0;

      if ( OA_PIX_FMT_GREY8 == self->videoFramePixelFormat ||
           ( OA_ISBAYER ( previewPixelFormat ) && ( !self->demosaic ||
           !config.demosaicPreview )) || reducedGreyscaleBitDepth ) {
        newImage = new QImage (( const uint8_t* ) previewBuffer,
            config.imageSizeX, config.imageSizeY, config.imageSizeX,
            QImage::Format_Indexed8 );
        if (( OA_PIX_FMT_GREY8 == self->videoFramePixelFormat ||
            reducedGreyscaleBitDepth ) && config.colourise ) {
          newImage->setColorTable ( self->falseColourTable );
        } else {
          newImage->setColorTable ( self->greyscaleColourTable );
        }
        swappedImage = newImage;
      } else {
        // Need the stride size here or QImage appears to "tear" the
        // right hand edge of the image when the X dimension is an odd
        // number of pixels
        newImage = new QImage (( const uint8_t* ) previewBuffer,
            config.imageSizeX, config.imageSizeY, config.imageSizeX * 3,
            QImage::Format_RGB888 );
        if ( OA_PIX_FMT_BGR24 == previewPixelFormat ) {
          swappedImage = new QImage ( newImage->rgbSwapped());
        } else {
          swappedImage = newImage;
        }
      }

      // This call should be thread-safe
      int zoomFactor = state->zoomWidget->getZoomFactor();
      if ( zoomFactor && zoomFactor != self->currentZoom ) {
        self->recalculateDimensions ( zoomFactor );
      }

      if ( self->currentZoom != 100 ) {
        QImage scaledImage = swappedImage->scaled ( self->currentZoomX,
          self->currentZoomY );

        if ( config.showFocusAid ) {
        }

        pthread_mutex_lock ( &self->imageMutex );
        self->image = scaledImage.copy();
        pthread_mutex_unlock ( &self->imageMutex );
      } else {
        pthread_mutex_lock ( &self->imageMutex );
        self->image = swappedImage->copy();
        pthread_mutex_unlock ( &self->imageMutex );
      }
      if ( swappedImage != newImage ) {
        delete swappedImage;
      }
      delete newImage;
    }
  }

  OutputHandler* output = 0;
  if ( !state->pauseEnabled ) {
    // This should be thread-safe
    output = state->captureWidget->getOutputHandler();
    if ( output && self->recordingInProgress ) {
      if ( self->setNewFirstFrameTime ) {
        state->firstFrameTime = now;
        self->setNewFirstFrameTime = 0;
      }
      state->lastFrameTime = now;
      if ( config.demosaicOutput && OA_ISBAYER ( writePixelFormat )) {
        if ( writeDemosaicPreviewBuffer ) {
          writeBuffer = previewBuffer;
        } else {
          // we can use the preview buffer here because we're done with it
          // for actual preview purposes
          // If it's possible that the write CFA pattern is not the same
          // as the preview one, this code will need fixing to reset
          // cfaPattern, but I can't see that such a thing is possible
          // at the moment
          ( void ) oademosaic ( writeBuffer,
              self->previewImageBuffer[0], config.imageSizeX,
              config.imageSizeY, 8, cfaPattern, config.demosaicMethod );
          writeBuffer = self->previewImageBuffer[0];
        }
        writePixelFormat = OA_DEMOSAIC_FMT ( writePixelFormat );
      }
      // These calls should be thread-safe
      if ( state->timer->isInitialised() && state->timer->isRunning()) {
        timestamp = state->timer->readTimestamp();
      } else {
        timestamp = 0;
      }
      if ( output->addFrame ( writeBuffer, timestamp,
          // This call should be thread-safe
          state->controlWidget->getCurrentExposure()) < 0 ) {
        self->recordingInProgress = 0;
        self->manualStop = 0;
        state->autorunEnabled = 0;
        emit self->stopRecording();
        emit self->frameWriteFailed();
      } else {
        if (( self->lastCapturedFramesUpdateTime +
            self->capturedFramesDisplayInterval ) < now ) {
          emit self->updateFrameCount ( output->getFrameCount());
          self->lastCapturedFramesUpdateTime = now;
        }
      }
    }
  }

  self->framesInFpsCalcPeriod++;
  if ( self->framesInFpsCalcPeriod &&
      now - self->fpsCalcPeriodStartTime > 1000 ) {
    double fpsCalcPeriodDuration_s = (now - self->fpsCalcPeriodStartTime)/1000.0;
    emit self->updateActualFrameRate (self->framesInFpsCalcPeriod / fpsCalcPeriodDuration_s);

    self->fpsCalcPeriodStartTime = now;
    self->framesInFpsCalcPeriod = 0;

    if ( state->histogramOn ) {
      // This call should be thread-safe
      state->histogramWidget->process ( writeBuffer, length,
          writePixelFormat );
      doHistogram = 1;
    }
  }

  if ( self->hasTemp && t.tv_sec != self->secondForTemperature &&
      t.tv_sec % 5 == 0 ) {
    emit self->updateTemperature();
    self->secondForTemperature = t.tv_sec;
  }
  if ( self->hasDroppedFrames && t.tv_sec != self->secondForDropped &&
      t.tv_sec % 2 == 0 ) {
    emit self->updateDroppedFrames();
    self->secondForTemperature = t.tv_sec;
  }

  if ( doDisplay ) {
    emit self->updateDisplay();
  }

  // check histogram control here just in case it got changed
  // this ought to be done rather more safely
  if ( state->histogramOn && state->histogramWidget && doHistogram ) {
    emit self->updateHistogram();
  }

  if ( self->manualStop ) {
    self->recordingInProgress = 0;
    emit self->stopRecording();
    self->manualStop = 0;
  }

if ( output && self->recordingInProgress ) {
    if ( config.limitEnabled ) {
      int finished = 0;
      float percentage = 0;
      int frames = output->getFrameCount();
      switch ( config.limitType ) {
        case 0: // FIX ME -- nasty magic number
          // start and current times here are in ms, but the limit value is in
          // secs, so rather than ( current - start ) / time * 100 to get the
          // %age, we do ( current - start ) / time / 10
          percentage = ( now - state->captureWidget->recordingStartTime ) /
              ( config.secondsLimitValue * 1000.0 +
              state->captureWidget->totalTimePaused ) * 100.0;
          if ( now > state->captureWidget->recordingEndTime ) {
            finished = 1;
          }
          break;
        case 1: // FIX ME -- nasty magic number
          percentage = ( 100.0 * frames ) / config.framesLimitValue;
          if ( frames >= config.framesLimitValue ) {
            finished = 1;
          }
          break;
      }
      if ( finished ) {
        // need to stop now, even if we don't know what's happening
        // with the UI
        self->recordingInProgress = 0;
        self->manualStop = 0;
        emit self->stopRecording();
        // these two are really just tidying up the display
        emit self->updateProgress ( 100 );
        emit self->updateFrameCount ( frames );
        if ( state->autorunEnabled ) {
          // returns non-zero if more runs are left
          // This call is thread-safe because the called function is aware
          // that the GUI changes it makes must be done indirectly
          // FIX ME -- doing it with invokeMethod would be nicer though
          if ( state->captureWidget->singleAutorunFinished()) {
            state->autorunStartNext = now + 1000 * config.autorunDelay;
          }
        }
      } else {
        emit self->updateProgress (( unsigned int ) percentage );
      }
    }
  }

  if ( state->autorunEnabled && state->autorunStartNext &&
      now > state->autorunStartNext ) {
    state->autorunStartNext = 0;
    // Have to do it this way rather than calling direct to ensure
    // thread-safety
    QMetaObject::invokeMethod ( state->captureWidget, "startNewAutorun",
        Qt::BlockingQueuedConnection );
  }

  return 0;
}


int
PreviewWidget::formatToCfaPattern ( int format )
{
  switch ( format ) {
    case OA_PIX_FMT_BGGR8:
    case OA_PIX_FMT_BGGR16LE:
    case OA_PIX_FMT_BGGR16BE:
      return OA_DEMOSAIC_BGGR;
      break;
    case OA_PIX_FMT_RGGB8:
    case OA_PIX_FMT_RGGB16LE:
    case OA_PIX_FMT_RGGB16BE:
      return OA_DEMOSAIC_RGGB;
      break;
    case OA_PIX_FMT_GBRG8:
    case OA_PIX_FMT_GBRG16LE:
    case OA_PIX_FMT_GBRG16BE:
      return OA_DEMOSAIC_GBRG;
      break;
    case OA_PIX_FMT_GRBG8:
    case OA_PIX_FMT_GRBG16LE:
    case OA_PIX_FMT_GRBG16BE:
      return OA_DEMOSAIC_GRBG;
      break;
  }
  qWarning() << "Invalid format (" << OA_PIX_FMT_STRING(format) <<
      ") in" << __FUNCTION__;
  return 0;
}
