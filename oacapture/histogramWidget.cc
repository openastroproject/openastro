/*****************************************************************************
 *
 * histogramWidget.cc -- class for the histogram display
 *
 * Copyright 2013,2014 James Fidell (james@openastroproject.org)
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
  setWindowIcon ( QIcon ( ":/icons/barchart.png" ));
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
HistogramWidget::process ( void* imageData, int length, int format )
{
  int maxCount = 1;
  int minIntensity = 0xffff;

  // Can't do this in the constructor because previewWidget might not exist
  // at that time
  if ( !signalConnected ) {
    connect ( state.previewWidget, SIGNAL( updateHistogram ( void )),
        this, SLOT( update ( void )));
    signalConnected = 1;
  }

  colours = 1;
  if ( OA_PIX_FMT_RGB24 == format || OA_PIX_FMT_BGR24 == format ) {
    colours = 3;
  }
  doneProcess = 1;

  int step = ( length / colours ) / 10000;
  step *= colours;
  if ( step < 1 ) {
    step = 1;
  }

  fullIntensity = 0xff;
  if ( 1 == colours ) {
    maxIntensity = 0;
    bzero ( grey, sizeof( int ) * 256 );
    int intensity;
    if ( 2 == OA_BYTES_PER_PIXEL( format )) {
      step *= 2;
      fullIntensity = 0xffff;
      if ( OA_PIX_FMT_GREY16LE == format || OA_PIX_FMT_BGGR16LE == format ||
          OA_PIX_FMT_RGGB16LE == format || OA_PIX_FMT_GBRG16LE == format ||
          OA_PIX_FMT_GRBG16LE == format ) {
        int b1, b2;
        for ( int i = 0; i < length; i += step ) {
          b1 = *(( uint8_t* ) imageData + i );
          b2 = *(( uint8_t* ) imageData + i + 1 );
          intensity = b1 + ( b2 << 8 );
          maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
          minIntensity = intensity < minIntensity ? intensity : minIntensity;
          grey[ b2 ]++;
        }
      } else {
        int b1, b2;
        for ( int i = 0; i < length; i += step ) {
          b1 = *(( uint8_t* ) imageData + i );
          b2 = *(( uint8_t* ) imageData + i + 1 );
          intensity = ( b1 << 8 ) + b2;
          maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
          minIntensity = intensity < minIntensity ? intensity : minIntensity;
          grey[ b1 ]++;
        }
      }
    } else {
      for ( int i = 0; i < length; i += step ) {
        intensity = *(( uint8_t* ) imageData + i );
        maxIntensity = intensity > maxIntensity ? intensity : maxIntensity;
        minIntensity = intensity < minIntensity ? intensity : minIntensity;
        grey[ intensity ]++;
      }
    }
    for ( int i = 0; i < 256; i++ ) {
      maxCount = ( grey[i] > maxCount ) ? grey[i] : maxCount;
    }
    for ( int i = 0; i < 256; i++ ) {
      grey[i] = grey[i] * 100 / maxCount;
    }
  } else {
    int* swapRed = red;
    int* swapBlue = blue;
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
    for ( int i = 0; i < length; i += step ) {
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
    for ( int i = 0; i < 256; i++ ) {
      maxCount = ( red[i] > maxCount ) ? red[i] : maxCount;
      maxCount = ( green[i] > maxCount ) ? green[i] : maxCount;
      maxCount = ( blue[i] > maxCount ) ? blue[i] : maxCount;
    }
    for ( int i = 0; i < 256; i++ ) {
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
