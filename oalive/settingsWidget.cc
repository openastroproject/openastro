/*****************************************************************************
 *
 * settingsWidget.cc -- the main settings widget wrapper class
 *
 * Copyright 2013,2014,2015,2017,2018
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

#include "generalSettings.h"
#include "captureSettings.h"
#include "filterSettings.h"
#include "fitsSettings.h"

#include "configuration.h"
#include "state.h"
#include "mainWindow.h"
#include "profileSettings.h"
#include "settingsWidget.h"
#include "timerSettings.h"
#ifdef OACAPTURE
#include "cameraSettings.h"
#include "autorunSettings.h"
#include "histogramSettings.h"
#endif
#include "demosaicSettings.h"


SettingsWidget::SettingsWidget ( QWidget* topWidget, QString appName,
		unsigned int settings, int videoFormats, int demosaicOpts,
		trampolineFuncs* trampolines )
{
	applicationName = appName;
	reqdWindows = settings;

  setWindowTitle( applicationName + tr ( " Settings" ));
  setWindowIcon ( QIcon ( ":/qt-icons/configure-3.png" ));

  vbox = new QVBoxLayout ( this );
  tabSet = new QTabWidget ( this );
  buttonBox = new QHBoxLayout();
  cancelButton = new QPushButton ( tr ( "Close" ), this );
  saveButton = new QPushButton ( tr ( "Save" ), this );
  haveUnsavedData = 0;
  saveButton->setEnabled ( 0 );

	if ( reqdWindows & SETTINGS_GENERAL ) {
    general = new GeneralSettings ( this, topWidget, state.viewWidget,
				&generalConf, applicationName, 1, 1, trampolines );
    state.generalSettingsIndex = tabSet->addTab ( general, 
        QIcon ( ":/qt-icons/cog.png" ), tr ( "General" ));
	}
	if ( reqdWindows & SETTINGS_CAPTURE ) {
    capture = new CaptureSettings ( this, videoFormats, trampolines );
    state.captureSettingsIndex = tabSet->addTab ( capture,
        QIcon ( ":/qt-icons/capture.png" ), tr ( "Capture" ));
	}
#ifdef OACAPTURE
	if ( reqdWindows & SETTINGS_CAMERA ) {
    cameras = new CameraSettings ( this, trampolines );
    state.cameraSettingsIndex = tabSet->addTab ( cameras,
        QIcon ( ":/qt-icons/planetary-camera.png" ), tr ( "Camera" ));
	}
#endif
	if ( reqdWindows & SETTINGS_PROFILE ) {
    profiles = new ProfileSettings ( this, trampolines );
    state.profileSettingsIndex = tabSet->addTab ( profiles,
        QIcon ( ":/qt-icons/jupiter.png" ), tr ( "Profiles" ));
	}
	if ( reqdWindows & SETTINGS_FILTER ) {
    filters = new FilterSettings ( this, &filterConf, trampolines );
    state.filterSettingsIndex = tabSet->addTab ( filters,
        QIcon ( ":/qt-icons/filter-wheel.png" ), tr ( "Filters" ));
	}
#ifdef OACAPTURE
	if ( reqdWindows & SETTINGS_AUTORUN ) {
    autorun = new AutorunSettings ( this, trampolines );
    state.autorunSettingsIndex = tabSet->addTab ( autorun,
        QIcon ( ":/qt-icons/clicknrun.png" ), tr ( "Autorun" ));
	}
#endif
#ifdef OACAPTURE
	if ( reqdWindows & SETTINGS_HISTOGRAM ) {
    histogram = new HistogramSettings ( this, trampolines );
    state.histogramSettingsIndex = tabSet->addTab ( histogram,
        QIcon ( ":/qt-icons/barchart.png" ), tr ( "Histogram" ));
	}
#endif
	if ( reqdWindows & SETTINGS_DEMOSAIC ) {
    demosaic = new DemosaicSettings ( this, demosaicOpts, trampolines );
    state.demosaicSettingsIndex = tabSet->addTab ( demosaic,
        QIcon ( ":/qt-icons/mosaic.png" ), tr ( "Demosaic" ));
	}
	if ( reqdWindows & SETTINGS_FITS ) {
    fits = new FITSSettings ( this, trampolines );
    state.fitsSettingsIndex = tabSet->addTab ( fits,
        QIcon ( ":/qt-icons/fits.png" ), tr ( "FITS/SER Metadata" ));
	}
	if ( reqdWindows & SETTINGS_TIMER ) {
    timer = new TimerSettings ( this, applicationName, trampolines );
    state.timerSettingsIndex = tabSet->addTab ( timer,
        QIcon ( ":/qt-icons/timer.png" ), tr ( "Timer" ));
	}

  tabSet->setUsesScrollButtons ( false );

  buttonBox->addStretch ( 1 );
  buttonBox->addWidget ( cancelButton );
  buttonBox->addWidget ( saveButton );

  vbox->addWidget ( tabSet );
  vbox->addLayout ( buttonBox );
  setLayout ( vbox );

  connect ( cancelButton, SIGNAL( clicked()), state.mainWindow,
      SLOT( closeSettingsWindow()));
  connect ( saveButton, SIGNAL( clicked()), this, SLOT( storeSettings()));
}


SettingsWidget::~SettingsWidget()
{
  state.mainWindow->destroyLayout (( QLayout* ) vbox );
}


void
SettingsWidget::setActiveTab ( int index )
{
  tabSet->setCurrentIndex ( index );
}


void
SettingsWidget::enableTab ( int index, int state )
{
  tabSet->setTabEnabled ( index, state );
#ifdef OACAPTURE
	if ( reqdWindows & SETTINGS_CAMERA ) {
    cameras->configure();
	}
#endif
}


void
SettingsWidget::storeSettings ( void )
{
	if ( reqdWindows & SETTINGS_GENERAL ) {
    general->storeSettings();
	}
	if ( reqdWindows & SETTINGS_CAPTURE ) {
    capture->storeSettings();
	}
#ifdef OACAPTURE
	if ( reqdWindows & SETTINGS_CAMERA ) {
    cameras->storeSettings();
	}
#endif
	if ( reqdWindows & SETTINGS_PROFILE ) {
    profiles->storeSettings();
	}
	if ( reqdWindows & SETTINGS_FILTER ) {
    filters->storeSettings();
	}
#ifdef OACAPTURE
	if ( reqdWindows & SETTINGS_AUTORUN ) {
    autorun->storeSettings();
	}
#endif
#ifdef OACAPTURE
	if ( reqdWindows & SETTINGS_HISTOGRAM ) {
    histogram->storeSettings();
	}
#endif
	if ( reqdWindows & SETTINGS_DEMOSAIC ) {
    demosaic->storeSettings();
	}
	if ( reqdWindows & SETTINGS_FITS ) {
    fits->storeSettings();
	}
	if ( reqdWindows & SETTINGS_TIMER ) {
    timer->storeSettings();
	}
  state.mainWindow->updateConfig();
  state.mainWindow->showStatusMessage ( tr ( "Changes saved" ));
  cancelButton->setText ( tr ( "Close" ));
  haveUnsavedData = 0;
  saveButton->setEnabled ( 0 );
}


void
SettingsWidget::dataChanged ( void )
{
  haveUnsavedData = 1;
  cancelButton->setText ( tr ( "Cancel" ));
  saveButton->setEnabled ( 1 );
}


void
SettingsWidget::updateCFASetting ( void )
{
  demosaic->updateCFASetting();
}


void
SettingsWidget::configureCameraSettings ( void )
{
#ifdef OACAPTURE
  cameras->configure();
#endif
}


void
SettingsWidget::enableFlipX ( int state )
{
#ifdef OACAPTURE
  cameras->enableFlipX ( state );
#endif
}


void
SettingsWidget::enableFlipY ( int state )
{
#ifdef OACAPTURE
  cameras->enableFlipY ( state );
#endif
}


void
SettingsWidget::updateControl ( int control, int value )
{
#ifdef OACAPTURE
  cameras->updateControl ( control, value );
#endif
}


void
SettingsWidget::reconfigureControl ( int control )
{
#ifdef OACAPTURE
  cameras->reconfigureControl ( control);
#endif
}


void
SettingsWidget::propagateNewSlotName ( int slotIndex, const QString& name )
{
#ifdef OACAPTURE
  autorun->setSlotName ( slotIndex, name );
#endif
}


QString
SettingsWidget::getSlotFilterName ( int slotIndex )
{
  if ( !filters ) {
    qWarning() << __FUNCTION__ << ": filters not set";
    return "";
  }
  return filters->getSlotFilterName ( slotIndex );
}


void
SettingsWidget::setSlotCount ( int numSlots )
{
#ifdef OACAPTURE
  if ( autorun ) {
    autorun->setSlotCount ( numSlots );
  }
#endif
  if ( filters ) {
    filters->setSlotCount ( numSlots );
  }
}


void
SettingsWidget::updateFrameRate ( int index )
{
#ifdef OACAPTURE
  cameras->updateFrameRate ( index );
#endif
}


QWidget*
SettingsWidget::getTabset ( void )
{
  return ( QWidget* ) tabSet;
}
