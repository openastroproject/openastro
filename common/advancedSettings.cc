/*****************************************************************************
 *
 * advancedSettings.cc -- the advanced settings widget class
 *
 * Copyright 2014,2015,2016,2018 James Fidell (james@openastroproject.org)
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

extern "C" {
#include <openastro/camera.h>
#include <openastro/filterwheel.h>
#include <openastro/userConfig.h>
}

#include "commonConfig.h"
#include "commonState.h"
#include "captureSettings.h"
#include "advancedSettings.h"
#include "fitsSettings.h"


AdvancedSettings::AdvancedSettings ( QWidget* parent, int device,
		int interface, trampolineFuncs *redirs ) : trampolines ( redirs )
{
  QString			ifaceStr, deviceStr;
  QList<userDeviceConfig>	configList;
  QVBoxLayout*			inputVBox;
  int				haveItems = 0, currCol;

  deviceType =  device;
  interfaceType = interface;

  switch ( deviceType ) {
    case OA_DEVICE_FILTERWHEEL:
      ifaceStr = QString ( oaFilterWheelInterfaces[ interfaceType ].name );
      deviceStr = tr ( "Filter Wheel" );
      if ( !commonConfig.filterWheelConfig[ interfaceType ].isEmpty()) {
        configList = commonConfig.filterWheelConfig[ interfaceType ];
        haveItems = 1;
      }
      configFlags = oaFilterWheelInterfaces[ interfaceType ].userConfigFlags;
      break;

    case OA_DEVICE_PTR:
      ifaceStr = "PTR";
      deviceStr = tr ( "Timer" );
      if ( !commonConfig.timerConfig[ 0 ].isEmpty()) {
        configList = commonConfig.timerConfig[ 0 ];
        haveItems = 1;
      }
      configFlags = OA_UDC_FLAG_USB_ALL;
      break;

    default:
      ifaceStr = tr ( "Unknown" );
      deviceStr = ifaceStr;
      break;
  }

  rowList.clear();
  addedRows = 0;
  if ( haveItems ) {
    addedRows = configList.count() - 1;
    for ( int i = 0; i <= addedRows; i++ ) {
      userDeviceConfig	c;
      c.vendorId = configList[i].vendorId;
      c.productId = configList[i].productId;
      ( void ) strcpy ( c.manufacturer, configList[i].manufacturer );
      ( void ) strcpy ( c.product, configList[i].product );
      ( void ) strcpy ( c.serialNo, configList[i].serialNo );
      ( void ) strcpy ( c.filesystemPath, configList[i].filesystemPath );
      editedList.append ( c );
      rowList.append ( i );
    }
  } else {
    configList.clear();
    editedList.clear();
  }
  setWindowTitle ( ifaceStr + " " + deviceStr + tr ( " Advanced Settings" ));

  inputBoxes = new QHBoxLayout();
  grid = new QGridLayout();
  currCol = 0;

  vidPidInput = manufacturerInput = productInput = serialInput =
      fsPathInput = 0;

  if ( configFlags & OA_UDC_FLAG_VID_PID ) {
    vidPidTitle1 = new QLabel ( tr ( "VID:PID" ), this );
    vidPidTitle2 = new QLabel ( tr ( "USB VID:PID" ), this );
    grid->addWidget ( vidPidTitle1, 0, currCol++ );
    vidPidInput = new QLineEdit ( this );
    // vidPidInput->setPlaceholderText ( "95bf:a1c4" );
    vidPidInput->setInputMask ( "HHHH:HHHH" );
    inputVBox = new QVBoxLayout();
    inputVBox->addWidget ( vidPidTitle2 );
    inputVBox->addWidget ( vidPidInput );
    inputBoxes->addLayout ( inputVBox );
    connect ( vidPidInput, SIGNAL( textEdited ( const QString& )), this,
        SLOT ( inputBoxesChanged()));
  }
  if ( configFlags & OA_UDC_FLAG_MANUFACTURER ) {
    manufacturerTitle1 = new QLabel ( tr ( "Manufacturer" ), this );
    manufacturerTitle2 = new QLabel ( tr ( "Manufacturer" ), this );
    grid->addWidget ( manufacturerTitle1, 0, currCol++ );
    manufacturerInput = new QLineEdit ( this );
    manufacturerInput->setMaxLength ( OA_USB_MANUFACTURER_MAX_LEN - 1 );
    inputVBox = new QVBoxLayout();
    inputVBox->addWidget ( manufacturerTitle2 );
    inputVBox->addWidget ( manufacturerInput );
    inputBoxes->addLayout ( inputVBox );
    connect ( manufacturerInput, SIGNAL( textEdited ( const QString& )), this,
        SLOT( inputBoxesChanged()));
  }
  if ( configFlags & OA_UDC_FLAG_PRODUCT ) {
    productTitle1 = new QLabel ( tr ( "Product" ), this );
    productTitle2 = new QLabel ( tr ( "Product" ), this );
    grid->addWidget ( productTitle1, 0, currCol++ );
    productInput = new QLineEdit ( this );
    productInput->setMaxLength ( OA_USB_PRODUCT_MAX_LEN - 1 );
    inputVBox = new QVBoxLayout();
    inputVBox->addWidget ( productTitle2 );
    inputVBox->addWidget ( productInput );
    inputBoxes->addLayout ( inputVBox );
    connect ( productInput, SIGNAL( textEdited ( const QString& )), this,
        SLOT ( inputBoxesChanged()));
  }
  if ( configFlags & OA_UDC_FLAG_SERIAL ) {
    serialTitle1 = new QLabel ( tr ( "Serial Number" ), this );
    serialTitle2 = new QLabel ( tr ( "Serial Number" ), this );
    grid->addWidget ( serialTitle1, 0, currCol++ );
    serialInput = new QLineEdit ( this );
    serialInput->setMaxLength ( OA_USB_SERIAL_MAX_LEN - 1 );
    inputVBox = new QVBoxLayout();
    inputVBox->addWidget ( serialTitle2 );
    inputVBox->addWidget ( serialInput );
    inputBoxes->addLayout ( inputVBox );
    connect ( serialInput, SIGNAL( textEdited ( const QString& )), this,
        SLOT ( inputBoxesChanged()));
  }
  if ( configFlags & OA_UDC_FLAG_FS_PATH ) {
    fsPathTitle1 = new QLabel ( tr ( "Filesystem Path" ), this );
    fsPathTitle2 = new QLabel ( tr ( "Filesystem Path" ), this );
    grid->addWidget ( fsPathTitle1, 0, currCol++ );
    fsPathInput = new QLineEdit ( this );
    fsPathInput->setMaxLength ( PATH_MAX - 1 );
    inputVBox = new QVBoxLayout();
    inputVBox->addWidget ( fsPathTitle2 );
    inputVBox->addWidget ( fsPathInput );
    inputBoxes->addLayout ( inputVBox );
    connect ( fsPathInput, SIGNAL( textEdited ( const QString& )), this,
        SLOT ( inputBoxesChanged()));
  }

  deleteMapper = new QSignalMapper ( this );

  if ( haveItems ) {
    for ( int i = 0; i < configList.count(); i++ ) {
      QPushButton*	deleteButton;
      currCol = 0;
      if ( configFlags & OA_UDC_FLAG_VID_PID ) {
        QString str;
        if ( configList[i].vendorId ) {
          str = QString ( "%1:%2" ).arg ( configList[i].vendorId,
              4, 16, QLatin1Char('0')).arg ( configList[i].productId, 4, 16,
              QLatin1Char('0'));
        } else {
          str = tr ( "any" );
        }
        grid->addWidget ( new QLabel ( str, this ), i+1, currCol );
        currCol++;
      }
      if ( configFlags & OA_UDC_FLAG_MANUFACTURER ) {
        if ( *( configList[i].manufacturer )) {
          grid->addWidget ( new QLabel ( configList[i].manufacturer, this ),
              i+1, currCol );
        } else {
          grid->addWidget ( new QLabel ( tr ( "any" ), this ), i+1, currCol );
        }
        currCol++;
      }
      if ( configFlags & OA_UDC_FLAG_PRODUCT ) {
        if ( *( configList[i].product )) {
          grid->addWidget ( new QLabel ( configList[i].product, this ),
            i+1, currCol );
        } else {
          grid->addWidget ( new QLabel ( tr ( "any" ), this ), i+1, currCol );
        }
        currCol++;
      }
      if ( configFlags & OA_UDC_FLAG_SERIAL ) {
        if ( *( configList[i].serialNo )) {
          grid->addWidget ( new QLabel ( configList[i].serialNo, this ),
              i+1, currCol );
        } else {
          grid->addWidget ( new QLabel ( tr ( "any" ), this ), i+1, currCol );
        }
        currCol++;
      }
      if ( configFlags & OA_UDC_FLAG_FS_PATH ) {
        if ( *( configList[i].filesystemPath )) {
          grid->addWidget ( new QLabel ( configList[i].filesystemPath, this ),
              i+1, currCol );
        } else {
          grid->addWidget ( new QLabel ( tr ( "any" ), this ), i+1, currCol );
        }
        currCol++;
      }
      deleteButton = new QPushButton ( QIcon ( ":/qt-icons/list-remove-4.png" ),
          tr ( "Remove" ));
      grid->addWidget ( deleteButton, i+1, currCol );
      deleteMapper->setMapping ( deleteButton, i );
      connect ( deleteButton, SIGNAL( clicked()), deleteMapper, SLOT( map()));
    }
  }

  connect ( deleteMapper, SIGNAL( mapped ( int )), this,
      SLOT( deleteFilter ( int )));

  addButton = new QPushButton ( tr ( "Add" ), this );
  addButton->setEnabled ( 0 );
  inputVBox = new QVBoxLayout();
  inputVBox->addStretch ( 1 );
  inputVBox->addWidget ( addButton );
  inputBoxes->addLayout ( inputVBox );
  connect ( addButton, SIGNAL( clicked()), this, SLOT ( addFilterToGrid()));

  cancelButton = new QPushButton ( tr ( "Close" ), this );
  connect ( cancelButton, SIGNAL( clicked()), parent,
      SLOT( closeAdvancedWindow()));

  saveButton = new QPushButton ( tr ( "Save" ), this );
  saveButton->setEnabled ( 0 );
  connect ( saveButton, SIGNAL( clicked()), this, SLOT ( saveFilters()));

  buttonBox = new QHBoxLayout();
  buttonBox->addStretch ( 1 );
  buttonBox->addWidget ( cancelButton );
  buttonBox->addWidget ( saveButton );

  /*
  QLabel* intro = new QLabel;
  intro->setText ( "<p>If you need to add entries here, please consider "
      "contacting the author giving the details you're adding and the "
      "device they're for so they can be added permanently to the code.  "
      "Thank you.</p>" );
  */
  vbox = new QVBoxLayout ( this );
  // vbox->addWidget ( intro );
  vbox->addLayout ( inputBoxes );
  vbox->addSpacing ( 25 );
  vbox->addWidget ( new QLabel ( tr ( "Current Settings" )));
  vbox->addLayout ( grid );
  vbox->addSpacing ( 15 );
  vbox->addLayout ( buttonBox );
}


