/*****************************************************************************
 *
 * histogramWidget.cc -- class for the histogram display
 *
 * Copyright 2013,2014,2017,2018 James Fidell (james@openastroproject.org)
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
#include <openastro/camera.h>
}

#include "version.h"
#include "configuration.h"
#include "state.h"
#include "histogramWidget.h"


HistogramWidget::HistogramWidget()
{
  currentLayoutIsSplit = 0;
  newLayoutIsSplit = config.splitHistogram;
  showingThreeGraphs = 0;
  resize ( 300, 150 );
  setWindowTitle( APPLICATION_NAME " Histogram" );
  setWindowIcon ( QIcon ( ":/qt-icons/barchart.png" ));
  setSizePolicy ( QSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Fixed ));
  grey = red;
  bzero ( red, sizeof( int ) * 256 );
  bzero ( green, sizeof( int ) * 256 );
  bzero ( blue, sizeof( int ) * 256 );
  colours = 1;
  doneProcess = 0;
  maxRedIntensity = maxGreenIntensity = maxBlueIntensity = maxIntensity = 0;
  fullIntensity = 0;
  statsEnabled = 0;
  signalConnected = 0;
}


HistogramWidget::~HistogramWidget()
{
}


void
HistogramWidget::process ( void* imageData, unsigned int width,
    unsigned int height, unsigned int length, int format )
{
  // Can't do this in the constructor because previewWidget might not exist
  // at that time
  if ( !signalConnected ) {
    connect ( state.previewWidget, SIGNAL( updateHistogram ( void )),
        this, SLOT( update ( void )));
    signalConnected = 1;
  }

  fullIntensity = 0xff;
  minIntensity = 0xffff;
  doneProcess = 1;

  if ( OA_PIX_FMT_RGB24 == format || OA_PIX_FMT_BGR24 == format ) {
    _processRGBHistogram ( imageData, width, height, length, format );
  } else {
    if ( oaFrameFormats[ format ].rawColour && config.rawRGBHistogram ) {
      _processMosaicHistogram ( imageData, width, height, length, format );
    } else {
      _processGreyscaleHistogram ( imageData, width, height, length, format );
    }
  }

  if ( statsEnabled ) {
    histogramMin = minIntensity < histogramMin ? minIntensity : histogramMin;
    histogramMax = maxIntensity > histogramMax ? maxIntensity : histogramMax;
  }
}


void
HistogramWidget::paintEvent ( QPaintEvent* event )
{
  Q_UNUSED ( event );

  if ( !doneProcess ) {
    return;
  }

  if ( 1 == colours && showingThreeGraphs ) {
    // resize ( 300, 150 );
    setFixedSize ( 300, 150 );
    showingThreeGraphs = 0;
  } else {
    if ( newLayoutIsSplit != currentLayoutIsSplit ) {
      if ( !newLayoutIsSplit && showingThreeGraphs ) {
        // resize ( 300, 150 );
        setFixedSize ( 300, 150 );
        currentLayoutIsSplit = 0;
        showingThreeGraphs = 0;
      }
      if ( newLayoutIsSplit && !showingThreeGraphs ) {
        // resize ( 300, 450 );
        setFixedSize ( 300, 450 );
        currentLayoutIsSplit = 1;
        showingThreeGraphs = 1;
      }
    }
  }

  char s[40];
  QPainter painter ( this );

  painter.setRenderHint ( QPainter::Antialiasing, false );

  painter.fillRect ( 25, 25, 256, 100 + 300 * showingThreeGraphs,
      palette().color( QWidget::backgroundRole()));

  painter.setPen ( QPen ( Qt::black, 1, Qt::SolidLine, Qt::FlatCap ));
  painter.drawLine ( 24, 125, 24, 25 );
  painter.drawLine ( 24, 125, 280, 125 );
  if ( showingThreeGraphs ) {
    painter.drawLine ( 24, 275, 24, 175 );
    painter.drawLine ( 24, 275, 280, 275 );
    painter.drawLine ( 24, 425, 24, 325 );
    painter.drawLine ( 24, 425, 280, 425 );
  }

  if ( fullIntensity ) {
    if ( colours > 1 ) {
      painter.setPen ( QPen ( Qt::red, 1, Qt::SolidLine, Qt::FlatCap ));
      sprintf ( s, "%d (%d%%)", maxRedIntensity,
          int ( float ( maxRedIntensity ) / fullIntensity * 100 ));
    } else {
      painter.setPen ( QPen ( Qt::gray, 1, Qt::SolidLine, Qt::FlatCap ));
      sprintf ( s, "%d (%d%%)", maxIntensity,
          int ( float ( maxIntensity ) / fullIntensity * 100 ));
    }
    painter.drawText ( 25, 15, s );
    for ( int i = 0; i < 256; i++ ) {
      if ( red[i] ) {
        painter.drawLine ( 25+i, 124, 25+i, 124-red[i] );
      }
    }

    if ( colours > 1 ) {

      painter.setPen ( QPen ( Qt::darkGreen, 1, Qt::SolidLine, Qt::FlatCap ));

      sprintf ( s, "%d (%d%%)", maxGreenIntensity,
          int ( float ( maxGreenIntensity ) / fullIntensity * 100 ));
      painter.drawText ( 105, 15, s );

      int offset;
      if ( !showingThreeGraphs ) {
        painter.setOpacity ( 0.5 );
      }
      offset = showingThreeGraphs ? 274 : 124;
      for ( int i = 0; i < 256; i++ ) {
        if ( green[i] ) {
          painter.drawLine ( 25+i, offset, 25+i, offset-green[i] );
        }
      }

      painter.setOpacity ( 1 );
      offset = showingThreeGraphs ? 424 : 124;
      painter.setPen ( QPen ( Qt::blue, 1, Qt::SolidLine, Qt::FlatCap ));
      sprintf ( s, "%d (%d%%)", maxBlueIntensity,
          int ( float ( maxBlueIntensity ) / fullIntensity * 100 ));
      painter.drawText ( 185, 15, s );

    if ( !showingThreeGraphs ) {
        painter.setOpacity ( 0.5 );
      }
      offset = showingThreeGraphs ? 424 : 124;
      for ( int i = 0; i < 256; i++ ) {
        if ( blue[i] ) {
          painter.drawLine ( 25+i, offset, 25+i, offset-blue[i] );
        }
      }
    }
  }
}


void
HistogramWidget::updateLayout ( void )
{
  newLayoutIsSplit = config.splitHistogram;
}


void
HistogramWidget::resetStats ( void )
{
  histogramMin = 0xffff;
  histogramMax = 0;
  fullIntensity = 0;
  statsEnabled = 1;
}


void
HistogramWidget::stopStats ( void )
{
  statsEnabled = 0;
}


void
HistogramWidget::_processGreyscaleHistogram ( void* imageData,
    unsigned int width, unsigned int height, unsigned int length, int format )
{
  int maxCount = 1;
  int intensity, step;
  unsigned int i;

  colours = 1;
  step = length / 10000;
  if ( step < 1 ) {
    step = 1;
  }

  maxIntensity = 0;
  bzero ( grey, sizeof( int ) * 256 );

  if ( 2 == oaFrameFormats[ format ].bytesPerPixel ) {
    step *= 2;
    fullIntensity = 0xffff;
    if ( oaFrameFormats[ format ].littleEndian ) {
      int b1, b2;
      for ( i = 0; i < length; i += step ) {
        b1 = *(( uint8_t* ) imageData + i );
        b2 = *(( uint8_t* ) imageData + i + 1 );
        intensity = b1 + ( b2 << 8 );
        maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
        minIntensity = intensity < minIntensity ? intensity : minIntensity;
        grey[ b2 ]++;
      }
    } else {
      int b1, b2;
      for ( i = 0; i < length; i += step ) {
        b1 = *(( uint8_t* ) imageData + i );
        b2 = *(( uint8_t* ) imageData + i + 1 );
        intensity = ( b1 << 8 ) + b2;
        maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
        minIntensity = intensity < minIntensity ? intensity : minIntensity;
        grey[ b1 ]++;
      }
    }
  } else {
    for ( i = 0; i < length; i += step ) {
      intensity = *(( uint8_t* ) imageData + i );
      maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
      minIntensity = intensity < minIntensity ? intensity : minIntensity;
      grey[ intensity ]++;
    }
  }
  for ( i = 0; i < 256; i++ ) {
    maxCount = ( grey[i] > maxCount ) ? grey[i] : maxCount;
  }
  for ( i = 0; i < 256; i++ ) {
    grey[i] = grey[i] * 100 / maxCount;
  }
}


void
HistogramWidget::_processRGBHistogram ( void* imageData,
    unsigned int width, unsigned int height, unsigned int length, int format )
{
  int* swapRed = red;
  int* swapBlue = blue;
  int maxCount = 1;
  int step;
  unsigned int i;

  colours = 3;
  step = ( length / colours ) / 10000;
  step *= colours;
  if ( step < 1 ) {
    step = 1;
  }

  if ( OA_PIX_FMT_BGR24 == format ) {
    swapRed = blue;
    swapBlue = red;
  }

  maxRedIntensity = 0;
  maxGreenIntensity = 0;
  maxBlueIntensity = 0;
  bzero ( red, sizeof( int ) * 256 );
  bzero ( green, sizeof( int ) * 256 );
  bzero ( blue, sizeof( int ) * 256 );
  int intensity;
  for ( i = 0; i < length; i += step ) {
    intensity = *(( uint8_t* ) imageData + i );
    maxRedIntensity = intensity > maxRedIntensity ? intensity :
        maxRedIntensity;
    maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
    minIntensity = intensity < minIntensity ? intensity : minIntensity;
    swapRed[ intensity ]++;

    intensity = *(( uint8_t* ) imageData + i + 1 );
    maxGreenIntensity = intensity > maxGreenIntensity ? intensity :
        maxGreenIntensity;
    maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
    minIntensity = intensity < minIntensity ? intensity : minIntensity;
    green[ intensity ]++;

    intensity = *(( uint8_t* ) imageData + i + 2 );
    maxBlueIntensity = intensity > maxBlueIntensity ? intensity :
        maxBlueIntensity;
    maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
    minIntensity = intensity < minIntensity ? intensity : minIntensity;
    swapBlue[ intensity ]++;
  }
  for ( i = 0; i < 256; i++ ) {
    maxCount = ( red[i] > maxCount ) ? red[i] : maxCount;
    maxCount = ( green[i] > maxCount ) ? green[i] : maxCount;
    maxCount = ( blue[i] > maxCount ) ? blue[i] : maxCount;
  }
  for ( i = 0; i < 256; i++ ) {
    red[i] = red[i] * 100 / maxCount;
    green[i] = green[i] * 100 / maxCount;
    blue[i] = blue[i] * 100 / maxCount;
  }
  if ( OA_PIX_FMT_BGR24 == format ) {
    int s = maxBlueIntensity;
    maxBlueIntensity = maxRedIntensity;
    maxRedIntensity = s;
  }
}


void
HistogramWidget::_processMosaicHistogram ( void* imageData,
    unsigned int width, unsigned int height, unsigned int length, int format )
{
  unsigned int i, x, y, bytesPerLine, bytesPerPixel;
  int maxCount = 1;
  int step;
  char colour;
  const char* pattern = "    ";

  bytesPerPixel = oaFrameFormats[ format ].bytesPerPixel;
  bytesPerLine = width * bytesPerPixel;
  colours = 3;
  step = length / 10000;
  if ( step < 1 ) {
    step = 1;
  }
  // make sure step is odd, so we pick up all photosite colours
  step &= 1;
  // and step whole photosites
  step *= bytesPerPixel;

  maxRedIntensity = 0;
  maxGreenIntensity = 0;
  maxBlueIntensity = 0;
  bzero ( red, sizeof( int ) * 256 );
  bzero ( green, sizeof( int ) * 256 );
  bzero ( blue, sizeof( int ) * 256 );

  fullIntensity = ( 1 << oaFrameFormats[ format ].bitsPerPixel ) - 1;

  int intensity;
  for ( i = 0; i < length; i += step ) {
    x = ( i % bytesPerLine ) / bytesPerPixel;
    y = i / bytesPerLine;
    switch ( format ) {
      case OA_PIX_FMT_BGGR8:
      case OA_PIX_FMT_BGGR16LE:
      case OA_PIX_FMT_BGGR16BE:
        pattern = "BGGR";
        break;

      case OA_PIX_FMT_RGGB8:
      case OA_PIX_FMT_RGGB16LE:
      case OA_PIX_FMT_RGGB16BE:
        pattern = "RGGB";
        break;

      case OA_PIX_FMT_GBRG8:
      case OA_PIX_FMT_GBRG16LE:
      case OA_PIX_FMT_GBRG16BE:
        pattern = "GBRG";
        break;

      case OA_PIX_FMT_GRBG8:
      case OA_PIX_FMT_GRBG16LE:
      case OA_PIX_FMT_GRBG16BE:
        pattern = "GRBG";
        break;
    }
    colour = pattern[ 2 * ( y % 2 ) + ( x % 2 )];

    if ( oaFrameFormats[ format ].bitsPerPixel == 16 ) {
      if ( oaFrameFormats[ format ].littleEndian ) {
        int b1, b2;
        b1 = *(( uint8_t* ) imageData + i );
        b2 = *(( uint8_t* ) imageData + i + 1 );
        intensity = b1 + ( b2 << 8 );
      } else {
        int b1, b2;
        b1 = *(( uint8_t* ) imageData + i );
        b2 = *(( uint8_t* ) imageData + i + 1 );
        intensity = ( b1 << 8 ) + b2;
      }
    } else {
      if ( oaFrameFormats[ format ].bitsPerPixel == 8 ) {
        intensity = *(( uint8_t* ) imageData + i );
      } else {
        qWarning() << __FUNCTION__ << "can't handle bit depth" <<
            oaFrameFormats[ format ].bitsPerPixel;
      }
    }

    switch ( colour ) {
      case 'R':
        maxRedIntensity = intensity > maxRedIntensity ? intensity :
            maxRedIntensity;
        maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
        minIntensity = intensity < minIntensity ? intensity : minIntensity;
        red[ intensity ]++;
        break;

      case 'G':
        maxGreenIntensity = intensity > maxGreenIntensity ? intensity :
            maxGreenIntensity;
        maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
        minIntensity = intensity < minIntensity ? intensity : minIntensity;
        green[ intensity ]++;
        break;

      case 'B':
        maxBlueIntensity = intensity > maxBlueIntensity ? intensity :
            maxBlueIntensity;
        maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
        minIntensity = intensity < minIntensity ? intensity : minIntensity;
        blue[ intensity ]++;
        break;
    }
  }

  for ( i = 0; i < 256; i++ ) {
    maxCount = ( red[i] > maxCount ) ? red[i] : maxCount;
    maxCount = ( green[i] > maxCount ) ? green[i] : maxCount;
    maxCount = ( blue[i] > maxCount ) ? blue[i] : maxCount;
  }
  for ( i = 0; i < 256; i++ ) {
    red[i] = red[i] * 100 / maxCount;
    green[i] = green[i] * 100 / maxCount;
    blue[i] = blue[i] * 100 / maxCount;
  }
}
