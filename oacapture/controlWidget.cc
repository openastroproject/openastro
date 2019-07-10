/*****************************************************************************
 *
 * controlWidget.cc -- class for the control widget in the UI
 *
 * Copyright 2013,2014,2015,2017,2019
 *   James Fidell (james@openastroproject.org)
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
}

#include "commonState.h"
#include "commonConfig.h"

#include "configuration.h"
#include "controlWidget.h"
#include "state.h"

#define INTERVAL_USEC	0
#define INTERVAL_MSEC	1
#define INTERVAL_SEC	2
#define INTERVAL_MIN	3

ControlWidget::ControlWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  gainLabel = new QLabel ( tr ( "Gain" ), this );
  exposureLabel = new QLabel ( tr ( "Exposure" ), this );
  framerateLabel = new QLabel ( tr ( "Framerate (fps)" ), this );
  expRangeLabel = new QLabel ( tr ( "Exp. Range" ), this );
  autoLabel = new QLabel ( tr ( "Auto" ), this );

  exposureSlider = new QSlider ( Qt::Horizontal, this );
  framerateSlider = new QSlider ( Qt::Horizontal, this );

  exposureSpinbox = new QSpinBox ( this );
  framerateMenu = new QComboBox ( this );

  intervalSizeMenu = new QComboBox ( this );
  intervalMultipliers << 1 << 1000 << 1000000 << 60000000;
  intervalsList << "usec" << "msec" << "sec" << "min";
  intervalSizeMenu->addItems ( intervalsList );
  enabledIntervals << INTERVAL_USEC << INTERVAL_MSEC << INTERVAL_SEC <<
      INTERVAL_MIN;
  // this is only true for the moment as for specific cameras some options
  // may disappear
  intervalSizeMenu->setCurrentIndex ( config.intervalMenuOption );
  QString s = "Exp. Range (" + intervalsList[ config.intervalMenuOption ] + ")";
  expRangeLabel->setText ( tr ( s.toStdString().c_str() ));
  intervalSizeMenu->setEnabled ( 0 );

  selectableControlMenu[0] = new QComboBox ( this );
  selectableControlMenu[1] = new QComboBox ( this );
  selectableControlMenu[0]->setToolTip ( tr ( "Select control to display" ));
  selectableControlMenu[1]->setToolTip ( tr ( "Select control to display" ));

  selectableControlsAllowed[0][0] = config.selectableControl[0];
  selectableControlsAllowed[1][0] = config.selectableControl[1];
  selectableControlIndexes[0][ config.selectableControl[0]] = 0;
  selectableControlIndexes[1][ config.selectableControl[1]] = 1;

  int baseVal = OA_CAM_CTRL_MODE_BASE( config.selectableControl[0]);
  if ( baseVal > 0 && baseVal <OA_CAM_CTRL_LAST_P1 ) {
    selectableControlMenu[0]->addItem (
        tr ( oaCameraControlLabel[ baseVal ]));
    baseVal = OA_CAM_CTRL_MODE_BASE( config.selectableControl[1]);
    if ( baseVal > 0 && baseVal <OA_CAM_CTRL_LAST_P1 ) {
      selectableControlMenu[1]->addItem (
          tr ( oaCameraControlLabel[ baseVal ]));
    }
  }

  connect ( selectableControlMenu[0], SIGNAL( currentIndexChanged ( int )),
      this, SLOT ( setSelectableControl1 ( int )));
  connect ( selectableControlMenu[1], SIGNAL( currentIndexChanged ( int )),
      this, SLOT ( setSelectableControl2 ( int )));
  ignoreSelectableControlChanges = 1;

  expMenu = new QComboBox ( this );
  QStringList exposures;
  exposures << "0 - 5" << "5 - 30" << "30 - 100" << "100 - 1000";
  exposures << "1000 - 10000";
  minSettings[0] = 0;
  minSettings[1] = 5;
  minSettings[2] = 30;
  minSettings[3] = 100;
  maxSettings[0] = 5;
  maxSettings[1] = 30;
  maxSettings[2] = 100;
  maxSettings[3] = 1000;
  expMenu->addItems ( exposures );
  expMenu->setCurrentIndex ( config.exposureMenuOption );
  usingAbsoluteExposure = 0;
  ignoreExposureMenuChanges = 0;
  useExposureDropdown = 1;
  theoreticalFPSNumerator = theoreticalFPSDenominator = 1;

  intervalBox = new QHBoxLayout;
  intervalBox->addWidget ( exposureLabel );
  intervalBox->addWidget ( intervalSizeMenu );

  grid = new QGridLayout ( this );

  grid->addWidget ( autoLabel, 0, 1 );

  grid->addWidget ( gainLabel, 1, 0 );
  grid->addWidget ( selectableControlMenu[0], 2, 0 );
  if ( -1 == config.selectableControl[0] ) {
    selectableControlMenu[0]->hide();
  }
  grid->addWidget ( selectableControlMenu[1], 3, 0 );
  if ( -1 == config.selectableControl[1] ) {
    selectableControlMenu[1]->hide();
  }
  grid->addWidget ( framerateLabel, 4, 0 );
  grid->addLayout ( intervalBox, 5, 0 );
  grid->addWidget ( expRangeLabel, 6, 0 );
  
  grid->addWidget ( framerateSlider, 4, 2 );
  grid->addWidget ( exposureSlider, 5, 2 );
  grid->addWidget ( expMenu, 6, 2 );

  grid->addWidget ( framerateMenu, 4, 3 );
  grid->addWidget ( exposureSpinbox, 5, 3 );

  grid->setColumnStretch ( 2, 4 );

  // Now for the selectable slider we create a label, slider and spinbox
  // for each control relevant control.  When the control is selected we'll
  // display those control widgets.

  sliderSignalMapper = new QSignalMapper ( this );
  checkboxSignalMapper = new QSignalMapper ( this );
  for ( uint8_t c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    selectableControlSlider[ c ] = nullptr;
    selectableControlSpinbox[ c ] = nullptr;
    selectableControlCheckbox[ c ] = nullptr;

    // This is a bit hacky.  We create the control sliders and checkboxes
    // on the assumption that they will exist when a camera is connected
    // later, but we've no idea if that is actually the case or not

    if ( c != OA_CAM_CTRL_EXPOSURE_UNSCALED &&
        c != OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) {
      selectableControlSlider[ c ] = new QSlider ( Qt::Horizontal, this );
      selectableControlSlider[ c ]->setFocusPolicy ( Qt::TabFocus );
      selectableControlSpinbox[ c ] = new QSpinBox ( this );
      if ( c != config.selectableControl[0] &&
          c != config.selectableControl[1] ) {
        selectableControlSlider[ c ]->hide();
        selectableControlSpinbox[ c ]->hide();
      }

      sliderSignalMapper->setMapping ( selectableControlSpinbox[c], c );

      connect ( selectableControlSlider[ c ], SIGNAL( sliderMoved ( int )),
          selectableControlSpinbox[ c ], SLOT( setValue( int )));
      connect ( selectableControlSlider[ c ], SIGNAL( valueChanged ( int )),
          selectableControlSpinbox[ c ], SLOT( setValue( int )));
      connect ( selectableControlSpinbox[ c ], SIGNAL( valueChanged ( int )),
          selectableControlSlider[ c ], SLOT( setValue( int )));

      connect ( selectableControlSpinbox[ c ], SIGNAL( valueChanged ( int )),
          sliderSignalMapper, SLOT( map()));
    }

    if ( oaGetAutoForControl ( c )) {
      selectableControlCheckbox[ c ] = new QCheckBox ( "", this );
      selectableControlCheckbox[ c ]->hide();
      checkboxSignalMapper->setMapping ( selectableControlCheckbox[ c ], c );

      connect ( selectableControlCheckbox[ c ], SIGNAL (
          stateChanged ( int )), checkboxSignalMapper, SLOT( map()));
    }
  }
  connect ( sliderSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( updateSelectableControl ( int )));
  connect ( checkboxSignalMapper, SIGNAL( mapped ( int )), this,
      SLOT ( updateSelectableCheckbox ( int )));

  // Always add the gain control as a visible control
  if ( selectableControlCheckbox[ OA_CAM_CTRL_GAIN ] ) {
    grid->addWidget (
        selectableControlCheckbox[ OA_CAM_CTRL_GAIN ], 1, 1 );
  }
  if ( selectableControlSlider[ OA_CAM_CTRL_GAIN ] ) {
    grid->addWidget ( selectableControlSlider[ OA_CAM_CTRL_GAIN ], 1, 2 );
    grid->addWidget ( selectableControlSpinbox[ OA_CAM_CTRL_GAIN ], 1, 3 );
  }

  if ( config.selectableControl[0] > 0 &&
      config.selectableControl[0] < OA_CAM_CTRL_LAST_P1 ) {
    // FIX ME -- probably only one of these "if" conditions is required to
    // wrap them all.  I'm just being paranoid
    if ( selectableControlCheckbox[ config.selectableControl[0]] ) {
      grid->addWidget (
          selectableControlCheckbox[ config.selectableControl[0]], 2, 1 );
    }
    if ( selectableControlSlider[ config.selectableControl[0]] ) {
      grid->addWidget (
          selectableControlSlider[ config.selectableControl[0]], 2, 2 );
    }
    if ( selectableControlSpinbox[ config.selectableControl[0]] ) {
      grid->addWidget (
          selectableControlSpinbox[ config.selectableControl[0]], 2, 3 );
    }

  }
  if ( config.selectableControl[1] > 0 &&
      config.selectableControl[1] < OA_CAM_CTRL_LAST_P1 ) {
    // FIX ME -- probably only one of these "if" conditions is required to
    // wrap them all.  I'm just being paranoid
    if ( selectableControlCheckbox[ config.selectableControl[1]] ) {
      grid->addWidget (
          selectableControlCheckbox[ config.selectableControl[1]], 3, 1 );
    }
    if ( selectableControlSlider[ config.selectableControl[1]] ) {
      grid->addWidget (
          selectableControlSlider[ config.selectableControl[1]], 3, 2 );
    }
    if ( selectableControlSpinbox[ config.selectableControl[1]] ) {
      grid->addWidget (
          selectableControlSpinbox[ config.selectableControl[1]], 3, 3 );
    }
  }

  framerateLabel->hide();
  framerateSlider->hide();
  framerateMenu->hide();

  if ( !generalConf.dockableControls ) {
    setTitle ( tr ( "Control" ));
  }
  setLayout ( grid );

  connect ( selectableControlSlider[ OA_CAM_CTRL_GAIN ], SIGNAL(
      sliderMoved ( int )), selectableControlSpinbox[ OA_CAM_CTRL_GAIN ],
      SLOT( setValue( int )));
  connect ( selectableControlSlider[ OA_CAM_CTRL_GAIN ], SIGNAL(
      valueChanged ( int )), selectableControlSpinbox[ OA_CAM_CTRL_GAIN ],
      SLOT( setValue( int )));
  connect ( selectableControlSpinbox[ OA_CAM_CTRL_GAIN ], SIGNAL(
      valueChanged ( int )), selectableControlSlider[ OA_CAM_CTRL_GAIN ],
      SLOT( setValue( int )));
  connect ( selectableControlSpinbox[ OA_CAM_CTRL_GAIN ], SIGNAL(
      valueChanged ( int )), this, SLOT( updateGain ( int )));
  selectableControlSlider[ OA_CAM_CTRL_GAIN ]->setFocusPolicy ( Qt::TabFocus );

  exposureSlider->setFocusPolicy ( Qt::TabFocus );
  connect ( exposureSlider, SIGNAL( sliderMoved ( int )), exposureSpinbox,
      SLOT( setValue( int )));
  connect ( exposureSlider, SIGNAL( valueChanged ( int )), exposureSpinbox,
      SLOT( setValue( int )));
  connect ( exposureSpinbox, SIGNAL( valueChanged ( int )), exposureSlider,
      SLOT( setValue( int )));
  connect ( exposureSpinbox, SIGNAL( valueChanged ( int )), this,
      SLOT( updateExposure( int )));

  framerateSlider->setFocusPolicy ( Qt::TabFocus );
  connect ( framerateSlider, SIGNAL( sliderMoved ( int )), framerateMenu,
      SLOT( setCurrentIndex( int )));
  connect ( framerateSlider, SIGNAL( valueChanged ( int )), framerateMenu,
      SLOT( setCurrentIndex( int )));
  connect ( framerateSlider, SIGNAL( sliderReleased()), this,
      SLOT( frameRateChanged()));
  connect ( framerateMenu, SIGNAL( currentIndexChanged ( int )),
      framerateSlider, SLOT( setValue( int )));
  connect ( framerateMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( frameRateChanged()));

  // do these at the end so nothing prior to this triggers the slots to be
  // called

  connect ( expMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( exposureMenuChanged ( int )));
  connect ( intervalSizeMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( intervalMenuChanged ( int )));

  ignoreFrameRateChanges = ignoreSelectableControlChanges =
      ignoreIntervalChanges = ignoreExposureChanges = 0;
}


ControlWidget::~ControlWidget()
{
  state.mainWindow->destroyLayout (( QLayout* ) grid );
}


void
ControlWidget::configure ( void )
{
  int64_t	min, max, step, def;
  int		minOption, maxOption, readableControls;

	// There's a side-effect here in that hasReadableControls() returns 0
	// if the camera is not initialised, so we can test this and rely on not
	// attempting to read stuff off the camera if it isn't yet initialised.
  readableControls = commonState.camera->hasReadableControls();

  // Now we need to set up the menus for the two selectable sliders
  // from the controls available.
  //
  // Dump the existing menus, then create new ones based on the available
  // INT32/64 controls.  Use the existing controls for the current settings
  // if they're still present, otherwise default to the first available.
  // Make the selected controls in each menu unavailable in the other.
  //
  // Also need to hide the old sliders/spinboxes and remove them from
  // the grid widget, adding the new ones and showing them.

  // Step 1.  Clear the existing data

  ignoreSelectableControlChanges = 1;

  for ( int c = 0; c < 2; c++ ) {
    if ( config.selectableControl[c] > 0 &&
        config.selectableControl[c] < OA_CAM_CTRL_LAST_P1 ) {
      selectableControlMenu[c]->hide();
      selectableControlMenu[c]->clear();
      // FIX ME -- possibly a single "if" conditional wrapping all three
      // statements is sufficient here, but I'm being paranoid
      if ( selectableControlCheckbox[ config.selectableControl[c]] ) {
        selectableControlCheckbox[ config.selectableControl[c]]->hide();
      }
      if ( selectableControlSlider[ config.selectableControl[c]] ) {
        selectableControlSlider[ config.selectableControl[c]]->hide();
      }
      if ( selectableControlSpinbox[ config.selectableControl[c]] ) {
        selectableControlSpinbox[ config.selectableControl[c]]->hide();
      }
    }
  }

  // Step 2.  Find out if we can re-use the existing controls

  // Need to know what the preferred exposure control is going to be
  // here.

  state.preferredExposureControl = 0;
  if ( commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) {
    state.preferredExposureControl = OA_CAM_CTRL_EXPOSURE_ABSOLUTE;
  } else {
    if ( commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_UNSCALED )) {
      state.preferredExposureControl = OA_CAM_CTRL_EXPOSURE_UNSCALED;
    }
  }

  int replaceSelectable1 = 0;
  int type;
  if ( config.selectableControl[0] == -1 ) {
    // we can't allow this one
    replaceSelectable1 = -1;
  }
  type = 0;
  if ( config.selectableControl[0] >= 0 && config.selectableControl[0] !=
      state.preferredExposureControl ) {
    type = commonState.camera->hasControl ( config.selectableControl[0]);
  }
  if ( type != OA_CTRL_TYPE_INT32 && type != OA_CTRL_TYPE_INT64 ) {
    // we can't allow this one
    replaceSelectable1 = -1;
  }

  int replaceSelectable2 = 0;
  if ( config.selectableControl[1] == -1 ||
      config.selectableControl[1] == config.selectableControl[0] ) {
    // we can't allow this one
    replaceSelectable2 = -1;
  }
  type = 0;
  if ( config.selectableControl[1] >= 0 && config.selectableControl[1] !=
      state.preferredExposureControl ) {
    type = commonState.camera->hasControl ( config.selectableControl[1]);
  }
  if ( type != OA_CTRL_TYPE_INT32 && type != OA_CTRL_TYPE_INT64 ) {
    // we can't allow this one
    replaceSelectable2 = -1;
  }

  // Step 3.  Rebuild the menus whilst assigning new controls if required

  int c1, c2;
  c1 = c2 = 0;
  for ( int c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    selectableControlsAllowed[0][ c ] = -1; // -1 means not allowed in the menu
    selectableControlsAllowed[1][ c ] = -1;
    // the index of this control in each of the drop-downs
    selectableControlIndexes[0][ c ] = selectableControlIndexes[1][ c ] = -1;
    if ( c != OA_CAM_CTRL_GAIN && c != state.preferredExposureControl ) {
      type = commonState.camera->hasControl ( c );
      if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
        if ( replaceSelectable1 <= 0 ) {
          if ( replaceSelectable1 == 0 && c == config.selectableControl[0] ) {
            replaceSelectable1 = c;
          } else {
            replaceSelectable1 = config.selectableControl[0] = c;
            if ( c == config.selectableControl[1] ) {
              replaceSelectable2 = -1;
            }
          }
        } else {
          if ( replaceSelectable2 <= 0 && c != config.selectableControl[0] ) {
            if ( replaceSelectable2 == 0 && c == config.selectableControl[1] ) {
              replaceSelectable2 = c;
            } else {
              replaceSelectable2 = config.selectableControl[1] = c;
            }
          }
        }
        if ( c != replaceSelectable2 ) {
          selectableControlMenu[0]->addItem ( tr ( oaCameraControlLabel[c] ));
          selectableControlIndexes[0][ c ] = c1;
          selectableControlsAllowed[0][ c1 ] = c;
          c1++;
        }
        if ( c != replaceSelectable1 ) {
          selectableControlMenu[1]->addItem ( tr ( oaCameraControlLabel[c] ));
          selectableControlIndexes[1][ c ] = c2;
          selectableControlsAllowed[1][ c2 ] = c;
          c2++;
        }

        commonState.camera->controlRange ( c, &min, &max, &step, &def );
        selectableControlSlider[ c ]->setRange ( min, max );
        selectableControlSlider[ c ]->setSingleStep ( step );
        selectableControlSpinbox[ c ]->setRange ( min, max );
        selectableControlSpinbox[ c ]->setSingleStep ( step );
        if ( readableControls ) {
					cameraConf.CONTROL_VALUE( c ) = commonState.camera->readControl ( c );
				} else {
					cameraConf.CONTROL_VALUE( c ) = def;
				}
        selectableControlSlider[ c ]->setValue (
						cameraConf.CONTROL_VALUE( c ));
        selectableControlSpinbox[ c ]->setValue (
						cameraConf.CONTROL_VALUE( c ));
      }
    }
  }

  // Step 4, remove the current widgets from the grid and display the
  // correct ones

  grid->removeItem ( grid->itemAtPosition ( 2, 1 ));
  grid->removeItem ( grid->itemAtPosition ( 2, 2 ));
  grid->removeItem ( grid->itemAtPosition ( 2, 3 ));
  int autoctrl;
  if ( replaceSelectable1 >= 0 ) {
    selectableControlMenu[0]->show();
    autoctrl = oaGetAutoForControl ( config.selectableControl[0] );
    if ( selectableControlCheckbox[ config.selectableControl[0]] &&
        autoctrl >= 0 && commonState.camera->hasControl ( autoctrl ) ==
        OA_CTRL_TYPE_BOOLEAN ) {
      grid->addWidget (
          selectableControlCheckbox[ config.selectableControl[0]], 2, 1 );
      selectableControlCheckbox[ config.selectableControl[0]]->show();
    }
    grid->addWidget (
        selectableControlSlider[ config.selectableControl[0]], 2, 2 );
    selectableControlSlider[ config.selectableControl[0]]->show();
    grid->addWidget (
        selectableControlSpinbox[ config.selectableControl[0]], 2, 3 );
    selectableControlSpinbox[ config.selectableControl[0]]->show();
  }

  grid->removeItem ( grid->itemAtPosition ( 3, 1 ));
  grid->removeItem ( grid->itemAtPosition ( 3, 2 ));
  grid->removeItem ( grid->itemAtPosition ( 3, 3 ));
  if ( replaceSelectable2 >= 0 ) {
    selectableControlMenu[1]->show();
    autoctrl = oaGetAutoForControl ( config.selectableControl[1] );
    if ( selectableControlCheckbox[ config.selectableControl[1]] &&
        autoctrl >= 0 && commonState.camera->hasControl ( autoctrl ) ==
        OA_CTRL_TYPE_BOOLEAN ) {
      grid->addWidget (
          selectableControlCheckbox[ config.selectableControl[1]], 3, 1 );
      selectableControlCheckbox[ config.selectableControl[1]]->show();
    }
    grid->addWidget (
        selectableControlSlider[ config.selectableControl[1]], 3, 2 );
    selectableControlSlider[ config.selectableControl[1]]->show();
    grid->addWidget (
        selectableControlSpinbox[ config.selectableControl[1]], 3, 3 );
    selectableControlSpinbox[ config.selectableControl[1]]->show();
  }

  // Step 5, set the current items to be the visible ones in the menu

  if ( replaceSelectable1 >= 0 ) {
    selectableControlMenu[0]->setCurrentIndex (
        selectableControlIndexes[0][ config.selectableControl[0]]);
    if ( replaceSelectable2 >= 0 ) {
      selectableControlMenu[1]->setCurrentIndex (
          selectableControlIndexes[1][ config.selectableControl[1]]);
    } else {
      config.selectableControl[1] = -1;
    }
  } else {
    config.selectableControl[0] = -1;
    config.selectableControl[1] = -1;
  }

  // Step 6, set any auto controls to have the correct checked status
  // and enable/disable sliders and spinboxes appropriately

  int i;
  for ( i = 0; i < 2; i++ ) {
    // make sure they're enabled by default
    if ( config.selectableControl[i] >= 0 &&
        selectableControlSlider[ config.selectableControl[i]] ) {
      selectableControlSlider[ config.selectableControl[i]]->setEnabled ( 1 );
      selectableControlSpinbox[ config.selectableControl[i]]->setEnabled ( 1 );
      int32_t autoControl;
      if (( autoControl = oaGetAutoForControl ( config.selectableControl[i]))) {
        if ( selectableControlCheckbox[ config.selectableControl[i]] &&
            autoControl >= 0 && commonState.camera->hasControl (
						autoControl ) == OA_CTRL_TYPE_BOOLEAN ) {
					// FIX ME -- what if this is not boolean?
          uint32_t value;
					if ( readableControls ) {
						cameraConf.CONTROL_VALUE( autoControl ) =
								commonState.camera->readControl ( autoControl );
					} else {
						// Turn all the auto stuff off by default
						cameraConf.CONTROL_VALUE( autoControl ) = 0;
					}
					value = cameraConf.CONTROL_VALUE( autoControl );
          selectableControlCheckbox[ config.selectableControl[i]]->
              setChecked ( value );
          if ( selectableControlSlider[ config.selectableControl[i]] ) {
            selectableControlSlider[ config.selectableControl[i]]->
                setEnabled ( !value );
            selectableControlSpinbox[ config.selectableControl[i]]->
                setEnabled ( !value );
          }
        }
      }
    }
  }

  // And we're all done with the selectable controls

  ignoreSelectableControlChanges = 0;

  type = commonState.camera->hasControl ( OA_CAM_CTRL_GAIN );
  if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
    ignoreGainChanges = 1;
    commonState.camera->controlRange ( OA_CAM_CTRL_GAIN,
				&min, &max, &step, &def );
    // This used to be set by calling camera::readControl ( OA_CAM_CTRL_GAIN )
    // but that doesn't always work if the camera isn't running, so instead
    // we just set it to 50%
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_GAIN ) = ( max - min ) / 2;
    selectableControlSlider[ OA_CAM_CTRL_GAIN ]->setRange ( min, max );
    selectableControlSlider[ OA_CAM_CTRL_GAIN ]->setSingleStep ( step );
    selectableControlSpinbox[ OA_CAM_CTRL_GAIN ]->setRange ( min, max );
    selectableControlSpinbox[ OA_CAM_CTRL_GAIN ]->setSingleStep ( step );
    ignoreGainChanges = 0;
		if ( readableControls ) {
			cameraConf.CONTROL_VALUE( OA_CAM_CTRL_GAIN ) =
					commonState.camera->readControl ( OA_CAM_CTRL_GAIN );
		}
    selectableControlSpinbox[ OA_CAM_CTRL_GAIN ]->setValue (
        cameraConf.CONTROL_VALUE( OA_CAM_CTRL_GAIN ));
    selectableControlSlider[ OA_CAM_CTRL_GAIN ]->show();
    selectableControlSpinbox[ OA_CAM_CTRL_GAIN ]->show();
    if ( commonState.camera->hasControl (
        OA_CAM_CTRL_MODE_AUTO ( OA_CAM_CTRL_GAIN )) == OA_CTRL_TYPE_BOOLEAN ) {
      // FIX ME -- what if this is not boolean?
      uint32_t value;
      if ( readableControls ) {
        cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO ( OA_CAM_CTRL_GAIN )) =
            commonState.camera->readControl ( OA_CAM_CTRL_MODE_AUTO (
						OA_CAM_CTRL_GAIN ));
      } else {
				// Turn it off by default
        cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO ( OA_CAM_CTRL_GAIN )) =
						0;
			}
      value = cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO (
					OA_CAM_CTRL_GAIN ));
      selectableControlCheckbox[ OA_CAM_CTRL_GAIN ]->setChecked ( value );
      if ( selectableControlSlider[ OA_CAM_CTRL_GAIN ] ) {
        selectableControlSlider[ OA_CAM_CTRL_GAIN ]->setEnabled ( !value );
        selectableControlSpinbox[ OA_CAM_CTRL_GAIN ]->setEnabled ( !value );
      }
      selectableControlCheckbox[ OA_CAM_CTRL_GAIN ]->show();
    }
  } else {
    selectableControlSlider[ OA_CAM_CTRL_GAIN ]->hide();
    selectableControlSpinbox[ OA_CAM_CTRL_GAIN ]->hide();
    selectableControlCheckbox[ OA_CAM_CTRL_GAIN ]->hide();
  }

  // Exposure is a bit more messy.  If we have an absolute exposure time
  // then prefer that.  If we don't, but have some sort of exposure control
  // use that, but the labels will need adjusting.  If we have neither then
  // the controls need to be disabled.
  // At the same time we need to create a new set of labels for the
  // exposure combobox.

  int updateExposureControls = 0;
  int setting;

  if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == state.preferredExposureControl ) {
    // FIX ME -- what if the type of this control is not INT32 or INT64 ?
    commonState.camera->controlRange ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE,
				&min, &max, &step, &def );
    usingAbsoluteExposure = 1;

    int currIndex = 0, newIndex = 0;
    QStringList opts;

    // find the minimum and maximum interval menu options we can use for
    // the exposure times we have

    minOption = maxOption = -1;
    if ( min < intervalMultipliers[ INTERVAL_MSEC]) {
      minOption = INTERVAL_USEC;
    } else {
      if ( min < intervalMultipliers[ INTERVAL_SEC]) {
        minOption = INTERVAL_MSEC;
      } else {
        if ( min < intervalMultipliers[ INTERVAL_MIN]) {
          minOption = INTERVAL_SEC;
        } else {
          minOption = INTERVAL_MIN;
        }
      }
    }
    if ( max >= intervalMultipliers[ INTERVAL_MIN]) {
      maxOption = INTERVAL_MIN;
    } else {
      if ( max >= intervalMultipliers[ INTERVAL_MSEC]) {
        maxOption = INTERVAL_SEC;
      } else {
        if ( max >= intervalMultipliers[ INTERVAL_USEC] ) {
          maxOption = INTERVAL_MSEC;
        } else {
          maxOption = INTERVAL_USEC;
        }
      }
    }

    if ( config.intervalMenuOption < minOption ) {
      config.intervalMenuOption = minOption;
      SET_PROFILE_INTERVAL( minOption );
    }
    if ( config.intervalMenuOption > maxOption ) {
      config.intervalMenuOption = maxOption;
      SET_PROFILE_INTERVAL( maxOption );
    }

    enabledIntervals.clear();
    for ( int i = INTERVAL_USEC; i <= INTERVAL_MIN; i++ ) {
      if ( i >= minOption && i <= maxOption ) {
        opts << intervalsList[i];
        enabledIntervals << i;
        if ( i == config.intervalMenuOption ) {
          newIndex = currIndex;
        }
        currIndex++;
      }
    }

    ignoreIntervalChanges = 1;
    intervalSizeMenu->clear();
    intervalSizeMenu->addItems ( opts );
    intervalSizeMenu->setCurrentIndex ( newIndex );
    ignoreIntervalChanges = 0;
    intervalSizeMenu->setEnabled ( 1 );
    intervalSizeMenu->show();

		if ( readableControls ) {
			cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
				commonState.camera->readControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
		} else {
			cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = def;
		}
    setting = cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );

    switch ( config.intervalMenuOption ) {
      case INTERVAL_USEC:
        // min value is fine, but let's set a maximum of 10000us
        if ( max > 10000 ) {
          max = 10000;
        }
        break;

      case INTERVAL_MSEC:
        // The settings from liboacam are in units of 1us so we need
        // to convert those to milliseconds by dividing by 1000.
        min /= 1000;
        max /= 1000;
        // set a maximum of 10000ms
        if ( max > 10000 ) {
          max = 10000;
        }
        step /= 1000;
        setting /= 1000;
        break;

      case INTERVAL_SEC:
        // The settings from liboacam are in units of 1us so we need
        // to convert those to seconds by dividing by 1000000.
        min /= 1000000;
        max /= 1000000;
        step /= 1000000;
        setting /= 1000000;
        break;

      case INTERVAL_MIN:
        // The settings from liboacam are in units of 1us so we need
        // to convert those to minutes by dividing by 60000000.
        min /= 60000000;
        max /= 60000000;
        step /= 60000000;
        setting /= 60000000;
        break;
    }

    if ( min < 1 ) { min = 1; }
    if ( max < 1 ) { max = 1; }
    if ( step < 1 ) { step = 1; }
    if ( setting < min ) {
      setting = min;
    } else {
      if ( setting > max ) {
        setting = max;
      }
    }
    updateExposureControls = 1;
    QString s = "Exp. Range (" + intervalsList[ config.intervalMenuOption ] +
        ")";
    expRangeLabel->setText ( tr ( s.toStdString().c_str() ));
  } else {
    if ( OA_CAM_CTRL_EXPOSURE_UNSCALED == state.preferredExposureControl ) {
      // FIX ME -- what if the type of this control is not INT32 or INT64 ?
      usingAbsoluteExposure = 0;
      commonState.camera->controlRange ( OA_CAM_CTRL_EXPOSURE_UNSCALED,
					&min, &max, &step, &def );
			if ( readableControls ) {
				cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED ) =
						commonState.camera->readControl ( OA_CAM_CTRL_EXPOSURE_UNSCALED );
			} else {
				cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED ) = def;
			}
      setting = cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED );
      if ( setting < min ) {
        setting = min;
      } else {
        if ( setting > max ) {
          setting = max;
        }
      }
      updateExposureControls = 1;
      expRangeLabel->setText ( tr ( "Exp. Range" ));
    } else {
      exposureSlider->hide();
      exposureSpinbox->hide();
      expRangeLabel->hide();
      expMenu->hide();
    }

    intervalSizeMenu->setEnabled ( 0 );
    intervalSizeMenu->hide();
  }

  if ( updateExposureControls ) {

    // now we need to work out the ranges for the combobox
    // if the range is no greater than 256 we hide the combobox and
    // set the slider to the appropriate range.
    // otherwise for non-absolute exposures we divide the range up into
    // five ranges and for absolute have increasing wider scales

    int diff = max - min + 1;
    int showMin = 1, showMax = 1;
    if ( usingAbsoluteExposure || ( diff / step + 1 ) > 256 ) {

      QStringList exposures;
      int v1 = min;
      int v2;
      int items = 0, showItem = 0;
      do {
        if ( usingAbsoluteExposure ) {
          v2 = v1;
          if ( !v2 ) { v2++; }
          v2 *= 10;
          if ( v2 > max ) { v2 = max; }
        } else {
         v2 = v1 + diff / 5;
        }
        exposures << QString::number ( v1 ) + " - " + QString::number ( v2 );
        if ( setting >= v1 && setting <= v2 ) {
          showMin = v1;
          showMax = v2;
          showItem = items;
        }
        minSettings[ items ] = v1;
        maxSettings[ items ] = v2;
        items++;
        v1 = v2;
        // we have to stop this somewhere...
      } while ( items < 7 && v2 < max );

      // that should be the string list done.  now delete all the old items
      // and add the new string list and set the current item

      ignoreExposureMenuChanges = 1;
      expMenu->clear();
      expMenu->addItems ( exposures );
      config.exposureMenuOption = showItem;
      ignoreExposureMenuChanges = 0;
      expMenu->setCurrentIndex ( showItem );
      expRangeLabel->show();
      expMenu->show();
      useExposureDropdown = 1;
    } else {
      showMin = min;
      showMax = max;
      expRangeLabel->hide();
      expMenu->hide();
      useExposureDropdown = 0;
    }

    // finally we can set the sliders up

    ignoreExposureChanges = 1;
    exposureSlider->setRange ( showMin, showMax );
    exposureSlider->setSingleStep ( step );
    exposureSlider->setValue ( setting );
    exposureSpinbox->setRange ( showMin, showMax );
    exposureSpinbox->setSingleStep ( step );
    ignoreExposureChanges = 0;
    exposureSpinbox->setValue ( setting );
    exposureSlider->show();
    exposureSpinbox->show();

    if ( usingAbsoluteExposure ) {
      if ( setting ) {
        state.cameraWidget->showFPSMaxValue ( 1000000 /
            intervalMultipliers [ config.intervalMenuOption ] / setting );
      }
    } else {
      state.cameraWidget->clearFPSMaxValue();
    }
  }

  grid->removeItem ( grid->itemAtPosition ( 5, 1 ));
  // FIX ME -- Should handle auto exposure options that aren't just
  // boolean and menu?
	uint32_t control = OA_CAM_CTRL_MODE_AUTO( state.preferredExposureControl );
	type = commonState.camera->hasControl ( control );
	if ( type == OA_CTRL_TYPE_BOOLEAN || type == OA_CTRL_TYPE_MENU ) {
    // FIX ME -- what if there is no non-auto control?  Issue #131
    if ( commonState.camera->hasControl ( state.preferredExposureControl )) {
      uint32_t autoOn = 0;
      grid->addWidget (
          selectableControlCheckbox[ state.preferredExposureControl ], 5, 1 );
      if ( readableControls ) {
        cameraConf.CONTROL_VALUE( control ) =
            commonState.camera->readControl ( control );
      }
			switch ( type ) {
				case OA_CTRL_TYPE_BOOLEAN:
					autoOn = cameraConf.CONTROL_VALUE( control );
					break;
				case OA_CTRL_TYPE_MENU:
					autoOn = ( cameraConf.CONTROL_VALUE( control ) ==
							OA_EXPOSURE_MANUAL ) ? 0 : 1;
					break;
			}
      selectableControlCheckbox[ state.preferredExposureControl ]->
					setChecked ( autoOn );
      exposureSlider->setEnabled ( !autoOn );
      exposureSpinbox->setEnabled ( !autoOn );
      intervalSizeMenu->setEnabled ( !autoOn );
      expMenu->setEnabled ( !autoOn );
      selectableControlCheckbox[ state.preferredExposureControl ]->show();
    }
  }

	// Now run through all possible controls and set all of those that are
	// boolean and we haven't already dealt with to their default values

  for ( int c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
		for ( int m = OA_CAM_CTRL_MODIFIER_STD; m < OA_CAM_CTRL_MODIFIERS_P1;
        m++ ) {
			int control = ( m << 8 ) + c;
			if ( OA_CTRL_TYPE_BOOLEAN == commonState.camera->hasControl ( control )) {
				if ( c != OA_CAM_CTRL_GAIN && c != state.preferredExposureControl &&
						c != config.selectableControl[0] &&
						c != config.selectableControl[1] ) {
					int64_t min, max, step, def;

					commonState.camera->controlRange ( c, &min, &max, &step, &def );
          cameraConf.CONTROL_VALUE( control ) = def;
				  SET_PROFILE_CONTROL( control, def );
					commonState.camera->setControl ( control, def );
					if ( state.settingsWidget ) {
						state.settingsWidget->updateControl ( control, def );
					}
				}
			}
		}
	}

  updateFrameRates();

  if ( !commonState.camera->hasFrameRateSupport()) {
    if ( usingAbsoluteExposure ) {
      theoreticalFPSNumerator =
          cameraConf.CONTROL_VALUE ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
      if ( !theoreticalFPSNumerator ) {
        theoreticalFPSNumerator = 1;
      }
      theoreticalFPSDenominator = 1000000;
      if ( theoreticalFPSNumerator && theoreticalFPSDenominator ) {
        while ( theoreticalFPSNumerator % 10 == 0 &&
            theoreticalFPSDenominator % 10 == 0 ) {
          theoreticalFPSNumerator /= 10;
          theoreticalFPSDenominator /= 10;
        }
      }
    } else {
      // we don't really have a clue here, so set it to 1?
      theoreticalFPSNumerator = 1;
      theoreticalFPSDenominator = 1;
      theoreticalFPS = 1;
    }
  }
}


void
ControlWidget::updateFrameRates ( void )
{
  if ( !commonState.camera->isInitialised()) {
    return;
  }
  if ( !commonState.camera->hasFrameRateSupport()) {
    framerateLabel->hide();
    framerateSlider->hide();
    framerateMenu->hide();
    return;
  }

  if ( commonState.camera->hasFixedFrameRates ( commonConfig.imageSizeX,
      commonConfig.imageSizeY )) {

    // Ugly sorting of the frame rates here.  We don't know what
    // order we'll get them in so we have to sort them into rate
    // order for the slider to work.

    const FRAMERATES* rates = commonState.camera->frameRates (
				commonConfig.imageSizeX, commonConfig.imageSizeY );

    // don't use the frame rate slider if there's only one frame rate
    if ( 1 == rates->numRates ) {
      framerateLabel->hide();
      framerateSlider->hide();
      framerateMenu->hide();
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
      if ( *n == commonConfig.frameRateNumerator &&
          *d == commonConfig.frameRateDenominator ) {
        showItem = numItems;
      }
      numItems++;
    }

    ignoreFrameRateChanges = 1;
    framerateMenu->clear();
    framerateMenu->addItems ( rateList );
    framerateSlider->setRange ( 0, numItems - 1 );
    framerateSlider->setSingleStep ( 1 );
    commonConfig.frameRateNumerator = frameRateNumerator[ showItem ];
    commonConfig.frameRateDenominator = frameRateDenominator[ showItem ];
    SET_PROFILE_CONFIG( frameRateNumerator, commonConfig.frameRateNumerator );
    SET_PROFILE_CONFIG( frameRateDenominator,
				commonConfig.frameRateDenominator );
    ignoreFrameRateChanges = 0;

    // Handle only having one frame rate
    if ( numItems == 1 ) {
      _doFrameRateChange ( 0, 0 );
      return;
    }

    framerateMenu->setCurrentIndex ( showItem );
    // unfortunately the above statement may not change the frame rate even
    // though the rate has changed, because its position in the menu might
    // be the same, so we have to do the update manually;
    frameRateChanged();

    framerateLabel->show();
    framerateSlider->show();
    framerateMenu->show();

  } else {
    fprintf ( stderr, "Camera::hasFixedFrameRates failed\n" );
    framerateLabel->hide();
    framerateSlider->hide();
    framerateMenu->hide();
  }
}


void
ControlWidget::exposureMenuChanged ( int index )
{
  if ( ignoreExposureMenuChanges ) {
    return;
  }

  int newMin = minSettings[ index ];
  int newMax = maxSettings[ index ];
  int newSetting, control;

  control = usingAbsoluteExposure ? OA_CAM_CTRL_EXPOSURE_ABSOLUTE :
      OA_CAM_CTRL_EXPOSURE_UNSCALED;
  newSetting = cameraConf.CONTROL_VALUE( control );
  if ( newSetting < newMin ) { newSetting = newMin; }
  if ( newSetting > newMax ) { newSetting = newMax; }
  
  exposureSlider->setRange ( newMin, newMax );
  exposureSlider->setValue ( newSetting );
  exposureSpinbox->setRange ( newMin, newMax );
  exposureSpinbox->setValue ( newSetting );

  ignoreExposureChanges = 1;
  if ( state.settingsWidget ) {
    state.settingsWidget->reconfigureControl ( control );
  }
  ignoreExposureChanges = 0;

  if ( !commonState.camera->hasFrameRateSupport()) {
    if ( usingAbsoluteExposure ) {
      theoreticalFPSNumerator = newSetting *
          intervalMultipliers [ config.intervalMenuOption ];
      theoreticalFPSDenominator = 1000000;
      if ( theoreticalFPSNumerator && theoreticalFPSDenominator ) {
        while ( theoreticalFPSNumerator % 10 == 0 &&
            theoreticalFPSDenominator % 10 == 0 ) {
          theoreticalFPSNumerator /= 10;
          theoreticalFPSDenominator /= 10;
        }
      }
      theoreticalFPS = 1000000 /
          intervalMultipliers [ config.intervalMenuOption ] / newSetting;
    } else {
      // we don't really have a clue here, so set it to 1?
      theoreticalFPSNumerator = 1;
      theoreticalFPSDenominator = 1;
      theoreticalFPS = 1;
    }
  }
  if ( usingAbsoluteExposure ) {
    if ( newSetting ) {
      state.cameraWidget->showFPSMaxValue ( 1000000 /
          intervalMultipliers [ config.intervalMenuOption ] / newSetting );
    }
  } else {
    state.cameraWidget->clearFPSMaxValue();
  }
}


void
ControlWidget::updateExposure ( int value )
{
  int64_t usecValue;

  if ( ignoreExposureChanges ) {
    return;
  }

  if ( usingAbsoluteExposure ) {
    usecValue = value * intervalMultipliers [ config.intervalMenuOption ];
    if ( value ) {
      state.cameraWidget->showFPSMaxValue ( 1000000 / usecValue );
    }
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = value;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_EXPOSURE_ABSOLUTE, value );
    // convert value back to microseconds from milliseconds
    commonState.camera->setControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE, usecValue );
    if ( !commonState.camera->hasFrameRateSupport()) {
      theoreticalFPSNumerator = usecValue;
      theoreticalFPSDenominator = 1000000;
      if ( theoreticalFPSNumerator && theoreticalFPSDenominator ) {
        while ( theoreticalFPSNumerator % 10 == 0 &&
            theoreticalFPSDenominator % 10 == 0 ) {
          theoreticalFPSNumerator /= 10;
          theoreticalFPSDenominator /= 10;
        }
      }
    }
    if ( state.settingsWidget ) {
      state.settingsWidget->updateControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE,
          value );
    }
  } else {
    state.cameraWidget->clearFPSMaxValue();
    commonState.camera->setControl ( OA_CAM_CTRL_EXPOSURE_UNSCALED, value );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED ) = value;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_EXPOSURE_UNSCALED, value );
    if ( state.settingsWidget ) {
      state.settingsWidget->updateControl ( OA_CAM_CTRL_EXPOSURE_UNSCALED,
          value );
    }
  }
}


void
ControlWidget::frameRateChanged ( void )
{
  if ( ignoreFrameRateChanges ) {
    return;
  }
  int index = framerateSlider->sliderPosition();
  if ( framerateSlider->isSliderDown()) {
    return;
  }

  _doFrameRateChange ( index, 1 );
}


void
ControlWidget::_doFrameRateChange ( int index, int updateUI )
{
  /*
   * Leaving this in means the first frame setting for a camera breaks
   *
  if ( config.frameRateNumerator == frameRateNumerator[ index ] &&
      config.frameRateDenominator == frameRateDenominator[ index ] ) {
    return;
  }
  */

  commonConfig.frameRateNumerator = frameRateNumerator[ index ];
  commonConfig.frameRateDenominator = frameRateDenominator[ index ];
  SET_PROFILE_CONFIG( frameRateNumerator, commonConfig.frameRateNumerator );
  SET_PROFILE_CONFIG( frameRateDenominator, commonConfig.frameRateDenominator );
  theoreticalFPSNumerator = commonConfig.frameRateNumerator;
  theoreticalFPSDenominator = commonConfig.frameRateDenominator;
  theoreticalFPS = commonConfig.frameRateDenominator /
      commonConfig.frameRateNumerator;

  if ( updateUI && state.settingsWidget ) {
    state.settingsWidget->updateFrameRate ( index );
  }

  // FIX ME -- this last bit of the function could be tidier

  if ( !commonState.camera->isInitialised()) {
    return;
  }

  commonState.camera->setFrameInterval ( commonConfig.frameRateNumerator,
      commonConfig.frameRateDenominator );
}


