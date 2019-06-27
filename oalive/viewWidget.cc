/*****************************************************************************
 *
 * viewWidget.cc -- class for the preview window in the UI (and more)
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

#include "captureSettings.h"
#include "fitsSettings.h"
#include "outputHandler.h"
#include "focusOverlay.h"
#include "commonState.h"

#include "state.h"
#include "configuration.h"
#ifdef OACAPTURE
#include "previewWidget.h"
#include "histogramWidget.h"
#else
#include "viewWidget.h"
#endif


// FIX ME -- Lots of this stuff needs refactoring or placing elsewhere
// as it's really not anything to do with the actual preview window
// any more

ViewWidget::ViewWidget ( QWidget* parent ) : QFrame ( parent )
{
  currentZoom = 100;
#ifdef OACAPTURE
  int zoomFactor = state.zoomWidget->getZoomFactor();
#else
  int zoomFactor = 100;
#endif
  // setAttribute( Qt::WA_NoSystemBackground, true );
  lastCapturedFramesUpdateTime = 0;
  capturedFramesDisplayInterval = 200;
  lastDisplayUpdateTime = 0;
  frameDisplayInterval = 1000/15; // display frames per second
#ifdef OACAPTURE
  previewEnabled = 1;
#endif
  videoFramePixelFormat = OA_PIX_FMT_RGB24;
  framesInLastSecond = 0;
  secondForFrameCount = secondForTemperature = secondForDropped = 0;
  flipX = flipY = 0;
  movingReticle = rotatingReticle = rotationAngle = 0;
  savedXSize = savedYSize = 0;
  recalculateDimensions ( zoomFactor );
#ifdef OACAPTURE
  demosaic = config.demosaic;
  previewBufferLength = 0;
  previewImageBuffer[0] = writeImageBuffer[0] = 0;
  previewImageBuffer[1] = writeImageBuffer[1] = 0;
  expectedSize = config.imageSizeX * config.imageSizeY *
      oaFrameFormats[ videoFramePixelFormat ].bytesPerPixel;
#else
  stackBufferLength = viewBufferLength = 0;
  viewImageBuffer[0] = writeImageBuffer[0] = stackBuffer[0] = 0;
  viewImageBuffer[1] = writeImageBuffer[1] = stackBuffer[1] = 0;
  currentStackBuffer = -1;
  stackBufferInUse = 0;
  averageBuffer = 0;
  totalFrames = 0;
#endif

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

	blackPoint = 0;
	whitePoint = 255;
	coeff_b = 0;
	coeff_c = 1;
	coeff_t = 0;
	coeff_s = 1;
	coeff_r = 1;
	coeff_tbr = 0;
	// Magic numbers be here for luminance values.  Apparently the
	// lumR, lumG, lumB triplet can be either ( 0.3086, 0.6094, 0.0820 )
	// or ( 0.2125, 0.7154, 0.0721 )
	coeff_sr = ( 1.0 - coeff_s ) * 0.3086;
	coeff_sg = ( 1.0 - coeff_s ) * 0.6094;
	coeff_sb = ( 1.0 - coeff_s ) * 0.0820;
	coeff_r1 = coeff_r * coeff_c * ( coeff_sr + coeff_s );
	coeff_r2 = coeff_r * coeff_c * coeff_sg;
	coeff_r3 = coeff_r * coeff_c * coeff_sb;
	coeff_g1 = coeff_r * coeff_c * coeff_sr;
	coeff_g2 = coeff_r * coeff_c * ( coeff_sg + coeff_s );
	coeff_g3 = coeff_r * coeff_c * coeff_sb;
	coeff_b1 = coeff_r * coeff_c * coeff_sr;
	coeff_b2 = coeff_r * coeff_c * coeff_sg;
	coeff_b3 = coeff_r * coeff_c * ( coeff_sb + coeff_s );

	/*
	qDebug() << "b" << coeff_b << "c" << coeff_c << "t" << coeff_t;
	qDebug() << "s" << coeff_s << "r" << coeff_r << "tbr" << coeff_tbr;
	qDebug() << "sr" << coeff_sr << "sg" << coeff_sg << "sb" << coeff_sb;
	qDebug() << "r1" << coeff_r1 << "r2" << coeff_r2 << "r3" << coeff_r3;
	qDebug() << "g1" << coeff_g1 << "g2" << coeff_g2 << "g3" << coeff_g3;
	qDebug() << "b1" << coeff_b1 << "b2" << coeff_b2 << "b3" << coeff_b3;
	*/

	gammaExponent = 1.0;

  pthread_mutex_init ( &imageMutex, 0 );
  recordingInProgress = 0;
  manualStop = 0;

  connect ( this, SIGNAL( updateDisplay ( void )),
      this, SLOT( update ( void )));
}


