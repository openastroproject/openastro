/*****************************************************************************
 *
 * settingsWidget.cc -- the main settings widget wrapper class
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

#include "version.h"
#include "configuration.h"
#include "state.h"
#include "mainWindow.h"
#include "settingsWidget.h"
#include "generalSettings.h"
#include "captureSettings.h"
#ifdef OACAPTURE
#include "cameraSettings.h"
#endif
#include "profileSettings.h"
#include "filterSettings.h"
#ifdef OACAPTURE
#include "autorunSettings.h"
#include "histogramSettings.h"
#include "timerSettings.h"
#endif
#include "demosaicSettings.h"
#include "fitsSettings.h"


SettingsWidget::SettingsWidget()
{
  setWindowTitle( APPLICATION_NAME + tr ( " Settings" ));
  setWindowIcon ( QIcon ( ":/qt-icons/configure-3.png" ));

  general = new GeneralSettings ( this );
  capture = new CaptureSettings ( this );
#ifdef OACAPTURE
  cameras = new CameraSettings ( this );
#endif
  profiles = new ProfileSettings ( this );
  filters = new FilterSettings ( this );
#ifdef OACAPTURE
  autorun = new AutorunSettings ( this );
  histogram = new HistogramSettings ( this );
  timer = new TimerSettings ( this );
#endif
  demosaic = new DemosaicSettings ( this );
  fits = new FITSSettings ( this );
  vbox = new QVBoxLayout ( this );
  tabSet = new QTabWidget ( this );
  buttonBox = new QHBoxLayout();
  cancelButton = new QPushButton ( tr ( "Close" ), this );
  saveButton = new QPushButton ( tr ( "Save" ), this );
  haveUnsavedData = 0;
  saveButton->setEnabled ( 0 );

  state.generalSettingsIndex = tabSet->addTab ( general, 
      QIcon ( ":/qt-icons/cog.png" ), tr ( "General" ));
  state.captureSettingsIndex = tabSet->addTab ( capture,
      QIcon ( ":/qt-icons/capture.png" ), tr ( "Capture" ));
#ifdef OACAPTURE
  state.cameraSettingsIndex = tabSet->addTab ( cameras,
      QIcon ( ":/qt-icons/planetary-camera.png" ), tr ( "Camera" ));
#endif
  state.profileSettingsIndex = tabSet->addTab ( profiles,
      QIcon ( ":/qt-icons/jupiter.png" ), tr ( "Profiles" ));
  state.filterSettingsIndex = tabSet->addTab ( filters,
      QIcon ( ":/qt-icons/filter-wheel.png" ), tr ( "Filters" ));
#ifdef OACAPTURE
  state.autorunSettingsIndex = tabSet->addTab ( autorun,
      QIcon ( ":/qt-icons/clicknrun.png" ), tr ( "Autorun" ));
  state.histogramSettingsIndex = tabSet->addTab ( histogram,
      QIcon ( ":/qt-icons/barchart.png" ), tr ( "Histogram" ));
#endif
  state.demosaicSettingsIndex = tabSet->addTab ( demosaic,
      QIcon ( ":/qt-icons/mosaic.png" ), tr ( "Demosaic" ));
  state.fitsSettingsIndex = tabSet->addTab ( fits,
      QIcon ( ":/qt-icons/fits.png" ), tr ( "FITS/SER Metadata" ));
#ifdef OACAPTURE
  state.timerSettingsIndex = tabSet->addTab ( timer,
      QIcon ( ":/qt-icons/timer.png" ), tr ( "Timer" ));
#endif

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
  cameras->configure();
#endif
}


void
SettingsWidget::storeSettings ( void )
{
  general->storeSettings();
  capture->storeSettings();
#ifdef OACAPTURE
  cameras->storeSettings();
#endif
  profiles->storeSettings();
  filters->storeSettings();
#ifdef OACAPTURE
  autorun->storeSettings();
  histogram->storeSettings();
  timer->storeSettings();
#endif
  demosaic->storeSettings();
  fits->storeSettings();
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


#ifdef OACAPTURE
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
#endif


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

#ifdef OACAPTURE
void
SettingsWidget::updateFrameRate ( int index )
{
  cameras->updateFrameRate ( index );
}
#endif