void
ControlWidget::updateGain ( int value )
{
  if ( !ignoreGainChanges ) {
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_GAIN ) = value;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_GAIN, value );
    commonState.camera->setControl ( OA_CAM_CTRL_GAIN, value );
    if ( state.settingsWidget ) {
      state.settingsWidget->updateControl ( OA_CAM_CTRL_GAIN, value );
    }
  }
}


void
ControlWidget::updateSelectableControl ( int control )
{
  if ( !ignoreSelectableControlChanges ) {
    int value = selectableControlSpinbox[ control ]->value();
    cameraConf.CONTROL_VALUE( control ) = value;
    SET_PROFILE_CONTROL( control, value );
    commonState.camera->setControl ( control, value );
    if ( state.settingsWidget ) {
      state.settingsWidget->updateControl ( control, value );
    }
  }
}


void
ControlWidget::updateSelectableCheckbox ( int control )
{
  int autoControl = oaGetAutoForControl ( control );
  int value = ( selectableControlCheckbox[ control ]->isChecked()) ? 1 : 0;
  int origValue = value;
	int	isExposure = 0;

  if (( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == autoControl
			|| OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) ==
			autoControl )) {
		switch ( commonState.camera->hasControl ( autoControl )) {
			case OA_CTRL_TYPE_BOOLEAN:
				break;
			case OA_CTRL_TYPE_MENU:
				value = value ? OA_EXPOSURE_AUTO : OA_EXPOSURE_MANUAL;
				break;
			default:
				qWarning() << "Unhandled exposure auto type in" << __FUNCTION__;
				return;
				break;
		}
		isExposure = 1;
  }
  cameraConf.CONTROL_VALUE( autoControl ) = value;
  SET_PROFILE_CONTROL( autoControl, value );
  commonState.camera->setControl ( autoControl, value );
	if ( isExposure ) {
		exposureSlider->setEnabled ( !origValue );
		exposureSpinbox->setEnabled ( !origValue );
		expMenu->setEnabled ( !origValue );
		intervalSizeMenu->setEnabled ( !origValue );
	} else {
		if ( selectableControlSlider[ control ] ) {
			selectableControlSlider[ control ]->setEnabled ( !value );
			selectableControlSpinbox[ control ]->setEnabled ( !value );
		}
	}
  if ( state.settingsWidget ) {
    state.settingsWidget->updateControl ( autoControl, origValue );
  }
}