ViewWidget::~ViewWidget()
{
#ifdef OACAPTURE
  if ( previewImageBuffer[0] ) {
    free ( previewImageBuffer[0] );
    free ( previewImageBuffer[1] );
  }
#else
  if ( viewImageBuffer[0] ) {
    free ( viewImageBuffer[0] );
    free ( viewImageBuffer[1] );
  }
#endif
  if ( writeImageBuffer[0] ) {
    free ( writeImageBuffer[0] );
    free ( writeImageBuffer[1] );
  }
#ifdef OALIVE
  if ( stackBuffer[0] ) {
    free ( stackBuffer[0] );
    free ( stackBuffer[1] );
  }
  if ( averageBuffer ) {
    free (( void* ) averageBuffer );
  }
#endif
}


void
ViewWidget::configure ( void )
{
  // setGeometry ( 0, 0, config.imageSizeX, config.imageSizeY );
}


void
#ifdef OACAPTURE
PreviewWidget::updatePreviewSize ( void )
#else
ViewWidget::updateFrameSize ( void )
#endif
{
#ifdef OACAPTURE
  int zoomFactor = state.zoomWidget->getZoomFactor();
#else
  int zoomFactor = 100;
#endif
  recalculateDimensions ( zoomFactor );
  expectedSize = config.imageSizeX * config.imageSizeY *
      oaFrameFormats[ videoFramePixelFormat ].bytesPerPixel;
  int newBufferLength = config.imageSizeX * config.imageSizeY * 3;
#ifdef OACAPTURE
  if ( newBufferLength > previewBufferLength ) {
    if (!( previewImageBuffer[0] = realloc ( previewImageBuffer[0],
#else
  if ( newBufferLength > viewBufferLength ) {
    if (!( viewImageBuffer[0] = realloc ( viewImageBuffer[0],
#endif
        newBufferLength ))) {
#ifdef OACAPTURE
      qWarning() << "preview image buffer realloc failed";
#else
      qWarning() << "view image buffer realloc failed";
#endif
    }
#ifdef OACAPTURE
    if (!( previewImageBuffer[1] = realloc ( previewImageBuffer[1],
#else
    if (!( viewImageBuffer[1] = realloc ( viewImageBuffer[1],
#endif
        newBufferLength ))) {
#ifdef OACAPTURE
      qWarning() << "preview image buffer realloc failed";
#else
      qWarning() << "view image buffer realloc failed";
#endif
    }
#ifdef OALIVE
    if (!( stackBuffer[0] = realloc ( stackBuffer[0], newBufferLength ))) {
      qWarning() << "stack image buffer realloc failed";
    }
#endif
#ifdef OACAPTURE
    previewBufferLength = newBufferLength;
#else
    if (!( stackBuffer[1] = realloc ( stackBuffer[1], newBufferLength ))) {
      qWarning() << "stack image buffer realloc failed";
    }
    stackBufferLength = viewBufferLength = newBufferLength;
#endif
  }
  diagonalLength = sqrt ( config.imageSizeX * config.imageSizeX +
      config.imageSizeY * config.imageSizeY );
#ifdef OALIVE
  memset ( stackBuffer[0], 0, stackBufferLength );
  memset ( stackBuffer[1], 0, stackBufferLength );
#endif
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


#ifdef OACAPTURE
void
ViewWidget::setEnabled ( int state )
{
  previewEnabled = state;
}
#endif


void
ViewWidget::setVideoFramePixelFormat ( int format )
{
  videoFramePixelFormat = format;
  expectedSize = config.imageSizeX * config.imageSizeY *
      oaFrameFormats[ videoFramePixelFormat ].bytesPerPixel;
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


#ifdef OACAPTURE
void
ViewWidget::enableDemosaic ( int state )
{
  demosaic = state;
}
#endif


void
ViewWidget::setDisplayFPS ( int fps )
{
  frameDisplayInterval = 1000 / fps;
}


#ifdef OACAPTURE
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
#endif


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
#ifdef OACAPTURE
PreviewWidget::updatePreview ( void* args, void* imageData, int length,
		void* metadata )
#else
ViewWidget::addImage ( void* args, void* imageData, int length, void* metadata )
#endif
{
	COMMON_STATE*	commonState = ( COMMON_STATE* ) args;
  STATE*		state = ( STATE* ) commonState->localState;
#ifdef OACAPTURE
  PreviewWidget*	self = state->previewWidget;
#else
  ViewWidget*		self = state->viewWidget;
#endif
  struct timeval	t;
  int			doDisplay = 0;
  int			doHistogram = 0;
#ifdef OACAPTURE
  int			previewPixelFormat, writePixelFormat;
#else
  int			viewPixelFormat, writePixelFormat;
#endif
  // write straight from the data if possible
#ifdef OACAPTURE
  void*			previewBuffer = imageData;
  int			currentPreviewBuffer = -1;
  int			writeDemosaicPreviewBuffer = 0;
  int			previewIsDemosaicked = 0;
  int			maxLength;
  char			commentStr[ 64 ];
#else
  void*			viewBuffer = imageData;
  int			currentViewBuffer = -1;
  int			ret;
  OutputHandler*	output;
#endif
  void*			writeBuffer = imageData;
  const char*		timestamp;
  char*			comment;

  // don't do anything if the length is not as expected
  if ( length != self->expectedSize ) {
    // qWarning() << "size mismatch.  have:" << length << " expected: "
    //    << self->expectedSize;
    return 0;
  }

#ifdef OACAPTURE
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

#else

   if (( ret = self->checkBuffers ( self ))) {
     return 0;
   }
   viewPixelFormat = writePixelFormat = self->videoFramePixelFormat;

#endif

  // if we have a luminance/chrominance or packed mono/raw colour frame
  // format then we need to unpack that first

  if ( oaFrameFormats[ self->videoFramePixelFormat ].lumChrom ||
      oaFrameFormats[ self->videoFramePixelFormat ].packed ) {
    // this is going to make the flip quite ugly and means we need to
    // start using currentPreviewBuffer too
#ifdef OACAPTURE
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
        self->previewImageBuffer[ currentPreviewBuffer ], config.imageSizeX,
        config.imageSizeY, self->videoFramePixelFormat, previewPixelFormat );
    previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];

    // we can flip the preview image here if required, but not the
    // image that is going to be written out.
    // FIX ME -- work this out some time

    if ( self->flipX || self->flipY ) {
			int axis = ( self->flipX ? OA_FLIP_X : 0 ) | ( self->flipY ?
					OA_FLIP_Y : 0 );
      oaFlipImage ( previewBuffer, config.imageSizeX,
					config.imageSizeY, previewPixelFormat, axis );
    }
#else /* OACAPTURE */
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
        !currentViewBuffer;
    // Convert luminance/chrominance and packed raw colour to RGB.
    // Packed mono should become GREY8.  We're only converting for
    // preview here, so nothing needs to be more than 8 bits wide
    if ( oaFrameFormats[ self->videoFramePixelFormat ].lumChrom ||
        oaFrameFormats[ self->videoFramePixelFormat ].rawColour ) {
      viewPixelFormat = OA_PIX_FMT_RGB24;
    } else {
      if ( oaFrameFormats[ self->videoFramePixelFormat ].monochrome ) {
        viewPixelFormat = OA_PIX_FMT_GREY8;
      } else {
        qWarning() << "Don't know how to unpack frame format" <<
            self->videoFramePixelFormat;
      }
    }
    ( void ) oaconvert ( viewBuffer,
        self->viewImageBuffer[ currentViewBuffer ], config.imageSizeX,
        config.imageSizeY, self->videoFramePixelFormat, viewPixelFormat );
    viewBuffer = self->viewImageBuffer [ currentViewBuffer ];

    // we can flip the preview image here if required, but not the
    // image that is going to be written out.
    // FIX ME -- work this out some time

    if ( self->flipX || self->flipY ) {
			int axis = ( self->flipX ? OA_FLIP_X : 0 ) | ( self->flipY ?
					OA_FLIP_Y : 0 );
      oaFlipImage ( viewBuffer, config.imageSizeX,
					config.imageSizeY, viewPixelFormat, axis );
    }
#endif	/* OACAPTURE */
  } else {
#ifdef OACAPTURE
    // do a vertical/horizontal flip if required
    if ( self->flipX || self->flipY ) {
      // this is going to make a mess for data we intend to demosaic.
      // the user will have to deal with that
			int axis = ( self->flipX ? OA_FLIP_X : 0 ) | ( self->flipY ?
					OA_FLIP_Y : 0 );
      ( void ) memcpy ( self->writeImageBuffer[0], writeBuffer, length );
      oaFlipImage ( self->writeImageBuffer[0], config.imageSizeX,
					config.imageSizeY, writePixelFormat, axis );
      // both preview and write will come from this buffer for the
      // time being.  This may change later on
      previewBuffer = self->writeImageBuffer[0];
      writeBuffer = self->writeImageBuffer[0];
    }
#else	/* OACAPTURE */
    // do a vertical/horizontal flip if required
    if ( self->flipX || self->flipY ) {
      // this is going to make a mess for data we intend to demosaic.
      // the user will have to deal with that
			int axis = ( self->flipX ? OA_FLIP_X : 0 ) | ( self->flipY ?
					OA_FLIP_Y : 0 );
      ( void ) memcpy ( self->writeImageBuffer[0], writeBuffer, length );
      oaFlipImage ( self->writeImageBuffer[0], config.imageSizeX,
					config.imageSizeY, writePixelFormat, axis );
      // both preview and write will come from this buffer for the
      // time being.  This may change later on
      viewBuffer = self->writeImageBuffer[0];
      writeBuffer = self->writeImageBuffer[0];
    }
#endif	/* OACAPTURE */
  }

#ifdef OACAPTURE
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
        config.imageSizeX, config.imageSizeY, previewPixelFormat );
    previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];
  }
#else
  if (( !oaFrameFormats[ viewPixelFormat ].fullColour &&
      oaFrameFormats[ viewPixelFormat ].bytesPerPixel > 1 ) ||
      ( oaFrameFormats[ viewPixelFormat ].fullColour &&
      oaFrameFormats[ viewPixelFormat ].bytesPerPixel > 3 )) {
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
        !currentViewBuffer;
    ( void ) memcpy ( self->viewImageBuffer[ currentViewBuffer ],
        viewBuffer, length );
    // Do this reduction "in place"
    viewPixelFormat = self->reduceTo8Bit (
        self->viewImageBuffer[ currentViewBuffer ],
        self->viewImageBuffer[ currentViewBuffer ],
        config.imageSizeX, config.imageSizeY, viewPixelFormat );
    viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
  }

	if ( oaFrameFormats[ viewPixelFormat ].monochrome ) {
		// apply image transforms to monochrome image
		// we know we have an 8-bit mono image at this point
		int			i, totSize = config.imageSizeX * config.imageSizeY;
		double	val, newVal;
		uint8_t*	src = ( uint8_t* ) viewBuffer;
		uint8_t*	tgt;
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
        !currentViewBuffer;
		tgt = ( uint8_t* ) self->viewImageBuffer [ currentViewBuffer ];
		for ( i = 0; i < totSize; i++ ) {
			val = src[ i ];
			newVal = oaclamp ( 0, 255, pow ( val * self->coeff_r * self->coeff_c
					+ self->coeff_tbr, self->gammaExponent ));
			tgt[ i ] = ( uint8_t )( newVal + 0.5 );
		}
		viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
	} else {
		// apply image transforms to colour image
		// we know we have an RGB24 image at this point
		int			i, totSize = config.imageSizeX * config.imageSizeY * 3;
		double	valR, valG, valB, newValR, newValG, newValB;
		uint8_t*	src = ( uint8_t* ) viewBuffer;
		uint8_t*	tgt;
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
        !currentViewBuffer;
		tgt = ( uint8_t* ) self->viewImageBuffer [ currentViewBuffer ];
		for ( i = 0; i < totSize; i += 3 ) {
			valR = src[ i ];
			valG = src[ i + 1 ];
			valB = src[ i + 2 ];
			newValR = oaclamp ( 0, 255, pow ( valR * self->coeff_r1 +
					valG * self->coeff_r2 + valB * self->coeff_r3 + self->coeff_tbr,
					self->gammaExponent ));
			newValG = oaclamp ( 0, 255, pow ( valR * self->coeff_g1 +
					valG * self->coeff_g2 + valB * self->coeff_g3 + self->coeff_tbr,
					self->gammaExponent ));
			newValB = oaclamp ( 0, 255, pow ( valR * self->coeff_b1 +
					valG * self->coeff_b2 + valB * self->coeff_b3 + self->coeff_tbr,
					self->gammaExponent ));
			tgt[ i ] = ( int )( newValR + 0.5 );
			tgt[ i + 1 ] = ( int )( newValG + 0.5 );
			tgt[ i + 2 ] = ( int )( newValB + 0.5 );
		}
		viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
	}

#endif /* OACAPTURE */

#ifdef OACAPTURE
  ( void ) gettimeofday ( &t, 0 );
  unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
      ( unsigned long ) t.tv_usec / 1000;

  int cfaPattern = config.cfaPattern;
  if ( oaFrameFormats[ previewPixelFormat ].rawColour ) {
    if ( OA_DEMOSAIC_AUTO == cfaPattern ) {
      cfaPattern = oaFrameFormats[ previewPixelFormat ].cfaPattern;
    }
  }
#else
  if ( state->stackingMethod != OA_STACK_NONE ) {
    self->totalFrames++;
    const int viewFrameLength = config.imageSizeX * config.imageSizeY *
        oaFrameFormats[ viewPixelFormat ].bytesPerPixel;
    switch ( state->stackingMethod ) {
      case OA_STACK_SUM:
        if ( -1 == self->currentStackBuffer && OA_STACK_NONE !=
            state->stackingMethod ) {
          self->currentStackBuffer = 0;
          self->stackBufferInUse = self->stackBuffer[0];
          memcpy ( self->stackBufferInUse, viewBuffer, viewFrameLength );
        } else {
          self->currentStackBuffer = self->currentStackBuffer ? 0 : 1;
        }
        oaStackSum8 ( viewBuffer, self->stackBufferInUse,
            self->stackBuffer[ self->currentStackBuffer ], viewFrameLength );
        viewBuffer = self->stackBufferInUse =
            self->stackBuffer[ self->currentStackBuffer ];
        break;

      case OA_STACK_MEAN:
        currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
            !currentViewBuffer;
        oaStackMean8 ( viewBuffer, self->averageBuffer,
            self->viewImageBuffer[ currentViewBuffer ], self->totalFrames,
            viewFrameLength );
        viewBuffer = self->viewImageBuffer[ currentViewBuffer ];
        break;
    }
  }
#endif /* OACAPTURE */

#ifdef OACAPTURE
  if ( self->previewEnabled ) {
    if (( self->lastDisplayUpdateTime + self->frameDisplayInterval ) < now ) {
      self->lastDisplayUpdateTime = now;
      doDisplay = 1;

      if ( self->demosaic && config.demosaicPreview ) {
        if ( oaFrameFormats[ previewPixelFormat ].rawColour ) {
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
          previewPixelFormat = OA_DEMOSAIC_FMT ( previewPixelFormat );
          previewBuffer = self->previewImageBuffer [ currentPreviewBuffer ];
        }
      }

      if ( config.showFocusAid ) {
        // This call should be thread-safe
        state->focusOverlay->addScore ( oaFocusScore ( previewBuffer,
            0, config.imageSizeX, config.imageSizeY, fmt ));
      }

      QImage* newImage;
      QImage* swappedImage = 0;

      if ( OA_PIX_FMT_GREY8 == self->videoFramePixelFormat ||
           ( oaFrameFormats[ previewPixelFormat ].rawColour &&
           ( !self->demosaic || !config.demosaicPreview )) ||
           reducedGreyscaleBitDepth ) {
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
    output = state->captureWidget->getOutputHandler();
    if ( output && self->recordingInProgress ) {
      if ( self->setNewFirstFrameTime ) {
        state->firstFrameTime = now;
        self->setNewFirstFrameTime = 0;
      }
      state->lastFrameTime = now;
      if ( config.demosaicOutput &&
          oaFrameFormats[ writePixelFormat ].rawColour ) {
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
      if ( state->timer->isInitialised() && state->timer->isRunning()) {
        timestamp = state->timer->readTimestamp();
      } else {
        timestamp = 0;
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

  self->framesInLastSecond++;
  if ( t.tv_sec != self->secondForFrameCount ) {
    self->secondForFrameCount = t.tv_sec;
    emit self->updateActualFrameRate ( self->framesInLastSecond );
    self->framesInLastSecond = 0;
    if ( state->histogramOn ) {
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
    state->captureWidget->startNewAutorun();
  }
#else /* OACAPTURE */

  ( void ) gettimeofday ( &t, 0 );
  unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
      ( unsigned long ) t.tv_usec / 1000;

  int cfaPattern = demosaicConf.cfaPattern;
  if ( OA_DEMOSAIC_AUTO == cfaPattern &&
      oaFrameFormats[ viewPixelFormat ].rawColour ) {
    cfaPattern = oaFrameFormats[ viewPixelFormat ].cfaPattern;
  }

  if (( self->lastDisplayUpdateTime + self->frameDisplayInterval ) < now ) {
    self->lastDisplayUpdateTime = now;
    doDisplay = 1;

    if ( config.showFocusAid ) {
      state->focusOverlay->addScore ( oaFocusScore ( viewBuffer,
          0, config.imageSizeX, config.imageSizeY, viewPixelFormat ));
    }

    QImage* newImage;
    QImage* swappedImage = 0;

    // At this point, one way or another we should have an 8-bit image
    // for the preview

    // First deal with anything that's mono, including untouched raw
    // colour

    if ( OA_PIX_FMT_GREY8 == self->videoFramePixelFormat ||
         ( oaFrameFormats[ viewPixelFormat ].rawColour &&
         !self->demosaic )) {
      newImage = new QImage (( const uint8_t* ) viewBuffer,
          config.imageSizeX, config.imageSizeY, config.imageSizeX,
          QImage::Format_Indexed8 );
      if ( OA_PIX_FMT_GREY8 == viewPixelFormat && config.colourise ) {
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

  output = state->controlsWidget->getProcessedOutputHandler();
  if ( output ) {
    timestamp = 0;
    comment = 0;
    output->addFrame ( viewBuffer, timestamp,
        state->cameraControls->getCurrentExposure(), comment,
				( FRAME_METADATA* ) metadata );
  }
  commonState->captureIndex++;

  self->framesInLastSecond++;
  if ( t.tv_sec != self->secondForFrameCount ) {
    const int viewFrameLength = config.imageSizeX * config.imageSizeY *
        oaFrameFormats[ viewPixelFormat ].bytesPerPixel;
    self->secondForFrameCount = t.tv_sec;
    emit self->updateActualFrameRate ( self->framesInLastSecond );
    self->framesInLastSecond = 0;
    state->processingControls->histogram->process ( viewBuffer,
				config.imageSizeX, config.imageSizeY, viewFrameLength,
				viewPixelFormat );
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
    emit self->updateHistogram();
  }

  if ( self->manualStop ) {
    self->recordingInProgress = 0;
    emit self->stopRecording();
    self->manualStop = 0;
  }
#endif /* OACAPTURE */

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


unsigned int
ViewWidget::reduceTo8Bit ( void* sourceData, void* targetData, int xSize,
    int ySize, int format )
{
  int   outputFormat = 0;

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


void
ViewWidget::setBlackLevel ( int val )
{
	val >>= 8;
	if ( val >= whitePoint ) {
		return;
	}
	blackPoint = val;
	coeff_r = 255.0 / ( whitePoint - blackPoint );
	coeff_tbr = 255 * ( coeff_t + coeff_b ) - blackPoint;
	_recalcCoeffs();
}


void
ViewWidget::setWhiteLevel ( int val )
{
	val >>= 8;
	if ( val <= blackPoint ) {
		return;
	}
	whitePoint = val;
	coeff_r = 255.0 / ( whitePoint - blackPoint );
	coeff_tbr = 255 * ( coeff_t + coeff_b ) - blackPoint;
	_recalcCoeffs();
}


void
ViewWidget::setBrightness ( int val )
{
	brightness = val;
	coeff_b = val / 100.0;
	coeff_tbr = 255 * ( coeff_t + coeff_b ) - blackPoint;
}


void
ViewWidget::setContrast ( int val )
{
	contrast = val;
	coeff_c = val / 50.0;
	coeff_t = ( 1.0 - coeff_c ) / 2.0;
	_recalcCoeffs();
}


void
ViewWidget::setSaturation ( int val )
{
	saturation = val;
	coeff_s = val / 100.0;
	// Magic numbers be here for luminance values.  Apparently the
	// lumR, lumG, lumB triplet can be either ( 0.3086, 0.6094, 0.0820 )
	// or ( 0.2125, 0.7154, 0.0721 )
	coeff_sr = ( 1.0 - coeff_s ) * 0.3086;
	coeff_sg = ( 1.0 - coeff_s ) * 0.6094;
	coeff_sb = ( 1.0 - coeff_s ) * 0.0820;
	_recalcCoeffs();
}


void
ViewWidget::setGamma ( int val )
{
	gammaExponent = val / 100.0;
}


void
ViewWidget::_recalcCoeffs ( void )
{
	coeff_r1 = coeff_r * coeff_c * ( coeff_sr + coeff_s );
	coeff_r2 = coeff_r * coeff_c * coeff_sg;
	coeff_r3 = coeff_r * coeff_c * coeff_sb;
	coeff_g1 = coeff_r * coeff_c * coeff_sr;
	coeff_g2 = coeff_r * coeff_c * ( coeff_sg + coeff_s );
	coeff_g3 = coeff_r * coeff_c * coeff_sb;
	coeff_b1 = coeff_r * coeff_c * coeff_sr;
	coeff_b2 = coeff_r * coeff_c * coeff_sg;
	coeff_b3 = coeff_r * coeff_c * ( coeff_sb + coeff_s );
}


void
ViewWidget::_displayCoeffs ( void )
{
	qDebug() << "b" << coeff_b << "c" << coeff_c << "t" << coeff_t;
	qDebug() << "s" << coeff_s << "r" << coeff_r << "tbr" << coeff_tbr;
	qDebug() << "sr" << coeff_sr << "sg" << coeff_sg << "sb" << coeff_sb;
	qDebug() << "r1" << coeff_r1 << "r2" << coeff_r2 << "r3" << coeff_r3;
	qDebug() << "g1" << coeff_g1 << "g2" << coeff_g2 << "g3" << coeff_g3;
	qDebug() << "b1" << coeff_b1 << "b2" << coeff_b2 << "b3" << coeff_b3;
}
