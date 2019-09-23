/*****************************************************************************
 *
 * cameraSettings.cc -- class for the camera tab in the settings dialog
 *
 * Copyright 2014,2015,2016,2017,2018,2019
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

extern "C" {
#include <openastro/timer.h>
#include <strings.h>
}

#include <QtCore>
#include <QtGui>

#include "commonState.h"
#include "commonConfig.h"
#include "cameraSettings.h"
#include "profileSettings.h"
#include "timerSettings.h"

#define SLIDERS_PER_ROW		2
#define CHECKBOXES_PER_ROW	4
#define BUTTONS_PER_ROW		4
#define MENU_ITEMS_PER_ROW	9
#define UNHANDLED_PER_ROW	6


CameraSettings::CameraSettings ( QWidget* parent, QWidget* top,
		trampolineFuncs* redirs ) :
		QWidget ( parent ), topWidget ( top ), trampolines ( redirs )
{
  layout = 0;
  sliderSignalMapper = 0;
  checkboxSignalMapper = 0;
  buttonSignalMapper = 0;
  menuSignalMapper = 0;
  memset ( controlType, 0, sizeof ( controlType ));
}

cameraConfig cameraConf;


void
CameraSettings::configure ( void )
{
  unsigned int c, baseVal, mod, frameRateIndex, format;
  int added[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
  int numSliders = 0, numCheckboxes = 0, numMenus = 0, numButtons = 0;
  int numSliderCheckboxes = 0, numUnhandled = 0;

  if ( layout ) {
    trampolines->destroyLayout (( QLayout* ) layout );
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

      // Ignore the frame format control
      if ( baseVal == OA_CAM_CTRL_FRAME_FORMAT ) {
        continue;
      }

      if ( commonState.camera->Camera::isInitialised()) {
        controlType[mod][baseVal] = commonState.camera->hasControl ( c );

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

              int min = trampolines->getCameraSpinboxMinimum ( c );
              controlSlider[mod][baseVal]->setMinimum ( min );
              controlSpinbox[mod][baseVal]->setMinimum ( min );

              int max = trampolines-> getCameraSpinboxMaximum ( c );
              controlSlider[mod][baseVal]->setMaximum ( max );
              controlSpinbox[mod][baseVal]->setMaximum ( max );

              int step = trampolines-> getCameraSpinboxStep ( c );
              controlSlider[mod][baseVal]->setSingleStep ( step );
              controlSpinbox[mod][baseVal]->setSingleStep ( step );

              int val = trampolines-> getCameraSpinboxValue ( c );
              controlSlider[mod][baseVal]->setValue ( val );
              controlSpinbox[mod][baseVal]->setValue ( val );

              sliderSignalMapper->setMapping ( controlSpinbox[mod][baseVal], c );
              connect ( controlSpinbox[mod][baseVal], SIGNAL(
                  valueChanged ( int )), sliderSignalMapper, SLOT( map()));
              break;
            }

            case OA_CTRL_TYPE_BOOLEAN:
						{
              numCheckboxes++;
              controlCheckbox[mod][baseVal] = new QCheckBox ( QString ( tr (
                  oaCameraControlModifierPrefix[mod] )) + QString ( tr (
                  oaCameraControlLabel[baseVal] )), this );
              controlCheckbox[mod][baseVal]->setChecked (
									cameraConf.CONTROL_VALUE(c));
              checkboxSignalMapper->setMapping (
                  controlCheckbox[mod][baseVal], c );
              connect ( controlCheckbox[mod][baseVal], SIGNAL (
                  stateChanged ( int )), checkboxSignalMapper, SLOT ( map()));
              break;
						}
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
							char		labelText[64];

							labelText[0] = '\0';
							if ( oaCameraControlModifierPrefix[ mod ] ) {
								strncpy ( labelText, oaCameraControlModifierPrefix[ mod ], 64 );
								strncat ( labelText, " ", 64 );
							}
							strncat ( labelText, oaCameraControlLabel[baseVal], 64 );
              controlLabel[mod][baseVal] = new QLabel ( tr ( labelText ));
              commonState.camera->controlRange ( c, &min, &max, &step, &def );
              if ( 1 == step && 0 == min ) {
                numMenus++;
                controlMenu[mod][baseVal] = new QComboBox ( this );
                for ( int i = min; i <= max; i += step ) {
                  controlMenu[mod][baseVal]->addItem ( tr (
                      commonState.camera->getMenuString ( c, i )));
                }
                controlMenu[mod][baseVal]->setCurrentIndex (
                    cameraConf.CONTROL_VALUE(c));
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
							char		labelText[64];

							labelText[0] = '\0';
							if ( oaCameraControlModifierPrefix[ mod ] ) {
								strncpy ( labelText, oaCameraControlModifierPrefix[ mod ], 64 );
								strncat ( labelText, " ", 64 );
							}
							strncat ( labelText, oaCameraControlLabel[baseVal], 64 );
              controlLabel[mod][baseVal] = new QLabel ( tr ( labelText ));
              commonState.camera->controlDiscreteSet ( c, &count, &values );
              numMenus++;
              controlMenu[mod][baseVal] = new QComboBox ( this );
              for ( int i = 0; i < count; i++ ) {
                controlMenu[mod][baseVal]->addItem ( tr (
                    commonState.camera->getMenuString ( c, values[i] )));
								if ( cameraConf.CONTROL_VALUE(c) == values[i] ) {
									controlMenu[mod][baseVal]->setCurrentIndex ( i );
								}
              }
              menuSignalMapper->setMapping ( controlMenu[mod][baseVal], c );
              connect ( controlMenu[mod][baseVal], SIGNAL(
                  currentIndexChanged ( int )), menuSignalMapper, SLOT( map()));
              break;
            }

            case OA_CTRL_TYPE_READONLY:
              added[mod][baseVal] = 1; // prevents this from showing up
              break;

            case OA_CTRL_TYPE_DISCRETE:
              // FIX ME -- these really ought to show
              // don't show these up as unhandled
              if ( OA_CAM_CTRL_BINNING == c ) {
                added[mod][baseVal] = 1;
                break;
              }
							/* FALLTHROUGH */
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

  if ( commonState.camera->isInitialised() &&
			commonState.camera->hasFrameRateSupport()) {
    frameRateSlider = new QSlider ( Qt::Horizontal, this );
    frameRateLabel = new QLabel ( tr ( "Framerate (fps)" ), this );
    frameRateMenu = new QComboBox ( this );
    frameRateSlider->setFocusPolicy ( Qt::TabFocus );
    QStringList rateList = trampolines->getCameraFrameRates();
    frameRateSlider->setRange ( 0, rateList.count());
    frameRateSlider->setSingleStep ( 1 );
    frameRateMenu->addItems ( rateList );
    frameRateIndex = trampolines->getCameraFrameRateIndex();
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
            if ( OA_CAM_CTRL_MODIFIER_AUTO == mod ) {
              sliderGrid->addWidget ( controlCheckbox[mod][baseVal], row, col,
                  Qt::AlignCenter );
            }
            if ( OA_CAM_CTRL_MODIFIER_ON_OFF == mod ) {
              sliderGrid->addWidget ( controlCheckbox[mod][baseVal], row, col,
                  Qt::AlignCenter );
            }
            added[mod][baseVal] = 1;
            numSliderCheckboxes++;
            if ( OA_CAM_CTRL_MODIFIER_AUTO == mod ) {
              int autoMode =
                cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO ( baseVal ));
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

  if ( commonState.camera->isInitialised() &&
			commonState.camera->hasFrameRateSupport()) {
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
        commonState.camera->controlRange ( c, &min, &max, &step, &def );
        if ( 1 == step && 0 == min ) {
          menuGrid->addWidget ( controlLabel[mod][baseVal], row, col++,
              Qt::AlignRight );
          menuGrid->addWidget ( controlMenu[mod][baseVal], row, col++,
              Qt::AlignLeft );
          added[mod][baseVal] = 1;
          addedMenus++;
          col++;
        }
      }
      if ( OA_CTRL_TYPE_DISC_MENU == controlType[mod][baseVal] ) {
        menuGrid->addWidget ( controlLabel[mod][baseVal], row, col++,
            Qt::AlignRight );
        menuGrid->addWidget ( controlMenu[mod][baseVal], row, col++,
            Qt::AlignLeft );
        added[mod][baseVal] = 1;
        addedMenus++;
          col++;
      }
      if ( MENU_ITEMS_PER_ROW == col ) {
        col = 0;
        row++;
      }
    }
  }
  if ( addedMenus && addedMenus < MENU_ITEMS_PER_ROW ) {
    menuGrid->setColumnStretch ( addedMenus * 2 - 1, 1 );
  }
  if ( addedMenus > 1 ) {
    int maxMenus, i;
    maxMenus = ( addedMenus < MENU_ITEMS_PER_ROW / 3 ) ? addedMenus * 3 :
        MENU_ITEMS_PER_ROW;
    addedMenus -= 2;
    for ( i = 2; i < maxMenus; i += 3 ) {
      menuGrid->setColumnMinimumWidth ( i, 20 );
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

  forceFrameFormat = new QCheckBox ( tr ( "Force Input Frame Format" ), this );
  forceFrameFormat->setChecked ( cameraConf.forceInputFrameFormat );

  selectedFrameFormat = new QComboBox ( this );
  for ( format = 1; format < OA_PIX_FMT_LAST_P1; format++ ) {
    selectedFrameFormat->addItem ( tr ( oaFrameFormats[ format ].name ));
    selectedFrameFormat->setItemData ( format - 1,
          tr ( oaFrameFormats[ format ].simpleName ), Qt::ToolTipRole );
  }
  if ( cameraConf.forceInputFrameFormat ) {
    selectedFrameFormat->setCurrentIndex (
				cameraConf.forceInputFrameFormat - 1 );
  }
  frameHBoxLayout = new QHBoxLayout();
  frameHBoxLayout->addWidget ( forceFrameFormat );
  frameHBoxLayout->addWidget ( selectedFrameFormat );
  frameHBoxLayout->addStretch ( 1 );
  connect ( forceFrameFormat, SIGNAL( stateChanged ( int )), this,
      SLOT( forceFrameFormatChanged ( int )));
  connect ( selectedFrameFormat, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( selectedFrameFormatChanged ( int )));

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
  layout->addStretch ( 1 );
  layout->addLayout ( frameHBoxLayout );
  layout->addStretch ( 2 );

  setLayout ( layout );
}


CameraSettings::~CameraSettings()
{
  if ( layout ) {
    trampolines->destroyLayout (( QLayout* ) layout );
  }
  if ( sliderSignalMapper ) {
    delete sliderSignalMapper;
    delete checkboxSignalMapper;
  }
}


void
CameraSettings::updateSliderControl ( int control )
{
	trampolines->updateCameraSpinbox ( control, controlSpinbox[
      OA_CAM_CTRL_MODIFIER(control)][OA_CAM_CTRL_MODE_BASE(control)]->value());
}


void
CameraSettings::updateCheckboxControl ( int control )
{
  int value = controlCheckbox[OA_CAM_CTRL_MODIFIER(
      control)][OA_CAM_CTRL_MODE_BASE(control)]->isChecked() ? 1 : 0;

  switch ( control ) {
    case OA_CAM_CTRL_HFLIP:
      trampolines->setCameraFlipX ( value );
      break;

    case OA_CAM_CTRL_VFLIP:
      trampolines->setCameraFlipY ( value );
      break;

    case OA_CAM_CTRL_TRIGGER_ENABLE:
			timerConf.timerMode = ( value ? OA_TIMER_MODE_TRIGGER :
					OA_TIMER_MODE_STROBE );
      trampolines->updateCameraControlCheckbox ( control, value );
      break;

    case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
      trampolines->updateCameraControlCheckbox ( control, value );
      break;

    case OA_CAM_CTRL_STROBE_ENABLE:
			timerConf.timerMode = ( value ?  OA_TIMER_MODE_STROBE :
					OA_TIMER_MODE_TRIGGER );
      trampolines->updateCameraControlCheckbox ( control, value );
      break;

    default:
      trampolines->updateCameraControlCheckbox ( control, value );
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
    commonState.camera->setControl ( control, 1 );
    if ( control == OA_CAM_CTRL_RESTORE_USER ||
        control == OA_CAM_CTRL_RESTORE_FACTORY ) {

      for ( baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
        for ( mod = 0; mod <= OA_CAM_CTRL_MODIFIER_AUTO; mod++ ) {
          c = baseVal | ( mod ? OA_CAM_CTRL_MODIFIER_AUTO_MASK : 0 );
          switch ( controlType[mod][baseVal] ) {

            case OA_CTRL_TYPE_BOOLEAN:
              v = commonState.camera->readControl ( c );
              cameraConf.CONTROL_VALUE(c) = v;
              SET_PROFILE_CONTROL( c, v );
              controlCheckbox[mod][baseVal]->setChecked ( v );
              break;

            case OA_CTRL_TYPE_INT32:
            case OA_CTRL_TYPE_INT64:
              v = commonState.camera->readControl ( c );
              cameraConf.CONTROL_VALUE(c) = v;
              SET_PROFILE_CONTROL( c, v );
              controlSpinbox[mod][baseVal]->setValue ( v );
              break;

            case OA_CTRL_TYPE_MENU:
              v = commonState.camera->readControl ( c );
              cameraConf.CONTROL_VALUE(c) = v;
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

      QMessageBox::warning ( topWidget, tr ( "Restore Settings" ),
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
            commonState.camera->controlRange ( c, &min, &max, &step, &def );
            controlCheckbox[mod][baseVal]->setChecked ( def );
            cameraConf.CONTROL_VALUE(c) = def;
            SET_PROFILE_CONTROL( c, def );
            break;

          case OA_CTRL_TYPE_INT32:
          case OA_CTRL_TYPE_INT64:
            commonState.camera->controlRange ( c, &min, &max, &step, &def );
            controlSpinbox[mod][baseVal]->setValue ( def );
            cameraConf.CONTROL_VALUE(c) = def;
            SET_PROFILE_CONTROL( c, def );
            break;

          case OA_CTRL_TYPE_MENU:
            commonState.camera->controlRange ( c, &min, &max, &step, &def );
            controlMenu[mod][baseVal]->setCurrentIndex ( def );
            cameraConf.CONTROL_VALUE(c) = def;
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

	if ( OA_CAM_CTRL_IS_AUTO ( control )) {
		controlSlider[ OA_CAM_CTRL_MODIFIER_STD ]
			[ OA_CAM_CTRL_MODE_BASE( control )]->setEnabled ( !value );
		controlSpinbox[ OA_CAM_CTRL_MODIFIER_STD ]
			[ OA_CAM_CTRL_MODE_BASE( control )]->setEnabled ( !value );
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
    commonState.camera->controlDiscreteSet ( control, &count, &values );
    if ( value < count ) {
      value = values[value];
    } else {
      qWarning() << "Invalid menu value for discrete menu";
      return;
    }
  }
  commonState.camera->setControl ( control, value );
}


void
CameraSettings::updateFrameRate ( int index )
{
  frameRateSlider->setValue ( index );
}


void
CameraSettings::frameRateChanged ( void )
{
  trampolines->updateCameraFrameRate ( frameRateMenu->currentIndex());
}


void
CameraSettings::reconfigureControl ( int control )
{
  int min = trampolines->getCameraSpinboxMinimum ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMinimum ( min );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMinimum ( min );

  int max = trampolines->getCameraSpinboxMaximum ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMaximum ( max );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setMaximum ( max );

  int step = trampolines->getCameraSpinboxStep ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setSingleStep ( step );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setSingleStep ( step );

  int val = trampolines->getCameraSpinboxValue ( control );
  controlSlider[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setValue ( val );
  controlSpinbox[ OA_CAM_CTRL_MODIFIER( control )][ OA_CAM_CTRL_MODE_BASE(
      control )]->setValue ( val );
}


void
CameraSettings::forceFrameFormatChanged ( int newState )
{
  unsigned int oldState = 0;

  oldState = cameraConf.forceInputFrameFormat;
  if ( newState == Qt::Unchecked ) {
    cameraConf.forceInputFrameFormat = 0;
  } else {
    cameraConf.forceInputFrameFormat =
				selectedFrameFormat->currentIndex() + 1;
  }
  trampolines->updateForceFrameFormat ( oldState,
      cameraConf.forceInputFrameFormat );
}


void
CameraSettings::selectedFrameFormatChanged ( int newIndex )
{
  unsigned int oldState = 0;

  if ( cameraConf.forceInputFrameFormat ) {
    oldState = cameraConf.forceInputFrameFormat;
    cameraConf.forceInputFrameFormat = newIndex + 1;
    trampolines->updateForceFrameFormat ( oldState,
        cameraConf.forceInputFrameFormat );
  }
}
