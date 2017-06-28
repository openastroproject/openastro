/*****************************************************************************
 *
 * generalSettings.cc -- class for general settings in the settings UI
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

#include "generalSettings.h"

#include "configuration.h"
#include "state.h"


GeneralSettings::GeneralSettings ( QWidget* parent ) : QWidget ( parent )
{
#ifdef SAVE_OPTION
  saveBox = new QCheckBox ( tr ( "Load and save settings automatically" ),
      this );
  saveBox->setChecked ( config.saveSettings );
#endif

  reticleButtons = new QButtonGroup ( this );
  circleButton = new QRadioButton ( tr ( "Use circular reticle" ));
  crossButton = new QRadioButton ( tr ( "Use cross reticle" ));
  tramlineButton = new QRadioButton ( tr ( "Use tramline reticle" ));
  circleButton->setChecked ( config.reticleStyle == RETICLE_CIRCLE ? 1 : 0 );
  crossButton->setChecked ( config.reticleStyle == RETICLE_CROSS ? 1 : 0 );
  tramlineButton->setChecked ( config.reticleStyle == RETICLE_TRAMLINES ?
      1 : 0 );
  reticleButtons->addButton ( circleButton );
  reticleButtons->addButton ( crossButton );
  reticleButtons->addButton ( tramlineButton );

  recentreButton = new QPushButton ( tr ( "Recentre Reticle" ));
  derotateButton = new QPushButton ( tr ( "Derotate Reticle" ));
  reticlebox = new QHBoxLayout();
  reticlebox->addWidget ( recentreButton );
  reticlebox->addWidget ( derotateButton );
  reticlebox->addStretch ( 1 );

  tempButtons = new QButtonGroup ( this );
  degCButton = new QRadioButton ( tr ( "Display temperatures in degrees C" ));
  degFButton = new QRadioButton ( tr ( "Display temperatures in degrees F" ));
  degCButton->setChecked ( config.tempsInC );
  degFButton->setChecked ( !config.tempsInC );
  tempButtons->addButton ( degCButton );
  tempButtons->addButton ( degFButton );

  saveCaptureSettings = new QCheckBox ( tr (
      "Write capture settings to file" ));
  saveCaptureSettings->setChecked ( config.saveCaptureSettings );

  connectSole = new QCheckBox ( tr (
      "Connect single camera on startup" ));
  connectSole->setChecked ( config.connectSoleCamera );

  box = new QVBoxLayout ( this );
  topBox = new QHBoxLayout();
  leftBox = new QVBoxLayout();
  rightBox = new QVBoxLayout();

#ifdef SAVE_OPTION
  leftBox->addWidget ( saveBox );
  leftBox->addSpacing ( 15 );
#endif
  leftBox->addWidget ( circleButton );
  leftBox->addWidget ( crossButton );
  leftBox->addWidget ( tramlineButton );
  leftBox->addSpacing ( 15 );
  leftBox->addLayout ( reticlebox );
  leftBox->addStretch ( 1 );

  rightBox->addWidget ( degCButton );
  rightBox->addWidget ( degFButton );
  rightBox->addSpacing ( 15 );
  rightBox->addWidget ( saveCaptureSettings );
  rightBox->addSpacing ( 15 );
  rightBox->addWidget ( connectSole );
  rightBox->addStretch ( 1 );

  topBox->addLayout ( leftBox );
  topBox->addLayout ( rightBox );
  topBox->addStretch ( 1 );
  box->addLayout ( topBox );
  box->addStretch ( 1 );
  setLayout ( box );

#ifdef SAVE_OPTION
  connect ( saveBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
#endif
  connect ( reticleButtons, SIGNAL ( buttonClicked ( int )), parent,
      SLOT ( dataChanged()));
  connect ( saveCaptureSettings, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
  connect ( connectSole, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
  connect ( recentreButton, SIGNAL ( clicked()), state.viewWidget,
      SLOT ( recentreReticle()));
  connect ( derotateButton, SIGNAL ( clicked()), state.viewWidget,
      SLOT ( derotateReticle()));
}


GeneralSettings::~GeneralSettings()
{
  state.mainWindow->destroyLayout (( QLayout* ) box );
}


void
GeneralSettings::storeSettings ( void )
{
#ifdef SAVE_OPTION
  config.saveSettings = saveBox->isChecked() ? 1 : 0;
#else
  config.saveSettings = 1;
#endif
  if ( circleButton->isChecked()) {
    config.reticleStyle = RETICLE_CIRCLE;
  }
  if ( crossButton->isChecked()) {
    config.reticleStyle = RETICLE_CROSS;
  }
  if ( tramlineButton->isChecked()) {
    config.reticleStyle = RETICLE_TRAMLINES;
  }
  config.tempsInC = degCButton->isChecked();
  state.mainWindow->resetTemperatureLabel();
  config.saveCaptureSettings = saveCaptureSettings->isChecked() ? 1 : 0;
  config.connectSoleCamera = connectSole->isChecked() ? 1 : 0;
}