void
ControlWidget::enableFPSControl ( int state )
{
  framerateSlider->setEnabled ( state );
  framerateMenu->setEnabled ( state );
}


int
ControlWidget::getFPSNumerator ( void )
{
  return theoreticalFPSNumerator;
}


int
ControlWidget::getFPSDenominator ( void )
{
  return theoreticalFPSDenominator;
}


void
ControlWidget::updateFromConfig ( void )
{
  int setFrameRate = -1;
  int exposureSetting = -1;
  int64_t min, max, step, def;
  int type;

  if ( commonState.camera->isInitialised()) {

    for ( int c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
      type = commonState.camera->hasControl ( c );
      if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {

        commonState.camera->controlRange ( c, &min, &max, &step, &def );

        // FIX ME -- should this perhaps also happen for EXPOSURE_UNSCALED?

        if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == c ) {
          // FIX ME -- why check the type again?
          type = commonState.camera->hasControl (
							OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
          if ( OA_CTRL_TYPE_INT32 == type ||
              OA_CTRL_TYPE_INT64 == type ) {
            commonState.camera->controlRange ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE,
								&min, &max, &step, &def );
            // convert from microseconds to current interval type
            min /= intervalMultipliers [ config.intervalMenuOption ];
            if ( min < 1 ) { min = 1; }
            max /= intervalMultipliers [ config.intervalMenuOption ];
            if ( max < 1 ) { max = 1; }
 
            if ( cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) <
								min ) {
              cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = min;
              SET_PROFILE_CONTROL( OA_CAM_CTRL_EXPOSURE_ABSOLUTE, min );
            }
            if ( cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) >
								max ) {
              cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = max;
              SET_PROFILE_CONTROL( OA_CAM_CTRL_EXPOSURE_ABSOLUTE, max );
            }
          }
        } else {
          if ( cameraConf.CONTROL_VALUE( c ) < min ) {
            cameraConf.CONTROL_VALUE( c ) = min;
            SET_PROFILE_CONTROL( c, min );
          }
          if ( cameraConf.CONTROL_VALUE( c ) > max ) {
            cameraConf.CONTROL_VALUE( c ) = max;
            SET_PROFILE_CONTROL( c, max );
          }
        }
      }
    }

    type = commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
    if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
      exposureSetting = cameraConf.CONTROL_VALUE(
					OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
    } else {
    type = commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_UNSCALED );
      if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
        exposureSetting = cameraConf.CONTROL_VALUE(
						OA_CAM_CTRL_EXPOSURE_UNSCALED );
      }
    }

