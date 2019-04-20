/*****************************************************************************
 *
 * autorunSettings.cc -- class for autorun settings in the settings widget
 *
 * Copyright 2013,2014,2018,2019 James Fidell (james@openastroproject.org)
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

#include "commonState.h"
#include "autorunSettings.h"
#include "filterSettings.h"

// This is global.  All applications using this code share it.

autorunConfig autorunConf;

AutorunSettings::AutorunSettings ( QWidget* parent, trampolineFuncs* redirs ) :
		QWidget ( parent ), trampolines ( redirs ), parentWidget ( parent )
{
  filterWheelSlots = 0;
	if ( commonState.filterWheel && commonState.filterWheel->isInitialised()) {
		filterWheelSlots = commonState.filterWheel->numSlots();
	}
  filterMenus.clear();
  filterContainers.clear();
  removeButtons.clear();
  sequenceLength = 0;

  numRunsLabel = new QLabel ( tr ( "Number of runs" ), this );
  numRuns = new QLineEdit ( this );
  numRunsValidator = new QIntValidator ( 1, 99999, this );
  numRuns->setValidator ( numRunsValidator );
  if ( autorunConf.autorunCount ) {
    QString n = QString::number ( autorunConf.autorunCount );
    numRuns->setText ( n );
  }

  delayLabel = new QLabel ( tr ( "Delay between runs" ), this );
  delayLabel2 = new QLabel ( tr ( "(seconds)" ), this );
  delay = new QLineEdit ( this );
  delayValidator = new QIntValidator ( 0, 99999, this );
  delay->setValidator ( delayValidator );
  if ( autorunConf.autorunDelay ) {
    QString n = QString::number ( autorunConf.autorunDelay );
    delay->setText ( n );
  }

  enableButton = new QPushButton ( tr ( "Enable Autorun" ), this );

  buttonGrid = new QGridLayout();
  buttonGrid->addWidget ( numRunsLabel, 0, 0 );
  buttonGrid->addWidget ( numRuns, 0, 1 );
  buttonGrid->addWidget ( delayLabel, 1, 0 );
  buttonGrid->addWidget ( delay, 1, 1 );
  buttonGrid->addWidget ( delayLabel2, 1, 2 );
  buttonGrid->addWidget ( enableButton, 2, 1 );
  buttonGrid->setRowStretch ( 3, 1 );

  addButtonBox = new QHBoxLayout();
  addButtonBox->addLayout ( buttonGrid );
  addButtonBox->addStretch ( 1 );

  titleLabel = new QLabel ( tr ( "Filter Sequence" ));

  addButton = new QPushButton ( QIcon ( ":/qt-icons/list-add-4.png" ),
      tr ( "Extend Sequence" ), this );
  addButton->setStyleSheet("Text-align:left");

  addFilterBox = new QHBoxLayout();
  addFilterBox->addWidget ( addButton );
  addFilterBox->addStretch ( 1 );

  removeButtonMapper = new QSignalMapper ( this );
  connect ( removeButtonMapper, SIGNAL( mapped ( int )), this,
      SLOT( removeFilter ( int )));

  filterGrid = new QGridLayout();
  filterGridBox = new QHBoxLayout();
  filterGridBox->addLayout ( filterGrid );
  filterGridBox->addStretch ( 1 );

  int numExisting = filterConf.autorunFilterSequence.count();
  for ( int i = 0; i < numExisting; i++ ) {
    addFilterWidgets ( 1 );
    filterMenus[i]->setCurrentIndex ( filterConf.autorunFilterSequence[i] );
    connect ( filterMenus[i], SIGNAL ( currentIndexChanged ( int )), parent,
      SLOT ( dataChanged()));
  }

  filterPromptOption = new QCheckBox ( tr (
      "Prompt for filter change with no filter wheel connected" ), this );
  filterPromptBox = new QHBoxLayout();
  filterPromptBox->addWidget ( filterPromptOption );
  filterPromptBox->addStretch ( 1 );
  filterPromptOption->setChecked ( filterConf.promptForFilterChange );

#ifdef INTER_FILTER_DELAY
  filterDelay = new QLineEdit ( this );
  filterDelayValidator = new QIntValidator ( 1, 99, this );
  filterDelay->setValidator ( filterDelayValidator );
  filterDelayLabel = new QLabel ( tr (
      "seconds delay after moving motorised filter wheel" ), this );
  if ( filterConf.interFilterDelay ) {
    QString n = QString::number ( filterConf.interFilterDelay );
    filterDelay->setText ( n );
  }

  filterDelayBox = new QHBoxLayout();
  filterDelayBox->addWidget ( filterDelay );
  filterDelayBox->addWidget ( filterDelayLabel );
  filterDelayBox->addStretch ( 1 );
#endif

  mainBox = new QVBoxLayout ( this );
  mainBox->addLayout ( addButtonBox );
  mainBox->addStretch ( 1 );
  mainBox->addWidget ( titleLabel );
  mainBox->addLayout ( addFilterBox );
  mainBox->addLayout ( filterGridBox );
  mainBox->addStretch ( 1 );
  mainBox->addLayout ( filterPromptBox );
#ifdef INTER_FILTER_DELAY
  mainBox->addLayout ( filterDelayBox );
#endif
  mainBox->addStretch ( 1 );

  setLayout ( mainBox );

  connect ( numRuns, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( delay, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( enableButton, SIGNAL ( clicked()), this, SLOT ( enableAutorun()));

  connect ( addButton, SIGNAL ( clicked()), this, SLOT ( addFilter()));

#ifdef INTER_FILTER_DELAY
  connect ( filterDelay, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
#endif
  connect ( filterPromptOption, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
}


AutorunSettings::~AutorunSettings()
{
  delete delayValidator;
  delete numRunsValidator;
  trampolines->destroyLayout (( QLayout* ) mainBox );
}


void
AutorunSettings::storeSettings ( void )
{
  QString countStr = numRuns->text();
  QString delayStr = delay->text();
#ifdef INTER_FILTER_DELAY
  QString filterDelayStr = filterDelay->text();
#endif
  if ( countStr != "" ) {
    autorunConf.autorunCount = countStr.toInt();
  }
  if ( delayStr != "" ) {
    autorunConf.autorunDelay = delayStr.toInt();
  }
#ifdef INTER_FILTER_DELAY
  if ( filterDelayStr != "" ) {
    filterConf.interFilterDelay = filterDelayStr.toInt();
  }
#endif

  filterConf.promptForFilterChange = filterPromptOption->isChecked() ? 1 : 0;

  int numSeqs;
  filterConf.autorunFilterSequence.clear();
  if (( numSeqs = filterMenus.count())) {
    for ( int i = 0; i < numSeqs; i++ ) {
      filterConf.autorunFilterSequence.append (
					filterMenus[i]->currentIndex());
    }
  }
}


void
AutorunSettings::enableAutorun ( void )
{
  storeSettings();
  trampolines->resetAutorun();
}


void
AutorunSettings::addFilterWidgets ( int inCtor )
{
  QComboBox* cb = new QComboBox ( this );
  QPushButton* pb = new QPushButton ( QIcon ( ":/qt-icons/list-remove-4.png" ),
      "", this );
  QHBoxLayout *hb = new QHBoxLayout();

  if ( commonState.filterWheel && commonState.filterWheel->isInitialised()) {
    for ( int j = 0; j < filterWheelSlots; j++ ) {
      QString filterName;
      // can't call via settingsWidget if we're in the constructor because
      // settingsWidget is in the constructor too
      if ( !inCtor ) {
        filterName = trampolines->slotFilterName ( j );
      } else {
        int filterNum = filterConf.filterSlots[ j ];
        filterName = filterConf.filters[ filterNum ].filterName;
      }
      QString label;
      label = QString ( "%1: ").arg ( j + 1 );
      label += filterName;
      cb->addItem ( filterName );
    }
  } else {
    if ( filterConf.numFilters ) {
      for ( int j = 0; j < filterConf.numFilters; j++ ) {
        cb->addItem ( filterConf.filters[ j ].filterName );
      }
    }
  }

  hb->addWidget ( cb );
  hb->addWidget ( pb );

  filterMenus.append ( cb );
  removeButtons.append ( pb );
  filterContainers.append ( hb );

  removeButtonMapper->setMapping ( pb, sequenceLength );
  connect ( pb, SIGNAL ( clicked()), removeButtonMapper, SLOT( map()));

  int row = sequenceLength / 5;
  int col = sequenceLength % 5;
  filterGrid->addLayout ( filterContainers [ sequenceLength ], row, col );
  sequenceLength++;
}


void
AutorunSettings::addFilter ( void )
{
  addFilterWidgets ( 0 );
  connect ( filterMenus[ sequenceLength - 1 ], SIGNAL (
    currentIndexChanged ( int )), parentWidget, SLOT ( dataChanged()));
  QMetaObject::invokeMethod ( parentWidget, "dataChanged",
		Qt::QueuedConnection );
}


void
AutorunSettings::removeFilter ( int filterIndex )
{
  sequenceLength--;

  if ( filterIndex < sequenceLength ) {
    for ( int i = filterIndex; i < sequenceLength; i++ ) {
      filterMenus[ i ]->setCurrentIndex ( filterMenus[ i+1 ]->currentIndex());
    }
  }

  disconnect ( removeButtons[ sequenceLength ], SIGNAL( clicked()),
      removeButtonMapper, SLOT( map ()));
  disconnect ( filterMenus[ sequenceLength ], SIGNAL (
      currentIndexChanged ( int )), parentWidget,
      SLOT ( dataChanged()));
  int row = sequenceLength / 5;
  int col = sequenceLength % 5;
  removeButtons[ sequenceLength ]->hide();
  filterMenus[ sequenceLength ]->hide();
  filterContainers[ sequenceLength ]->removeWidget (
      removeButtons[ sequenceLength ] );
  filterContainers[ sequenceLength ]->removeWidget (
      filterMenus[ sequenceLength ] );
  filterGrid->removeItem ( filterGrid->itemAtPosition ( row, col ));
  filterContainers.removeLast();
  removeButtons.removeLast();
  filterMenus.removeLast();

  QMetaObject::invokeMethod ( parentWidget, "dataChanged",
			Qt::QueuedConnection );
}


void
AutorunSettings::setSlotName ( int slotIndex, const QString& filterName )
{
  if ( sequenceLength ) {
    for ( int i = 0; i < sequenceLength; i++ ) {
      QString label;
      label = QString ( "%1: ").arg ( slotIndex + 1 );
      label += filterName;
      filterMenus[i]->setItemText ( slotIndex, label );
    }
  }
}



void
AutorunSettings::setSlotCount ( int numSlots )
{
  int seqLen = filterMenus.count();
  if ( !seqLen ) {
    return;
  }

  int currentSlots = filterMenus[0]->count();

  if ( numSlots ) {
    if ( seqLen ) {
      // If we have more slots than currently in the menus we need to add
      // more menu items to be populated later via setSlotName().  If we
      // have fewer the extra ones must be removed
      if ( currentSlots < numSlots ) {
        for ( int s = currentSlots; s < numSlots; s++ ) {
          QString label = QString ( "%1: ").arg ( s + 1 );
          for ( int i = 0; i < seqLen; i++ ) {
            filterMenus[i]->addItem ( label );
          }
        }
        for ( int i = 0; i < seqLen; i++ ) {
          filterMenus[i]->setCurrentIndex (
							filterConf.autorunFilterSequence[i] );
        }
      }
      if ( currentSlots > numSlots ) {
        for ( int s = currentSlots; s < numSlots; s++ ) {
          for ( int i = 0; i < seqLen; i++ ) {
            filterMenus[i]->removeItem ( currentSlots );
          }
        }
      }
    }
  } else {
    if ( filterConf.numFilters ) {
      for ( int s = 0; s < filterConf.numFilters; s++ ) {
        for ( int i = 0; i < seqLen; i++ ) {
          if ( s < currentSlots ) {
            filterMenus[i]->setItemText ( s,
								filterConf.filters[s].filterName );
          } else {
            filterMenus[i]->addItem ( filterConf.filters[s].filterName );
          }
        }
      }
    }
    if ( filterConf.numFilters < currentSlots ) {
      for ( int s = filterConf.numFilters; s < currentSlots; s++ ) {
        for ( int i = 0; i < seqLen; i++ ) { 
          filterMenus[i]->removeItem ( filterConf.numFilters );
        }
      }
    }
  }
}
