/*****************************************************************************
 *
 * viewWidget.cc -- class for the viewing window in the UI (and more)
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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
#include "viewWidget.h"
#include "outputHandler.h"
#include "focusOverlay.h"
#include "state.h"


// FIX ME -- Lots of this stuff needs refactoring or placing elsewhere
// as it's really not anything to do with the actual view window
// any more

ViewWidget::ViewWidget ( QWidget* parent ) : QFrame ( parent )
{
  // FIX ME -- need to read this from the controls widget
  currentZoom = 100;
  int zoomFactor = 100; // state.zoomWidget->getZoomFactor();
  lastCapturedFramesUpdateTime = 0;
  capturedFramesDisplayInterval = 200;
  lastDisplayUpdateTime = 0;
  frameDisplayInterval = 1000/15; // display frames per second
  videoFramePixelFormat = OA_PIX_FMT_RGB24;
  framesInLastSecond = 0;
  secondForFrameCount = secondForTemperature = secondForDropped = 0;
  flipX = flipY = 0;
  movingReticle = rotatingReticle = rotationAngle = 0;
  savedXSize = savedYSize = 0;
  recalculateDimensions ( zoomFactor );
  stackBufferLength = viewBufferLength = 0;
  viewImageBuffer[0] = writeImageBuffer[0] = stackBuffer[0] = 0;
  viewImageBuffer[1] = writeImageBuffer[1] = stackBuffer[1] = 0;
  currentStackBuffer = -1;
  stackBufferInUse = 0;
  averageBuffer = 0;
  expectedSize = config.imageSizeX * config.imageSizeY *
      OA_BYTES_PER_PIXEL( videoFramePixelFormat );
  totalFrames = 0;

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


ViewWidget::~ViewWidget()
{
  if ( viewImageBuffer[0] ) {
    free ( viewImageBuffer[0] );
    free ( viewImageBuffer[1] );
  }
  if ( writeImageBuffer[0] ) {
    free ( writeImageBuffer[0] );
    free ( writeImageBuffer[1] );
  }
  if ( stackBuffer[0] ) {
    free ( stackBuffer[0] );
    free ( stackBuffer[1] );
  }
  if ( averageBuffer ) {
    free (( void* ) averageBuffer );
  }
}


void
ViewWidget::configure ( void )
{
  // setGeometry ( 0, 0, config.imageSizeX, config.imageSizeY );
}


void
ViewWidget::updateFrameSize ( void )
{
  // FIX ME -- need to read this from the controls widget
  int zoomFactor = 100; // state.zoomWidget->getZoomFactor();
  recalculateDimensions ( zoomFactor );
  expectedSize = config.imageSizeX * config.imageSizeY *
      OA_BYTES_PER_PIXEL( videoFramePixelFormat );
qWarning() << "check new buffer length";
  int newBufferLength = config.imageSizeX * config.imageSizeY * 3;
  if ( newBufferLength > viewBufferLength ) {
    if (!( viewImageBuffer[0] = realloc ( viewImageBuffer[0],
        newBufferLength ))) {
      qWarning() << "view image buffer realloc failed";
    }
    if (!( viewImageBuffer[1] = realloc ( viewImageBuffer[1],
        newBufferLength ))) {
      qWarning() << "view image buffer realloc failed";
    }
    if (!( stackBuffer[0] = realloc ( stackBuffer[0], newBufferLength ))) {
      qWarning() << "stack image buffer realloc failed";
    }
    if (!( stackBuffer[1] = realloc ( stackBuffer[1], newBufferLength ))) {
      qWarning() << "stack image buffer realloc failed";
    }
    stackBufferLength = viewBufferLength = newBufferLength;
  }
  diagonalLength = sqrt ( config.imageSizeX * config.imageSizeX +
      config.imageSizeY * config.imageSizeY );
  memset ( stackBuffer[0], 0, stackBufferLength );
  memset ( stackBuffer[1], 0, stackBufferLength );
}


void
ViewWidget::recalculateDimensions ( int zoomFactor )
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
ViewWidget::paintEvent ( QPaintEvent* event )
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
ViewWidget::mousePressEvent ( QMouseEvent* event )
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
ViewWidget::mouseMoveEvent ( QMouseEvent* event )
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
ViewWidget::mouseReleaseEvent ( QMouseEvent* event )
{
  if ( event->button() == Qt::LeftButton ) {
    movingReticle = rotatingReticle = 0;
  }
}


void
ViewWidget::wheelEvent ( QWheelEvent* event )
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
ViewWidget::recentreReticle ( void )
{
  reticleCentreX = currentZoomX / 2;
  reticleCentreY = currentZoomY / 2;
}


void
ViewWidget::derotateReticle ( void )
{
  rotationTransform.reset();
  rotationAngle = 0;
}


void
ViewWidget::setCapturedFramesDisplayInterval ( int millisecs )
{
  capturedFramesDisplayInterval = millisecs;
}


void
ViewWidget::setVideoFramePixelFormat ( int format )
{
  videoFramePixelFormat = format;
  expectedSize = config.imageSizeX * config.imageSizeY *
      OA_BYTES_PER_PIXEL( videoFramePixelFormat );
}


void
ViewWidget::enableTempDisplay ( int state )
{
  hasTemp = state;
}


void
ViewWidget::enableDroppedDisplay ( int state )
{
  hasDroppedFrames = state;
}


void
ViewWidget::enableFlipX ( int state )
{
  flipX = state;
}


void
ViewWidget::enableFlipY ( int state )
{
  flipY = state;
}


void
ViewWidget::setDisplayFPS ( int fps )
{
qWarning() << "Does ViewWidget::setDisplayFPS make sense any more?";
  frameDisplayInterval = 1000 / fps;
}

/*
// FIX ME -- could combine this with beginRecording() ?
void
ViewWidget::setFirstFrameTime ( void )
{
  setNewFirstFrameTime = 1;
}


void
ViewWidget::beginRecording ( void )
{
  recordingInProgress = 1;
}


void
ViewWidget::forceRecordingStop ( void )
{
  manualStop = 1;
}
*/

