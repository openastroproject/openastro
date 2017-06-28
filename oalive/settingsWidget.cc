/*****************************************************************************
 *
 * settingsWidget.cc -- the main settings widget wrapper class
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

#include <QtGui>

#include "version.h"
#include "configuration.h"
#include "state.h"
#include "mainWindow.h"
#include "settingsWidget.h"
#include "generalSettings.h"
#include "captureSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "demosaicSettings.h"


SettingsWidget::SettingsWidget()
{
  setWindowTitle( APPLICATION_NAME + tr ( " Settings" ));
  setWindowIcon ( QIcon ( ":/icons/configure-3.png" ));

  general = new GeneralSettings ( this );
  capture = new CaptureSettings ( this );
  profiles = new ProfileSettings ( this );
  filters = new FilterSettings ( this );
  demosaic = new DemosaicSettings ( this );
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
  state.profileSettingsIndex = tabSet->addTab ( profiles,
      QIcon ( ":/icons/jupiter.png" ), tr ( "Profiles" ));
  state.filterSettingsIndex = tabSet->addTab ( filters,
      QIcon ( ":/icons/filter-wheel.png" ), tr ( "Filters" ));
  state.demosaicSettingsIndex = tabSet->addTab ( demosaic,
      QIcon ( ":/icons/mosaic.png" ), tr ( "Demosaic" ));

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
}


void
SettingsWidget::storeSettings ( void )
{
  general->storeSettings();
  profiles->storeSettings();
  filters->storeSettings();
  demosaic->storeSettings();
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
SettingsWidget::setSlotCount ( int numSlots )
{
  if ( filters ) {
    filters->setSlotCount ( numSlots );
  }
}
