/*****************************************************************************
 *
 * demosaicSettings.cc -- class for the demosaic settings in the settings UI
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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

extern "C" {
#include <openastro/demosaic.h>
}

#include "demosaicSettings.h"

#include "configuration.h"
#include "state.h"

DemosaicSettings::DemosaicSettings ( QWidget* parent ) : QWidget ( parent )
{
  cfaLabel = new QLabel ( tr ( "Bayer format" ));
  cfaButtons = new QButtonGroup ( this );
  rggbButton = new QRadioButton ( tr ( "RGGB" ));
  bggrButton = new QRadioButton ( tr ( "BGGR" ));
  grbgButton = new QRadioButton ( tr ( "GRBG" ));
  gbrgButton = new QRadioButton ( tr ( "GBRG" ));
  autoButton = new QRadioButton ( tr ( "Auto" ));
  rggbButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_RGGB ? 1 : 0 );
  bggrButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_BGGR ? 1 : 0 );
  grbgButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_GRBG ? 1 : 0 );
  gbrgButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_GBRG ? 1 : 0 );
  autoButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_AUTO ? 1 : 0 );
  rggbButton->setIcon ( QIcon ( ":/icons/RGGB.png" ));
  bggrButton->setIcon ( QIcon ( ":/icons/BGGR.png" ));
  grbgButton->setIcon ( QIcon ( ":/icons/GRBG.png" ));
  gbrgButton->setIcon ( QIcon ( ":/icons/GBRG.png" ));

  cfaButtons->addButton ( rggbButton );
  cfaButtons->addButton ( bggrButton );
  cfaButtons->addButton ( grbgButton );
  cfaButtons->addButton ( gbrgButton );
  cfaButtons->addButton ( autoButton );

  methodLabel = new QLabel ( tr ( "Demosaic method" ));
  methodButtons = new QButtonGroup ( this );
  nnButton = new QRadioButton ( tr ( "Nearest Neighbour" ));
  bilinearButton = new QRadioButton ( tr ( "Bilinear" ));
  smoothHueButton = new QRadioButton ( tr ( "Smooth Hue" ));
  vngButton = new QRadioButton ( tr ( "VNG" ));
  nnButton->setChecked ( config.demosaicMethod ==
      OA_DEMOSAIC_NEAREST_NEIGHBOUR ? 1 : 0 );
  bilinearButton->setChecked ( config.demosaicMethod ==
      OA_DEMOSAIC_BILINEAR ? 1 : 0 );
  smoothHueButton->setChecked ( config.demosaicMethod ==
      OA_DEMOSAIC_SMOOTH_HUE ? 1 : 0 );
  vngButton->setChecked ( config.demosaicMethod ==
      OA_DEMOSAIC_VNG ? 1 : 0 );

  methodButtons->addButton ( nnButton );
  methodButtons->addButton ( bilinearButton );
  methodButtons->addButton ( smoothHueButton );
  methodButtons->addButton ( vngButton );

  box = new QVBoxLayout ( this );
  box->addWidget ( cfaLabel );
  box->addWidget ( rggbButton );
  box->addWidget ( bggrButton );
  box->addWidget ( grbgButton );
  box->addWidget ( gbrgButton );
  box->addWidget ( autoButton );
  box->addSpacing ( 15 );
  box->addWidget ( methodLabel );
  box->addWidget ( nnButton );
  box->addWidget ( bilinearButton );
  box->addWidget ( smoothHueButton );
  box->addWidget ( vngButton );
  box->addStretch ( 1 );
  setLayout ( box );
  connect ( cfaButtons, SIGNAL ( buttonClicked ( int )), parent,
      SLOT ( dataChanged()));
  connect ( methodButtons, SIGNAL ( buttonClicked ( int )), parent,
      SLOT ( dataChanged()));
}


DemosaicSettings::~DemosaicSettings()
{
  state.mainWindow->destroyLayout (( QLayout* ) box );
}


void
DemosaicSettings::storeSettings ( void )
{
  if ( rggbButton->isChecked()) {
    config.cfaPattern = OA_DEMOSAIC_RGGB;
  }
  if ( bggrButton->isChecked()) {
    config.cfaPattern = OA_DEMOSAIC_BGGR;
  }
  if ( grbgButton->isChecked()) {
    config.cfaPattern = OA_DEMOSAIC_GRBG;
  }
  if ( gbrgButton->isChecked()) {
    config.cfaPattern = OA_DEMOSAIC_GBRG;
  }
  if ( autoButton->isChecked()) {
    config.cfaPattern = OA_DEMOSAIC_AUTO;
  }
  if ( nnButton->isChecked()) {
    config.demosaicMethod = OA_DEMOSAIC_NEAREST_NEIGHBOUR;
  }
  if ( bilinearButton->isChecked()) {
    config.demosaicMethod = OA_DEMOSAIC_BILINEAR;
  }
  if ( smoothHueButton->isChecked()) {
    config.demosaicMethod = OA_DEMOSAIC_SMOOTH_HUE;
  }
  if ( vngButton->isChecked()) {
    config.demosaicMethod = OA_DEMOSAIC_VNG;
  }
  if ( state.camera->isInitialised()) {
qWarning() << "Need to enable suitable output formats";
/*
    int format = state.camera->videoFramePixelFormat ( 0 );
    state.captureWidget->enableFITSCapture ( OA_ISGREYSCALE( format ) ? 1 : 0 );
    state.captureWidget->enableMOVCapture ( QUICKTIME_OK( format ) ? 1 : 0 );
*/
  }
}


void
DemosaicSettings::updateCFASetting ( void )
{
  rggbButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_RGGB ? 1 : 0 );
  bggrButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_BGGR ? 1 : 0 );
  grbgButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_GRBG ? 1 : 0 );
  gbrgButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_GBRG ? 1 : 0 );
  autoButton->setChecked ( config.cfaPattern == OA_DEMOSAIC_AUTO ? 1 : 0 );
}
