/*****************************************************************************
 *
 * cameraControls.cc -- class for the camera tab in the settings dialog
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

#include "cameraControls.h"
#include "state.h"
#include "strings.h"

#define SLIDERS_PER_ROW		1
#define CHECKBOXES_PER_ROW	2
#define BUTTONS_PER_ROW		2
#define MENUS_PER_ROW		2
#define UNHANDLED_PER_ROW	3

#define MENU_RANGE_UNUSED	0
#define MENU_RANGE_USEC		1
#define MENU_RANGE_MSEC		2
#define MENU_RANGE_SECS		3
#define MENU_RANGE_MINS		4

static int64_t		rangeMenuMax[5] = {
  1LL, 1000LL, 1000000LL, 60000000LL, 3600000000LL
};

static const char*	rangeMenuLabels[5] = {
  "unused", "microseconds", "milliseconds", "seconds", "minutes"
};


CameraControls::CameraControls ( QWidget* parent ) : QWidget ( parent )
{
  layout = 0;
  sliderSignalMapper = 0;
  checkboxSignalMapper = 0;
  buttonSignalMapper = 0;
  menuSignalMapper = 0;
  frameRateLabel = 0;
  frameRateSlider = 0;
  frameRateMenu = 0;
  memset ( controlType, 0, sizeof ( controlType ));
  ignoreFrameRateChanges = 0;
}


void
CameraControls::configure ( void )
{
  uint8_t	c;
  int		added[ OA_CAM_CTRL_LAST_P1 ];
  int		numSliders = 0, numCheckboxes = 0, numMenus = 0;
  int		numSliderCheckboxes = 0, numUnhandled = 0, numButtons = 0;
  int		haveRangeMenu = 0;

  if ( layout ) {
    state.mainWindow->destroyLayout (( QLayout* ) layout );
    delete sliderSignalMapper;
    delete checkboxSignalMapper;
    delete buttonSignalMapper;
    memset ( controlType, 0, sizeof ( controlType ));
    layout = 0;
    frameRateLabel = 0;
    frameRateSlider = 0;
    frameRateMenu = 0;
  }

  // Create all the controls to show

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
            int64_t	min, max, step, def, showMin, showMax, showStep;

            numSliders++;
            controlLabel[c] = new QLabel ( tr ( oaCameraControlLabel[c] ));
            controlLabel[c]->setWordWrap ( 1 );
            controlSlider[ c ] = new QSlider ( Qt::Horizontal, this );
            controlSlider[ c ]->setFocusPolicy ( Qt::TabFocus );
            controlSlider[ c ]->setMinimumWidth ( 100 );
            controlSpinbox[ c ] = new QSpinBox ( this );

            connect ( controlSlider[ c ], SIGNAL( sliderMoved ( int )),
                controlSpinbox[ c ], SLOT( setValue( int )));
            connect ( controlSlider[ c ], SIGNAL( valueChanged ( int )),
                controlSpinbox[ c ], SLOT( setValue( int )));
            connect ( controlSpinbox[ c ], SIGNAL( valueChanged ( int )),
                controlSlider[ c ], SLOT( setValue( int )));
            connect ( controlSpinbox[ c ], SIGNAL( valueChanged ( int )),
                sliderSignalMapper, SLOT( map()));

            state.camera->controlRange ( c, &min, &max, &step, &def );
            showMin = min;
            showMax = max;
            showStep = step;

            if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == c ) {
              int minIndex = 0, maxIndex = 0;
              int i, numRanges;

              numRanges = sizeof ( rangeMenuMax ) / sizeof ( uint64_t );
              for ( i = 0; i < numRanges - 1; i++ ) {
                if ( min > rangeMenuMax[i] ) {
                  minIndex++;
                }
                if ( max > rangeMenuMax[i] ) {
                  maxIndex++;
                }
              }
              if ( minIndex != maxIndex ) {
                haveRangeMenu = 1;
                showStep = 1;
                exposureRangeMenu = new QComboBox ( this );
                for ( i = minIndex; i < maxIndex; i++ ) {
                  exposureRangeMenu->addItem ( tr ( rangeMenuLabels[i] ));
                }

              }
            }

            controlSlider[ c ]->setMinimum ( showMin );
            controlSpinbox[ c ]->setMinimum ( showMin );

            controlSlider[ c ]->setMaximum ( showMax );
            controlSpinbox[ c ]->setMaximum ( showMax );

            controlSlider[ c ]->setSingleStep ( showStep );
            controlSpinbox[ c ]->setSingleStep ( showStep );

            controlSlider[ c ]->setValue ( def );
            controlSpinbox[ c ]->setValue ( def );

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
            controlLabel[c]->setWordWrap ( 1 );
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
            controlLabel[c]->setWordWrap ( 1 );
            numUnhandled++;
            break;
        }
      }
    }
  }

  numButtons++;

  connect ( sliderSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( updateSliderControl ( int )));
  connect ( checkboxSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( updateCheckboxControl ( int )));
  connect ( buttonSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( buttonPushed ( int )));
  connect ( menuSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( menuChanged ( int )));

  frameRateSlider = new QSlider ( Qt::Horizontal, this );
  frameRateLabel = new QLabel ( tr ( "Framerate (fps)" ), this );
  frameRateMenu = new QComboBox ( this );
  frameRateSlider->setFocusPolicy ( Qt::TabFocus );
  frameRateSlider->setSingleStep ( 1 );

  connect ( frameRateSlider, SIGNAL( sliderMoved ( int )), frameRateMenu,
      SLOT( setCurrentIndex( int )));
  connect ( frameRateSlider, SIGNAL( valueChanged ( int )), frameRateMenu,
      SLOT( setCurrentIndex( int )));
/*
  connect ( frameRateSlider, SIGNAL( sliderReleased()), this,
      SLOT( frameRateChanged()));
*/
  connect ( frameRateMenu, SIGNAL( currentIndexChanged ( int )),
      frameRateSlider, SLOT( setValue( int )));
  connect ( frameRateMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( frameRateChanged()));

  // Commenting this out as the frame rates probably aren't set yet
  // updateFrameRates();

  // Now run through the controls one at a time and add them to the layout
  // in groups depending on their type

  sliderGrid = new QGridLayout();
  autoLabel1 = new QLabel ( tr ( "Auto" ));

  int row = 1, col = 0;

  sliderGrid->addWidget ( autoLabel1, 0, 1 );

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
      if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == c && haveRangeMenu ) {
        if (( 4 * SLIDERS_PER_ROW ) == col ) {
          col = 0;
          row++;
        }
        exposureRangeLabel = new QLabel ( tr ( "Exposure Units" ));
        sliderGrid->addWidget ( exposureRangeLabel, row, col++ );
        col++;
        sliderGrid->addWidget ( exposureRangeMenu, row, col++ );
        col++;
      }
    }
    if (( 4 * SLIDERS_PER_ROW ) == col ) {
      col = 0;
      row++;
    }
  }

  if ( haveRangeMenu ) {
    connect ( exposureRangeMenu, SIGNAL( currentIndexChanged ( int )),
        this, SLOT ( updateExposureUnits()));
  }