#ifdef BUGGY_CODE
    if ( commonState.camera->hasFrameRateSupport()) {
      updateFrameRates();

      const FRAMERATES* rates = commonState.camera->frameRates (
					config.imageSizeX, config.imageSizeY );

      for ( unsigned int i = 0; setFrameRate < 0 && i < rates->numRates; i++ ) {
        if ( rates->rates[i].numerator == config.frameRateNumerator &&
            rates->rates[i].denominator == config.frameRateDenominator ) {
          setFrameRate = i;
        }
      }
    }
#endif /* BUGGY_CODE */
  }

  for ( int c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE != c &&
        OA_CAM_CTRL_EXPOSURE_UNSCALED != c ) {
      type = commonState.camera->hasControl ( c );
      if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
        selectableControlSpinbox[ c ]->setValue (
						cameraConf.CONTROL_VALUE( c ));
      }
    }
  }

  if ( useExposureDropdown ) {
    int foundDropdownValue = -1;
    int numItems = expMenu->count();
    for ( int i = 0; i < numItems; i++ ) {
      if ( exposureSetting >= minSettings[i] && exposureSetting <=
          maxSettings[i] ) {
        foundDropdownValue = i;
      }
    }
    if ( foundDropdownValue == -1 ) {
      qWarning() << "can't find new exposure setting in dropdown";
    } else {
      disconnect ( expMenu, SIGNAL( currentIndexChanged ( int )), this,
          SLOT( exposureMenuChanged ( int )));
      disconnect ( exposureSlider, SIGNAL( sliderMoved ( int )),
          exposureSpinbox, SLOT( setValue( int )));
      disconnect ( exposureSlider, SIGNAL( valueChanged ( int )),
          exposureSpinbox, SLOT( setValue( int )));
      expMenu->setCurrentIndex ( foundDropdownValue );
      exposureSlider->setRange ( minSettings[ foundDropdownValue ],
          maxSettings[ foundDropdownValue ] );
      exposureSpinbox->setRange ( minSettings[ foundDropdownValue ],
          maxSettings[ foundDropdownValue ] );
      connect ( expMenu, SIGNAL( currentIndexChanged ( int )), this,
          SLOT( exposureMenuChanged ( int )));
      connect ( exposureSlider, SIGNAL( sliderMoved ( int )), exposureSpinbox,
          SLOT( setValue( int )));
      connect ( exposureSlider, SIGNAL( valueChanged ( int )), exposureSpinbox,
          SLOT( setValue( int )));
    }
  }
  exposureSpinbox->setValue ( exposureSetting );

  if ( setFrameRate >= 0 ) {
    framerateMenu->setCurrentIndex ( setFrameRate );
  }

  if ( usingAbsoluteExposure ) {
    if ( exposureSetting ) {
      state.cameraWidget->showFPSMaxValue ( 1000000 /
          intervalMultipliers [ config.intervalMenuOption ] / exposureSetting );
    }
  } else {
    state.cameraWidget->clearFPSMaxValue();
  }

}


