/*****************************************************************************
 *
 * profileSettings.cc -- class for profile settings tab in the settings UI
 *
 * Copyright 2013,2014,2018 James Fidell (james@openastroproject.org)
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

#include "commonConfig.h"
#include "captureSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "cameraSettings.h"
#include "targets.h"

// This is global.  All applications using this code share it.

profileConfig profileConf;

ProfileSettings::ProfileSettings ( QWidget* parent, trampolineFuncs* redirs ) :
		QWidget ( parent ), trampolines ( redirs ), parentWidget ( parent )
{
  firstTime = 1;

  list = new QListWidget ( this );
  if ( profileConf.numProfiles ) {
    for ( int i = 0; i < profileConf.numProfiles; i++ ) {
      list->addItem ( profileConf.profiles[i].profileName );
      QListWidgetItem* entry = list->item ( i );
      entry->setFlags ( entry->flags() | Qt :: ItemIsEditable );
    }
  }

  targetLabel = new QLabel ( tr ( "Target: " ), this );
  targetMenu = new QComboBox ( this );
  for ( int i = 0; i < NUM_TARGETS; i++ ) {
    if ( i != TGT_EARTH ) {
      QVariant v(i);
      targetMenu->addItem ( targetName ( i ), v );
    }
  }
  targetMenu->setEnabled ( 0 );
  connect ( targetMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( targetChanged ( int )));
  addButton = new QPushButton ( QIcon ( ":/qt-icons/list-add-4.png" ),
      tr ( "Add Profile" ));
  addButton->setToolTip (
      tr ( "Create a new profile with the current settings" ));
  addButton->setStyleSheet("Text-align:left");
  connect ( addButton, SIGNAL ( clicked()), this, SLOT ( addEntry()));
  connect ( addButton, SIGNAL ( clicked()), parent, SLOT ( dataChanged()));
  removeButton = new QPushButton ( QIcon ( ":/qt-icons/list-remove-4.png" ),
      tr ( "Remove Profile" ));
  if ( 1 == profileConf.numProfiles ) {
    removeButton->setEnabled ( 0 );
  }
  removeButton->setStyleSheet("Text-align:left");
  connect ( removeButton, SIGNAL ( clicked()), this, SLOT ( removeEntry()));
  connect ( removeButton, SIGNAL ( clicked()), parent, SLOT ( dataChanged()));
  connect ( list, SIGNAL ( currentItemChanged ( QListWidgetItem*,
      QListWidgetItem* )), this, SLOT ( focusChanged ( QListWidgetItem*,
      QListWidgetItem* )));
  connect ( list, SIGNAL ( itemClicked ( QListWidgetItem* )), this,
      SLOT ( listEntered ( QListWidgetItem* )));
  connect ( list, SIGNAL ( itemChanged ( QListWidgetItem* )), this,
      SLOT ( itemChanged ( QListWidgetItem* )));
  connect ( list, SIGNAL ( itemChanged ( QListWidgetItem* )), parent,
      SLOT ( dataChanged()));

  hboxTarget = new QHBoxLayout();
  hboxTarget->addWidget ( targetLabel );
  hboxTarget->addWidget ( targetMenu );
  hboxTarget->addStretch ( 3 );

  vbox = new QVBoxLayout();
  vbox->addStretch ( 3 );
  vbox->addLayout ( hboxTarget );
  vbox->addWidget ( addButton );
  vbox->addWidget ( removeButton );
  vbox->addStretch ( 3 );

  hbox = new QHBoxLayout ( this );
  hbox->addWidget ( list );
  hbox->addLayout ( vbox );
  hbox->addStretch ( 1 );
  setLayout ( hbox );
  listAltered = 0;

  // we work on a copy of the profile list to avoid messing up the live
  // one
  if ( profileConf.numProfiles ) {
    for ( int i = 0; i < profileConf.numProfiles; i++ ) {
      changedProfiles.append ( profileConf.profiles.at ( i ));
    }
  }
}


ProfileSettings::~ProfileSettings()
{
  trampolines->destroyLayout (( QLayout* ) hbox );
}


void
ProfileSettings::storeSettings ( void )
{
  if ( listAltered ) {
    if ( profileConf.numProfiles ) {
      profileConf.profiles.clear();
    }
    profileConf.numProfiles = list->count();
    profileConf.profiles = changedProfiles;
    trampolines->reloadProfiles();
  }
}


void
ProfileSettings::addEntry ( void )
{
  PROFILE p;

  int nameIndex = 1;
  int found = 0;
  int numProfiles = changedProfiles.size();
  QString newName;
  do {
    newName = "new profile";
    if ( nameIndex > 1 ) {
      newName += QString ( " %1" ).arg ( nameIndex );
    }
    found = 0;
    for ( int i = 0; !found && i < numProfiles; i++ ) {
      if ( !QString::compare ( newName, changedProfiles[ i ].profileName,
          Qt::CaseInsensitive )) {
        found = 1;
        nameIndex++;
      }
    }
  } while ( found );

  p.profileName = newName;
  p.binning2x2 = commonConfig.binning2x2;
  p.colourise = commonConfig.colourise;
  p.useROI = commonConfig.useROI;
  p.imageSizeX = commonConfig.imageSizeX;
  p.imageSizeY = commonConfig.imageSizeY;
  for ( int j = 0; j < filterConf.numFilters; j++ ) {
    FILTER_PROFILE fp;
    fp.filterName = filterConf.filters[ j ].filterName;
    for ( int i = 1; i < OA_CAM_CTRL_LAST_P1; i++ ) {
      for ( int j = 0; j < OA_CAM_CTRL_MODIFIERS_P1; j++ ) {
        fp.controls[j][i] = cameraConf.controlValues[ j ][ i ];
      }
    }
    p.filterProfiles.append ( fp );
  }
  p.frameRateNumerator = commonConfig.frameRateNumerator;
  p.frameRateDenominator = commonConfig.frameRateDenominator;
  p.filterOption = commonConfig.filterOption;
  p.fileTypeOption = commonConfig.fileTypeOption;
	p.frameFileNameTemplate = commonConfig.frameFileNameTemplate;
  p.fileNameTemplate = commonConfig.fileNameTemplate;
  p.limitEnabled = commonConfig.limitEnabled;
  p.framesLimitValue = commonConfig.framesLimitValue;
  p.secondsLimitValue = commonConfig.secondsLimitValue;
  ignoreTargetChange = 1;
  targetMenu->setCurrentIndex ( 0 );
  ignoreTargetChange = 0;
  changedProfiles.append ( p );
  QListWidgetItem* entry = new QListWidgetItem ( newName );
  entry->setFlags ( entry->flags() | Qt :: ItemIsEditable );
  list->addItem ( entry );
  list->setCurrentRow ( numProfiles );
  list->editItem ( entry );
  listAltered = 1;
  removeButton->setEnabled ( 1 );
}


void
ProfileSettings::removeEntry ( void )
{
  int position = list->currentRow();
  QListWidgetItem* entry = list->takeItem ( position );
  if ( entry ) {
    delete entry;
    changedProfiles.removeAt ( position );
    listAltered = 1;
    if ( changedProfiles.count() == 1 ) {
      removeButton->setEnabled ( 0 );
    }
  }
}


void
ProfileSettings::focusChanged ( QListWidgetItem* curr, QListWidgetItem* old )
{
  // we get a focus change signal when the list is created.  We really
  // want to ignore that
  if ( firstTime ) {
    firstTime = 0;
  } else {
    if ( old && old->text() == "" ) {
      int position = list->row ( old );
      ( void ) list->takeItem ( position );
      delete old;
      changedProfiles.removeAt ( position );
      listAltered = 1;
    }
    targetMenu->setEnabled ( 1 );
    currentProfile = list->row ( curr );
    ignoreTargetChange = 1;
    targetMenu->setCurrentIndex ( changedProfiles[ currentProfile ].target );
    ignoreTargetChange = 0;
  }
}


void
ProfileSettings::listEntered ( QListWidgetItem* curr )
{
  disconnect ( list, SIGNAL ( itemClicked ( QListWidgetItem* )), 0, 0 );
  focusChanged ( curr, 0 );
}


void
ProfileSettings::itemChanged ( QListWidgetItem* item )
{
  currentProfile = list->row ( item );
  if ( item->text() != changedProfiles[ currentProfile ].profileName ) {
    changedProfiles[ currentProfile ].profileName = item->text();
  }
  listAltered = 1;
}


void
ProfileSettings::targetChanged ( int index )
{
  if ( !ignoreTargetChange ) {
    QVariant v = targetMenu->itemData ( index );
    changedProfiles[ currentProfile ].target = v.toInt();
    listAltered = 1;
		QMetaObject::invokeMethod ( parentWidget, "dataChanged",
        Qt::QueuedConnection );
  }
}