/*
  if ( state.camera->hasFixedFrameRates ( config.imageSizeX,
      config.imageSizeY )) {
*/
    sliderGrid->addWidget ( frameRateLabel, row, col++ );
    col++;
    sliderGrid->addWidget ( frameRateSlider, row, col++ );
    sliderGrid->addWidget ( frameRateMenu, row, col++ );
/*
  }
*/

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


CameraControls::~CameraControls()
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
CameraControls::disableAutoControls ( void )
{
  // These ones we just want off all the time
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_GAIN )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_GAIN, 0 );
    config.controlValues[ OA_CAM_CTRL_AUTO_GAIN ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_GAIN, 0 );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_HUE_AUTO )) {
    state.camera->setControl ( OA_CAM_CTRL_HUE_AUTO, 0 );
    config.controlValues[ OA_CAM_CTRL_HUE_AUTO ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_HUE_AUTO, 0 );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_BRIGHTNESS )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_BRIGHTNESS, 0 );
    config.controlValues[ OA_CAM_CTRL_AUTO_BRIGHTNESS ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_BRIGHTNESS, 0 );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_EXPOSURE )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_EXPOSURE, OA_EXPOSURE_MANUAL );
    config.controlValues[ OA_CAM_CTRL_AUTO_EXPOSURE ] = OA_EXPOSURE_MANUAL;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_EXPOSURE, OA_EXPOSURE_MANUAL );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_GAMMA )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_GAMMA, 0 );
    config.controlValues[ OA_CAM_CTRL_AUTO_GAMMA ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_GAMMA, 0 );
  }
  int AWBtype = state.camera->hasControl ( OA_CAM_CTRL_AUTO_WHITE_BALANCE );
  if ( AWBtype ) {
    int AWBManual = 0;
    if ( OA_CTRL_TYPE_MENU == AWBtype ) {
      AWBManual = state.camera->getAWBManualSetting();
    }
    state.camera->setControl ( OA_CAM_CTRL_AUTO_WHITE_BALANCE, AWBManual );
    config.controlValues[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] = AWBManual;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_WHITE_BALANCE, AWBManual );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_RED_BALANCE )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_RED_BALANCE, 0 );
    config.controlValues[ OA_CAM_CTRL_AUTO_RED_BALANCE ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_RED_BALANCE, 0 );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_BLUE_BALANCE )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_BLUE_BALANCE, 0 );
    config.controlValues[ OA_CAM_CTRL_AUTO_BLUE_BALANCE ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_BLUE_BALANCE, 0 );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_USBTRAFFIC )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_USBTRAFFIC, 0 );
    config.controlValues[ OA_CAM_CTRL_AUTO_USBTRAFFIC ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_USBTRAFFIC, 0 );
  }
  if ( state.camera->hasControl ( OA_CAM_CTRL_AUTO_CONTRAST )) {
    state.camera->setControl ( OA_CAM_CTRL_AUTO_CONTRAST, 0 );
    config.controlValues[ OA_CAM_CTRL_AUTO_CONTRAST ] = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_AUTO_CONTRAST, 0 );
  }
}