void
ViewWidget::processFlip ( void* imageData, int length, int format )
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
      qWarning() << __FUNCTION__ << " unable to flip format " << format;
      break;
  }
}


void
ViewWidget::processFlip8Bit ( uint8_t* imageData, int length )
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
ViewWidget::processFlip16Bit ( uint8_t* imageData, int length )
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
ViewWidget::processFlip24BitColour ( uint8_t* imageData, int length )
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
ViewWidget::convert16To8Bit ( void* imageData, int length, int format )
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
ViewWidget::setMonoPalette ( QColor colour )
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
ViewWidget::addImage ( void* args, void* imageData, int length )
{
  STATE*		state = ( STATE* ) args;
  ViewWidget*		self = state->viewWidget;
  struct timeval	t;
  int			doDisplay = 0;
  int			doHistogram = 0;
  int			viewPixelFormat, writePixelFormat;
  // write straight from the data if possible
  void*			viewBuffer = imageData;
  void*			writeBuffer = imageData;
  int			currentViewBuffer = -1;
  // commented to stop unused error int			writeDemosaicViewBuffer = 0;
  int			viewIsDemosaicked = 0;
  int			ret;
  OutputHandler*	output;

  // don't do anything if the length is not as expected
  if ( length != self->expectedSize ) {
    qWarning() << "size mismatch.  have:" << length << " expected: "
       << self->expectedSize;
    return 0;
  }

  if (( ret = self->checkBuffers ( self ))) {
    return 0;
  }

  viewPixelFormat = writePixelFormat = self->videoFramePixelFormat;

  // if we have a luminance/chrominance video format then we need to
  // unpack that first

  if ( OA_IS_LUM_CHROM( self->videoFramePixelFormat )) {
    // this is going to make the flip quite ugly and means we need to
    // start using currentViewBuffer too
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
        !currentViewBuffer;
    viewPixelFormat = OA_PIX_FMT_RGB24;
    ( void ) oaconvert ( viewBuffer,
        self->viewImageBuffer[ currentViewBuffer ], config.imageSizeX,
        config.imageSizeY, self->videoFramePixelFormat, viewPixelFormat );
    viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
  } else {
    if ( OA_ISBAYER(  self->videoFramePixelFormat )) {
      int cfaPattern = config.cfaPattern;
      if ( OA_ISBAYER ( viewPixelFormat )) {
        if ( OA_DEMOSAIC_AUTO == cfaPattern ) {
          cfaPattern = self->formatToCfaPattern ( viewPixelFormat );
        }
      }
      currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
          !currentViewBuffer;
      // Use the demosaicking to copy the data to the viewImageBuffer
      ( void ) oademosaic ( viewBuffer,
          self->viewImageBuffer[ currentViewBuffer ],
          config.imageSizeX, config.imageSizeY, 8, cfaPattern,
          config.demosaicMethod );
      if ( viewBuffer == writeBuffer ) {
        // commented to stop unused error writeDemosaicViewBuffer = 1;
      }
      viewIsDemosaicked = 1;
      viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
    }
  }

  // Hopefully at this point we have the original frame in the write
  // buffer and RGB or mono in the view buffer (which may be the same
  // as the write buffer

  /*
   * FIX ME -- handle a manual flip here if required
   *
    // we can flip the view image here if required, but not the
    // image that is going to be written out.
    // FIX ME -- work this out some time

    if ( self->flipX || self->flipY ) {
      self->processFlip ( viewBuffer, length, viewPixelFormat );
    }
  } else {
    // do a vertical/horizontal flip if required
    if ( self->flipX || self->flipY ) {
      // this is going to make a mess for data we intend to demosaic.
      // the user will have to deal with that
      ( void ) memcpy ( self->writeImageBuffer[0], writeBuffer, length );
      self->processFlip ( self->writeImageBuffer[0], length,
          writePixelFormat );
      // both view and write will come from this buffer for the
      // time being.  This may change later on
      viewBuffer = self->writeImageBuffer[0];
      writeBuffer = self->writeImageBuffer[0];
    }
  }
   */

  int reducedGreyscaleBitDepth = 0;
  if ( OA_PIX_FMT_GREY16BE == viewPixelFormat ||
      OA_PIX_FMT_GREY16LE == viewPixelFormat ||
      OA_ISBAYER16 ( viewPixelFormat )) {
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
        !currentViewBuffer;
    ( void ) memcpy ( self->viewImageBuffer[ currentViewBuffer ],
        viewBuffer, length );
    self->convert16To8Bit ( self->viewImageBuffer[ currentViewBuffer ],
        length, viewPixelFormat );
    viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
    if (  OA_PIX_FMT_GREY16BE == viewPixelFormat ||
        OA_PIX_FMT_GREY16LE == viewPixelFormat ) {
      reducedGreyscaleBitDepth = 1;
    }
  }

  if ( state->stackingMethod != OA_STACK_NONE ) {
    self->totalFrames++;
    switch ( state->stackingMethod ) {
      case OA_STACK_SUM:
        if ( -1 == self->currentStackBuffer && OA_STACK_NONE !=
            state->stackingMethod ) {
          self->currentStackBuffer = 0;
          self->stackBufferInUse = self->stackBuffer[0];
          memcpy ( self->stackBufferInUse, viewBuffer, length );
        } else {
          self->currentStackBuffer = self->currentStackBuffer ? 0 : 1;
        }
        oaStackSum8 ( viewBuffer, self->stackBufferInUse,
            self->stackBuffer[ self->currentStackBuffer ], length );
        viewBuffer = self->stackBufferInUse =
            self->stackBuffer[ self->currentStackBuffer ];
        break;

      case OA_STACK_MEAN:
        currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
            !currentViewBuffer;
        oaStackMean8 ( viewBuffer, self->averageBuffer,
            self->viewImageBuffer[ currentViewBuffer ], self->totalFrames,
            length );
        viewBuffer = self->viewImageBuffer[ currentViewBuffer ];
        break;
    }
  }

  ( void ) gettimeofday ( &t, 0 );
  unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
      ( unsigned long ) t.tv_usec / 1000;

  if (( self->lastDisplayUpdateTime + self->frameDisplayInterval ) < now ) {
    self->lastDisplayUpdateTime = now;
    doDisplay = 1;

    if ( config.showFocusAid ) {
      int fmt = viewPixelFormat;

      if ( viewIsDemosaicked ) {
        fmt = OA_ISBAYER16 ( fmt ) ? OA_PIX_FMT_RGB48BE :
            OA_PIX_FMT_RGB24;
      }
      state->focusOverlay->addScore ( oaFocusScore ( viewBuffer,
          0, config.imageSizeX, config.imageSizeY, fmt ));
    }

    QImage* newImage;
    QImage* swappedImage = 0;

    if ( OA_PIX_FMT_GREY8 == self->videoFramePixelFormat ||
        reducedGreyscaleBitDepth ) {
      newImage = new QImage (( const uint8_t* ) viewBuffer,
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
      newImage = new QImage (( const uint8_t* ) viewBuffer,
          config.imageSizeX, config.imageSizeY, config.imageSizeX * 3,
          QImage::Format_RGB888 );
      if ( OA_PIX_FMT_BGR24 == viewPixelFormat ) {
        swappedImage = new QImage ( newImage->rgbSwapped());
      } else {
        swappedImage = newImage;
      }
    }

    // FIX ME -- need to read this from the controls widget
    int zoomFactor = 100; // state->zoomWidget->getZoomFactor();
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

  output = state->controlsWidget->getProcessedOutputHandler();
  if ( output ) {
    output->addFrame ( viewBuffer, 0 );
  }
  state->captureIndex++;
/*
  if ( output && self->recordingInProgress ) {
    if ( self->setNewFirstFrameTime ) {
      state->firstFrameTime = now;
      self->setNewFirstFrameTime = 0;
    }
    state->lastFrameTime = now;
    if ( config.demosaicOutput && OA_ISBAYER ( writePixelFormat )) {
      if ( writeDemosaicViewBuffer ) {
        writeBuffer = viewBuffer;
      } else {
        // we can use the view buffer here because we're done with it
        // for actual view purposes
        // If it's possible that the write CFA pattern is not the same
        // as the view one, this code will need fixing to reset
        // cfaPattern, but I can't see that such a thing is possible
        // at the moment
        ( void ) oademosaic ( writeBuffer,
            self->viewImageBuffer[0], config.imageSizeX,
            config.imageSizeY, 8, cfaPattern, config.demosaicMethod );
        writeBuffer = self->viewImageBuffer[0];
      }
      writePixelFormat = OA_DEMOSAIC_FMT ( writePixelFormat );
    }
    if (( self->lastCapturedFramesUpdateTime +
        self->capturedFramesDisplayInterval ) < now ) {
      emit self->updateFrameCount ( output->getFrameCount());
      self->lastCapturedFramesUpdateTime = now;
    }
  }
*/

  self->framesInLastSecond++;
  if ( t.tv_sec != self->secondForFrameCount ) {
    self->secondForFrameCount = t.tv_sec;
    emit self->updateActualFrameRate ( self->framesInLastSecond );
    self->framesInLastSecond = 0;
    // state->histogramWidget->process ( writeBuffer, length, writePixelFormat );
    doHistogram = 1;
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

  if ( doHistogram ) {
    // emit self->updateHistogram();
  }

  if ( self->manualStop ) {
    self->recordingInProgress = 0;
    emit self->stopRecording();
    self->manualStop = 0;
  }

  return 0;
}


int
ViewWidget::formatToCfaPattern ( int format )
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
  qWarning() << "Invalid format in" << __FUNCTION__;
  return 0;
}


int
ViewWidget::checkBuffers ( ViewWidget* self )
{
  int			maxLength;

  // assign the temporary buffers for image transforms if they
  // don't already exist or the existing ones are too small

  maxLength = config.imageSizeX * config.imageSizeY * 6;
  if ( !self->viewImageBuffer[0] || self->viewBufferLength < maxLength ) {
    self->viewBufferLength = maxLength;
    if (!( self->viewImageBuffer[0] =
        realloc ( self->viewImageBuffer[0], self->viewBufferLength ))) {
      qWarning() << "view image buffer 0 malloc failed";
      return OA_ERR_MEM_ALLOC;
    }
    if (!( self->viewImageBuffer[1] =
        realloc ( self->viewImageBuffer[1], self->viewBufferLength ))) {
      qWarning() << "view image buffer 1 malloc failed";
      return OA_ERR_MEM_ALLOC;
    }
  }
  if ( !self->stackBuffer[0] || self->stackBufferLength < maxLength ) {
    self->stackBufferLength = maxLength;
    if (!( self->stackBuffer[0] =
        realloc ( self->stackBuffer[0], self->stackBufferLength ))) {
      qWarning() << "stack buffer 0 malloc failed";
      return OA_ERR_MEM_ALLOC;
    }
    if (!( self->stackBuffer[1] =
        realloc ( self->stackBuffer[1], self->stackBufferLength ))) {
      qWarning() << "stack buffer 1 malloc failed";
      return OA_ERR_MEM_ALLOC;
    }
    memset ( self->stackBuffer[0], 0, self->stackBufferLength );
    memset ( self->stackBuffer[1], 0, self->stackBufferLength );
  }
  if ( !self->writeImageBuffer[0] ||
      self->writeBufferLength < maxLength ) {
    self->writeBufferLength = maxLength;
    if (!( self->writeImageBuffer[0] =
        realloc ( self->writeImageBuffer[0], self->writeBufferLength ))) {
      qWarning() << "write image buffer 0 malloc failed";
      return OA_ERR_MEM_ALLOC;
    }
    if (!( self->writeImageBuffer[1] =
        realloc ( self->writeImageBuffer[1], self->writeBufferLength ))) {
      qWarning() << "write image buffer 1 malloc failed";
      return OA_ERR_MEM_ALLOC;
    }
  }

  // FIX ME -- probably doesn't need to be quite this long.
  if ( !self->averageBuffer ||
      self->averageBufferLength < ( maxLength * sizeof ( unsigned int ))) {
    self->averageBufferLength = maxLength * sizeof ( unsigned int );
    if (!( self->averageBuffer = ( unsigned int* )
        realloc ( self->averageBuffer, self->averageBufferLength ))) {
      qWarning() << "average buffer malloc failed";
      return OA_ERR_MEM_ALLOC;
    }
  }

  return OA_ERR_NONE;
}


void
ViewWidget::restart()
{
  // FIX ME -- should perhaps protect these with a mutex?
  if ( stackBuffer[0] ) {
    memset ( stackBuffer[0], 0, stackBufferLength );
    memset ( stackBuffer[1], 0, stackBufferLength );
  }
  if ( averageBuffer ) {
    memset ( averageBuffer, 0, averageBufferLength );
  }
  totalFrames = 0;
}