void
ControlWidget::setSelectableControl1 ( int index )
{
  _setSelectableControl ( 0, index );
}


void
ControlWidget::setSelectableControl2 ( int index )
{
  _setSelectableControl ( 1, index );
}


void
ControlWidget::_setSelectableControl ( int selector, int index )
{
  if ( ignoreSelectableControlChanges ) {
    return;
  }
  ignoreSelectableControlChanges = 1;

  // The easy bits -- change the current control, drop the old widgets and
  // display the new ones

  int thisOne = selector;
  int theOther = 1 - selector;

  if ( selectableControlCheckbox[ config.selectableControl[ thisOne ]] ) {
    selectableControlCheckbox[ config.selectableControl[ thisOne ]]->hide();
  }
  selectableControlSlider[ config.selectableControl[ thisOne ]]->hide();
  selectableControlSpinbox[ config.selectableControl[ thisOne ]]->hide();

  config.selectableControl[ thisOne ] =
      selectableControlsAllowed[ thisOne ][ index ];

  grid->removeItem ( grid->itemAtPosition ( thisOne + 2, 1 ));
  grid->removeItem ( grid->itemAtPosition ( thisOne + 2, 2 ));
  grid->removeItem ( grid->itemAtPosition ( thisOne + 2, 3 ));

  grid->addWidget (
      selectableControlSlider[ config.selectableControl[ thisOne ]],
      thisOne + 2, 2 );
  grid->addWidget (
      selectableControlSpinbox[ config.selectableControl[ thisOne ]],
      thisOne + 2, 3 );
  selectableControlSlider[ config.selectableControl[ thisOne ]]->show();
  selectableControlSpinbox[ config.selectableControl[ thisOne ]]->show();
  // Enable these by default
  selectableControlSlider[ config.selectableControl[ thisOne ]]->
      setEnabled( 1 );
  selectableControlSpinbox[ config.selectableControl[ thisOne ]]->
      setEnabled( 1 );

  int autoctrl;
  autoctrl = oaGetAutoForControl ( config.selectableControl[ thisOne ] );
  if ( selectableControlCheckbox[ config.selectableControl[ thisOne ]] &&
      autoctrl >= 0 && commonState.camera->hasControl ( autoctrl )) {
    grid->addWidget (
        selectableControlCheckbox[ config.selectableControl[ thisOne ]],
        thisOne + 2, 1 );
    selectableControlCheckbox[ config.selectableControl[ thisOne ]]->show();
    // If we have an auto control, make sure the checkbox is set correctly
    // and that the slider/spinbox are appropriately enabled
    uint32_t value = cameraConf.CONTROL_VALUE( autoctrl );
    selectableControlCheckbox[ config.selectableControl[ thisOne ]]->
        setChecked ( value );
    selectableControlSlider[ config.selectableControl[ thisOne ]]->
        setEnabled ( !value );
    selectableControlSpinbox[ config.selectableControl[ thisOne ]]->
        setEnabled ( !value );
  }

  // Now the entire second menu needs rebuilding from the available
  // controls (minus the one we just chose).

  selectableControlMenu[ theOther ]->clear();

  int c2 = 0, type;
  for ( int c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    selectableControlIndexes[ theOther ][ c ] = -1;
    selectableControlsAllowed[ theOther ][ c ] = 0;
    if ( c != OA_CAM_CTRL_GAIN && c != state.preferredExposureControl ) {
      type = commonState.camera->hasControl ( c );
      if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
        if ( c != config.selectableControl[ thisOne ]) {
          selectableControlMenu[ theOther ]->addItem (
              tr ( oaCameraControlLabel[c] ));
          selectableControlIndexes[ theOther ][ c ] = c2;
          selectableControlsAllowed[ theOther ][ c2 ] = c;
          c2++;
        }
      }
    }
  }

  selectableControlMenu[ theOther ]->setCurrentIndex (
      selectableControlIndexes[ theOther ][
      config.selectableControl[ theOther ]]);

  ignoreSelectableControlChanges = 0;
}