void
CameraControls::updateSliderControl ( int control )
{
  int value = controlSpinbox[ control ]->value();
  config.controlValues[ control ] = value;
  SET_PROFILE_CONTROL( control, value );
  state.camera->setControl ( control, value );
}


void
CameraControls::updateCheckboxControl ( int control )
{
  int value = ( controlCheckbox[ control ]->isChecked()) ? 1 : 0;
  int baseControl, controlType;

  if ( OA_CAM_CTRL_AUTO_EXPOSURE == control &&
       state.camera->hasControl ( control ) == OA_CTRL_TYPE_BOOLEAN ) {
    value = value ? OA_EXPOSURE_AUTO : OA_EXPOSURE_MANUAL;
  }
  config.controlValues[ control ] = value;
  SET_PROFILE_CONTROL( control, value );
  state.camera->setControl ( control, value );
  if (( baseControl = oaGetControlForAuto ( control )) >= 0 ) {
    controlType = state.camera->hasControl ( baseControl );
    if ( OA_CTRL_TYPE_INT32 == controlType || OA_CTRL_TYPE_INT64 ==
        controlType ) {
      controlSlider[ baseControl ]->setEnabled ( !value );
      controlSpinbox[ baseControl ]->setEnabled ( !value );
    }
  }
}


void
CameraControls::buttonPushed ( int control )
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


/*
void
CameraControls::storeSettings ( void )
{
}


void
CameraControls::updateControl ( int control, int value )
{
  if ( OA_CTRL_TYPE_INT32 == controlType [ control ] ||
      OA_CTRL_TYPE_INT64 == controlType [ control ] ) {
    controlSlider [ control ]->setValue ( value );
  } else if ( controlType [ control ] == OA_CTRL_TYPE_BOOLEAN ) {
    controlCheckbox [ control ]->setChecked ( value );
  } else {
    qWarning() << "CameraControls::update not implemented for control"
        << control << "type" << controlType [ control ];
  }
}
*/

void
CameraControls::menuChanged ( int control )
{
  int value = controlMenu [ control ]->currentIndex();
  state.camera->setControl ( control, value );
}


void
CameraControls::frameRateChanged ( void )
{
  if ( !ignoreFrameRateChanges ) {
    _doFrameRateChange ( frameRateMenu->currentIndex());
  }
}

void
CameraControls::_doFrameRateChange ( int index )
{
  /*
   * Leaving this in means the first frame setting for a camera breaks
   *
  if ( config.frameRateNumerator == frameRateNumerator[ index ] &&
      config.frameRateDenominator == frameRateDenominator[ index ] ) {
    return;
  }
  */

  config.frameRateNumerator = frameRateNumerator[ index ];
  config.frameRateDenominator = frameRateDenominator[ index ];
  theoreticalFPSNumerator = config.frameRateNumerator;
  theoreticalFPSDenominator = config.frameRateDenominator;

  if ( state.camera->isInitialised()) {
    state.camera->setFrameInterval ( config.frameRateNumerator,
        config.frameRateDenominator );
  }
}


