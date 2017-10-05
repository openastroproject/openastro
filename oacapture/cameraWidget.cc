/*****************************************************************************
 *
 * cameraWidget.cc -- class for the camera widget in the UI
 *
 * Copyright 2013,2014,2015 James Fidell (james@openastroproject.org)
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
  preview = new QCheckBox ( tr ( "Preview" ), this );
  preview->setToolTip ( tr ( "Turn preview on/off" ));
  preview->setChecked ( config.preview );
  connect ( preview, SIGNAL( stateChanged ( int )), parent,
      SLOT( changePreviewState ( int )));

  nightMode = new QCheckBox ( tr ( "Night Mode" ), this );
  nightMode->setToolTip ( tr ( "Turn night colours on/off" ));
  connect ( nightMode, SIGNAL( stateChanged ( int )), state.mainWindow,
      SLOT( setNightMode ( int )));
  nightMode->setChecked ( config.nightMode );

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

  colourise = new QCheckBox ( tr ( "Enable false colour" ), this );
  colourise->setToolTip ( tr ( "Colourise the preview for mono cameras" ));
  colourise->setChecked ( config.colourise );
  connect ( colourise, SIGNAL( stateChanged ( int )), this,
      SLOT( setColouriseMode ( int )));

  if ( !config.dockableControls ) {
    setTitle ( tr ( "Camera" ));
  }

  box1 = box2 = 0;
  hbox = 0;
  box1 = new QVBoxLayout();
  box1->addWidget ( preview );
  box1->addWidget ( nightMode );
  box1->addWidget ( sixteenBit );

  if ( config.controlsOnRight ) {
    hbox = new QHBoxLayout();
    box2 = new QVBoxLayout();
    box2->addWidget ( binning2x2 );
    box2->addWidget ( rawMode );
    box2->addWidget ( colourise );
    hbox->addLayout ( box1 );
    hbox->addLayout ( box2 );
    setLayout ( hbox );
  } else {
    box1->addWidget ( binning2x2 );
    box1->addWidget ( rawMode );
    box1->addWidget ( colourise );
    setLayout ( box1 );
  }
}


CameraWidget::~CameraWidget()
{
  if ( box1 ) {
    state.mainWindow->destroyLayout (( QLayout* ) box1 );
  }
  if ( box2 ) {
    state.mainWindow->destroyLayout (( QLayout* ) box2 );
  }
  if ( hbox ) {
    state.mainWindow->destroyLayout (( QLayout* ) hbox );
  }
}


void
CameraWidget::configure ( void )
{
  sixteenBit->setEnabled ( state.camera->has16Bit() ? 1 : 0 );
  binning2x2->setEnabled ( state.camera->hasBinning ( 2 ) ? 1 : 0 );
  rawMode->setEnabled (( state.camera->hasRawMode() &&
      state.camera->hasDemosaicMode() && state.camera->isColour()) ? 1 : 0 );
  colourise->setEnabled (( OA_ISGREYSCALE(
      state.camera->videoFramePixelFormat ( 0 ))));
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
    state.captureWidget->enableFITSCapture (( !OA_ISBAYER( format ) ||
        ( OA_ISBAYER8( format ) && config.demosaic &&
        config.demosaicOutput )) ? 1 : 0 );
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
    state.captureWidget->enableFITSCapture (( !OA_ISBAYER( format ) ||
        ( OA_ISBAYER8( format ) && config.demosaic &&
        config.demosaicOutput )) ? 1 : 0 );
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
  if ( colourise->isEnabled()) {
    colourise->setChecked ( config.colourise );
  }
}


void
CameraWidget::setColouriseMode ( int state )
{
  config.colourise = state;
  SET_PROFILE_CONFIG( colourise, config.colourise );
}
