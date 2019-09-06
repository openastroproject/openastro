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
#if HAVE_CSTDLIB
#include <cstdlib>
#endif
#if HAVE_CMATH
#include <cmath>
#endif
#include <libraw/libraw.h>

extern "C" {
#include <pthread.h>
#include <jpeglib.h>

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
#include "controlsWidget.h"

#include "configuration.h"
#include "viewWidget.h"
#include "histogramWidget.h"
#include "state.h"


// FIX ME -- Lots of this stuff needs refactoring or placing elsewhere
// as it's really not anything to do with the actual preview window
// any more

ViewWidget::ViewWidget ( QWidget* parent ) : QFrame ( parent )
{
  currentZoom = 100;
  int zoomFactor = 100;
  // setAttribute( Qt::WA_NoSystemBackground, true );
  lastCapturedFramesUpdateTime = 0;
  capturedFramesDisplayInterval = 200;
  lastDisplayUpdateTime = 0;
  frameDisplayInterval = 1000/15; // display frames per second
  videoFramePixelFormat = OA_PIX_FMT_RGB24;
	secondForTemperature = secondForDropped = secondForAutoControls = 0;
  flipX = flipY = 0;
  movingReticle = rotatingReticle = rotationAngle = 0;
  savedXSize = savedYSize = 0;
  recalculateDimensions ( zoomFactor );
  stackBufferLength = viewBufferLength = 0;
  viewImageBuffer[0] = writeImageBuffer[0] = stackBuffer[0] = 0;
  viewImageBuffer[1] = writeImageBuffer[1] = stackBuffer[1] = 0;
	previousFrames = 0;
  currentStackBuffer = -1;
  stackBufferInUse = 0;
  averageBuffer = 0;
	totalFrames = previousFrameArraySize = 0;
	rgbBuffer = 0;
	rgbBufferSize = 0;

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

	if ( previousFrames ) {
		unsigned int i;
		if ( totalFrames ) {
			for ( i = 0; i < totalFrames; i++ ) {
				free (( void* ) previousFrames[i] );
			}
		}
		free (( void* ) previousFrames );
	}

	if ( rgbBuffer ) {
		free (( void* ) rgbBuffer );
	}
}


void
ViewWidget::configure ( void )
{
  // setGeometry ( 0, 0, commonConfig.imageSizeX, commonConfig.imageSizeY );
}


void
ViewWidget::updateFrameSize ( void )
{
  int zoomFactor = 100;
  recalculateDimensions ( zoomFactor );
  expectedSize = commonConfig.imageSizeX * commonConfig.imageSizeY *
      oaFrameFormats[ videoFramePixelFormat ].bytesPerPixel;
  int newBufferLength = commonConfig.imageSizeX * commonConfig.imageSizeY * 6;
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
  diagonalLength = sqrt ( commonConfig.imageSizeX * commonConfig.imageSizeX +
      commonConfig.imageSizeY * commonConfig.imageSizeY );
  memset ( stackBuffer[0], 0, stackBufferLength );
  memset ( stackBuffer[1], 0, stackBufferLength );

	// Throw away all the saved frames, but keep the space for the array of
	// pointers
	if ( previousFrames && totalFrames ) {
		unsigned int i;
		for ( i = 0; i < totalFrames; i++ ) {
			free (( void* ) previousFrames[i] );
		}
	}
}


void
ViewWidget::recalculateDimensions ( int zoomFactor )
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
ViewWidget::paintEvent ( QPaintEvent* event )
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
  expectedSize = commonConfig.imageSizeX * commonConfig.imageSizeY *
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


void
ViewWidget::setDisplayFPS ( int fps )
{
  frameDisplayInterval = 1000 / fps;
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
ViewWidget::addImage ( void* args, void* imageData, int length, void* metadata )
{
	COMMON_STATE*	commonState = ( COMMON_STATE* ) args;
  STATE*		state = ( STATE* ) commonState->localState;
  ViewWidget*		self = state->viewWidget;
  struct timeval	t;
  int			doDisplay = 0;
  int			doHistogram = 0;
  int			viewPixelFormat, writePixelFormat, originalPixelFormat;
  // write straight from the data if possible
  void*			viewBuffer = imageData;
  int			currentViewBuffer = -1;
  int			ret;
  OutputHandler*	output;
  void*			writeBuffer = imageData;
  const char*		timestamp;
  char*			comment;
	unsigned int	width, height;

	viewPixelFormat = writePixelFormat = originalPixelFormat =
			self->videoFramePixelFormat;

  if ( self->videoFramePixelFormat == OA_PIX_FMT_JPEG8 || 
			oaFrameFormats[ self->videoFramePixelFormat ].useLibraw ) {
		if ( self->_unpackImageFrame ( self, imageData, &length, &viewPixelFormat,
				&width, &height ) != OA_ERR_NONE ) {
			qWarning() << "unpackImageFrame failed";
			return 0;
		}
		if ( width != commonConfig.imageSizeX ||
				height != commonConfig.imageSizeY ) {
			commonConfig.imageSizeX = width;
			commonConfig.imageSizeY = height;
      QMetaObject::invokeMethod ( state->controlsWidget, "doResolutionChange",
					Qt::DirectConnection, Q_ARG( int, 0 ));
		}
		viewBuffer = self->rgbBuffer;
		writePixelFormat = originalPixelFormat = viewPixelFormat;
	} else {
		// don't do anything if the length is not as expected
		if ( length != self->expectedSize ) {
			// qWarning() << "size mismatch.  have:" << length << " expected: "
			//    << self->expectedSize;
			return 0;
		}
	}

	if (( ret = self->checkBuffers ( self ))) {
		return 0;
	}

  // if we have a luminance/chrominance or packed mono/raw colour frame
  // format then we need to unpack that first

  if ( oaFrameFormats[ viewPixelFormat ].lumChrom ||
      oaFrameFormats[ viewPixelFormat ].packed ) {
    // this is going to make the flip quite ugly and means we need to
    // start using currentPreviewBuffer too
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 :
        !currentViewBuffer;
    // Convert luminance/chrominance and packed raw colour to RGB.
    // Packed mono should become GREY8.  We're only converting for
    // preview here, so nothing needs to be more than 8 bits wide
    if ( oaFrameFormats[ viewPixelFormat ].lumChrom ||
        oaFrameFormats[ viewPixelFormat ].rawColour ) {
      viewPixelFormat = OA_PIX_FMT_RGB24;
    } else {
      if ( oaFrameFormats[ viewPixelFormat ].monochrome ) {
        viewPixelFormat = OA_PIX_FMT_GREY8;
      } else {
        qWarning() << "Don't know how to unpack frame format" <<
            viewPixelFormat;
      }
    }
    ( void ) oaconvert ( viewBuffer,
        self->viewImageBuffer[ currentViewBuffer ], commonConfig.imageSizeX,
        commonConfig.imageSizeY, originalPixelFormat, viewPixelFormat );
    viewBuffer = self->viewImageBuffer [ currentViewBuffer ];

    // we can flip the preview image here if required, but not the
    // image that is going to be written out.
    // FIX ME -- work this out some time

    if ( self->flipX || self->flipY ) {
			int axis = ( self->flipX ? OA_FLIP_X : 0 ) | ( self->flipY ?
					OA_FLIP_Y : 0 );
      oaFlipImage ( viewBuffer, commonConfig.imageSizeX,
					commonConfig.imageSizeY, viewPixelFormat, axis );
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
      viewBuffer = self->writeImageBuffer[0];
      writeBuffer = self->writeImageBuffer[0];
    }
  }

  int cfaPattern = demosaicConf.cfaPattern;
  if ( OA_DEMOSAIC_AUTO == cfaPattern &&
      oaFrameFormats[ viewPixelFormat ].rawColour ) {
    cfaPattern = oaFrameFormats[ viewPixelFormat ].cfaPattern;
  }

  if ( oaFrameFormats[ viewPixelFormat ].rawColour ) {
    currentViewBuffer = ( -1 == currentViewBuffer ) ? 0 : !currentViewBuffer;
    // Use the demosaicking to copy the data to the previewImageBuffer
    ( void ) oademosaic ( viewBuffer,
        self->viewImageBuffer[ currentViewBuffer ],
        commonConfig.imageSizeX, commonConfig.imageSizeY,
				oaFrameFormats[ viewPixelFormat ].bitsPerPixel, cfaPattern,
        demosaicConf.demosaicMethod );
    viewPixelFormat = OA_DEMOSAIC_FMT ( viewPixelFormat );
    viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
  }

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
        commonConfig.imageSizeX, commonConfig.imageSizeY, viewPixelFormat );
    viewBuffer = self->viewImageBuffer [ currentViewBuffer ];
  }

	if ( oaFrameFormats[ viewPixelFormat ].monochrome ) {
		// apply image transforms to monochrome image
		// we know we have an 8-bit mono image at this point
		int			i, totSize = commonConfig.imageSizeX * commonConfig.imageSizeY;
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
		int			i, totSize = commonConfig.imageSizeX * commonConfig.imageSizeY * 3;
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

  if ( state->stackingMethod != OA_STACK_NONE ) {
    self->totalFrames++;
    const int viewFrameLength = commonConfig.imageSizeX *
				commonConfig.imageSizeY *
        oaFrameFormats[ viewPixelFormat ].bytesPerPixel;
    switch ( state->stackingMethod ) {

      case OA_STACK_SUM:
        if ( -1 == self->currentStackBuffer ) {
          self->currentStackBuffer = 0;
          self->stackBufferInUse = self->stackBuffer[0];
          memcpy ( self->stackBufferInUse, viewBuffer, viewFrameLength );
        } else {
          self->currentStackBuffer = self->currentStackBuffer ? 0 : 1;
					oaStackSum8 ( viewBuffer, self->stackBufferInUse,
							self->stackBuffer[ self->currentStackBuffer ], viewFrameLength );
        }
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

			case OA_STACK_MEDIAN:
			{
				// assign more memory for the array of frame pointers if required
				if ( self->previousFrameArraySize < self->totalFrames ) {
					void**		tmpPtr;
					if (!( tmpPtr = ( void** ) realloc ( self->previousFrames,
							( self->previousFrameArraySize + 20 ) * sizeof ( void* )))) {
						qDebug() << "realloc of frame history failed!";
						self->totalFrames--;
						break;
					}
					/*
					for ( i = self->previousFrameArraySize;
							i < ( self->previousFrameArraySize + 20 ); i++ ) {
						self->previousFrames[i] = 0;
					}
					*/
					self->previousFrames = tmpPtr;
					self->previousFrameArraySize += 20;
				}
				if (!( self->previousFrames[ self->totalFrames - 1 ] =
						malloc ( viewFrameLength ))) {
					qDebug() << "malloc of frame history failed!";
					self->totalFrames--;
					break;
				}
				memcpy ( self->previousFrames[ self->totalFrames - 1 ], viewBuffer,
						viewFrameLength );
				// no point doing any real work if we don't have at least three
				// frames
				if ( self->totalFrames > 2 ) {
					oaStackMedian8 ( self->previousFrames, self->totalFrames, viewBuffer,
							viewFrameLength );
				}
				break;
			}

      case OA_STACK_MAXIMUM:
        if ( -1 == self->currentStackBuffer ) {
          self->currentStackBuffer = 0;
          self->stackBufferInUse = self->stackBuffer[0];
          memcpy ( self->stackBufferInUse, viewBuffer, viewFrameLength );
        } else {
          self->currentStackBuffer = self->currentStackBuffer ? 0 : 1;
					oaStackMaximum8 ( viewBuffer, self->stackBufferInUse,
							self->stackBuffer[ self->currentStackBuffer ], viewFrameLength );
				}
        viewBuffer = self->stackBufferInUse =
            self->stackBuffer[ self->currentStackBuffer ];
        break;

			case OA_STACK_KAPPA_SIGMA:
			{
				// assign more memory for the array of frame pointers if required
				if ( self->previousFrameArraySize < self->totalFrames ) {
					void**		tmpPtr;
					if (!( tmpPtr = ( void** ) realloc ( self->previousFrames,
							( self->previousFrameArraySize + 20 ) * sizeof ( void* )))) {
						qDebug() << "realloc of frame history failed!";
						self->totalFrames--;
						break;
					}
					/*
					for ( i = self->previousFrameArraySize;
							i < ( self->previousFrameArraySize + 20 ); i++ ) {
						self->previousFrames[i] = 0;
					}
					*/
					self->previousFrames = tmpPtr;
					self->previousFrameArraySize += 20;
				}
				if (!( self->previousFrames[ self->totalFrames - 1 ] =
						malloc ( viewFrameLength ))) {
					qDebug() << "malloc of frame history failed!";
					self->totalFrames--;
					break;
				}
				memcpy ( self->previousFrames[ self->totalFrames - 1 ], viewBuffer,
						viewFrameLength );
				// no point doing any real work if we don't have at least three
				// frames
				if ( self->totalFrames > 2 ) {
					oaStackKappaSigma8 ( self->previousFrames, self->totalFrames,
							viewBuffer, viewFrameLength, config.stackKappa );
				}
				break;
			}
    }
  }

  ( void ) gettimeofday ( &t, 0 );
  unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
      ( unsigned long ) t.tv_usec / 1000;

  if (( self->lastDisplayUpdateTime + self->frameDisplayInterval ) < now ) {
    self->lastDisplayUpdateTime = now;
    doDisplay = 1;

    if ( config.showFocusAid ) {
      state->focusOverlay->addScore ( oaFocusScore ( viewBuffer,
          0, commonConfig.imageSizeX, commonConfig.imageSizeY,
					viewPixelFormat ));
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
          commonConfig.imageSizeX, commonConfig.imageSizeY,
					commonConfig.imageSizeX, QImage::Format_Indexed8 );
      if ( OA_PIX_FMT_GREY8 == viewPixelFormat && commonConfig.colourise ) {
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
          commonConfig.imageSizeX, commonConfig.imageSizeY,
					commonConfig.imageSizeX * 3, QImage::Format_RGB888 );
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
    const int viewFrameLength = commonConfig.imageSizeX *
			commonConfig.imageSizeY *
        oaFrameFormats[ viewPixelFormat ].bytesPerPixel;
    self->secondForFrameCount = t.tv_sec;
    emit self->updateActualFrameRate ( self->framesInLastSecond );
    self->framesInLastSecond = 0;
    state->processingControls->histogram->process ( viewBuffer,
				commonConfig.imageSizeX, commonConfig.imageSizeY, viewFrameLength,
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

	emit self->startNextExposure();

  return 0;
}


int
ViewWidget::checkBuffers ( ViewWidget* self )
{
  int			maxLength;

  // assign the temporary buffers for image transforms if they
  // don't already exist or the existing ones are too small

  maxLength = commonConfig.imageSizeX * commonConfig.imageSizeY * 6;
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

	// FIX ME -- how should the previousFrame buffers be handled here?

  return OA_ERR_NONE;
}


void
ViewWidget::restart()
{
	unsigned int i;

  // FIX ME -- should perhaps protect these with a mutex?
  if ( stackBuffer[0] ) {
    memset ( stackBuffer[0], 0, stackBufferLength );
    memset ( stackBuffer[1], 0, stackBufferLength );
  }
  if ( averageBuffer ) {
    memset ( averageBuffer, 0, averageBufferLength );
  }

	if ( previousFrames && totalFrames ) {
		for ( i = 0; i < totalFrames; i++ ) {
			free (( void* ) previousFrames[i] );
		}
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


int
ViewWidget::_unpackImageFrame ( ViewWidget* self, void* frame, int* size,
		int* format, unsigned int *imageWidth, unsigned int *imageHeight )
{
	if ( self->videoFramePixelFormat != OA_PIX_FMT_JPEG8 ) {
		return self->_unpackLibraw ( self, frame, size, format, imageWidth,
				imageHeight);
	}
	return self->_unpackJPEG8 ( self, frame, size, format, imageWidth,
			imageHeight);
}


int
ViewWidget::_unpackJPEG8 ( ViewWidget* self, void* frame, int* size,
		int* format, unsigned int *imageWidth, unsigned int *imageHeight )
{
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr					jerr;
	int							stride, width, height, pixelSize;
	int							requiredSize;
	void*						ptr;
	unsigned char*	bufferPtr;

	cinfo.err = jpeg_std_error ( &jerr );
	jpeg_create_decompress ( &cinfo );
	jpeg_mem_src ( &cinfo, ( const unsigned char* ) frame, *size );
	if ( jpeg_read_header ( &cinfo, TRUE ) != 1 ) {
		qWarning() << "jpeg_read_header failed";
		jpeg_destroy_decompress ( &cinfo );
		return -OA_ERR_SYSTEM_ERROR;
	}

	jpeg_start_decompress ( &cinfo );
	width = cinfo.output_width;
	height = cinfo.output_height;
	*imageWidth = width;
	*imageHeight = height;
	pixelSize = cinfo.output_components;
	stride = width * pixelSize;
	requiredSize = stride * height;
	*size = requiredSize;
	if ( cinfo.jpeg_color_space == JCS_GRAYSCALE ) {
		*format = OA_PIX_FMT_GREY8;
	} else {
		*format = OA_PIX_FMT_RGB24;
	}

	if ( self->rgbBufferSize < requiredSize ) {
		if ( self->rgbBuffer ) {
			if (!( ptr = realloc ( self->rgbBuffer, requiredSize ))) {
				qWarning() << "failed to realloc memory to decode jpeg";
				jpeg_abort_decompress ( &cinfo );
				jpeg_destroy_decompress ( &cinfo );
				return -OA_ERR_MEM_ALLOC;
			}
			self->rgbBuffer = ptr;
			self->rgbBufferSize = requiredSize;
		} else {
			if (!( self->rgbBuffer = malloc ( requiredSize ))) {
				qWarning() << "failed to malloc memory to decode jpeg";
				jpeg_abort_decompress ( &cinfo );
				jpeg_destroy_decompress ( &cinfo );
				return -OA_ERR_MEM_ALLOC;
			}
			self->rgbBufferSize = requiredSize;
		}
	}

	bufferPtr = ( unsigned char* ) self->rgbBuffer;
	while ( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines ( &cinfo, &bufferPtr, 1 );
		bufferPtr += stride;
	}

	jpeg_finish_decompress ( &cinfo );
	jpeg_destroy_decompress ( &cinfo );

	return -OA_ERR_NONE;
}


int
ViewWidget::_unpackLibraw ( ViewWidget* self, void* frame, int* size,
		int* format, unsigned int *imageWidth, unsigned int *imageHeight )
{
	LibRaw					handler;
	int							ret;
	int							width, height, colours, bpp, stride;
	int							requiredSize;
	void*						ptr;

	if (( ret = handler.open_buffer ( frame, *size ))) {
		qWarning() << "failed to open libraaw buffer, error" << ret;
		return -OA_ERR_SYSTEM_ERROR;
	}

	handler.imgdata.params.half_size = 0;
	handler.imgdata.params.use_auto_wb = 0;
	handler.imgdata.params.use_camera_wb = 0;
	handler.imgdata.params.output_bps = 16;

	/*
	other possible options:

	handler.imgdata.params.greybox
	handler.imgdata.params.cropbox
	handler.imgdata.params.aber
	handler.imgdata.params.gamm[0]
	handler.imgdata.params.gamm[1]
	handler.imgdata.params.user_mul
	handler.imgdata.params.shot_select
	handler.imgdata.params.bright
	handler.imgdata.params.threshold
	handler.imgdata.params.four_color_rgb
	handler.imgdata.params.highlight
	handler.imgdata.params.use_camera_matrix
	handler.imgdata.params.output_color
	handler.imgdata.params.output_profile
	handler.imgdata.params.camera_profile
	handler.imgdata.params.bad_pixels
	handler.imgdata.params.dark_frame
	handler.imgdata.params.output_tiff
	handler.imgdata.params.user_flip
	handler.imgdata.params.user_qual
	handler.imgdata.params.user_black
	handler.imgdata.params.user_sat
	handler.imgdata.params.med_passes
	handler.imgdata.params.no_auto_bright
	handler.imgdata.params.adjust_maximum_thr
	handler.imgdata.params.use_fuji_rotate
	handler.imgdata.params.green_matching
	handler.imgdata.params.dcb_iterations
	handler.imgdata.params.dcb_enhance_fl
	handler.imgdata.params.exp_correc
	handler.imgdata.params.use_rawspeed
	handler.imgdata.params.use_dngsdk
	handler.imgdata.params.no_auto_scale
	handler.imgdata.params.no_interpolation
	handler.imgdata.params.raw_processing_options
	*/

	if (( ret = handler.unpack())) {
		qWarning() << "libraw unpack failed, error" << ret;
		return -OA_ERR_SYSTEM_ERROR;
	}

	if (( ret = handler.dcraw_process())) {
		qWarning() << "libraw process failed, error" << ret;
		handler.recycle();
		return -OA_ERR_SYSTEM_ERROR;
	}

	handler.get_mem_image_format ( &width, &height, &colours, &bpp );
	stride = width * colours * bpp / 8;
	requiredSize = stride * height;
	// FIX ME -- on big endian machines this may well be big-endian
	*format = OA_PIX_FMT_RGB48LE;

#if 0
	height = handler.sizes.raw_height;
	width = handler.sizes.raw_width;
	*format = OA_PIX_FMT_RGGB16LE;
	requiredSize = width * height * 2;
#endif

	*imageWidth = width;
	*imageHeight = height;
	*size = requiredSize;

	if ( self->rgbBufferSize < requiredSize ) {
		if ( self->rgbBuffer ) {
			if (!( ptr = realloc ( self->rgbBuffer, requiredSize ))) {
				qWarning() << "failed to realloc memory to decode raw image";
				handler.recycle();
				return -OA_ERR_MEM_ALLOC;
			}
			self->rgbBuffer = ptr;
			self->rgbBufferSize = requiredSize;
		} else {
			if (!( self->rgbBuffer = malloc ( requiredSize ))) {
				qWarning() << "failed to malloc memory to decode raw image";
				handler.recycle();
				return -OA_ERR_MEM_ALLOC;
			}
			self->rgbBufferSize = requiredSize;
		}
	}

	if (( ret = handler.copy_mem_image ( self->rgbBuffer, stride, 0 ))) {
		qWarning() << "libraw copy_mem_image failed, error" << ret;
		handler.recycle();
		return -OA_ERR_SYSTEM_ERROR;
	}

#if 0
	memcpy ( self->rgbBuffer, handler.rawdata.raw_image, requiredSize );
#endif

	handler.recycle();
	return -OA_ERR_NONE;
}