void
CameraControls::updateFrameRateSlider ( void )
{
  if ( !state.camera->isInitialised()) {
    return;
  }
  if ( !state.camera->hasFrameRateSupport()) {
    frameRateLabel->hide();
    frameRateSlider->hide();
    frameRateMenu->hide();
    return;
  }

  if ( state.camera->hasFixedFrameRates ( config.imageSizeX,
      config.imageSizeY )) {

    // Ugly sorting of the frame rates here.  We don't know what
    // order we'll get them in so we have to sort them into rate
    // order for the slider to work.

    const FRAMERATES* rates = state.camera->frameRates ( config.imageSizeX,
        config.imageSizeY );

    // don't use the frame rate slider if there's only one frame rate
    if ( !rates || rates->numRates < 2 ) {
      frameRateLabel->hide();
      frameRateSlider->hide();
      frameRateMenu->hide();
      if ( !rates ) {
        return;
      }
    }

    QMap<float,QString> sortMap;
    QMap<float,int> numeratorMap;
    QMap<float,int> denominatorMap;
    unsigned int i;
    for ( i = 0; i < rates->numRates; i++ ) {
      float frameTime = float ( rates->rates[i].numerator ) /
          float ( rates->rates[i].denominator );
      QString rateStr;
      if ( 1 == rates->rates[i].numerator ) {
        rateStr = QString::number ( rates->rates[i].denominator );
      } else {
        rateStr = QString::number ( rates->rates[i].denominator ) + "/" +
            QString::number ( rates->rates[i].numerator );
      }
      sortMap [ frameTime ] = rateStr;
      numeratorMap [ frameTime ] = rates->rates[i].numerator;
      denominatorMap [ frameTime ] = rates->rates[i].denominator;
    }

    QStringList rateList;
    QMap<float,QString>::iterator j;
    QMap<float,int>::iterator n, d;
    frameRateNumerator.clear();
    frameRateDenominator.clear();
    int numItems = 0, showItem = 0;
    for ( j = sortMap.begin(), n = numeratorMap.begin(),
        d = denominatorMap.begin(); j != sortMap.end(); j++, n++, d++ ) {
      rateList << *j;
      frameRateNumerator << *n;
      frameRateDenominator << *d;
      if ( *n == config.frameRateNumerator &&
          *d == config.frameRateDenominator ) {
        showItem = numItems;
      }
      numItems++;
    }

    // Handle only having one frame rate
    if ( numItems == 1 ) {
      _doFrameRateChange ( 0 );
      return;
    }

    ignoreFrameRateChanges = 1;
    frameRateMenu->clear();
    frameRateMenu->addItems ( rateList );
    frameRateSlider->setRange ( 0, numItems - 1 );
    frameRateSlider->setSingleStep ( 1 );
    config.frameRateNumerator = frameRateNumerator[ showItem ];
    config.frameRateDenominator = frameRateDenominator[ showItem ];
    ignoreFrameRateChanges = 0;
    frameRateMenu->setCurrentIndex ( showItem );
    // unfortunately the above statement may not change the frame rate even
    // though the rate has changed, because its position in the menu might
    // be the same, so we have to do the update manually;
    _doFrameRateChange ( showItem );

    frameRateLabel->show();
    frameRateSlider->show();
    frameRateMenu->show();

  } else {
    fprintf ( stderr, "Camera::hasFixedFrameRates failed\n" );
    frameRateLabel->hide();
    frameRateSlider->hide();
    frameRateMenu->hide();
  }
}


unsigned int
CameraControls::getCurrentGain ( void )
{
  if ( state.camera->hasControl ( OA_CAM_CTRL_GAIN )) {
    return config.controlValues[ OA_CAM_CTRL_GAIN ];
  }
  return 0;
}


unsigned int
CameraControls::getCurrentExposure ( void )
{
  if ( state.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) {
    return config.controlValues[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ];
  } else {
    if ( state.camera->hasControl ( OA_CAM_CTRL_EXPOSURE )) {
      return config.controlValues[ OA_CAM_CTRL_EXPOSURE ];
    }
  }
  return 0;
}


int
CameraControls::getFPSNumerator ( void )
{
  return theoreticalFPSNumerator;
}


int
CameraControls::getFPSDenominator ( void )
{
  return theoreticalFPSDenominator;
}


void
CameraControls::updateExposureUnits ( void )
{
  qWarning() << "need to implement CameraControls::updateExposureUnits";
}
