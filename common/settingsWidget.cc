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

#include "captureSettings.h"
#include "generalSettings.h"
#include "captureSettings.h"
#include "filterSettings.h"
#include "fitsSettings.h"
#include "profileSettings.h"
#include "settingsWidget.h"
#include "timerSettings.h"
#include "cameraSettings.h"
#include "autorunSettings.h"
#include "histogramSettings.h"
#include "demosaicSettings.h"


SettingsWidget::SettingsWidget ( QWidget* parent, QWidget* topWidget,
		QString appName, unsigned int settings, int videoFormats, int demosaicOpts,
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
    general = new GeneralSettings ( this, topWidget, applicationName, 1, 1,
				trampolines );
    commonState.generalSettingsIndex = tabSet->addTab ( general, 
        QIcon ( ":/qt-icons/cog.png" ), tr ( "General" ));
	}
	if ( reqdWindows & SETTINGS_CAPTURE ) {
    capture = new CaptureSettings ( this, videoFormats, trampolines );
    commonState.captureSettingsIndex = tabSet->addTab ( capture,
        QIcon ( ":/qt-icons/capture.png" ), tr ( "Capture" ));
	}
	if ( reqdWindows & SETTINGS_CAMERA ) {
    cameras = new CameraSettings ( this, topWidget, trampolines );
    commonState.cameraSettingsIndex = tabSet->addTab ( cameras,
        QIcon ( ":/qt-icons/planetary-camera.png" ), tr ( "Camera" ));
	}
	if ( reqdWindows & SETTINGS_PROFILE ) {
    profiles = new ProfileSettings ( this, trampolines );
    commonState.profileSettingsIndex = tabSet->addTab ( profiles,
        QIcon ( ":/qt-icons/jupiter.png" ), tr ( "Profiles" ));
	}
	if ( reqdWindows & SETTINGS_FILTER ) {
    filters = new FilterSettings ( this, trampolines );
    commonState.filterSettingsIndex = tabSet->addTab ( filters,
        QIcon ( ":/qt-icons/filter-wheel.png" ), tr ( "Filters" ));
	}
	if ( reqdWindows & SETTINGS_AUTORUN ) {
    autorun = new AutorunSettings ( this, trampolines );
    commonState.autorunSettingsIndex = tabSet->addTab ( autorun,
        QIcon ( ":/qt-icons/clicknrun.png" ), tr ( "Autorun" ));
	}
	if ( reqdWindows & SETTINGS_HISTOGRAM ) {
    histogram = new HistogramSettings ( this, trampolines );
    commonState.histogramSettingsIndex = tabSet->addTab ( histogram,
        QIcon ( ":/qt-icons/barchart.png" ), tr ( "Histogram" ));
	}
	if ( reqdWindows & SETTINGS_DEMOSAIC ) {
    demosaic = new DemosaicSettings ( this, demosaicOpts, trampolines );
    commonState.demosaicSettingsIndex = tabSet->addTab ( demosaic,
        QIcon ( ":/qt-icons/mosaic.png" ), tr ( "Demosaic" ));
	}
	if ( reqdWindows & SETTINGS_FITS ) {
    fits = new FITSSettings ( this, trampolines );
    commonState.fitsSettingsIndex = tabSet->addTab ( fits,
        QIcon ( ":/qt-icons/fits.png" ), tr ( "FITS/SER Metadata" ));
	}
	if ( reqdWindows & SETTINGS_TIMER ) {
    timer = new TimerSettings ( this, applicationName, trampolines );
    commonState.timerSettingsIndex = tabSet->addTab ( timer,
        QIcon ( ":/qt-icons/timer.png" ), tr ( "Timer" ));
	}

  tabSet->setUsesScrollButtons ( false );

  buttonBox->addStretch ( 1 );
  buttonBox->addWidget ( cancelButton );
  buttonBox->addWidget ( saveButton );

  vbox->addWidget ( tabSet );
  vbox->addLayout ( buttonBox );
  setLayout ( vbox );

  connect ( cancelButton, SIGNAL( clicked()), parent,
      SLOT( closeSettingsWindow()));
  connect ( saveButton, SIGNAL( clicked()), this, SLOT( storeSettings()));
}


SettingsWidget::~SettingsWidget()
{
  trampolines.destroyLayout (( QLayout* ) vbox );
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
	if ( reqdWindows & SETTINGS_CAMERA ) {
    cameras->configure();
	}
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
	if ( reqdWindows & SETTINGS_CAMERA ) {
    cameras->storeSettings();
	}
	if ( reqdWindows & SETTINGS_PROFILE ) {
    profiles->storeSettings();
	}
	if ( reqdWindows & SETTINGS_FILTER ) {
    filters->storeSettings();
	}
	if ( reqdWindows & SETTINGS_AUTORUN ) {
    autorun->storeSettings();
	}
	if ( reqdWindows & SETTINGS_HISTOGRAM ) {
    histogram->storeSettings();
	}
	if ( reqdWindows & SETTINGS_DEMOSAIC ) {
    demosaic->storeSettings();
	}
	if ( reqdWindows & SETTINGS_FITS ) {
    fits->storeSettings();
	}
	if ( reqdWindows & SETTINGS_TIMER ) {
    timer->storeSettings();
	}
  trampolines.updateConfig();
  trampolines.showStatusMessage ( tr ( "Changes saved" ));
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
	if ( reqdWindows & SETTINGS_CAMERA ) {
		cameras->configure();
	}
}


void
SettingsWidget::enableFlipX ( int state )
{
	if ( reqdWindows & SETTINGS_CAMERA ) {
		cameras->enableFlipX ( state );
	}
}


void
SettingsWidget::enableFlipY ( int state )
{
	if ( reqdWindows & SETTINGS_CAMERA ) {
		cameras->enableFlipY ( state );
	}
}


void
SettingsWidget::updateControl ( int control, int value )
{
	if ( reqdWindows & SETTINGS_CAMERA ) {
		cameras->updateControl ( control, value );
	}
}


void
SettingsWidget::reconfigureControl ( int control )
{
	if ( reqdWindows & SETTINGS_CAMERA ) {
		cameras->reconfigureControl ( control);
	}
}


void
SettingsWidget::propagateNewSlotName ( int slotIndex, const QString& name )
{
	if ( reqdWindows & SETTINGS_AUTORUN ) {
		autorun->setSlotName ( slotIndex, name );
	}
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
	if ( reqdWindows & SETTINGS_AUTORUN ) {
		if ( autorun ) {
			autorun->setSlotCount ( numSlots );
		}
	}
  if ( filters ) {
    filters->setSlotCount ( numSlots );
  }
}


void
SettingsWidget::updateFrameRate ( int index )
{
	if ( reqdWindows & SETTINGS_CAMERA ) {
		cameras->updateFrameRate ( index );
	}
}


QWidget*
SettingsWidget::getTabset ( void )
{
  return ( QWidget* ) tabSet;
}
