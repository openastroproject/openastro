/*****************************************************************************
 *
 * cameraSettings.cc -- class for the camera tab in the settings dialog
 *
 * Copyright 2014,2015,2016,2017 James Fidell (james@openastroproject.org)
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
  unsigned int c, baseVal, mod, frameRateIndex;
  int added[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
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

  for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
    for ( mod = 0; mod < OA_CAM_CTRL_MODIFIERS_P1; mod++ ) {
      c = baseVal | ( mod ? ( 0x80 << mod ) : 0 );
      controlLabel[mod][baseVal] = 0;
      controlSlider[mod][baseVal] = 0;
      controlSpinbox[mod][baseVal] = 0;
      controlCheckbox[mod][baseVal] = 0;
      controlButton[mod][baseVal] = 0;
      controlMenu[mod][baseVal] = 0;
      added[mod][baseVal] = 0;

      if ( state.camera->Camera::isInitialised()) {
        controlType[mod][baseVal] = state.camera->hasControl ( c );

        if ( controlType[mod][baseVal] ) {

          switch ( controlType[mod][baseVal] ) {

            case OA_CTRL_TYPE_INT32:
            case OA_CTRL_TYPE_INT64:
            {
              numSliders++;
              controlLabel[mod][baseVal] = new QLabel ( tr (
                  oaCameraControlLabel[baseVal] ));
              controlSlider[mod][baseVal] = new QSlider ( Qt::Horizontal,
                  this );
              controlSlider[mod][baseVal]->setFocusPolicy ( Qt::TabFocus );
              controlSpinbox[mod][baseVal] = new QSpinBox ( this );
              connect ( controlSlider[mod][baseVal], SIGNAL(
                  sliderMoved ( int )), controlSpinbox[mod][baseVal],
                  SLOT( setValue( int )));
              connect ( controlSlider[mod][baseVal], SIGNAL(
                  valueChanged ( int )), controlSpinbox[mod][baseVal],
                  SLOT( setValue( int )));
              connect ( controlSpinbox[mod][baseVal], SIGNAL(
                  valueChanged ( int )), controlSlider[mod][baseVal],
                  SLOT( setValue( int )));

              int min = state.controlWidget->getSpinboxMinimum ( c );
              controlSlider[mod][baseVal]->setMinimum ( min );
              controlSpinbox[mod][baseVal]->setMinimum ( min );

              int max = state.controlWidget-> getSpinboxMaximum ( c );
              controlSlider[mod][baseVal]->setMaximum ( max );
              controlSpinbox[mod][baseVal]->setMaximum ( max );

              int step = state.controlWidget-> getSpinboxStep ( c );
              controlSlider[mod][baseVal]->setSingleStep ( step );
              controlSpinbox[mod][baseVal]->setSingleStep ( step );

              int val = state.controlWidget-> getSpinboxValue ( c );
              controlSlider[mod][baseVal]->setValue ( val );
              controlSpinbox[mod][baseVal]->setValue ( val );

              sliderSignalMapper->setMapping ( controlSpinbox[mod][baseVal], c );
              connect ( controlSpinbox[mod][baseVal], SIGNAL(
                  valueChanged ( int )), sliderSignalMapper, SLOT( map()));
              break;
            }

            case OA_CTRL_TYPE_BOOLEAN:
              numCheckboxes++;
              controlCheckbox[mod][baseVal] = new QCheckBox ( QString ( tr (
                  oaCameraControlModifierPrefix[mod] )) + QString ( tr (
                  oaCameraControlLabel[baseVal] )), this );
              if ( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) ==
                  c || OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) ==
                  c ) {
                controlCheckbox[mod][baseVal]->setChecked (
                    ( config.CONTROL_VALUE(c) == OA_EXPOSURE_MANUAL ) ? 0 : 1 );
              } else {
                controlCheckbox[mod][baseVal]->setChecked (
                    config.CONTROL_VALUE(c));
              }
              checkboxSignalMapper->setMapping (
                  controlCheckbox[mod][baseVal], c );
              connect ( controlCheckbox[mod][baseVal], SIGNAL (
                  stateChanged ( int )), checkboxSignalMapper, SLOT ( map()));
              break;

            case OA_CTRL_TYPE_BUTTON:
              numButtons++;
              controlButton[mod][baseVal] = new QPushButton ( QString ( tr (
                  oaCameraControlLabel[baseVal] )), this );
              buttonSignalMapper->setMapping ( controlButton[mod][baseVal], c );
              connect ( controlButton[mod][baseVal], SIGNAL ( clicked ()),
                  buttonSignalMapper, SLOT ( map()));
              break;

            case OA_CTRL_TYPE_MENU:
            {
              int64_t min, max, step, def;
              controlLabel[mod][baseVal] = new QLabel ( tr (
                  oaCameraControlLabel[baseVal] ));
              state.camera->controlRange ( c, &min, &max, &step, &def );
              if ( 1 == step && 0 == min ) {
                numMenus++;
                controlMenu[mod][baseVal] = new QComboBox ( this );
                for ( int i = min; i <= max; i += step ) {
                  controlMenu[mod][baseVal]->addItem ( tr (
                      state.camera->getMenuString ( c, i )));
                }
                controlMenu[mod][baseVal]->setCurrentIndex (
                    config.CONTROL_VALUE(c));
                menuSignalMapper->setMapping ( controlMenu[mod][baseVal], c );
                connect ( controlMenu[mod][baseVal], SIGNAL(
                    currentIndexChanged ( int )), menuSignalMapper,
                    SLOT( map()));
              } else {
                numUnhandled++;
                // FIX ME
                qWarning() << "Can't handle menu with min = " << min <<
                    " and step = " << step;
              }
              break;
            }

            case OA_CTRL_TYPE_DISC_MENU:
            {
              int32_t count;
              int64_t *values;
              controlLabel[mod][baseVal] = new QLabel ( tr (
                  oaCameraControlLabel[baseVal] ));
              state.camera->controlDiscreteSet ( c, &count, &values );
              numMenus++;
              controlMenu[mod][baseVal] = new QComboBox ( this );
              for ( int i = 0; i < count; i++ ) {
                controlMenu[mod][baseVal]->addItem ( tr (
                    state.camera->getMenuString ( c, values[i] )));
                controlMenu[mod][baseVal]->setCurrentIndex (
                    config.CONTROL_VALUE(c));
                menuSignalMapper->setMapping ( controlMenu[mod][baseVal], c );
                connect ( controlMenu[mod][baseVal], SIGNAL(
                    currentIndexChanged ( int )), menuSignalMapper,
                    SLOT( map()));
              }
              break;
            }

            case OA_CTRL_TYPE_READONLY:
              added[mod][baseVal] = 1; // prevents this from showing up
              break;

            case OA_CTRL_TYPE_DISCRETE:
              // FIX ME -- these really ought to show
              // don't show these up as unhandled
              if ( OA_CAM_CTRL_BIT_DEPTH == c || OA_CAM_CTRL_BINNING == c ) {
                added[mod][baseVal] = 1;
                break;
              }

            default:
              controlLabel[mod][baseVal] = new QLabel ( tr (
                  oaCameraControlLabel[baseVal] ));
              numUnhandled++;
              break;
          }
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
    frameRateIndex = state.controlWidget->getFrameRateIndex();
    frameRateSlider->setValue ( frameRateIndex );
    frameRateMenu->setCurrentIndex ( frameRateIndex );

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
  onOffLabel1 = new QLabel ( tr ( "On/Off" ));
  onOffLabel2 = new QLabel ( tr ( "On/Off" ));

  int row = 1, col = 0;

  sliderGrid->addWidget ( autoLabel1, 0, 1 );
  sliderGrid->addWidget ( onOffLabel1, 0, 2 );
  sliderGrid->setColumnMinimumWidth ( 5, 40 );
  sliderGrid->addWidget ( autoLabel2, 0, 7 );
  sliderGrid->addWidget ( onOffLabel2, 0, 8 );

  int autoControl;
  for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
    for ( mod = OA_CAM_CTRL_MODIFIER_STD; mod < OA_CAM_CTRL_MODIFIERS_P1;
        mod++ ) {
      if ( OA_CTRL_TYPE_INT32 ==
          controlType[OA_CAM_CTRL_MODIFIER_STD][baseVal] ||
          OA_CTRL_TYPE_INT64 ==
          controlType[OA_CAM_CTRL_MODIFIER_STD][baseVal] ) {
        if ( OA_CAM_CTRL_MODIFIER_STD == mod ) {
          sliderGrid->addWidget ( controlLabel[mod][baseVal], row, col++ );
          sliderGrid->addWidget ( controlSlider[mod][baseVal], row, col + 2 );
          sliderGrid->addWidget ( controlSpinbox[mod][baseVal], row, col + 3 );
          added[mod][baseVal] = 1;
        } else {
          if ( controlType[mod][baseVal] == OA_CTRL_TYPE_BOOLEAN ) {
            controlCheckbox[mod][baseVal]->setText ( "" );
            sliderGrid->addWidget ( controlCheckbox[mod][baseVal], row, col,
                Qt::AlignCenter );
            added[mod][baseVal] = 1;
            numSliderCheckboxes++;
            if ( OA_CAM_CTRL_MODIFIER_AUTO == mod ) {
              int autoMode =
                config.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO ( baseVal ));
              if ( OA_CAM_CTRL_EXPOSURE_UNSCALED == baseVal ||
                  OA_CAM_CTRL_EXPOSURE_ABSOLUTE == baseVal ) {
                autoMode = ( OA_EXPOSURE_MANUAL == autoMode ) ? 0 : 1;
              }
              if ( controlSlider[OA_CAM_CTRL_MODIFIER_STD][baseVal]) {
                controlSlider[OA_CAM_CTRL_MODIFIER_STD][baseVal]->setEnabled (
                    !autoMode );
                controlSpinbox[OA_CAM_CTRL_MODIFIER_STD][baseVal]->setEnabled (
                    !autoMode );
              }
            }
          }
          col++;
        }
        // last time through, we want to add two to account for the slider and
        // spinbox added above
        if (( OA_CAM_CTRL_MODIFIERS_P1 - 1 ) == mod ) {
          col += 2;
        }
      }
    }
    // skip the stretched column if we added anything since last time through
    if ( col % 6 ) {
      col++;
    }
    if (( 6 * SLIDERS_PER_ROW ) == col ) {
      col = 0;
      row++;
    }
  }

  if ( state.camera->isInitialised() && state.camera->hasFrameRateSupport()) {
    sliderGrid->addWidget ( frameRateLabel, row, col++ );
    col++;
    col++;
    sliderGrid->addWidget ( frameRateSlider, row, col++ );
    sliderGrid->addWidget ( frameRateMenu, row, col++ );
  }

  // And now the checkboxes that we haven't already used next to sliders

  checkboxGrid = new QGridLayout();

  row = 0;
  col = 0;
  int addedBoxes = 0;
  for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
    for ( mod = 0; mod < OA_CAM_CTRL_MODIFIERS_P1; mod++ ) {
      if ( OA_CTRL_TYPE_BOOLEAN == controlType[mod][baseVal] &&
          !added[mod][baseVal]) {
        checkboxGrid->addWidget ( controlCheckbox[mod][baseVal], row, col++ );
        added[mod][baseVal] = 1;
        addedBoxes++;
      }
      if ( CHECKBOXES_PER_ROW == col ) {
        col = 0;
        row++;
      }
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
  for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
    for ( mod = 0; mod <= OA_CAM_CTRL_MODIFIER_AUTO; mod++ ) {
      if ( OA_CTRL_TYPE_BUTTON == controlType[mod][baseVal] &&
          !added[mod][baseVal]) {
        buttonGrid->addWidget ( controlButton[mod][baseVal], row, col++ );
        added[mod][baseVal] = 1;
        addedButtons++;
      }
      if ( BUTTONS_PER_ROW == col ) {
        col = 0;
        row++;
      }
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
  for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
    for ( mod = 0; mod <= OA_CAM_CTRL_MODIFIER_AUTO; mod++ ) {
      c = baseVal | ( mod ? OA_CAM_CTRL_MODIFIER_AUTO_MASK : 0 );
      if ( OA_CTRL_TYPE_MENU == controlType[mod][baseVal] ) {
        int64_t min, max, step, def;
        state.camera->controlRange ( c, &min, &max, &step, &def );
        if ( 1 == step && 0 == min ) {
          menuGrid->addWidget ( controlLabel[mod][baseVal], row, col++ );
          menuGrid->addWidget ( controlMenu[mod][baseVal], row, col++ );
          added[mod][baseVal] = 1;
          addedMenus++;
        }
      }
      if ( OA_CTRL_TYPE_DISC_MENU == controlType[mod][baseVal] ) {
        menuGrid->addWidget ( controlLabel[mod][baseVal], row, col++ );
        menuGrid->addWidget ( controlMenu[mod][baseVal], row, col++ );
        added[mod][baseVal] = 1;
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
  }

  // For the sake of completeness, show the controls we're not handling

  unhandledGrid = new QGridLayout();
  int addedTodo = 0;
  if ( numUnhandled ) {
    QLabel* unhandled = new QLabel ( tr ( "Unhandled controls" ));
    unhandledGrid->addWidget ( unhandled, 0, 0 );
    row = 1;
    col = 0;
    for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
      for ( mod = 0; mod <= OA_CAM_CTRL_MODIFIER_AUTO; mod++ ) {
        if ( controlType[mod][baseVal] && !added[mod][baseVal] ) {
          unhandledGrid->addWidget ( controlLabel[mod][baseVal], row, col++ );
          added[mod][baseVal] = 1;
          addedTodo++;
        }
        if ( UNHANDLED_PER_ROW == col ) {
          col = 0;
          row++;
        }
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
  state.controlWidget->updateSpinbox ( control, controlSpinbox[
      OA_CAM_CTRL_MODIFIER(control)][OA_CAM_CTRL_MODE_BASE(control)]->value());
}


void
CameraSettings::updateCheckboxControl ( int control )
{
  int value = controlCheckbox[OA_CAM_CTRL_MODIFIER(
      control)][OA_CAM_CTRL_MODE_BASE(control)]->isChecked() ? 1 : 0;

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

  if ( OA_CAM_CTRL_IS_ON_OFF ( control )) {
    int baseControl = OA_CAM_CTRL_MODE_BASE ( control );
    if ( controlSlider[OA_CAM_CTRL_MODIFIER_STD][ baseControl ] ) {
      controlSlider[OA_CAM_CTRL_MODIFIER_STD][ baseControl ]->
          setEnabled ( value );
      controlSpinbox[OA_CAM_CTRL_MODIFIER_STD][ baseControl ]->
          setEnabled ( value );
    }
    if ( controlCheckbox[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ] ) {
      controlCheckbox[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ]->
          setEnabled ( value );
    }
    if ( controlButton[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ] ) {
      controlButton[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ]->
          setEnabled ( value );
    }
    if ( controlMenu[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ] ) {
      controlButton[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ]->
          setEnabled ( value );
    }
  }

  if ( OA_CAM_CTRL_IS_AUTO ( control )) {
    int baseControl = OA_CAM_CTRL_MODE_BASE ( control );
    if ( controlSlider[OA_CAM_CTRL_MODIFIER_STD][ baseControl ] ) {
      controlSlider[OA_CAM_CTRL_MODIFIER_STD][ baseControl ]->
          setEnabled ( !value );
      controlSpinbox[OA_CAM_CTRL_MODIFIER_STD][ baseControl ]->
          setEnabled ( !value );
    }
    if ( controlButton[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ] ) {
      controlButton[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ]->
          setEnabled ( !value );
    }
    if ( controlMenu[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ] ) {
      controlButton[OA_CAM_CTRL_MODIFIER_AUTO][ baseControl ]->
          setEnabled ( !value );
    }
  }
}


void
CameraSettings::buttonPushed ( int control )
{
  int c, v, baseVal, mod;

  if ( control > 0 ) {
    state.camera->setControl ( control, 1 );
    if ( control == OA_CAM_CTRL_RESTORE_USER ||
        control == OA_CAM_CTRL_RESTORE_FACTORY ) {

      for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
        for ( mod = 0; mod <= OA_CAM_CTRL_MODIFIER_AUTO; mod++ ) {
          c = baseVal | ( mod ? OA_CAM_CTRL_MODIFIER_AUTO_MASK : 0 );
          switch ( controlType[mod][baseVal] ) {

            case OA_CTRL_TYPE_BOOLEAN:
              v = state.camera->readControl ( c );
              config.CONTROL_VALUE(c) = v;
              SET_PROFILE_CONTROL( c, v );
              controlCheckbox[mod][baseVal]->setChecked ( v );
              break;

            case OA_CTRL_TYPE_INT32:
            case OA_CTRL_TYPE_INT64:
              v = state.camera->readControl ( c );
              config.CONTROL_VALUE(c) = v;
              SET_PROFILE_CONTROL( c, v );
              controlSpinbox[mod][baseVal]->setValue ( v );
              break;

            case OA_CTRL_TYPE_MENU:
              v = state.camera->readControl ( c );
              config.CONTROL_VALUE(c) = v;
              SET_PROFILE_CONTROL( c, v );
              controlMenu[mod][baseVal]->setCurrentIndex ( v );
              break;

            case OA_CTRL_TYPE_UNAVAILABLE:
            case OA_CTRL_TYPE_BUTTON:
            case OA_CTRL_TYPE_READONLY:
              break;

            default:
              fprintf ( stderr, "control type %d not handled in %s\n",
                  controlType[mod][baseVal], __FUNCTION__ );
              break;
          }
        }
      }

      QMessageBox::warning ( this, tr ( "Restore Settings" ),
          tr ( "Depending on how this function is implemented in the camera "
              "it is possible that the control settings may now be set to "
              "incorrect values" ));
    }
  } else {
    int64_t min, max, step, def;

    for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
      for ( mod = 0; mod <= OA_CAM_CTRL_MODIFIER_AUTO; mod++ ) {
        c = baseVal | ( mod ? OA_CAM_CTRL_MODIFIER_AUTO_MASK : 0 );
        switch ( controlType[mod][baseVal] ) {

          case OA_CTRL_TYPE_BOOLEAN:
            state.camera->controlRange ( c, &min, &max, &step, &def );
            controlCheckbox[mod][baseVal]->setChecked ( def );
            config.CONTROL_VALUE(c) = def;
            SET_PROFILE_CONTROL( c, def );
            break;

          case OA_CTRL_TYPE_INT32:
          case OA_CTRL_TYPE_INT64:
            state.camera->controlRange ( c, &min, &max, &step, &def );
            controlSpinbox[mod][baseVal]->setValue ( def );
            config.CONTROL_VALUE(c) = def;
            SET_PROFILE_CONTROL( c, def );
            break;

          case OA_CTRL_TYPE_MENU:
            state.camera->controlRange ( c, &min, &max, &step, &def );
            controlMenu[mod][baseVal]->setCurrentIndex ( def );
            config.CONTROL_VALUE(c) = def;
            SET_PROFILE_CONTROL( c, def );
            break;

          case OA_CTRL_TYPE_UNAVAILABLE:
          case OA_CTRL_TYPE_BUTTON:
          case OA_CTRL_TYPE_READONLY:
            break;

          default:
            fprintf ( stderr, "control type %d not handled in %s\n",
                controlType[mod][baseVal], __FUNCTION__ );
            break;
        }
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
  if ( controlType[ OA_CAM_CTRL_MODIFIER_STD ][ OA_CAM_CTRL_HFLIP ] ==
      OA_CTRL_TYPE_BOOLEAN ) {
    controlCheckbox [ OA_CAM_CTRL_MODIFIER_STD ][ OA_CAM_CTRL_HFLIP ]->
        setChecked ( state );
  } else {
    qWarning() << "CameraSettings::enableFlipX not implemented for type";
  }
}


void
CameraSettings::enableFlipY ( int state )
{
  if ( controlType[ OA_CAM_CTRL_MODIFIER_STD ][ OA_CAM_CTRL_VFLIP ] ==
      OA_CTRL_TYPE_BOOLEAN ) {
    controlCheckbox [ OA_CAM_CTRL_MODIFIER_STD ][ OA_CAM_CTRL_VFLIP ]->
        setChecked ( state );
  } else {
    qWarning() << "CameraSettings::enableFlipY not implemented for type";
  }
}


void
CameraSettings::updateControl ( int control, int value )
{
  if ( OA_CTRL_TYPE_INT32 == controlType[ OA_CAM_CTRL_MODIFIER( control )][
      OA_CAM_CTRL_MODE_BASE( control )] || OA_CTRL_TYPE_INT64 ==
      controlType[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]) {
    controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
        control )]->setValue ( value );
  } else if ( controlType[ OA_CAM_CTRL_MODIFIER( control )][
      OA_CAM_CTRL_MODE_BASE( control )] == OA_CTRL_TYPE_BOOLEAN ) {
    controlCheckbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
        control )]->setChecked ( value );
  } else {
    qWarning() << "CameraSettings::update not implemented for control"
        << control << "type" << controlType[ OA_CAM_CTRL_MODIFIER( control )][
        OA_CAM_CTRL_MODE_BASE( control )];
  }
}


void
CameraSettings::menuChanged ( int control )
{
  int mod = OA_CAM_CTRL_MODIFIER( control );
  int baseVal = OA_CAM_CTRL_MODE_BASE( control );
  int value;

  value = controlMenu[ mod ][ baseVal ]->currentIndex();
  // FIX ME -- there's an implicit assumption here that menus will have
  // the same value sequence as the items in the menu. (ie. starting at 0
  // and incrementing by 1
  if ( controlType [ mod ][ baseVal ] == OA_CTRL_TYPE_DISC_MENU ) {
    int32_t count;
    int64_t *values;
    state.camera->controlDiscreteSet ( control, &count, &values );
    if ( value < count ) {
      value = values[value];
    } else {
      qWarning() << "Invalid menu value for discrete menu";
      return;
    }
  }
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
  int min = state.controlWidget->getSpinboxMinimum ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMinimum ( min );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMinimum ( min );

  int max = state.controlWidget->getSpinboxMaximum ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMaximum ( max );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMaximum ( max );

  int step = state.controlWidget->getSpinboxStep ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setSingleStep ( step );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setSingleStep ( step );

  int val = state.controlWidget->getSpinboxValue ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setValue ( val );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setValue ( val );
}
