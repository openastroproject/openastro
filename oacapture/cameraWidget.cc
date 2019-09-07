/*****************************************************************************
 *
 * cameraWidget.cc -- class for the camera widget in the UI
 *
 * Copyright 2013,2014,2015,2017,2018,2019
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

extern "C" {
#include <openastro/demosaic.h>
}

#include "commonState.h"
#include "commonConfig.h"

#include "configuration.h"
#include "cameraWidget.h"
#include "controlWidget.h"
#include "state.h"


CameraWidget::CameraWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  inputFormatLabel = new QLabel( tr ( "Frame Format" ));
  inputFormatMenu = new QComboBox ( this );
  inputFormatMenu->addItem ( tr ( oaFrameFormats[1].name ));
  inputFormatMenu->setItemData ( 0, tr ( oaFrameFormats[1].simpleName ),
      Qt::ToolTipRole );

  binning2x2 = new QCheckBox ( tr ( "2x2 Binning" ), this );
  binning2x2->setToolTip ( tr ( "Enable 2x2 binning in camera" ));
  binning2x2->setChecked ( commonConfig.binning2x2 );
  connect ( binning2x2, SIGNAL( stateChanged ( int )), this,
      SLOT( setBinning ( int )));

  tempLabel = new QLabel();
  if ( generalConf.tempsInC ) {
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

  if ( !generalConf.dockableControls ) {
    setTitle ( tr ( "Camera" ));
  }

  grid = new QGridLayout();
  grid->addWidget ( binning2x2, 0, 0 );
  grid->addWidget ( inputFormatLabel, 1, 0 );
  grid->addWidget ( inputFormatMenu, 1, 1 );
  grid->addWidget ( tempLabel, 0, 2 );
  grid->addWidget ( fpsMaxLabel, 1, 2 );
  grid->addWidget ( fpsActualLabel, 2, 2 );
  grid->addWidget ( tempValue, 0, 3 );
  grid->addWidget ( fpsMaxValue, 1, 3 );
  grid->addWidget ( fpsActualValue, 2, 3 );
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
  unsigned int format;
  int numActions = 0, currentAction = 0, binning = 0;
  int foundConfiguredFormat = 0;
	int firstFormat = -1;

  if ( !inputFormatList.empty()) {
    disconnect ( inputFormatMenu, SIGNAL( currentIndexChanged ( int )), 
        this, SLOT( changeFrameFormat ( int )));
  }
  inputFormatMenu->clear();
  inputFormatList.clear();

  for ( format = 1; format < OA_PIX_FMT_LAST_P1; format++ ) {
    if ( commonState.camera->hasFrameFormat ( format )) {
			firstFormat = format;
      inputFormatMenu->addItem ( tr ( oaFrameFormats[ format ].name ));
      inputFormatMenu->setItemData ( numActions,
          tr ( oaFrameFormats[ format ].simpleName ), Qt::ToolTipRole );
      inputFormatList.append ( format );
      if ( format == config.inputFrameFormat ) {
        currentAction = numActions;
        inputFormatMenu->setCurrentIndex ( currentAction );
        foundConfiguredFormat = 1;
      }
      numActions++;
    }
  }
  if ( !foundConfiguredFormat ) {
    config.inputFrameFormat = firstFormat;
    currentAction = 0;
  }

  connect ( inputFormatMenu, SIGNAL( currentIndexChanged ( int )), 
      this, SLOT( changeFrameFormat ( int )));

	// Shouldn't need to do this is there's only one possible frame format
	if ( numActions > 1 ) {
		changeFrameFormat ( currentAction );
	}

  if ( cameraConf.forceInputFrameFormat ) {
    updateForceFrameFormat ( 0, cameraConf.forceInputFrameFormat );
  }
	binning = commonState.camera->hasBinning ( 2 ) ? 1 : 0;
	if ( binning ) {
		commonState.camera->setControl ( OA_CAM_CTRL_BINNING, OA_BIN_MODE_NONE );
	}
	binning2x2->setEnabled ( binning );
}


void
CameraWidget::setBinning ( int newState )
{
  int oldVal = commonConfig.binning2x2;

  if ( commonState.camera->isInitialised()) {
    if ( newState == Qt::Unchecked ) {
      commonState.camera->setControl ( OA_CAM_CTRL_BINNING, OA_BIN_MODE_NONE );
      commonConfig.binning2x2 = 0;
      // if we're "unbinning", double the expected resolution.
      // imageWidget::configure will sort it out if there's no match
      if ( oldVal ) {
        commonConfig.imageSizeX *= 2;
        commonConfig.imageSizeY *= 2;
      }
      commonState.binModeX = commonState.binModeY = 1;
      commonState.binningValid = 1;
    } else {
      commonState.camera->setControl ( OA_CAM_CTRL_BINNING, OA_BIN_MODE_2x2 );
      commonConfig.binning2x2 = 1;
      // if we're binning, half the expected resolution.
      // imageWidget::configure will sort it out if there's no match
      if ( !oldVal ) {
        commonConfig.imageSizeX /= 2;
        commonConfig.imageSizeY /= 2;
      }
      commonState.binModeX = commonState.binModeY = 2;
      commonState.binningValid = 1;
    }
    SET_PROFILE_CONFIG( binning2x2, commonConfig.binning2x2 );
    SET_PROFILE_CONFIG( imageSizeX, commonConfig.imageSizeX );
    SET_PROFILE_CONFIG( imageSizeY, commonConfig.imageSizeY );
    state.imageWidget->configure();
  }
}


void
CameraWidget::enableBinningControl ( int state )
{
  binning2x2->setEnabled ( state );
}


void
CameraWidget::updateFromConfig ( void )
{
  if ( binning2x2->isEnabled()) {
    binning2x2->setChecked ( commonConfig.binning2x2 );
  }
  /*
  if ( sixteenBit->isEnabled()) {
    sixteenBit->setChecked ( config.sixteenBit );
  }
  if ( rawMode->isEnabled()) {
    rawMode->setChecked ( config.rawMode );
  }
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

  temp = commonState.camera->getTemperature();
  commonState.cameraTempValid = 1;
  commonState.cameraTemp = temp;

  if ( updateTemperatureLabel == 1 ) {
    if ( generalConf.tempsInC ) {
      tempLabel->setText ( tr ( "Temp (C)" ));
    } else {
      tempLabel->setText ( tr ( "Temp (F)" ));
    }
    updateTemperatureLabel = 0;
  }

  if ( !generalConf.tempsInC ) {
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


void
CameraWidget::changeFrameFormat ( int menuOption )
{
  int newFormat = inputFormatList[ menuOption ];

  if ( !commonState.camera->isInitialised()) {
    return;
  }

  commonState.camera->setFrameFormat ( newFormat );
  state.previewWidget->setVideoFramePixelFormat ( newFormat );
  config.inputFrameFormat = newFormat;

  if ( oaFrameFormats[ newFormat ].rawColour ) {
    if ( state.settingsWidget ) {
      state.settingsWidget->updateCFASetting();
    }
  }
  state.captureWidget->enableTIFFCapture (
      ( !oaFrameFormats[ newFormat ].rawColour ||
      ( commonConfig.demosaic && demosaicConf.demosaicOutput )) ? 1 : 0 );
  state.captureWidget->enablePNGCapture (
      ( !oaFrameFormats[ newFormat ].rawColour ||
      ( commonConfig.demosaic && demosaicConf.demosaicOutput )) ? 1 : 0 );
  state.captureWidget->enableMOVCapture (( QUICKTIME_OK( newFormat ) ||
      ( oaFrameFormats[ newFormat ].rawColour && commonConfig.demosaic &&
      demosaicConf.demosaicOutput )) ? 1 : 0 );
  state.captureWidget->enableNamedPipeCapture (
      ( !oaFrameFormats[ newFormat ].rawColour ||
      ( commonConfig.demosaic && demosaicConf.demosaicOutput )) ? 1 : 0 );
  return;
}


void
CameraWidget::updateForceFrameFormat ( unsigned int oldFormat,
    unsigned int newFormat )
{
  unsigned int n;

  if ( oldFormat == 0 ) {
    // We didn't previously have a forced format set.  Disable the menu.
    // If the new format is not in the menu already, add it at the start.
    // Set the current index to the relevant option.

    inputFormatMenu->setEnabled ( 0 );
    disconnect ( inputFormatMenu, SIGNAL( currentIndexChanged ( int )), 
        this, SLOT( changeFrameFormat ( int )));
    if ( commonState.camera->hasFrameFormat ( newFormat )) {
      n = inputFormatList.indexOf ( newFormat );
      inputFormatMenu->setCurrentIndex ( n );
    } else {
      inputFormatMenu->insertItem ( 0, tr ( oaFrameFormats[ newFormat ].name ));
      inputFormatMenu->setItemData ( 0,
          tr ( oaFrameFormats[ newFormat ].simpleName ), Qt::ToolTipRole );
      inputFormatMenu->setCurrentIndex ( 0 );
    }
  }

  if ( newFormat == 0 ) {
    // forced format has been disabled.  Enable the menu.  If the old
    // format isn't supported by the camera, delete it from the menu.

    if ( !commonState.camera->hasFrameFormat ( oldFormat )) {
      inputFormatMenu->removeItem ( 0 );
    }
    inputFormatMenu->setEnabled ( 1 );
    connect ( inputFormatMenu, SIGNAL( currentIndexChanged ( int )), 
        this, SLOT( changeFrameFormat ( int )));
  }

  if ( oldFormat && newFormat ) {
    // just the forced format has been changed.  Menu should be disabled,
    // so leave that.  If the old format is not supported by the camera
    // then delete it from the menu.  If the new format is not supported
    // by the camera, add it to the menu.  Set the correct menu option.

    if ( !commonState.camera->hasFrameFormat ( oldFormat )) {
      inputFormatMenu->removeItem ( 0 );
    }
    if ( commonState.camera->hasFrameFormat ( newFormat )) {
      n = inputFormatList.indexOf ( newFormat );
      inputFormatMenu->setCurrentIndex ( n );
    } else {
      inputFormatMenu->insertItem ( 0, tr ( oaFrameFormats[ newFormat ].name ));
      inputFormatMenu->setCurrentIndex ( 0 );
    }
  }

  // And now set preview pixel format

  if ( newFormat ) {
    state.previewWidget->setVideoFramePixelFormat ( newFormat );
  } else {
    state.previewWidget->setVideoFramePixelFormat (
        inputFormatList[ inputFormatMenu->currentIndex()]);
  }
}
