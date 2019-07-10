/*****************************************************************************
 *
 * previewWidget.cc -- class for the preview window in the UI (and more)
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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
#if HAVE_CSTDLIB
#include <cstdlib>
#endif
#if HAVE_CMATH
#include <cmath>
#endif

extern "C" {
#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/video.h>
#include <openastro/imgproc.h>
#include <openastro/video/formats.h>
}

#include "commonState.h"
#include "commonConfig.h"
#include "outputHandler.h"
#include "focusOverlay.h"

#include "configuration.h"
#include "previewWidget.h"
#include "histogramWidget.h"
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
  secondForTemperature = secondForDropped = secondForAutoControls = 0;
  flipX = flipY = 0;
  movingReticle = rotatingReticle = rotationAngle = 0;
  savedXSize = savedYSize = 0;
  recalculateDimensions ( zoomFactor );
  previewBufferLength = 0;
  previewImageBuffer[0] = writeImageBuffer[0] = nullptr;
  previewImageBuffer[1] = writeImageBuffer[1] = nullptr;
  expectedSize = commonConfig.imageSizeX * commonConfig.imageSizeY *
      oaFrameFormats[ videoFramePixelFormat ].bytesPerPixel;
  demosaic = commonConfig.demosaic;

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
  expectedSize = commonConfig.imageSizeX * commonConfig.imageSizeY *
      oaFrameFormats[ videoFramePixelFormat ].bytesPerPixel;
  int newBufferLength = commonConfig.imageSizeX * commonConfig.imageSizeY * 3;
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
  diagonalLength = sqrt ( commonConfig.imageSizeX * commonConfig.imageSizeX +
      commonConfig.imageSizeY * commonConfig.imageSizeY );
}


void
PreviewWidget::recalculateDimensions ( int zoomFactor )
{
  currentZoom = zoomFactor;
  currentZoomX = commonConfig.imageSizeX * zoomFactor / 100;
  currentZoomY = commonConfig.imageSizeY * zoomFactor / 100;
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

  if ( commonState.cropMode ) {
    int x, y, w, h;
    painter.setRenderHint ( QPainter::Antialiasing, false );
    painter.setPen ( QPen ( Qt::yellow, 2, Qt::SolidLine, Qt::FlatCap ));
    x = ( commonConfig.imageSizeX - commonState.cropSizeX ) / 2 - 2;
    y = ( commonConfig.imageSizeY - commonState.cropSizeY ) / 2 - 2;
    w = commonState.cropSizeX + 4;
    h = commonState.cropSizeY + 4;
    if ( currentZoom != 100 ) {
      float zoomFactor = currentZoom / 100.0;
      x *= zoomFactor;
      y *= zoomFactor;
      w *= zoomFactor;
      h *= zoomFactor;
    }
    painter.drawRect ( x, y, w, h );
  }

  painter.setTransform ( rotationTransform );
  painter.setRenderHint ( QPainter::Antialiasing, true );
  painter.setPen ( QPen ( Qt::red, 2, Qt::SolidLine, Qt::FlatCap ));

  if ( config.showReticle ) {
    switch ( generalConf.reticleStyle ) {

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
  expectedSize = commonConfig.imageSizeX * commonConfig.imageSizeY *
      oaFrameFormats[ videoFramePixelFormat ].bytesPerPixel;
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
PreviewWidget::updatePreview ( void* args, void* imageData, int length,
		void* metadata )
{
  COMMON_STATE*		commonState = ( COMMON_STATE* ) args;
  STATE*					state = ( STATE* ) commonState->localState;
  PreviewWidget*	self = state->previewWidget;
  struct timeval	t;
  int			doDisplay = 0;
  int			doHistogram = 0;
  unsigned int		previewPixelFormat, writePixelFormat, pixelFormat;
  // write straight from the data if possible
  void*			previewBuffer = imageData;
  void*			writeBuffer = imageData;
  int			currentPreviewBuffer = -1;
  int			writeDemosaicPreviewBuffer = 0;
  int			maxLength;
  const char*		timestamp;
  char			commentStr[64];
  char*			comment;

  // don't do anything if the length is not as expected
  if ( length != self->expectedSize ) {
    //qWarning() << "size mismatch.  have:" << length << " expected: "
		//		<< self->expectedSize;
    return 0;
  }

  // assign the temporary buffers for image transforms if they
  // don't already exist or the existing ones are too small

  maxLength = commonConfig.imageSizeX * commonConfig.imageSizeY * 6;
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

  // if we have a luminance/chrominance or packed mono/raw colour frame
  // format then we need to unpack that first

  if ( oaFrameFormats[ self->videoFramePixelFormat ].lumChrom ||
      oaFrameFormats[ self->videoFramePixelFormat ].packed ) {
    // this is going to make the flip quite ugly and means we need to
    // start using currentPreviewBuffer too
    currentPreviewBuffer = ( -1 == currentPreviewBuffer ) ? 0 :
        !currentPreviewBuffer;
    // Convert luminance/chrominance and packed raw colour to RGB.
    // Packed mono should become GREY8.  We're only converting for
    // preview here, so nothing needs to be more than 8 bits wide
    if ( oaFrameFormats[ self->videoFramePixelFormat ].lumChrom ||
        oaFrameFormats[ self->videoFramePixelFormat ].rawColour ) {
      previewPixelFormat = OA_PIX_FMT_RGB24;
    } else {
      if ( oaFrameFormats[ self->videoFramePixelFormat ].monochrome ) {
        previewPixelFormat = OA_PIX_FMT_GREY8;
      } else {
        qWarning() << "Don't know how to unpack frame format" <<
            self->videoFramePixelFormat;
      }
    }
    ( void ) oaconvert ( previewBuffer,
        self->previewImageBuffer[ currentPreviewBuffer ],
				commonConfig.imageSizeX, commonConfig.imageSizeY,
				self->videoFramePixelFormat, previewPixelFormat );
    previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];

    // we can flip the preview image here if required, but not the
    // image that is going to be written out.
    // FIX ME -- work this out some time

    if ( self->flipX || self->flipY ) {
			int axis = ( self->flipX ? OA_FLIP_X : 0 ) | ( self->flipY ?
					OA_FLIP_Y : 0 );
      oaFlipImage ( previewBuffer, commonConfig.imageSizeX,
					commonConfig.imageSizeY, previewPixelFormat, axis );
    }
  } else {
    // do a vertical/horizontal flip if required
    if ( self->flipX || self->flipY ) {
      // this is going to make a mess for data we intend to demosaic.
      // the user will have to deal with that
			int axis = ( self->flipX ? OA_FLIP_X : 0 ) | ( self->flipY ?
					OA_FLIP_Y : 0 );
      ( void ) memcpy ( self->writeImageBuffer[0], writeBuffer, length );
      oaFlipImage ( self->writeImageBuffer[0], commonConfig.imageSizeX,
					commonConfig.imageSizeY, writePixelFormat, axis );
      // both preview and write will come from this buffer for the
      // time being.  This may change later on
      previewBuffer = self->writeImageBuffer[0];
      writeBuffer = self->writeImageBuffer[0];
    }
  }

  if (( !oaFrameFormats[ previewPixelFormat ].fullColour &&
      oaFrameFormats[ previewPixelFormat ].bytesPerPixel > 1 ) ||
      ( oaFrameFormats[ previewPixelFormat ].fullColour &&
      oaFrameFormats[ previewPixelFormat ].bytesPerPixel > 3 )) {
    currentPreviewBuffer = ( -1 == currentPreviewBuffer ) ? 0 :
        !currentPreviewBuffer;
    ( void ) memcpy ( self->previewImageBuffer[ currentPreviewBuffer ],
        previewBuffer, length );
    // Do this reduction "in place"
    previewPixelFormat = self->reduceTo8Bit (
        self->previewImageBuffer[ currentPreviewBuffer ],
        self->previewImageBuffer[ currentPreviewBuffer ],
        commonConfig.imageSizeX, commonConfig.imageSizeY, previewPixelFormat );
    previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];
  }

  ( void ) gettimeofday ( &t, 0 );
  unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
      ( unsigned long ) t.tv_usec / 1000;

  int cfaPattern = demosaicConf.cfaPattern;
  if ( OA_DEMOSAIC_AUTO == cfaPattern &&
      oaFrameFormats[ previewPixelFormat ].rawColour ) {
    cfaPattern = oaFrameFormats[ previewPixelFormat ].cfaPattern;
  }

  if ( self->previewEnabled ) {
    if (( self->lastDisplayUpdateTime + self->frameDisplayInterval ) < now ) {
      self->lastDisplayUpdateTime = now;
      doDisplay = 1;

      if ( self->demosaic && demosaicConf.demosaicPreview ) {
        if ( oaFrameFormats[ previewPixelFormat ].rawColour ) {
          currentPreviewBuffer = ( -1 == currentPreviewBuffer ) ? 0 :
              !currentPreviewBuffer;
          // Use the demosaicking to copy the data to the previewImageBuffer
          ( void ) oademosaic ( previewBuffer,
              self->previewImageBuffer[ currentPreviewBuffer ],
              commonConfig.imageSizeX, commonConfig.imageSizeY, 8, cfaPattern,
              demosaicConf.demosaicMethod );
          if ( demosaicConf.demosaicOutput && previewBuffer == writeBuffer
							&& oaFrameFormats[ self->videoFramePixelFormat ].bytesPerPixel
							== 1 ) {
            writeDemosaicPreviewBuffer = 1;
          }
          previewPixelFormat = OA_DEMOSAIC_FMT ( previewPixelFormat );
          previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];
        }
      }

      if ( config.showFocusAid ) {
        // This call should be thread-safe
        state->focusOverlay->addScore ( oaFocusScore ( previewBuffer,
            0, commonConfig.imageSizeX, commonConfig.imageSizeY,
						previewPixelFormat ));
      }

      QImage* newImage;
      QImage* swappedImage = nullptr;

      // At this point, one way or another we should have an 8-bit image
      // for the preview

      // First deal with anything that's mono, including untouched raw
      // colour

      if ( OA_PIX_FMT_GREY8 == previewPixelFormat ||
           ( oaFrameFormats[ previewPixelFormat ].rawColour &&
           ( !self->demosaic || !demosaicConf.demosaicPreview ))) {
        newImage = new QImage (( const uint8_t* ) previewBuffer,
            commonConfig.imageSizeX, commonConfig.imageSizeY,
						commonConfig.imageSizeX, QImage::Format_Indexed8 );
        if ( OA_PIX_FMT_GREY8 == previewPixelFormat &&
						commonConfig.colourise ) {
          newImage->setColorTable ( self->falseColourTable );
        } else {
          newImage->setColorTable ( self->greyscaleColourTable );
        }
        swappedImage = newImage;
      } else {
        // and full colour (should just be RGB24 or BGR24 at this point?)
        // here
        // Need the stride size here or QImage appears to "tear" the
        // right hand edge of the image when the X dimension is an odd
        // number of pixels
        newImage = new QImage (( const uint8_t* ) previewBuffer,
            commonConfig.imageSizeX, commonConfig.imageSizeY,
						commonConfig.imageSizeX * 3, QImage::Format_RGB888 );
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
          // FIX ME -- eh?
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

  OutputHandler* output = nullptr;
  int actualX, actualY;
  actualX = commonConfig.imageSizeX;
  actualY = commonConfig.imageSizeY;
  if ( !state->pauseEnabled ) {
    // This should be thread-safe
    output = state->captureWidget->getOutputHandler();
    if ( output && self->recordingInProgress ) {
      if ( self->setNewFirstFrameTime ) {
        state->firstFrameTime = now;
        self->setNewFirstFrameTime = 0;
      }
      state->lastFrameTime = now;
      if ( commonState->cropMode ) {
        if ( demosaicConf.demosaicOutput && writeDemosaicPreviewBuffer &&
            oaFrameFormats[ writePixelFormat ].rawColour ) {
          // This is a special case because as the preview buffer is no
          // longer required for previewing we can use it directly
          writeBuffer = previewBuffer;
          pixelFormat = OA_DEMOSAIC_FMT ( writePixelFormat );
        } else {
          pixelFormat = writePixelFormat;
        }
        oaInplaceCrop ( writeBuffer, commonConfig.imageSizeX,
						commonConfig.imageSizeY, commonState->cropSizeX,
						commonState->cropSizeY,
            oaFrameFormats[ pixelFormat ].bytesPerPixel );
        actualX = commonState->cropSizeX;
        actualY = commonState->cropSizeY;
        length = actualX * actualY *
            oaFrameFormats[ pixelFormat ].bytesPerPixel;
      }
      if ( demosaicConf.demosaicOutput &&
          oaFrameFormats[ writePixelFormat ].rawColour ) {
        if ( writeDemosaicPreviewBuffer ) {
          // could be redundant if we're also cropping, but it shouldn't
          // cause harm
          writeBuffer = previewBuffer;
        } else {
          // we can use the preview buffer here for the output image because
					// we're done with it for actual preview purposes
          // If it's possible that the write CFA pattern is not the same
          // as the preview one, this code will need fixing to reset
          // cfaPattern, but I can't see that such a thing is possible
          // at the moment
          ( void ) oademosaic ( writeBuffer,
              self->previewImageBuffer[0], actualX, actualY,
							oaFrameFormats[ self->videoFramePixelFormat ].bitsPerPixel,
							cfaPattern, demosaicConf.demosaicMethod );
          writeBuffer = self->previewImageBuffer[0];
        }
        writePixelFormat = OA_DEMOSAIC_FMT ( writePixelFormat );
      }
      // These calls should be thread-safe
      if ( commonState->timer->isInitialised() &&
					commonState->timer->isRunning()) {
        oaTimerStamp* ts = commonState->timer->readTimestamp();
        timestamp = ts->timestamp;
        comment = commentStr;
        ( void ) snprintf ( comment, 64, "Timer frame index: %d\n", ts->index );
      } else {
        timestamp = nullptr;
        comment = nullptr;
      }
      if ( output->addFrame ( writeBuffer, timestamp,
          // This call should be thread-safe
          state->controlWidget->getCurrentExposure(), comment,
					( FRAME_METADATA* ) metadata ) < 0 ) {
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
      state->histogramWidget->process ( writeBuffer, actualX, actualY,
          length, writePixelFormat );
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
	if ( t.tv_sec != self->secondForAutoControls ) {
		emit self->updateAutoControls();
		self->secondForAutoControls = t.tv_sec;
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
    if ( commonConfig.limitEnabled ) {
      int finished = 0;
      float percentage = 0;
      int frames = output->getFrameCount();
      switch ( commonConfig.limitType ) {
        case 0: // FIX ME -- nasty magic number
          // start and current times here are in ms, but the limit value is in
          // secs, so rather than ( current - start ) / time * 100 to get the
          // %age, we do ( current - start ) / time / 10
          percentage = ( now - state->captureWidget->recordingStartTime ) /
              ( commonConfig.secondsLimitValue * 1000.0 +
              state->captureWidget->totalTimePaused ) * 100.0;
          if ( now > state->captureWidget->recordingEndTime ) {
            finished = 1;
          }
          break;
        case 1: // FIX ME -- nasty magic number
          percentage = ( 100.0 * frames ) / commonConfig.framesLimitValue;
          if ( frames >= commonConfig.framesLimitValue ) {
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
            state->autorunStartNext = now + 1000 * autorunConf.autorunDelay;
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


unsigned int
PreviewWidget::reduceTo8Bit ( void* sourceData, void* targetData, int xSize,
    int ySize, int format )
{
  int	outputFormat = 0;

  if ( oaFrameFormats[ format ].monochrome ) {
    outputFormat = OA_PIX_FMT_GREY8;
  } else {
    if ( oaFrameFormats[ format ].rawColour &&
        !oaFrameFormats[ format ].packed ) {
      switch ( oaFrameFormats[ format ].cfaPattern ) {
        case OA_DEMOSAIC_RGGB:
          outputFormat = OA_PIX_FMT_RGGB8;
          break;
        case OA_DEMOSAIC_BGGR:
          outputFormat = OA_PIX_FMT_BGGR8;
          break;
        case OA_DEMOSAIC_GRBG:
          outputFormat = OA_PIX_FMT_GRBG8;
          break;
        case OA_DEMOSAIC_GBRG:
          outputFormat = OA_PIX_FMT_GBRG8;
          break;
        case OA_DEMOSAIC_CMYG:
          outputFormat = OA_PIX_FMT_CMYG8;
          break;
        case OA_DEMOSAIC_MCGY:
          outputFormat = OA_PIX_FMT_MCGY8;
          break;
        case OA_DEMOSAIC_YGCM:
          outputFormat = OA_PIX_FMT_YGCM8;
          break;
        case OA_DEMOSAIC_GYMC:
          outputFormat = OA_PIX_FMT_GYMC8;
          break;
      }
    } else {
      switch ( format ) {
        case OA_PIX_FMT_RGB30BE:
        case OA_PIX_FMT_RGB30LE:
        case OA_PIX_FMT_RGB36BE:
        case OA_PIX_FMT_RGB36LE:
        case OA_PIX_FMT_RGB42BE:
        case OA_PIX_FMT_RGB42LE:
        case OA_PIX_FMT_RGB48BE:
        case OA_PIX_FMT_RGB48LE:
          outputFormat = OA_PIX_FMT_RGB24;
          break;
        case OA_PIX_FMT_BGR48BE:
        case OA_PIX_FMT_BGR48LE:
          outputFormat = OA_PIX_FMT_BGR24;
          break;
      }
    }
  }

  if ( outputFormat ) {
    if ( oaconvert ( sourceData, targetData, xSize, ySize, format,
        outputFormat ) < 0 ) {
      qWarning() << "Unable to convert format" << format << "to format" <<
          outputFormat;
    }
  } else {
      qWarning() << "Can't handle 8-bit reduction of format" << format;
  }

  return outputFormat;
}