AdvancedSettings::~AdvancedSettings()
{
  trampolines->destroyLayout (( QLayout* ) vbox );
}


void
AdvancedSettings::inputBoxesChanged ( void )
{
  int	complete = 0;

  if (( configFlags & OA_UDC_FLAG_VID_PID ) &&
      vidPidInput->hasAcceptableInput()) {
    complete = 1;
  } else {
    if (( configFlags & OA_UDC_FLAG_MANUFACTURER ) &&
        manufacturerInput->text().length()) {
      complete = 1;
    } else {
      if (( configFlags & OA_UDC_FLAG_PRODUCT ) &&
          productInput->text().length()) {
        complete = 1;
      } else {
        if (( configFlags & OA_UDC_FLAG_SERIAL ) &&
            serialInput->text().length()) {
          complete = 1;
        } else {
          if (( configFlags & OA_UDC_FLAG_FS_PATH ) &&
              fsPathInput->text().length()) {
            complete = 1;
          }
        }
      }
    }
  }

  addButton->setEnabled ( complete );
}


void
AdvancedSettings::addFilterToGrid ( void )
{
  userDeviceConfig	c;
  QString		t;
  QPushButton*		deleteButton;

  addedRows++;
  int currCol = 0;

  c.vendorId = c.productId = 0;
  if ( configFlags & OA_UDC_FLAG_VID_PID ) {
    t = vidPidInput->text();
    if ( t.length() > 1 ) {
      sscanf ( vidPidInput->text().toStdString().c_str(), "%x:%x",
          &c.vendorId, &c.productId );
      QString str = QString ( "%1:%2" ).arg ( c.vendorId, 4, 16,
          QLatin1Char('0')).arg ( c.productId, 4, 16, QLatin1Char('0'));
      grid->addWidget ( new QLabel ( str, this ), addedRows, currCol );
    } else {
      grid->addWidget ( new QLabel ( tr ( "any" ), this ), addedRows, currCol );
    }
    currCol++;
    vidPidInput->setText("");
  }

  c.manufacturer[0] = 0;
  if ( configFlags & OA_UDC_FLAG_MANUFACTURER ) {
    t = manufacturerInput->text();
    if ( t.length()) {
      ( void ) strcpy ( c.manufacturer, t.toStdString().c_str());
      grid->addWidget ( new QLabel ( c.manufacturer, this ),
          addedRows, currCol );
    } else {
      grid->addWidget ( new QLabel ( tr ( "any" ), this ), addedRows, currCol );
    }
    manufacturerInput->setText("");
    currCol++;
  }

  c.product[0] = 0;
  if ( configFlags & OA_UDC_FLAG_PRODUCT ) {
    t = productInput->text();
    if ( t.length()) {
      ( void ) strcpy ( c.product, t.toStdString().c_str());
      grid->addWidget ( new QLabel ( c.product, this ), addedRows, currCol );
    } else {
      grid->addWidget ( new QLabel ( tr ( "any" ), this ), addedRows, currCol );
    }
    productInput->setText("");
    currCol++;
  }

  c.serialNo[0] = 0;
  if ( configFlags & OA_UDC_FLAG_SERIAL ) {
    t = serialInput->text();
    if ( t.length()) {
      ( void ) strcpy ( c.serialNo, t.toStdString().c_str());
      grid->addWidget ( new QLabel ( c.serialNo, this ), addedRows, currCol );
    } else {
      grid->addWidget ( new QLabel ( tr ( "any" ), this ), addedRows, currCol );
    }
    serialInput->setText("");
    currCol++;
  }

  c.filesystemPath[0] = 0;
  if ( configFlags & OA_UDC_FLAG_FS_PATH ) {
    t = fsPathInput->text();
    if ( t.length()) {
      ( void ) strcpy ( c.filesystemPath, t.toStdString().c_str());
      grid->addWidget ( new QLabel ( c.filesystemPath, this ), addedRows,
          currCol );
    } else {
      grid->addWidget ( new QLabel ( tr ( "any" ), this ), addedRows, currCol );
    }
    fsPathInput->setText("");
    currCol++;
  }

  editedList.append ( c );
  rowList.append ( addedRows );
  deleteButton = new QPushButton ( QIcon ( ":/qt-icons/list-remove-4.png" ),
      tr ( "Remove " ));
  grid->addWidget ( deleteButton, addedRows, currCol );
  deleteMapper->setMapping ( deleteButton, addedRows - 1 );
  connect ( deleteButton, SIGNAL( clicked()), deleteMapper, SLOT( map()));
  saveButton->setEnabled ( 1 );
}