int
ControlWidget::getSpinboxMinimum ( int control )
{
  // FIX ME -- at some point merge the gain and exposure sliders and
  // spinboxes with the others to avoid these special cases
  switch ( control ) {
    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      return exposureSpinbox->minimum();
      break;
    default:
      if ( !selectableControlSpinbox[ control ] ) {
        qWarning() << "ControlWidget::getSpinboxMinimum on unset control"
            << control;
        return 0;
      }
      return selectableControlSpinbox[ control ]->minimum();
      break;
  }
}


int
ControlWidget::getSpinboxMaximum ( int control )
{
  switch ( control ) {
    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      return exposureSpinbox->maximum();
      break;
    default:
      if ( !selectableControlSpinbox[ control ] ) {
        qWarning() << "ControlWidget::getSpinboxMaximum on unset control"
            << control;
        return 0;
      }
      return selectableControlSpinbox[ control ]->maximum();
      break;
  }
}


int
ControlWidget::getSpinboxValue ( int control )
{
  switch ( control ) {
    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      return exposureSpinbox->value();
      break;
    default:
      if ( !selectableControlSpinbox[ control ] ) {
        qWarning() << "ControlWidget::getSpinboxValue on unset control"
            << control;
        return 0;
      }
      return selectableControlSpinbox[ control ]->value();
      break;
  }
}


