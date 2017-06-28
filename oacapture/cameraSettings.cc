/*****************************************************************************
 *
 * cameraSettings.cc -- class for the camera tab in the settings dialog
 *
 * Copyright 2014,2015,2016 James Fidell (james@openastroproject.org)
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

#include "cameraSettings.h"
#include "state.h"
#include "strings.h"

#define SLIDERS_PER_ROW		2
#define CHECKBOXES_PER_ROW	4
#define BUTTONS_PER_ROW		4
#define MENUS_PER_ROW		4
#define UNHANDLED_PER_ROW	6

CameraSettings::CameraSettings ( QWidget* parent ) : QWidget ( parent )
{
  layout = 0;
  sliderSignalMapper = 0;
  checkboxSignalMapper = 0;
  buttonSignalMapper = 0;
  menuSignalMapper = 0;
  memset ( controlType, 0, sizeof ( controlType ));
}


void
CameraSettings::configure ( void )
{
  uint8_t c;
  int added[ OA_CAM_CTRL_LAST_P1 ];
  int numSliders = 0, numCheckboxes = 0, numMenus = 0, numButtons = 0;
  int numSliderCheckboxes = 0, numUnhandled = 0;

  if ( layout ) {
    state.mainWindow->destroyLayout (( QLayout* ) layout );
    delete sliderSignalMapper;
    delete checkboxSignalMapper;
    delete buttonSignalMapper;
    memset ( controlType, 0, sizeof ( controlType ));
    layout = 0;
  }

  // First create all the controls to show

  sliderSignalMapper = new QSignalMapper ( this );
  checkboxSignalMapper = new QSignalMapper ( this );
  buttonSignalMapper = new QSignalMapper ( this );
  menuSignalMapper = new QSignalMapper ( this );

  for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    controlLabel[c] = 0;
    controlSlider[c] = 0;
    controlSpinbox[c] = 0;
    controlCheckbox[c] = 0;
    controlButton[c] = 0;
    controlMenu[c] = 0;
    added[c] = 0;

    if ( state.camera->Camera::isInitialised()) {
      controlType[c] = state.camera->hasControl ( c );

      if ( controlType[c] ) {

        switch ( controlType[c] ) {

          case OA_CTRL_TYPE_INT32:
          case OA_CTRL_TYPE_INT64:
          {
            numSliders++;
            controlLabel[c] = new QLabel ( tr ( oaCameraControlLabel[c] ));
            controlSlider[ c ] = new QSlider ( Qt::Horizontal, this );
            controlSlider[ c ]->setFocusPolicy ( Qt::TabFocus );
            controlSpinbox[ c ] = new QSpinBox ( this );
            connect ( controlSlider[ c ], SIGNAL( sliderMoved ( int )),
                controlSpinbox[ c ], SLOT( setValue( int )));
            connect ( controlSlider[ c ], SIGNAL( valueChanged ( int )),
                controlSpinbox[ c ], SLOT( setValue( int )));
            connect ( controlSpinbox[ c ], SIGNAL( valueChanged ( int )),
                controlSlider[ c ], SLOT( setValue( int )));

            int min = state.controlWidget-> getSpinboxMinimum ( c );
            controlSlider[ c ]->setMinimum ( min );
            controlSpinbox[ c ]->setMinimum ( min );

            int max = state.controlWidget-> getSpinboxMaximum ( c );
            controlSlider[ c ]->setMaximum ( max );
            controlSpinbox[ c ]->setMaximum ( max );

            int step = state.controlWidget-> getSpinboxStep ( c );
            controlSlider[ c ]->setSingleStep ( step );
            controlSpinbox[ c ]->setSingleStep ( step );

            int val = state.controlWidget-> getSpinboxValue ( c );
            controlSlider[ c ]->setValue ( val );
            controlSpinbox[ c ]->setValue ( val );

            sliderSignalMapper->setMapping ( controlSpinbox[c], c );
            connect ( controlSpinbox[ c ], SIGNAL( valueChanged ( int )),
                sliderSignalMapper, SLOT( map()));
            break;
          }

          case OA_CTRL_TYPE_BOOLEAN:
            numCheckboxes++;
            controlCheckbox[ c ] = new QCheckBox ( QString ( tr (
                oaCameraControlLabel[c] )), this );
            if ( c == OA_CAM_CTRL_AUTO_EXPOSURE ) {
              controlCheckbox[ c ]->setChecked (
                  ( config.controlValues [ c ] == OA_EXPOSURE_MANUAL ) ? 0 : 1 );
            } else {
              controlCheckbox[ c ]->setChecked ( config.controlValues [ c ] );
            }
            checkboxSignalMapper->setMapping ( controlCheckbox[c], c );
            connect ( controlCheckbox[ c ], SIGNAL ( stateChanged ( int )),
                checkboxSignalMapper, SLOT ( map()));
            break;

          case OA_CTRL_TYPE_BUTTON:
            numButtons++;
            controlButton[ c ] = new QPushButton ( QString ( tr (
                oaCameraControlLabel[c] )), this );
            buttonSignalMapper->setMapping ( controlButton[c], c );
            connect ( controlButton[ c ], SIGNAL ( clicked ()),
                buttonSignalMapper, SLOT ( map()));
            break;

          case OA_CTRL_TYPE_MENU:
          {
            int64_t min, max, step, def;
            controlLabel[c] = new QLabel ( tr ( oaCameraControlLabel[c] ));
            state.camera->controlRange ( c, &min, &max, &step, &def );
            if ( 1 == step && 0 == min ) {
              numMenus++;
              controlMenu[ c ] = new QComboBox ( this );
              for ( int i = min; i <= max; i += step ) {
                controlMenu[ c ]->addItem ( tr (
                    state.camera->getMenuString ( c, i )));
              }
              controlMenu[ c ]->setCurrentIndex ( config.controlValues [ c ] );
              menuSignalMapper->setMapping ( controlMenu[c], c );
              connect ( controlMenu[ c ], SIGNAL( currentIndexChanged ( int )),
                  menuSignalMapper, SLOT ( map()));
            } else {
              numUnhandled++;
              qWarning() << "Can't handle menu with min = " << min <<
                  " and step = " << step;
            }
            break;
          }

          case OA_CTRL_TYPE_DISC_MENU:
          {
            int32_t count;
            int64_t *values;
            controlLabel[c] = new QLabel ( tr ( oaCameraControlLabel[c] ));
            state.camera->controlDiscreteSet ( c, &count, &values );
            numMenus++;
            controlMenu[ c ] = new QComboBox ( this );
            for ( int i = 0; i < count; i++ ) {
              controlMenu[ c ]->addItem ( tr (
                  state.camera->getMenuString ( c, values[i] )));
              controlMenu[ c ]->setCurrentIndex ( config.controlValues [ c ] );
              menuSignalMapper->setMapping ( controlMenu[c], values[i] );
              connect ( controlMenu[ c ], SIGNAL( currentIndexChanged ( int )),
                  menuSignalMapper, SLOT ( map()));
            }
            break;
          }

          case OA_CTRL_TYPE_READONLY:
            added[c] = 1; // prevents this from showing up
            break;

          case OA_CTRL_TYPE_DISCRETE:
            // don't show these up as unhandled
            if ( OA_CAM_CTRL_BIT_DEPTH == c || OA_CAM_CTRL_BINNING == c ) {
              added[c] = 1;
              break;
            }

          default:
            controlLabel[c] = new QLabel ( tr ( oaCameraControlLabel[c] ));
            numUnhandled++;
            break;
        }
      }
    }
  }

  numButtons++;
  defaultButton = new QPushButton ( QString ( tr ( "Reset to defaults" )),
      this );
  buttonSignalMapper->setMapping ( defaultButton, -1 );
  connect ( defaultButton, SIGNAL ( clicked ()), buttonSignalMapper,
      SLOT ( map()));

  connect ( sliderSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( updateSliderControl ( int )));
  connect ( checkboxSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( updateCheckboxControl ( int )));
  connect ( buttonSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( buttonPushed ( int )));
  connect ( menuSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( menuChanged ( int )));

  if ( state.camera->isInitialised() && state.camera->hasFrameRateSupport()) {
    frameRateSlider = new QSlider ( Qt::Horizontal, this );
    frameRateLabel = new QLabel ( tr ( "Framerate (fps)" ), this );
    frameRateMenu = new QComboBox ( this );
    frameRateSlider->setFocusPolicy ( Qt::TabFocus );
    QStringList rateList = state.controlWidget->getFrameRates();
    frameRateSlider->setRange ( 0, rateList.count());
    frameRateSlider->setSingleStep ( 1 );
    frameRateMenu->addItems ( rateList );

    connect ( frameRateSlider, SIGNAL( sliderMoved ( int )), frameRateMenu,
        SLOT( setCurrentIndex( int )));
    connect ( frameRateSlider, SIGNAL( valueChanged ( int )), frameRateMenu,
        SLOT( setCurrentIndex( int )));
    connect ( frameRateSlider, SIGNAL( sliderReleased()), this,
        SLOT( frameRateChanged()));
    connect ( frameRateMenu, SIGNAL( currentIndexChanged ( int )),
        frameRateSlider, SLOT( setValue( int )));
    connect ( frameRateMenu, SIGNAL( currentIndexChanged ( int )), this,
        SLOT( frameRateChanged()));
  }

  // Now run through the controls one at a time and add them to the layout
  // in groups depending on their type

  sliderGrid = new QGridLayout();
  autoLabel1 = new QLabel ( tr ( "Auto" ));
  autoLabel2 = new QLabel ( tr ( "Auto" ));

  int row = 1, col = 0;

  sliderGrid->addWidget ( autoLabel1, 0, 1 );
  sliderGrid->addWidget ( autoLabel2, 0, 5 );

  int autoControl;
  for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    if ( OA_CTRL_TYPE_INT32 == controlType[ c ] ||
        OA_CTRL_TYPE_INT64 == controlType[ c ] ) {
      sliderGrid->addWidget ( controlLabel[c], row, col++ );
      added[ c ] = 1;
      if (( autoControl = state.camera->hasAuto ( c ))) {
        if ( OA_CTRL_TYPE_BOOLEAN == controlType[ autoControl ] ) {
          controlCheckbox[ autoControl ]->setText ( "" );
          sliderGrid->addWidget ( controlCheckbox[ autoControl ], row, col );
          added[ autoControl ] = 1;
          numSliderCheckboxes++;
        }
      }
      col++;
      sliderGrid->addWidget ( controlSlider[c], row, col++ );
      sliderGrid->addWidget ( controlSpinbox[c], row, col++ );
    }
    if (( 4 * SLIDERS_PER_ROW ) == col ) {
      col = 0;
      row++;
    }
  }

  if ( state.camera->isInitialised() && state.camera->hasFrameRateSupport()) {
    sliderGrid->addWidget ( frameRateLabel, row, col++ );
    col++;
    sliderGrid->addWidget ( frameRateSlider, row, col++ );
    sliderGrid->addWidget ( frameRateMenu, row, col++ );
  }

  // And now the checkboxes that we haven't already used next to sliders

  checkboxGrid = new QGridLayout();

  row = 0;
  col = 0;
  int addedBoxes = 0;
  for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    if ( OA_CTRL_TYPE_BOOLEAN == controlType[ c ] && !added[ c ]) {
      checkboxGrid->addWidget ( controlCheckbox[c], row, col++ );
      added[ c ] = 1;
      addedBoxes++;
    }
    if ( CHECKBOXES_PER_ROW == col ) {
      col = 0;
      row++;
    }
  }
  if ( addedBoxes && addedBoxes < CHECKBOXES_PER_ROW ) {
    checkboxGrid->setColumnStretch ( addedBoxes, 1 );
  }

  // Next the controls that are pushbuttons

  buttonGrid = new QGridLayout();

  row = 0;
  col = 0;
  int addedButtons = 0;
  for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    if ( OA_CTRL_TYPE_BUTTON == controlType[ c ] && !added[ c ]) {
      buttonGrid->addWidget ( controlButton[c], row, col++ );
      added[ c ] = 1;
      addedButtons++;
    }
    if ( BUTTONS_PER_ROW == col ) {
      col = 0;
      row++;
    }
  }
  buttonGrid->addWidget ( defaultButton, row, col++ );
  addedButtons++;
  if ( addedButtons && addedButtons < BUTTONS_PER_ROW ) {
    buttonGrid->setColumnStretch ( addedButtons, 1 );
  }

  // And the menu controls

  menuGrid = new QGridLayout();

  row = 0;
  col = 0;
  int addedMenus = 0;
  for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    if ( OA_CTRL_TYPE_MENU == controlType[ c ] ) {
      int64_t min, max, step, def;
      state.camera->controlRange ( c, &min, &max, &step, &def );
      if ( 1 == step && 0 == min ) {
        menuGrid->addWidget ( controlLabel[c], row, col++ );
        menuGrid->addWidget ( controlMenu[c], row, col++ );
        added[ c ] = 1;
        addedMenus++;
      }
    }
    if ( OA_CTRL_TYPE_DISC_MENU == controlType[ c ] ) {
      menuGrid->addWidget ( controlLabel[c], row, col++ );
      menuGrid->addWidget ( controlMenu[c], row, col++ );
      added[ c ] = 1;
      addedMenus++;
    }
    if ( MENUS_PER_ROW == col ) {
      col = 0;
      row++;
    }
  }
  if ( addedMenus && addedMenus < MENUS_PER_ROW ) {
    menuGrid->setColumnStretch ( addedMenus * 2, 1 );
  }

  // For the sake of completeness, show the controls we're not handling

  unhandledGrid = new QGridLayout();
  int addedTodo = 0;
  if ( numUnhandled ) {
    QLabel* unhandled = new QLabel ( tr ( "Unhandled controls" ));
    unhandledGrid->addWidget ( unhandled, 0, 0 );
    row = 1;
    col = 0;
    for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
      if ( controlType[c] && !added[ c ] ) {
        unhandledGrid->addWidget ( controlLabel[c], row, col++ );
        added[ c ] = 1;
        addedTodo++;
      }
      if ( UNHANDLED_PER_ROW == col ) {
        col = 0;
        row++;
      }
    }
  }
  if ( addedTodo && addedTodo < UNHANDLED_PER_ROW ) {
    unhandledGrid->setColumnStretch ( addedTodo, 1 );
  }

  // Add all the bits to the layout

  layout = new QVBoxLayout ( this );
  layout->addLayout ( sliderGrid );
  layout->addStretch ( 1 );
  layout->addLayout ( checkboxGrid );
  layout->addStretch ( 1 );
  layout->addLayout ( buttonGrid );
  layout->addStretch ( 1 );
  layout->addLayout ( menuGrid );
  layout->addStretch ( 1 );
  layout->addLayout ( unhandledGrid );
  layout->addStretch ( 2 );

  setLayout ( layout );
}


CameraSettings::~CameraSettings()
{
  if ( layout ) {
    state.mainWindow->destroyLayout (( QLayout* ) layout );
  }
  if ( sliderSignalMapper ) {
    delete sliderSignalMapper;
    delete checkboxSignalMapper;
  }
}


void
CameraSettings::updateSliderControl ( int control )
{
  state.controlWidget->updateSpinbox ( control,
      controlSpinbox[ control ]->value());
}


void
CameraSettings::updateCheckboxControl ( int control )
{
  int value = controlCheckbox[ control ]->isChecked() ? 1 : 0;

  switch ( control ) {
    case OA_CAM_CTRL_HFLIP:
      state.mainWindow->setFlipX ( value );
      break;

    case OA_CAM_CTRL_VFLIP:
      state.mainWindow->setFlipY ( value );
      break;

    case OA_CAM_CTRL_TRIGGER_ENABLE:
      config.timerMode = value ? OA_TIMER_MODE_TRIGGER : OA_TIMER_MODE_STROBE;
      state.controlWidget->updateCheckbox ( control, value );
      break;

    case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
      state.controlWidget->updateCheckbox ( control, value );
      break;

    case OA_CAM_CTRL_STROBE_ENABLE:
      config.timerMode = value ? OA_TIMER_MODE_STROBE : OA_TIMER_MODE_TRIGGER;
      state.controlWidget->updateCheckbox ( control, value );
      break;

    default:
      state.controlWidget->updateCheckbox ( control, value );
      break;
  }

  if ( oaIsAuto ( control )) {
    int baseControl = oaGetControlForAuto ( control );
    if ( controlSlider[ baseControl ] ) {
      controlSlider[ baseControl ]->setEnabled ( !value );
      controlSpinbox[ baseControl ]->setEnabled ( !value );
    }
  }
}


void
CameraSettings::buttonPushed ( int control )
{
  int c, v;

  if ( control > 0 ) {
    state.camera->setControl ( control, 1 );
    if ( control == OA_CAM_CTRL_RESTORE_USER ||
        control == OA_CAM_CTRL_RESTORE_FACTORY ) {

      for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
        switch ( controlType [ c ] ) {

          case OA_CTRL_TYPE_BOOLEAN:
            v = state.camera->readControl ( c );
            config.controlValues [ c ] = v;
            SET_PROFILE_CONTROL( c, v );
            controlCheckbox[ c ]->setChecked ( v );
            break;

          case OA_CTRL_TYPE_INT32:
          case OA_CTRL_TYPE_INT64:
            v = state.camera->readControl ( c );
            config.controlValues [ c ] = v;
            SET_PROFILE_CONTROL( c, v );
            controlSpinbox[ c ]->setValue ( v );
            break;

          case OA_CTRL_TYPE_MENU:
            v = state.camera->readControl ( c );
            config.controlValues [ c ] = v;
            SET_PROFILE_CONTROL( c, v );
            controlMenu[ c ]->setCurrentIndex ( v );
            break;

          case OA_CTRL_TYPE_UNAVAILABLE:
          case OA_CTRL_TYPE_BUTTON:
          case OA_CTRL_TYPE_READONLY:
            break;

          default:
            fprintf ( stderr, "control type %d not handled in %s\n",
                controlType[ c ], __FUNCTION__ );
            break;
        }
      }

      QMessageBox::warning ( this, tr ( "Restore Settings" ),
          tr ( "Depending on how this function is implemented in the camera "
              "it is possible that the control settings may now be set to "
              "incorrect values" ));
    }
  } else {
    int64_t min, max, step, def;

    for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
      switch ( controlType [ c ] ) {

        case OA_CTRL_TYPE_BOOLEAN:
          state.camera->controlRange ( c, &min, &max, &step, &def );
          controlCheckbox[ c ]->setChecked ( def );
          config.controlValues [ c ] = def;
          SET_PROFILE_CONTROL( c, def );
          break;

        case OA_CTRL_TYPE_INT32:
        case OA_CTRL_TYPE_INT64:
          state.camera->controlRange ( c, &min, &max, &step, &def );
          controlSpinbox[ c ]->setValue ( def );
          config.controlValues [ c ] = def;
          SET_PROFILE_CONTROL( c, def );
          break;

        case OA_CTRL_TYPE_MENU:
          state.camera->controlRange ( c, &min, &max, &step, &def );
          controlMenu[ c ]->setCurrentIndex ( def );
          config.controlValues [ c ] = def;
          SET_PROFILE_CONTROL( c, def );
          break;

        case OA_CTRL_TYPE_UNAVAILABLE:
        case OA_CTRL_TYPE_BUTTON:
        case OA_CTRL_TYPE_READONLY:
          break;

        default:
          fprintf ( stderr, "control type %d not handled in %s\n",
              controlType[ c ], __FUNCTION__ );
          break;
      }
    }
  }
}


void
CameraSettings::storeSettings ( void )
{
}


void
CameraSettings::enableFlipX ( int state )
{
  if ( controlType [ OA_CAM_CTRL_HFLIP ] == OA_CTRL_TYPE_BOOLEAN ) {
    controlCheckbox [ OA_CAM_CTRL_HFLIP ]->setChecked ( state );
  } else {
    qWarning() << "CameraSettings::enableFlipX not implemented for type";
  }
}


void
CameraSettings::enableFlipY ( int state )
{
  if ( controlType [ OA_CAM_CTRL_VFLIP ] == OA_CTRL_TYPE_BOOLEAN ) {
    controlCheckbox [ OA_CAM_CTRL_VFLIP ]->setChecked ( state );
  } else {
    qWarning() << "CameraSettings::enableFlipY not implemented for type";
  }
}


void
CameraSettings::updateControl ( int control, int value )
{
  if ( OA_CTRL_TYPE_INT32 == controlType [ control ] ||
      OA_CTRL_TYPE_INT64 == controlType [ control ] ) {
    controlSlider [ control ]->setValue ( value );
  } else if ( controlType [ control ] == OA_CTRL_TYPE_BOOLEAN ) {
    controlCheckbox [ control ]->setChecked ( value );
  } else {
    qWarning() << "CameraSettings::update not implemented for control"
        << control << "type" << controlType [ control ];
  }
}


void
CameraSettings::menuChanged ( int control )
{
  int value = controlMenu [ control ]->currentIndex();
  state.camera->setControl ( control, value );
}


void
CameraSettings::updateFrameRate ( int index )
{
  frameRateSlider->setValue ( index );
}


void
CameraSettings::frameRateChanged ( void )
{
  state.controlWidget->updateFrameRate ( frameRateMenu->currentIndex());
}


void
CameraSettings::reconfigureControl ( int control )
{
  int min = state.controlWidget-> getSpinboxMinimum ( control );
  controlSlider[ control ]->setMinimum ( min );
  controlSpinbox[ control ]->setMinimum ( min );

  int max = state.controlWidget-> getSpinboxMaximum ( control );
  controlSlider[ control ]->setMaximum ( max );
  controlSpinbox[ control ]->setMaximum ( max );

  int step = state.controlWidget-> getSpinboxStep ( control );
  controlSlider[ control ]->setSingleStep ( step );
  controlSpinbox[ control ]->setSingleStep ( step );

  int val = state.controlWidget-> getSpinboxValue ( control );
  controlSlider[ control ]->setValue ( val );
  controlSpinbox[ control ]->setValue ( val );
}
