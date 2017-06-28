/*****************************************************************************
 *
 * imageWidget.cc -- class for the image controls in the UI
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

extern "C" {
#include <openastro/camera.h>
}

#include "configuration.h"
#include "imageWidget.h"
#include "controlWidget.h"
#include "state.h"


ImageWidget::ImageWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  roiXValidator = 0;
  roiYValidator = 0;

  grid = new QGridLayout ( this );

  roi = new QRadioButton ( tr ( "Use ROI" ), this );
  roi->setToolTip ( tr ( "Manually select a region of interest" ));
  max = new QRadioButton ( tr ( "Max Size" ), this );
  max->setToolTip ( tr ( "Set maximum size image frame" ));
  if ( config.useROI ) {
    roi->setChecked ( true );
    max->setChecked ( false );
  } else {
    roi->setChecked ( false );
    max->setChecked ( true );
  }
  buttonGroup = new QButtonGroup ( this );
  buttonGroup->addButton ( roi );
  buttonGroup->addButton ( max );
  buttonGroup->setExclusive ( true );
  connect ( max, SIGNAL( clicked()), this, SLOT( setMaxImageSize()));
  connect ( max, SIGNAL( clicked()), this, SLOT( enableROIEntry()));

  xSize = new QLineEdit ( this );
  ySize = new QLineEdit ( this );
  x = new QLabel ( " x ", this );

  xSize->setMaxLength ( 4 );
  xSize->setFixedWidth ( 45 );
  ySize->setMaxLength ( 4 );
  ySize->setFixedWidth ( 45 );
  QString xStr, yStr;
  if ( config.imageSizeX > 0 ) {
    xStr = QString::number ( config.imageSizeX );
  } else {
    xStr = "";
    config.imageSizeX = 0;
  }
  xSize->setText ( xStr );
  if ( config.imageSizeY > 0 ) {
    yStr = QString::number ( config.imageSizeY );
  } else {
    yStr = "";
    config.imageSizeY = 0;
  }
  ySize->setText ( yStr );

  roiButton = new QPushButton (
      QIcon ( ":/icons/roi.png" ), "", this );
  roiButton->setToolTip ( tr ( "Set new ROI" ));
  connect ( roiButton, SIGNAL( clicked()), this, SLOT( changeROI()));
  roiButton->setEnabled ( 0 );

  ignoreResolutionChanges = 0;
  resMenu = new QComboBox ( this );
  QStringList resolutions;
  resolutions << "640x480" << "1280x960";
  resMenu->addItems ( resolutions );
  connect ( resMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( resolutionChanged ( int )));

  setTitle ( tr ( "Image" ));

  roiInputBox = new QHBoxLayout();
  roiInputBox->addWidget ( xSize );
  roiInputBox->addWidget ( x );
  roiInputBox->addWidget ( ySize );
  roiInputBox->addWidget ( roiButton );

  grid->addWidget ( roi, 0, 0 );
  grid->addWidget ( max, 0, 2 );
  grid->addLayout ( roiInputBox, 1, 0 );
  grid->setColumnStretch ( 1, 1 );
  grid->addWidget ( resMenu, 1, 2 );

  setLayout ( grid );
}


ImageWidget::~ImageWidget()
{
  state.mainWindow->destroyLayout (( QLayout* ) grid );
}


void
ImageWidget::configure ( void )
{
  int maxX, maxY;
  maxX = maxY = 0;

  // This includes a particularly ugly way to sort the resolutions using
  // a QMap and some further QMap abuse to be able to find the X and Y
  // resolutions should we not have a setting that matches the config and
  // happen to choose the max size option
  const FRAMESIZES* sizeList = state.camera->frameSizes();
  QMap<int,QString> sortMap;
  QMap<int,int> xRes, yRes;
  QString showItemStr;
  int firstKey = -1, lastKey = -1;
  unsigned int i;

  for ( i = 0; i < sizeList->numSizes; i++ ) {
    int numPixels = sizeList->sizes[i].x * sizeList->sizes[i].y;
    QString resStr = QString::number ( sizeList->sizes[i].x ) + "x" +
        QString::number ( sizeList->sizes[i].y );
    if ( sizeList->sizes[i].x == config.imageSizeX &&
        sizeList->sizes[i].y == config.imageSizeY ) {
      showItemStr = resStr;
    }
    sortMap [ numPixels ] = resStr;
    xRes [ numPixels ] = sizeList->sizes[i].x;
    yRes [ numPixels ] = sizeList->sizes[i].y;
    if ( lastKey < numPixels ) {
      lastKey = numPixels;
    }
    if ( -1 == firstKey || firstKey > numPixels ) {
      firstKey = numPixels;
    }
  }

  int numItems = 0, showItem = -1, showXRes = -1, showYRes = -1;
  QStringList resolutions;
  QMap<int,QString>::iterator j;
  QMap<int,int>::iterator x, y;
  XResolutions.clear();
  YResolutions.clear();
  for ( j = sortMap.begin(), x = xRes.begin(), y = yRes.begin();
      j != sortMap.end(); j++, x++, y++ ) {
    resolutions << *j;
    XResolutions << *x;
    YResolutions << *y;
    if ( *j == showItemStr ) {
      showItem = numItems;
      showXRes = *x;
      showYRes = *y;
    }
    numItems++;
  }

  ignoreResolutionChanges = 1;
  resMenu->clear();
  resMenu->addItems ( resolutions );
  ignoreResolutionChanges = 0;

  if ( showItem >= 0 ) {
    config.imageSizeX = showXRes;
    config.imageSizeY = showYRes;
  } else {
    showItem = max->isChecked() ? numItems - 1: 0;
    if ( showItem ) {
      config.imageSizeX = xRes[ lastKey ];
      config.imageSizeY = yRes[ lastKey ];
    } else {
      config.imageSizeX = xRes[ firstKey ];
      config.imageSizeY = yRes[ firstKey ];
    }
  }
  maxX = xRes[ lastKey ];
  maxY = yRes[ lastKey ];

  // There's a gotcha here for cameras that only support a single
  // resolution, as the index won't actually change, and the slot
  // won't get called.  So, we have to get the current index and
  // if it is 0, call resolution changed manually
  if ( resMenu->currentIndex() == showItem ) {
    resolutionChanged ( showItem );
  } else {
    resMenu->setCurrentIndex ( showItem );
  }
  // xSize->setText ( QString::number ( config.imageSizeX ));
  // ySize->setText ( QString::number ( config.imageSizeY ));
  xSize->setEnabled ( 0 );
  ySize->setEnabled ( 0 );
  if ( 1 == numItems ) {
    max->setEnabled ( 0 );
  } else {
    max->setEnabled ( 1 );
  }

  if ( state.camera->hasROI()) {
    roi->setEnabled ( 1 );
    max->setEnabled ( 1 );
    roiButton->setEnabled(1);
    if ( !roiXValidator ) {
      roiXValidator = new QIntValidator ( 1, maxX, this );
      roiYValidator = new QIntValidator ( 1, maxY, this );
    } else {
      roiXValidator->setRange ( 1, maxX );
      roiYValidator->setRange ( 1, maxY );
    }
    xSize->setValidator ( roiXValidator );
    ySize->setValidator ( roiYValidator );
    xSize->setEnabled ( 1 );
    ySize->setEnabled ( 1 );
  } else {
    roi->setEnabled ( 0 );
    xSize->setEnabled ( 0 );
    ySize->setEnabled ( 0 );
    roiButton->setEnabled ( 0 );
  }
}


void
ImageWidget::resolutionChanged ( int index )
{
  // changes to this function may need to be replicated in
  // updateFromConfig()
  if ( !state.camera || ignoreResolutionChanges ||
      !state.camera->isInitialised()) {
    return;
  }

  if ( index == ( resMenu->count() - 1 )) {
    // last item -- max size
    max->setChecked ( true );
  } else {
    // Neither should be checked in this instance
    buttonGroup->setExclusive ( false );
    max->setChecked ( false );
    roi->setChecked ( false );
    buttonGroup->setExclusive ( true );
  }
  config.imageSizeX = XResolutions[ index ];
  config.imageSizeY = YResolutions[ index ];
  // xSize->setText ( QString::number ( config.imageSizeX ));
  // ySize->setText ( QString::number ( config.imageSizeY ));
  doResolutionChange ( 0 );
}


void
ImageWidget::doResolutionChange ( int roiChanged )
{
  // state.camera->delayFrameRateChanges();
  if ( state.controlWidget ) {
    state.controlWidget->updateFrameRates();
  }
  if ( roiChanged ) {
    state.camera->setROI ( config.imageSizeX, config.imageSizeY );
  } else {
    state.camera->setResolution ( config.imageSizeX, config.imageSizeY );
  }
  if ( state.previewWidget ) {
    state.previewWidget->updatePreviewSize();
  }
  if ( config.profileOption >= 0 ) {
    config.profiles[ config.profileOption ].imageSizeX = config.imageSizeX;
    config.profiles[ config.profileOption ].imageSizeY = config.imageSizeY;
  }
}


void
ImageWidget::enableAllControls ( int state )
{
  // actually we need to save the state if we're disabling because it
  // doesn't make sense to re-enable them all

  bool xSizeState, ySizeState, roiState, maxState, resMenuState;

  if ( state ) {
    xSizeState = xSizeSavedState;
    ySizeState = ySizeSavedState;
    roiState = roiSavedState;
    maxState = maxSavedState;
    resMenuState = resMenuSavedState;
  } else {
    xSizeSavedState = xSize->isEnabled();
    ySizeSavedState = ySize->isEnabled();
    roiSavedState = roi->isEnabled();
    maxSavedState = max->isEnabled();
    resMenuSavedState = resMenu->isEnabled();
    xSizeState = 0;
    ySizeState = 0;
    roiState = 0;
    maxState = 0;
    resMenuState = 0;
  }
  xSize->setEnabled ( xSizeState );
  ySize->setEnabled ( ySizeState );
  max->setEnabled ( maxState );
  roi->setEnabled ( roiState );
  resMenu->setEnabled ( resMenuState );
}


void
ImageWidget::setMaxImageSize ( void )
{
  int newItem = resMenu->count() - 1;
  int currentItem = resMenu->currentIndex();

  if ( newItem == currentItem ) {
    resetResolution();
  } else {
    resMenu->setCurrentIndex ( newItem );
  }
}


void
ImageWidget::enableROIEntry ( void )
{
  xSize->setEnabled ( 1 );
  ySize->setEnabled ( 1 );
  roiButton->setEnabled ( 1 );
}


void
ImageWidget::resetResolution ( void )
{
  resolutionChanged ( resMenu->currentIndex());
}


void
ImageWidget::updateFromConfig ( void )
{
  if ( !state.camera || !state.camera->isInitialised()) {
    return;
  }
  int numRes = resMenu->count();
  if ( numRes ) {
    int index = 0;
    for ( int i = 0; i < numRes && 0 == index; i++ ) {
      if ( XResolutions[ i ] == config.imageSizeX && YResolutions[ i ] ==
          config.imageSizeY ) {
        index = i;
      }
    }

    if (( numRes - 1 ) == index ) {
      // last item -- max size
      max->setChecked ( true );
    } else {
      // Neither should be checked in this instance
      buttonGroup->setExclusive ( false );
      max->setChecked ( false );
      roi->setChecked ( false );
      buttonGroup->setExclusive ( true );
    }
    // need to ignore changes here because changing the index will
    // call resolutionChanged() and we're doing its work ourselves
    ignoreResolutionChanges = 1;
    resMenu->setCurrentIndex ( index );
    ignoreResolutionChanges = 0;
    // xSize->setText ( QString::number ( config.imageSizeX ));
    // ySize->setText ( QString::number ( config.imageSizeY ));

    // if ( state.camera->isInitialised()) {
      // state.camera->delayFrameRateChanges();
    // }
    if ( state.controlWidget ) {
      state.controlWidget->updateFrameRates();
    }
    if ( state.camera->isInitialised()) {
      state.camera->setResolution ( config.imageSizeX, config.imageSizeY );
    }
    if ( state.previewWidget ) {
      state.previewWidget->updatePreviewSize();
    }
  }
}


void
ImageWidget::changeROI ( void )
{
  QString xStr = xSize->text();
  QString yStr = ySize->text();
  int x, y;

  x = y = -1;
  if ( xStr != "" ) {
    x = xStr.toInt();
  }
  if ( yStr != "" ) {
    y = yStr.toInt();
  }

  if ( x > 0 && y > 0 ) {
    unsigned int altX, altY;
    if ( state.camera->testROISize ( x, y, &altX, &altY )) {
      x = altX;
      y = altY;
    }
    config.imageSizeX = x;
    config.imageSizeY = y;
    xSize->setText ( QString::number ( config.imageSizeX ));
    ySize->setText ( QString::number ( config.imageSizeY ));
    doResolutionChange ( 1 );
  }
}
