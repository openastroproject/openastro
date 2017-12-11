/*****************************************************************************
 *
 * cameraWidget.cc -- class for the camera widget in the UI
 *
 * Copyright 2013,2014,2015,2017 James Fidell (james@openastroproject.org)
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
#include <openastro/demosaic.h>
}

#include "configuration.h"
#include "cameraWidget.h"
#include "controlWidget.h"
#include "state.h"


CameraWidget::CameraWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  sixteenBit = new QCheckBox ( tr ( "16-bit" ), this );
  sixteenBit->setToolTip ( tr ( "Capture in 16-bit mode" ));
  sixteenBit->setChecked ( config.sixteenBit );
  connect ( sixteenBit, SIGNAL( stateChanged ( int )), this,
      SLOT( set16Bit ( int )));

  binning2x2 = new QCheckBox ( tr ( "2x2 Binning" ), this );
  binning2x2->setToolTip ( tr ( "Enable 2x2 binning in camera" ));
  binning2x2->setChecked ( config.binning2x2 );
  connect ( binning2x2, SIGNAL( stateChanged ( int )), this,
      SLOT( setBinning ( int )));

  rawMode = new QCheckBox ( tr ( "Raw mode" ), this );
  rawMode->setToolTip ( tr ( "Enable raw mode for colour cameras" ));
  rawMode->setChecked ( config.rawMode );
  connect ( rawMode, SIGNAL( stateChanged ( int )), this,
      SLOT( setRawMode ( int )));

  tempLabel = new QLabel();
  if ( config.tempsInC ) {
    tempLabel->setText ( tr ( "Temp (C)" ));
  } else {
    tempLabel->setText ( tr ( "Temp (F)" ));
  }
  tempLabel->setFixedWidth ( 60 );
  fpsMaxLabel = new QLabel ( tr ( "FPS (max)" ));
  fpsMaxLabel->setFixedWidth ( 65 );
  fpsActualLabel = new QLabel ( tr ( "FPS (actual)" ));
  fpsActualLabel->setFixedWidth ( 80 );

  tempValue = new QLabel ( "" );
  tempValue->setFixedWidth ( 30 );
  fpsMaxValue = new QLabel ( "0" );
  fpsMaxValue->setFixedWidth ( 30 );
  fpsActualValue = new QLabel ( "0" );
  fpsActualValue->setFixedWidth ( 50 );

  if ( !config.dockableControls ) {
    setTitle ( tr ( "Camera" ));
  }

  grid = new QGridLayout();
  grid->addWidget ( sixteenBit, 0, 0 );
  grid->addWidget ( binning2x2, 1, 0 );
  grid->addWidget ( rawMode, 2, 0 );
  grid->addWidget ( tempLabel, 0, 1 );
  grid->addWidget ( fpsMaxLabel, 1, 1 );
  grid->addWidget ( fpsActualLabel, 2, 1 );
  grid->addWidget ( tempValue, 0, 2 );
  grid->addWidget ( fpsMaxValue, 1, 2 );
  grid->addWidget ( fpsActualValue, 2, 2 );
  setLayout ( grid );
}


CameraWidget::~CameraWidget()
{
  if ( grid ) {
    state.mainWindow->destroyLayout (( QLayout* ) grid );
  }
}


void
CameraWidget::configure ( void )
{
  sixteenBit->setEnabled ( state.camera->has16Bit() ? 1 : 0 );
  binning2x2->setEnabled ( state.camera->hasBinning ( 2 ) ? 1 : 0 );
  rawMode->setEnabled (( state.camera->hasRawMode() &&
      state.camera->hasDemosaicMode() && state.camera->isColour()) ? 1 : 0 );
}


void
CameraWidget::setBinning ( int newState )
{
  int oldVal = config.binning2x2;

  if ( state.camera->isInitialised()) {
    if ( newState == Qt::Unchecked ) {
      state.camera->setControl ( OA_CAM_CTRL_BINNING, OA_BIN_MODE_NONE );
      config.binning2x2 = 0;
      // if we're "unbinning", double the expected resolution.
      // imageWidget::configure will sort it out if there's no match
      if ( oldVal ) {
        config.imageSizeX *= 2;
        config.imageSizeY *= 2;
      }
      state.binModeX = state.binModeY = 1;
      state.binningValid = 1;
    } else {
      state.camera->setControl ( OA_CAM_CTRL_BINNING, OA_BIN_MODE_2x2 );
      config.binning2x2 = 1;
      // if we're binning, half the expected resolution.
      // imageWidget::configure will sort it out if there's no match
      if ( !oldVal ) {
        config.imageSizeX /= 2;
        config.imageSizeY /= 2;
      }
      state.binModeX = state.binModeY = 2;
      state.binningValid = 1;
    }
    SET_PROFILE_CONFIG( binning2x2, config.binning2x2 );
    SET_PROFILE_CONFIG( imageSizeX, config.imageSizeX );
    SET_PROFILE_CONFIG( imageSizeY, config.imageSizeY );
    state.imageWidget->configure();
  }
}


void
CameraWidget::enableBinningControl ( int state )
{
  binning2x2->setEnabled ( state );
}


void
CameraWidget::set16Bit ( int newState )
{
  if ( state.camera->isInitialised()) {
    int format;
    if ( newState == Qt::Unchecked ) {
      state.camera->setBitDepth ( 8 );
      config.sixteenBit = 0;
    } else {
      state.camera->setBitDepth ( 16 );
      config.sixteenBit = 1;
    }
    SET_PROFILE_CONFIG( sixteenBit, config.sixteenBit );
    format = state.camera->videoFramePixelFormat ( 0 );
    state.previewWidget->setVideoFramePixelFormat ( format );
    state.captureWidget->enableTIFFCapture (( !OA_ISBAYER( format ) ||
        ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
    state.captureWidget->enablePNGCapture (( !OA_ISBAYER( format ) ||
        ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
    state.captureWidget->enableMOVCapture (( QUICKTIME_OK( format ) ||
        ( OA_ISBAYER( format ) && config.demosaic &&
        config.demosaicOutput )) ? 1 : 0 );
  }
  return;
}


void
CameraWidget::setRawMode ( int newState )
{
  if ( state.camera->isInitialised()) {
    int format;
    if ( newState == Qt::Unchecked ) {
      config.rawMode = 0;
    } else {
      config.rawMode = 1;
    }
    SET_PROFILE_CONFIG( rawMode, config.rawMode );
    state.camera->setRawMode ( config.rawMode );
    format = state.camera->videoFramePixelFormat ( 0 );
    state.previewWidget->setVideoFramePixelFormat ( format );
/*
    if ( config.rawMode ) {
      switch ( format ) {
        case OA_PIX_FMT_RGGB8:
          config.cfaPattern = OA_DEMOSAIC_RGGB;
          break;
        case OA_PIX_FMT_BGGR8:
          config.cfaPattern = OA_DEMOSAIC_BGGR;
          break;
        case OA_PIX_FMT_GBRG8:
          config.cfaPattern = OA_DEMOSAIC_GBRG;
          break;
        case OA_PIX_FMT_GRBG8:
          config.cfaPattern = OA_DEMOSAIC_GRBG;
          break;
      }
      if ( state.settingsWidget ) {
        state.settingsWidget->updateCFASetting();
      }
    } else {
      state.captureWidget->enableMOVCapture ( QUICKTIME_OK( format ) ? 1 : 0 );
    }
*/
    state.captureWidget->enableTIFFCapture (( !OA_ISBAYER( format ) ||
        ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
    state.captureWidget->enablePNGCapture (( !OA_ISBAYER( format ) ||
        ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
    state.captureWidget->enableMOVCapture (( QUICKTIME_OK( format ) || 
        ( OA_ISBAYER( format ) && config.demosaic &&
        config.demosaicOutput )) ? 1 : 0 );
  }
}


void
CameraWidget::updateFromConfig ( void )
{
  if ( sixteenBit->isEnabled()) {
    sixteenBit->setChecked ( config.sixteenBit );
  }
  if ( binning2x2->isEnabled()) {
    binning2x2->setChecked ( config.binning2x2 );
  }
  if ( rawMode->isEnabled()) {
    rawMode->setChecked ( config.rawMode );
  }
  /*
  if ( colourise->isEnabled()) {
    colourise->setChecked ( config.colourise );
  }
  */
}


void
CameraWidget::setActualFrameRate ( double fps )
{
  QString stringVal;

  // precision, eg 100, 10.0, 1.00, 0.10, 0.01
  stringVal.setNum ( fps, 'f', std::min(2, std::max(0, 2-(int)log10(fps))) );
  fpsActualValue->setText ( stringVal );
  state.currentFPS = fps;
}


void
CameraWidget::setTemperature()
{
  float temp;
  QString stringVal;

  temp = state.camera->getTemperature();
  state.cameraTempValid = 1;
  state.cameraTemp = temp;

  if ( updateTemperatureLabel == 1 ) {
    if ( config.tempsInC ) {
      tempLabel->setText ( tr ( "Temp (C)" ));
    } else {
      tempLabel->setText ( tr ( "Temp (F)" ));
    }
    updateTemperatureLabel = 0;
  }

  if ( !config.tempsInC ) {
    temp = temp * 9 / 5 + 32;
  }
  stringVal.setNum ( temp, 'g', 3 );
  tempValue->setText ( stringVal );
}


void
CameraWidget::resetTemperatureLabel()
{
  updateTemperatureLabel = 1;
}


void
CameraWidget::clearTemperature ( void )
{
  tempValue->setText ( "" );
}


void
CameraWidget::showFPSMaxValue ( int value )
{
  fpsMaxValue->setText ( QString::number ( value ));
}


void
CameraWidget::clearFPSMaxValue ( void )
{
  fpsMaxValue->setText ( "" );
}
