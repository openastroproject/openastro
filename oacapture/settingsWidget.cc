/*****************************************************************************
 *
 * settingsWidget.cc -- the main settings widget wrapper class
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

#include "version.h"
#include "configuration.h"
#include "state.h"
#include "mainWindow.h"
#include "settingsWidget.h"
#include "generalSettings.h"
#include "captureSettings.h"
#include "cameraSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "autorunSettings.h"
#include "histogramSettings.h"
#include "demosaicSettings.h"
#include "fitsSettings.h"
#include "timerSettings.h"


SettingsWidget::SettingsWidget()
{
  setWindowTitle( APPLICATION_NAME + tr ( " Settings" ));
  setWindowIcon ( QIcon ( ":/icons/configure-3.png" ));

  general = new GeneralSettings ( this );
  capture = new CaptureSettings ( this );
  cameras = new CameraSettings ( this );
  profiles = new ProfileSettings ( this );
  filters = new FilterSettings ( this );
  autorun = new AutorunSettings ( this );
  histogram = new HistogramSettings ( this );
  demosaic = new DemosaicSettings ( this );
  fits = new FITSSettings ( this );
  timer = new TimerSettings ( this );
  vbox = new QVBoxLayout ( this );
  tabSet = new QTabWidget ( this );
  buttonBox = new QHBoxLayout();
  cancelButton = new QPushButton ( tr ( "Close" ), this );
  saveButton = new QPushButton ( tr ( "Save" ), this );
  haveUnsavedData = 0;
  saveButton->setEnabled ( 0 );

  state.generalSettingsIndex = tabSet->addTab ( general, 
      QIcon ( ":/icons/cog.png" ), tr ( "General" ));
  state.captureSettingsIndex = tabSet->addTab ( capture,
      QIcon ( ":/icons/capture.png" ), tr ( "Capture" ));
  state.cameraSettingsIndex = tabSet->addTab ( cameras,
      QIcon ( ":/icons/planetary-camera.png" ), tr ( "Camera" ));
  state.profileSettingsIndex = tabSet->addTab ( profiles,
      QIcon ( ":/icons/jupiter.png" ), tr ( "Profiles" ));
  state.filterSettingsIndex = tabSet->addTab ( filters,
      QIcon ( ":/icons/filter-wheel.png" ), tr ( "Filters" ));
  state.autorunSettingsIndex = tabSet->addTab ( autorun,
      QIcon ( ":/icons/clicknrun.png" ), tr ( "Autorun" ));
  state.histogramSettingsIndex = tabSet->addTab ( histogram,
      QIcon ( ":/icons/barchart.png" ), tr ( "Histogram" ));
  state.demosaicSettingsIndex = tabSet->addTab ( demosaic,
      QIcon ( ":/icons/mosaic.png" ), tr ( "Demosaic" ));
  state.fitsSettingsIndex = tabSet->addTab ( fits,
      QIcon ( ":/icons/fits.png" ), tr ( "FITS/SER Metadata" ));
  state.timerSettingsIndex = tabSet->addTab ( timer,
      QIcon ( ":/icons/timer.png" ), tr ( "Timer" ));

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
  cameras->configure();
}


void
SettingsWidget::storeSettings ( void )
{
  general->storeSettings();
  capture->storeSettings();
  cameras->storeSettings();
  profiles->storeSettings();
  filters->storeSettings();
  autorun->storeSettings();
  histogram->storeSettings();
  demosaic->storeSettings();
  fits->storeSettings();
  timer->storeSettings();
  state.mainWindow->writeConfig();
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
  cameras->configure();
}


void
SettingsWidget::enableFlipX ( int state )
{
  cameras->enableFlipX ( state );
}


void
SettingsWidget::enableFlipY ( int state )
{
  cameras->enableFlipY ( state );
}


void
SettingsWidget::updateControl ( int control, int value )
{
  cameras->updateControl ( control, value );
}


void
SettingsWidget::reconfigureControl ( int control )
{
  cameras->reconfigureControl ( control);
}


void
SettingsWidget::propagateNewSlotName ( int slotIndex, const QString& name )
{
  autorun->setSlotName ( slotIndex, name );
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
  if ( autorun ) {
    autorun->setSlotCount ( numSlots );
  }
  if ( filters ) {
    filters->setSlotCount ( numSlots );
  }
}


void
SettingsWidget::updateFrameRate ( int index )
{
  cameras->updateFrameRate ( index );
}