int
ControlWidget::getSpinboxStep ( int control )
{
  switch ( control ) {
    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      return exposureSpinbox->singleStep();
      break;
    default:
      if ( !selectableControlSpinbox[ control ] ) {
        qWarning() << "ControlWidget::getSpinboxStep on unset control"
            << control;
        return 0;
      }
      return selectableControlSpinbox[ control ]->singleStep();
      break;
  }
}


void
ControlWidget::updateSpinbox ( int control, int value )
{
  switch ( control ) {
    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      exposureSpinbox->setValue ( value );
      break;
    default:
      if ( !selectableControlSpinbox[ control ] ) {
        qWarning() << "ControlWidget::updateSpinbox on unset control"
            << control;
        return;
      }
      selectableControlSpinbox[ control ]->setValue ( value );
      break;
  }
}


void
ControlWidget::updateCheckbox ( int control, int value )
{
  if ( OA_CAM_CTRL_IS_AUTO ( control )) {
    int baseControl = OA_CAM_CTRL_MODE_BASE ( control );
    selectableControlCheckbox[ baseControl ]->setChecked ( value );
  } else {
    if ( OA_CAM_CTRL_IS_ON_OFF ( control )) {
      int baseControl = OA_CAM_CTRL_MODE_BASE ( control );
      if ( selectableControlSlider[ baseControl ] ) {
        selectableControlSlider[ baseControl ]->setEnabled ( value );
        selectableControlSpinbox[ baseControl ]->setEnabled ( value );
      }
      if ( selectableControlCheckbox[ baseControl ] ) {
        selectableControlCheckbox[ baseControl ]->setEnabled ( value );
      }
    }
    cameraConf.CONTROL_VALUE( control ) = value;
    SET_PROFILE_CONTROL( control, value );
    commonState.camera->setControl ( control, value );
  }
}


QStringList
ControlWidget::getFrameRates()
{
  int i, n;
  QStringList rates;

  n = framerateMenu->count();
  for ( i = 0; i < n; i++ ) {
    rates << framerateMenu->itemText ( i );
  }

  return rates;
}


void
ControlWidget::updateFrameRate ( int index )
{
  framerateSlider->setValue ( index );
}


unsigned int
ControlWidget::getFrameRateIndex ( void )
{
  return framerateSlider->value();
}


void
ControlWidget::disableAutoControls ( void )
{
  // These ones we just want off all the time
  if ( commonState.camera->hasControl (
				OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ))) {
    commonState.camera->setControl (
				OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN )) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ), 0 );
  }
  if ( commonState.camera->hasControl (
				OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ))) {
    commonState.camera->setControl (
				OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE )) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ), 0 );
  }
  if ( commonState.camera->hasControl (
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ))) {
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
					OA_CAM_CTRL_BRIGHTNESS )) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ), 0 );
  }
  if ( commonState.camera->hasControl (
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ))) {
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ),
        OA_EXPOSURE_MANUAL );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
        OA_CAM_CTRL_EXPOSURE_UNSCALED )) = OA_EXPOSURE_MANUAL;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ),
       OA_EXPOSURE_MANUAL );
  }
	int newval = -1;
	switch ( commonState.camera->hasControl ( 
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ))) {
		case OA_CTRL_TYPE_BOOLEAN:
			newval = 0;
			break;
		case OA_CTRL_TYPE_MENU:
			newval = OA_EXPOSURE_MANUAL;
			break;
	}
	if ( newval >= 0 ) {
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ), newval );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
        OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) = newval;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ),
       newval );
  }
  if ( commonState.camera->hasControl (
				OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ))) {
    commonState.camera->setControl (
				OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA )) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ), 0 );
  }
  int AWBtype = commonState.camera->hasControl (
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ));
  if ( AWBtype ) {
    int AWBManual = 0;
    if ( OA_CTRL_TYPE_MENU == AWBtype ) {
      AWBManual = commonState.camera->getAWBManualSetting();
    }
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ), AWBManual );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
					OA_CAM_CTRL_WHITE_BALANCE )) =
        AWBManual;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ),
        AWBManual );
  }
  if ( commonState.camera->hasControl (
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ))) {
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
					OA_CAM_CTRL_RED_BALANCE )) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ), 0 );
  }
  if ( commonState.camera->hasControl (
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ))) {
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
					OA_CAM_CTRL_BLUE_BALANCE)) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ), 0 );
  }
  if ( commonState.camera->hasControl (
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ))) {
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
					OA_CAM_CTRL_USBTRAFFIC )) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ), 0 );
  }
  if ( commonState.camera->hasControl (
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ))) {
    commonState.camera->setControl (
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ), 0 );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
					OA_CAM_CTRL_CONTRAST )) = 0;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ), 0 );
  }
}


unsigned int
ControlWidget::getCurrentGain ( void )
{
  if ( commonState.camera->hasControl ( OA_CAM_CTRL_GAIN )) {
    return cameraConf.CONTROL_VALUE( OA_CAM_CTRL_GAIN );
  }
  return 0;
}


unsigned int
ControlWidget::getCurrentExposure ( void )
{
  // FIX ME -- this is entirely the wrong thing to do if both exposure
  // controls are available
  if ( commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) {
    return cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) *
        intervalMultipliers [ config.intervalMenuOption ];
  } else {
    if ( commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_UNSCALED )) {
      return cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED );
    }
  }
  return 0;
}