void
AdvancedSettings::deleteFilter ( int rowIndex )
{
  int i;
  int deleteRow = -1;

  for ( i = 0; i < rowList.count() && deleteRow < 0; i++ ) {
    if ( rowList[i] == rowIndex ) {
      deleteRow = i;
    }
  }

  rowList.removeAt ( deleteRow );
  editedList.removeAt ( deleteRow );

  for ( i = 0; i < grid->columnCount(); i++ ) {
    QLayoutItem* item = grid->itemAtPosition( rowIndex + 1, i );
    QWidget* itemWidget = item->widget();
    if ( itemWidget ) {
      grid->removeWidget ( itemWidget );
      delete itemWidget;
    }
  }
  saveButton->setEnabled ( 1 );
}


void
AdvancedSettings::saveFilters ( void )
{
  int numDevices;

  numDevices = editedList.count(); 

  switch ( deviceType ) {

    case OA_DEVICE_FILTERWHEEL:
      commonConfig.filterWheelConfig[ interfaceType ].clear();
      if ( numDevices ) {
        for ( int i = 0; i < editedList.count(); i++ ) {
          commonConfig.filterWheelConfig[ interfaceType ].append (
              editedList.takeFirst());
        }
      }
      commonState.filterWheel->updateSearchFilters ( interfaceType );
      break;

    case OA_DEVICE_PTR:
      commonConfig.timerConfig[ 0 ].clear();
      if ( numDevices ) {
        for ( int i = 0; i < editedList.count(); i++ ) {
          commonConfig.timerConfig[ 0 ].append (
              editedList.takeFirst());
        }
      }
      commonState.timer->updateSearchFilters(0);
      break;

    default:
      break;
  }

  trampolines->updateConfig();
  trampolines->showStatusMessage ( tr ( "Changes saved" ));
  saveButton->setEnabled ( 0 );
}
