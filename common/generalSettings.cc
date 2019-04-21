/*****************************************************************************
 *
 * generalSettings.cc -- class for general settings in the settings UI
 *
 * Copyright 2013,2014,2015,2017,2018
 *   James Fidell (james@openastroproject.org)
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

#include "trampoline.h"
#include "captureSettings.h"
#include "generalSettings.h"
#include "fitsSettings.h"

// This is global.  All applications using this code share it.

generalConfig generalConf;

GeneralSettings::GeneralSettings ( QWidget* parent, QWidget* top,
		QString appName, int split, int fps, trampolineFuncs* redirs ) :
		QWidget ( parent ), topWidget ( top ), applicationName ( appName ),
		splitControls ( split ), fpsControls ( fps ), trampolines ( redirs ),
		parentWidget ( parent )
{
#ifdef SAVE_OPTION
  saveBox = new QCheckBox ( tr ( "Load and save settings automatically" ),
      this );
  saveBox->setChecked ( generalConf.saveSettings );
#endif

  reticleButtons = new QButtonGroup ( this );
  circleButton = new QRadioButton ( tr ( "Use circular reticle" ));
  crossButton = new QRadioButton ( tr ( "Use cross reticle" ));
  tramlineButton = new QRadioButton ( tr ( "Use tramline reticle" ));
  circleButton->setChecked ( generalConf.reticleStyle ==
			RETICLE_CIRCLE ? 1 : 0 );
  crossButton->setChecked ( generalConf.reticleStyle ==
			RETICLE_CROSS ? 1 : 0 );
  tramlineButton->setChecked ( generalConf.reticleStyle ==
			RETICLE_TRAMLINES ?
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
  degCButton->setChecked ( generalConf.tempsInC );
  degFButton->setChecked ( !generalConf.tempsInC );
  tempButtons->addButton ( degCButton );
  tempButtons->addButton ( degFButton );

  saveCaptureSettings = new QCheckBox ( tr (
      "Write capture settings to file" ));
  saveCaptureSettings->setChecked ( generalConf.saveCaptureSettings );

  connectSole = new QCheckBox ( tr (
      "Connect single camera on startup" ));
  connectSole->setChecked ( generalConf.connectSoleCamera );

  if ( splitControls ) {
    dockable = new QCheckBox ( tr ( "Make controls dockable" ));
    dockable->setChecked ( generalConf.dockableControls );

    controlPosn = new QCheckBox ( tr ( "Display controls on right" ));
    controlPosn->setChecked ( generalConf.controlsOnRight );

    separateControls = new QCheckBox ( tr (
				"Use separate window for controls" ));
    separateControls->setChecked ( generalConf.separateControls );
  }
	if ( fpsControls ) {
    fpsLabel = new QLabel ( tr ( "Display FPS" ), this );
    fpsCountLabel = new QLabel ( QString::number ( generalConf.displayFPS ),
				this );
    fpsSlider = new QSlider ( Qt::Horizontal, this );
    fpsSlider->setRange ( 1, 30 );
    fpsSlider->setSingleStep ( 1 );
    fpsSlider->setValue ( generalConf.displayFPS );
    fpsSlider->setTickPosition ( QSlider::TicksBelow );
    fpsSlider->setTickInterval ( 1 );

    fpsbox = new QHBoxLayout();
    fpsbox->addWidget ( fpsLabel );
    fpsbox->addWidget ( fpsCountLabel );
    fpsbox->addWidget ( fpsSlider );
  }

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
	if ( splitControls ) {
    rightBox->addSpacing ( 15 );
    rightBox->addWidget ( dockable );
    rightBox->addWidget ( controlPosn );
    rightBox->addWidget ( separateControls );
    rightBox->addStretch ( 1 );
	}

  topBox->addLayout ( leftBox );
  topBox->addLayout ( rightBox );
  topBox->addStretch ( 1 );
  box->addLayout ( topBox );
	if ( fpsControls ) {
    box->addLayout ( fpsbox );
	}
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
	if ( splitControls ) {
    connect ( dockable, SIGNAL ( stateChanged ( int )), this,
        SLOT ( showRestartWarning()));
    connect ( controlPosn, SIGNAL ( stateChanged ( int )), this,
        SLOT ( showRestartWarning()));
    connect ( separateControls, SIGNAL ( stateChanged ( int )), this,
        SLOT ( showRestartWarning()));
	}
	if ( fpsControls ) {
    connect ( fpsSlider, SIGNAL ( valueChanged ( int )), parent,
        SLOT ( dataChanged()));
    connect ( fpsSlider, SIGNAL ( valueChanged ( int )), this,
        SLOT ( updateFPSLabel ( int )));
	}
	  connect ( recentreButton, SIGNAL ( clicked()), commonState.viewerWidget,
      SLOT ( recentreReticle()));
  connect ( derotateButton, SIGNAL ( clicked()), commonState.viewerWidget,
      SLOT ( derotateReticle()));
}


GeneralSettings::~GeneralSettings()
{
  trampolines->destroyLayout (( QLayout* ) box );
}


void
GeneralSettings::storeSettings ( void )
{
#ifdef SAVE_OPTION
  generalConf.saveSettings = saveBox->isChecked() ? 1 : 0;
#else
  generalConf.saveSettings = 1;
#endif
  if ( circleButton->isChecked()) {
    generalConf.reticleStyle = RETICLE_CIRCLE;
  }
  if ( crossButton->isChecked()) {
    generalConf.reticleStyle = RETICLE_CROSS;
  }
  if ( tramlineButton->isChecked()) {
    generalConf.reticleStyle = RETICLE_TRAMLINES;
  }
  generalConf.tempsInC = degCButton->isChecked();
  trampolines->resetTemperatureLabel();
  generalConf.saveCaptureSettings = saveCaptureSettings->isChecked() ? 1 : 0;
  generalConf.connectSoleCamera = connectSole->isChecked() ? 1 : 0;

	if ( fpsControls ) {
    generalConf.displayFPS = fpsSlider->value();
    trampolines->setDisplayFPS ( generalConf.displayFPS );
	}
  if ( splitControls ) {
    generalConf.dockableControls = dockable->isChecked() ? 1 : 0;
    generalConf.controlsOnRight = controlPosn->isChecked() ? 1 : 0;
    generalConf.separateControls = separateControls->isChecked() ? 1 : 0;
	}
}


void
GeneralSettings::updateFPSLabel ( int value )
{
  fpsCountLabel->setText ( QString::number ( value ));
}


void
GeneralSettings::showRestartWarning ( void )
{
  QString msg1, msg2;

  msg1 = tr ( "The application must be restarted for this change to take "
       "effect." );
  msg2 = "";
  if (( dockable->isChecked() || controlPosn->isChecked()) &&
      separateControls->isChecked()) {
    msg2 = tr ( "\n\nPlacing the controls in a separate window overrides "
        "selections to make the controls dockable or appear on the right." );
  }

  QMessageBox::warning ( topWidget, applicationName, msg1 + msg2 );
	QMetaObject::invokeMethod ( parentWidget, "dataChanged",
			Qt::QueuedConnection );
}