void
ControlWidget::intervalMenuChanged ( int index )
{
  int64_t	min, max, step, def;
  int		prevOption;
  int		setting;

  if ( ignoreIntervalChanges ) {
    return;
  }

  prevOption = config.intervalMenuOption;
  config.intervalMenuOption = enabledIntervals [ index ];
  SET_PROFILE_INTERVAL( config.intervalMenuOption );

  // FIX ME -- huge amounts of this is duplicated from the configure()
  // function above, but it's very slightly different.  Ideally it needs
  // refactoring

  commonState.camera->controlRange ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE, &min, &max,
    &step, &def );

  // setting = commonState.camera->readControl (
  //     OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) / 1000;
  setting = cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
  setting *= intervalMultipliers[ prevOption ];

  switch ( config.intervalMenuOption ) {
    case INTERVAL_USEC:
      // min value is fine, but let's set a maximum of 10000us
      if ( max > 10000 ) {
        max = 10000;
      }
      break;

    case INTERVAL_MSEC:
      // The settings from liboacam are in units of 1us so we need
      // to convert those to milliseconds by dividing by 1000.
      min /= 1000;
      max /= 1000;
      // set a maximum of 10000ms
      if ( max > 10000 ) {
        max = 10000;
      }
      step /= 1000;
      setting /= 1000;
      break;

    case INTERVAL_SEC:
      // The settings from liboacam are in units of 1us so we need
      // to convert those to seconds by dividing by 1000000.
      min /= 1000000;
      max /= 1000000;
      step /= 1000000;
      setting /= 1000000;
      break;

    case INTERVAL_MIN:
      // The settings from liboacam are in units of 1us so we need
      // to convert those to minutes by dividing by 60000000.
      min /= 60000000;
      max /= 60000000;
      step /= 60000000;
      setting /= 60000000;
      break;
  }

  if ( min < 1 ) { min = 1; }
  if ( max < 1 ) { max = 1; }
  if ( step < 1 ) { step = 1; }
  if ( setting < min ) {
    setting = min;
  } else {
    if ( setting > max ) {
      setting = max;
    }
  }
  // cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = setting;
  QString s = "Exp. Range (" + intervalsList[ config.intervalMenuOption ] +
      ")";
  expRangeLabel->setText ( tr ( s.toStdString().c_str() ));

  // now we need to work out the ranges for the combobox
  // if the range is no greater than 256 we hide the combobox and
  // set the slider to the appropriate range.
  // otherwise for non-absolute exposures we divide the range up into
  // five ranges and for absolute have increasing wider scales

  int showMin = 1, showMax = 1;

  QStringList exposures;
  int v1 = min;
  int v2;
  int items = 0, showItem = 0;
  do {
    v2 = v1;
    if ( !v2 ) { v2++; }
    v2 *= 10;
    if ( v2 > max ) { v2 = max; }
    exposures << QString::number ( v1 ) + " - " + QString::number ( v2 );
    if ( setting >= v1 && setting <= v2 ) {
      showMin = v1;
      showMax = v2;
      showItem = items;
    }
    minSettings[ items ] = v1;
    maxSettings[ items ] = v2;
    items++;
    v1 = v2;
    // we have to stop this somewhere...
  } while ( items < 7 && v2 < max );

  // that should be the string list done.  now delete all the old items
  // and add the new string list and set the current item

  ignoreExposureMenuChanges = 1;
  expMenu->clear();
  expMenu->addItems ( exposures );
  config.exposureMenuOption = showItem;
  expMenu->setCurrentIndex ( showItem );
  expRangeLabel->show();
  expMenu->show();
  useExposureDropdown = 1;
  ignoreExposureMenuChanges = 0;

  // finally we can set the sliders up

  ignoreExposureChanges = 1;
  exposureSlider->setRange ( showMin, showMax );
  exposureSlider->setSingleStep ( step );
  exposureSpinbox->setRange ( showMin, showMax );
  exposureSpinbox->setSingleStep ( step );
  if ( state.settingsWidget ) {
    state.settingsWidget->reconfigureControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
  }
  ignoreExposureChanges = 0;
  exposureSlider->setValue ( setting );

  if ( setting ) {
    state.cameraWidget->showFPSMaxValue ( 1000000 /
        intervalMultipliers [ config.intervalMenuOption ] / setting );
  }
}


QString
ControlWidget::exposureIntervalString ( void )
{
  return intervalsList [ config.intervalMenuOption ];
}


void
ControlWidget::doAutoControlUpdate ( void )
{
	int				type, control, c, i;

	if ( !commonState.camera->hasReadableControls()) {
		return;
	}

  for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
		type = commonState.camera->hasControl ( c );
		if ( c == state.preferredExposureControl ) {
			// This is more complicated because the drop-down
			// for the exposure range may also need to be changed, the state of the
			// interval type (us/ms/sec) needs to be accounted for (and in fact there
			// are two possible options for the exposure control).

			if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
				control = OA_CAM_CTRL_MODE_AUTO( c );
				type = commonState.camera->hasControl ( control );
				if ( type == OA_CTRL_TYPE_BOOLEAN || type == OA_CTRL_TYPE_MENU ) {
					uint32_t autoOn = 0;
					switch ( type ) {
						case OA_CTRL_TYPE_BOOLEAN:
							autoOn = cameraConf.CONTROL_VALUE( control );
							break;
						case OA_CTRL_TYPE_MENU:
							autoOn = ( cameraConf.CONTROL_VALUE( control ) ==
									OA_EXPOSURE_MANUAL ) ? 0 : 1;
							break;
					}
					if ( autoOn ) {
						// Now we know that auto is enabled
						int64_t value;
						int done = 0;
						value = cameraConf.CONTROL_VALUE( c ) =
								commonState.camera->readControl( c );
						value /= intervalMultipliers [ config.intervalMenuOption ];
						if ( value < minSettings[ expMenu->currentIndex()] ||
								value > maxSettings[ expMenu->currentIndex()] ) {
							for ( i = 0; !done && i < expMenu->count(); i++ ) {
								if ( value >= minSettings[ i ] && value <= maxSettings[ i ] ) {
									expMenu->setCurrentIndex ( i );
									done = 1;
								}
							}
						}
						exposureSlider->setValue ( value );
						exposureSpinbox->setValue ( value );
					}
				}
			}
		} else {
			if (( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) &&
					commonState.camera->hasControl ( OA_CAM_CTRL_MODE_AUTO ( c )) ==
					OA_CTRL_TYPE_BOOLEAN ) {
				if ( cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO ( c ))) {
					// we have a control, an gain control, and auto mode is on
					cameraConf.CONTROL_VALUE( c ) = commonState.camera->readControl ( c );
					selectableControlSpinbox[ c ]->setValue (
							cameraConf.CONTROL_VALUE( c ));
					selectableControlSlider[ c ]->setValue (
							cameraConf.CONTROL_VALUE( c ));
				}
			}
		}
	}
}


void
ControlWidget::resetCamera ( void )
{
  int64_t min, max, step, def;
	int	type;

	// FIX ME -- should do something about a reset here if the camera
	// features say it has a reset function

  // Do checkboxes first in case their current settings would cause them
	// to ignore any of the slider settings

  for ( int c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
		for ( int m = OA_CAM_CTRL_MODIFIER_STD; m < OA_CAM_CTRL_MODIFIERS_P1;
        m++ ) {
			int control = ( m << 8 ) + c;
			commonState.camera->controlRange ( control, &min, &max, &step, &def );
			if ( OA_CTRL_TYPE_BOOLEAN == commonState.camera->hasControl ( control )) {
				if (( c == OA_CAM_CTRL_GAIN || c == state.preferredExposureControl ||
						c == config.selectableControl[0] ||
						c == config.selectableControl[1] ) &&
						OA_CAM_CTRL_MODIFIER_STD == m ) {
					selectableControlCheckbox[ c ]->setChecked ( def ? 1 : 0 );
				} else {
          cameraConf.CONTROL_VALUE( control ) = def;
				  SET_PROFILE_CONTROL( control, def );
					commonState.camera->setControl ( control, def );
					if ( state.settingsWidget ) {
						state.settingsWidget->updateControl ( control, def );
					}
				}
			}
		}
	}

  for ( int c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    type = commonState.camera->hasControl ( c );
    if ( OA_CTRL_TYPE_INT32 == type || OA_CTRL_TYPE_INT64 == type ) {
      commonState.camera->controlRange ( c, &min, &max, &step, &def );
			int64_t displayedValue = def;
			if ( c == OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) {
				int newUnit, newIndex;
				if ( def >= intervalMultipliers[ INTERVAL_MIN ]) {
					newUnit = INTERVAL_MIN;
					displayedValue /= intervalMultipliers[ INTERVAL_MIN ];
				} else {
					if ( def >= intervalMultipliers[ INTERVAL_SEC ]) {
						newUnit = INTERVAL_SEC;
						displayedValue /= intervalMultipliers[ INTERVAL_SEC ];
					} else {
						if ( def >= intervalMultipliers[ INTERVAL_MSEC ]) {
							newUnit = INTERVAL_MSEC;
							displayedValue /= intervalMultipliers[ INTERVAL_MSEC ];
						} else {
							newUnit = INTERVAL_USEC;
						}
					}
				}
				if ( useExposureDropdown ) {
					int foundDropdownValue = -1;
					int numItems = expMenu->count();
					for ( int i = 0; i < numItems; i++ ) {
						if ( displayedValue >= minSettings[i] && displayedValue <=
								maxSettings[i] ) {
							foundDropdownValue = i;
						}
					}
					if ( foundDropdownValue >= 0 ) {
						expMenu->setCurrentIndex ( foundDropdownValue );
					}
				}
				if ( newUnit != config.intervalMenuOption ) {
					newIndex = 0;
					while ( enabledIntervals[ newIndex ] != newUnit ) {
						newIndex++;
					}
					config.intervalMenuOption = newUnit;
					intervalSizeMenu->setCurrentIndex ( newIndex );
				}
			}
			if ( c == OA_CAM_CTRL_EXPOSURE_ABSOLUTE || c ==
					OA_CAM_CTRL_EXPOSURE_UNSCALED ) {
				exposureSlider->setValue ( displayedValue );
				exposureSpinbox->setValue ( displayedValue );
			} else {
				selectableControlSlider[ c ]->setValue ( displayedValue );
				selectableControlSpinbox[ c ]->setValue ( displayedValue );
			}
    }
  }

	// FIX ME -- also need to handle MENU, DISCRETE AND DISC_MENU types?
}
