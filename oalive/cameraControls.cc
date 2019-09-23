/*****************************************************************************
 *
 * cameraControls.cc -- class for the camera tab in the settings dialog
 *
 * Copyright 2015,2017,2018,2019 James Fidell (james@openastroproject.org)
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

#if HAVE_CSTRING
#include <cstring>
#endif

#include "captureSettings.h"
#include "fitsSettings.h"
#include "commonState.h"
#include "commonConfig.h"

#include "cameraControls.h"
#include "configuration.h"
#include "state.h"

#define SLIDERS_PER_ROW		1
#define CHECKBOXES_PER_ROW	2
#define BUTTONS_PER_ROW		2
#define MENUS_PER_ROW		2
#define UNHANDLED_PER_ROW	3

#define MENU_RANGE_USEC		0
#define MENU_RANGE_MSEC		1
#define MENU_RANGE_SECS		2
#define MENU_RANGE_MINS		3

static int64_t		rangeMenuMax[4] = {
  1LL, 1000LL, 1000000LL, 60000000LL
};

/*
static unsigned int			rangeIntervals[4] = {
	MENU_RANGE_USEC, MENU_RANGE_MSEC, MENU_RANGE_SECS,
		MENU_RANGE_MINS
};
*/

static int64_t			rangeMultipliers[4] = {
	1LL, 1000LL, 1000000LL, 60000000LL
};

static const char*	rangeMenuLabels[5] = {
  "microseconds", "milliseconds", "seconds", "minutes"
};


CameraControls::CameraControls ( QWidget* parent ) : QWidget ( parent )
{
  layout = nullptr;
  sliderSignalMapper = nullptr;
  checkboxSignalMapper = nullptr;
  buttonSignalMapper = nullptr;
  menuSignalMapper = nullptr;
  frameRateLabel = nullptr;
  frameRateSlider = nullptr;
  frameRateMenu = nullptr;
  memset ( controlType, 0, sizeof ( controlType ));
  ignoreFrameRateChanges = 0;
	histogram = new HistogramWidget ( 0, this );
	connectHistogramSignal();
}


void
CameraControls::connectHistogramSignal ( void )
{
	if ( state.viewWidget && !state.histogramCCSignalConnected ) {
		connect ( state.viewWidget, SIGNAL( updateHistogram ( void )),
				histogram, SLOT( update ( void )));
		state.histogramCCSignalConnected = 1;
	}
}


void
CameraControls::configure ( void )
{
  int		c;
  int		added[ OA_CAM_CTRL_LAST_P1 ];
  int		numSliders = 0, numCheckboxes = 0, numMenus = 0;
  int		numSliderCheckboxes = 0, numUnhandled = 0, numButtons = 0;
  int		haveRangeMenu = 0, readableControls = 0;

  if ( commonState.camera->Camera::isInitialised()) {
		readableControls = commonState.camera->hasReadableControls();
	}

  if ( layout ) {
    state.mainWindow->destroyLayout (( QLayout* ) layout );
    delete sliderSignalMapper;
    delete checkboxSignalMapper;
    delete buttonSignalMapper;
    memset ( controlType, 0, sizeof ( controlType ));
    layout = nullptr;
    frameRateLabel = nullptr;
    frameRateSlider = nullptr;
    frameRateMenu = nullptr;
		// destroyLayout will have killed this, so it must be recreated
		state.histogramCCSignalConnected = 0;
		histogram = new HistogramWidget ( 0, this );
		connectHistogramSignal();
  }

  // Create all the controls to show

  sliderSignalMapper = new QSignalMapper ( this );
  checkboxSignalMapper = new QSignalMapper ( this );
  buttonSignalMapper = new QSignalMapper ( this );
  menuSignalMapper = new QSignalMapper ( this );

  for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
    controlLabel[c] = nullptr;
    controlSlider[c] = nullptr;
    controlSpinbox[c] = nullptr;
    controlCheckbox[c] = nullptr;
    controlButton[c] = nullptr;
    controlMenu[c] = nullptr;
    added[c] = 0;

    if ( commonState.camera->Camera::isInitialised()) {
      controlType[c] = commonState.camera->hasControl ( c );

      if ( controlType[c] ) {

        switch ( controlType[c] ) {

          case OA_CTRL_TYPE_INT32:
          case OA_CTRL_TYPE_INT64:
          {
            int64_t	min, max, step, def, showMin, showMax, showStep;
						int64_t savedDef;

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

            commonState.camera->controlRange ( c, &min, &max, &step, &def );
            showMin = min;
            showMax = max;
            showStep = step;
						savedDef = def;

            if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == c ) {
              int i, numRanges;

              numRanges = sizeof ( rangeMenuMax ) / sizeof ( uint64_t );
							minRangeIndex = maxRangeIndex = 0;
              for ( i = 0; i < numRanges - 1; i++ ) {
                if ( min >= rangeMenuMax[i+1] ) {
                  minRangeIndex++;
									if ( maxRangeIndex < minRangeIndex ) {
										maxRangeIndex = minRangeIndex;
									}
                }
                if ( i && max > rangeMenuMax[i-1] ) {
                  maxRangeIndex = i+1;
                }
              }
              if ( minRangeIndex != maxRangeIndex ) {
                haveRangeMenu = 1;
                showStep = 1;
                exposureRangeMenu = new QComboBox ( this );
                for ( i = minRangeIndex; i < maxRangeIndex; i++ ) {
                  exposureRangeMenu->addItem ( tr ( rangeMenuLabels[i] ));
                }

								// update the intervalMenuOption if what we currently have
								// doesn't fit the menus

								if ( config.intervalMenuOption < minRangeIndex &&
										config.intervalMenuOption > maxRangeIndex ) {
									config.intervalMenuOption = minRangeIndex;
								}

								// modify showMin/showMax/def to match up with the current
								// exposure units

								if ( config.intervalMenuOption ) {
									showMin /= rangeMultipliers[ config.intervalMenuOption ];
									if ( showMin < 1 ) { showMin = 1; }
									showMax /= rangeMultipliers[ config.intervalMenuOption ];
									if ( showMax < 1 ) { showMax = 1; }
									def /= rangeMultipliers[ config.intervalMenuOption ];
									if ( def < 1 ) { def = 1; }
									exposureRangeMenu->setCurrentIndex (
											config.intervalMenuOption );
								}

								// FIX ME -- what if showMin and showMax are both now 1?
              }
            }

            controlSlider[ c ]->setMinimum ( showMin );
            controlSpinbox[ c ]->setMinimum ( showMin );

            controlSlider[ c ]->setMaximum ( showMax );
            controlSpinbox[ c ]->setMaximum ( showMax );

            controlSlider[ c ]->setSingleStep ( showStep );
            controlSpinbox[ c ]->setSingleStep ( showStep );

            sliderSignalMapper->setMapping ( controlSpinbox[c], c );
            connect ( controlSpinbox[ c ], SIGNAL( valueChanged ( int )),
                sliderSignalMapper, SLOT( map()));

						if (readableControls ) {
							cameraConf.CONTROL_VALUE( c ) =
									commonState.camera->readControl ( c );
						} else {
							if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == c ) {
								cameraConf.CONTROL_VALUE( c ) = savedDef;
							} else {
								cameraConf.CONTROL_VALUE( c ) = def;
							}
						}
            controlSlider[ c ]->setValue ( cameraConf.CONTROL_VALUE( c ));
            controlSpinbox[ c ]->setValue ( cameraConf.CONTROL_VALUE( c ));

            break;
          }

          case OA_CTRL_TYPE_BOOLEAN:
            numCheckboxes++;
            controlCheckbox[ c ] = new QCheckBox ( QString ( tr (
                oaCameraControlLabel[c] )), this );
						if ( readableControls ) {
							// FIX ME -- handle absolute exposure here
							cameraConf.CONTROL_VALUE( c ) =
									commonState.camera->readControl ( c );
						} else {
							cameraConf.CONTROL_VALUE( c ) = 0;
						}
            checkboxSignalMapper->setMapping ( controlCheckbox[c], c );
            connect ( controlCheckbox[ c ], SIGNAL ( stateChanged ( int )),
                checkboxSignalMapper, SLOT ( map()));
            controlCheckbox[ c ]->setChecked ( cameraConf.CONTROL_VALUE( c ));
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
            commonState.camera->controlRange ( c, &min, &max, &step, &def );
            if ( 1 == step && 0 == min ) {
              numMenus++;
              controlMenu[ c ] = new QComboBox ( this );
              for ( int i = min; i <= max; i += step ) {
                controlMenu[ c ]->addItem ( tr (
                    commonState.camera->getMenuString ( c, i )));
              }
							if ( readableControls ) {
								cameraConf.CONTROL_VALUE( c ) =
										commonState.camera->readControl ( c );
							} else {
								cameraConf.CONTROL_VALUE( c ) = def;
							}
              menuSignalMapper->setMapping ( controlMenu[c], c );
              connect ( controlMenu[ c ], SIGNAL( currentIndexChanged ( int )),
                  menuSignalMapper, SLOT ( map()));
              controlMenu[ c ]->setCurrentIndex (
									cameraConf.CONTROL_VALUE( c ));
            } else {
              numUnhandled++;
              qWarning() << "Can't handle menu with min = " << min <<
                  " and step = " << step;
            }
            break;
          }

          case OA_CTRL_TYPE_DISC_MENU:
          {
						// most of this will be the same as for the menus, but the set
						// of possible values is non-contiguous and needs to be fetched
						// from liboacam

						int32_t		count;
						int64_t		*values;
            int64_t		min, max, step, def;

            controlLabel[c] = new QLabel ( tr ( oaCameraControlLabel[c] ));
            controlLabel[c]->setWordWrap ( 1 );
						commonState.camera->controlDiscreteSet ( c, &count, &values );
						// need this just for the default value
            commonState.camera->controlRange ( c, &min, &max, &step, &def );
						numMenus++;
            controlMenu[ c ] = new QComboBox ( this );
            for ( int i = 0; i < count; i++ ) {
							if ( c == OA_CAM_CTRL_FRAME_FORMAT ) {
								controlMenu[ c ]->addItem ( tr (
										oaFrameFormats[ values[i]].name ));
							} else {
								controlMenu[ c ]->addItem ( tr (
										commonState.camera->getMenuString ( c, values[i] )));
							}
						}
						if ( readableControls ) {
							cameraConf.CONTROL_VALUE( c ) =
									commonState.camera->readControl ( c );
						} else {
							cameraConf.CONTROL_VALUE( c ) = def;
						}
            menuSignalMapper->setMapping ( controlMenu[c], c );
            connect ( controlMenu[ c ], SIGNAL( currentIndexChanged ( int )),
                menuSignalMapper, SLOT ( map()));
						for ( int i = 0; i < count; i++ ) {
							if ( cameraConf.CONTROL_VALUE( c ) == values[i] ) {
								controlMenu[ c ]->setCurrentIndex ( i );
							}
						}
            break;
          }

          case OA_CTRL_TYPE_READONLY:
            controlLabel[c] = new QLabel ( tr ( oaCameraControlLabel[c] ));
            controlLabel[c]->setWordWrap ( 1 );
            numUnhandled++;
            break;

					case OA_CTRL_TYPE_DISCRETE:
						// FIX ME -- these really ought to show
						// don't show these up as unhandled
						if ( OA_CAM_CTRL_BINNING == c ) {
							added[c] = 1;
							break;
						}
						/* FALLTHROUGH */
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
      if (( autoControl = commonState.camera->hasAuto ( c ))) {
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
        this, SLOT ( updateExposureUnits ( int )));
  }

  if ( commonState.camera->hasFixedFrameRates ( commonConfig.imageSizeX,
      commonConfig.imageSizeY )) {
    sliderGrid->addWidget ( frameRateLabel, row, col++ );
    col++;
    sliderGrid->addWidget ( frameRateSlider, row, col++ );
    sliderGrid->addWidget ( frameRateMenu, row, col++ );
		numSliders++;
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
      commonState.camera->controlRange ( c, &min, &max, &step, &def );
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

	// Status information

	statusGrid = new QGridLayout();
  row = 0;
	int addedStatus = 0;
	if ( controlType[ OA_CAM_CTRL_POWER_SOURCE ] == OA_CTRL_TYPE_READONLY ) {
		statusGrid->addWidget ( controlLabel[OA_CAM_CTRL_POWER_SOURCE], row++, 0 );
		powerSource = new QLabel ( tr (( commonState.camera->readControl (
				OA_CAM_CTRL_POWER_SOURCE ) == OA_AC_POWER ) ? "AC" : "Battery" ));
		statusGrid->addWidget ( controlLabel[OA_CAM_CTRL_POWER_SOURCE], row, 0 );
		statusGrid->addWidget ( powerSource, row++, 1 );
		addedStatus++;
	}
	if ( controlType[ OA_CAM_CTRL_BATTERY_LEVEL ] == OA_CTRL_TYPE_READONLY ) {
		batteryLevel = new QProgressBar();
		batteryLevel->setRange ( 0, 100 );
		batteryLevel->setOrientation ( Qt::Horizontal );
		batteryLevel->setValue ( commonState.camera->readControl (
				OA_CAM_CTRL_BATTERY_LEVEL ));
		batteryLevel->setTextVisible ( 0 );
		batteryLevel->setStyleSheet ( QString ( "QProgressBar::chunk:horizontal {"
				"background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, "
				"stop: 0 darkred, stop: 1 lightgreen);}" ) +
				QString ( "QProgressBar::horizontal { border: 1px solid gray; "
				"border-radius: 3px; background: lightgray; padding: 0px; "
				"text-align: left; margin-right: 4ex;}" ));
		statusGrid->addWidget ( controlLabel[OA_CAM_CTRL_BATTERY_LEVEL], row, 0 );
		statusGrid->addWidget ( batteryLevel, row, 1 );
		addedStatus++;
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
	if ( numSliders ) {
		layout->addLayout ( sliderGrid );
		layout->addStretch ( 1 );
	}
	if ( addedBoxes ) {
		layout->addLayout ( checkboxGrid );
		layout->addStretch ( 1 );
	}
	if ( addedButtons ) {
		layout->addLayout ( buttonGrid );
		layout->addStretch ( 1 );
	}
	if ( addedMenus ) {
		layout->addLayout ( menuGrid );
		layout->addStretch ( 1 );
	}
	if ( addedStatus ) {
		layout->addLayout ( statusGrid );
		layout->addStretch ( 1 );
	}
	if ( addedTodo ) {
		layout->addLayout ( unhandledGrid );
		layout->addStretch ( 2 );
	}
	layout->addWidget ( histogram );

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
    commonState.camera->setControl ( OA_CAM_CTRL_MODE_AUTO(
        OA_CAM_CTRL_EXPOSURE_UNSCALED ), OA_EXPOSURE_MANUAL );
    cameraConf.CONTROL_VALUE( OA_CAM_CTRL_MODE_AUTO(
        OA_CAM_CTRL_EXPOSURE_UNSCALED )) = OA_EXPOSURE_MANUAL;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_MODE_AUTO(
        OA_CAM_CTRL_EXPOSURE_UNSCALED ), OA_EXPOSURE_MANUAL );
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
    SET_PROFILE_CONTROL(
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ), AWBManual );
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


void
CameraControls::updateSliderControl ( int control )
{
  int value = controlSpinbox[ control ]->value();
	if ( control == OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) {
		if ( ignoreExposureChanges ) {
			return;
		}
		value *= rangeMultipliers [ config.intervalMenuOption ];
	}
  cameraConf.CONTROL_VALUE( control ) = value;
  SET_PROFILE_CONTROL( control, value );
  commonState.camera->setControl ( control, value );
}


void
CameraControls::updateCheckboxControl ( int control )
{
  int value = ( controlCheckbox[ control ]->isChecked()) ? 1 : 0;
  int baseControl, controlType;

	/*
	 * Misthink here?  If this is boolean, we just leave it as is.
	 *
  if (( OA_CAM_CTRL_MODE_AUTO ( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == control ||
    OA_CAM_CTRL_MODE_AUTO ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) == control ) &&
       commonState.camera->hasControl ( control ) == OA_CTRL_TYPE_BOOLEAN ) {
    value = value ? OA_EXPOSURE_AUTO : OA_EXPOSURE_MANUAL;
  }
	 */
  cameraConf.CONTROL_VALUE( control ) = value;
  SET_PROFILE_CONTROL( control, value );
  commonState.camera->setControl ( control, value );
  if (( baseControl = oaGetControlForAuto ( control )) >= 0 ) {
    controlType = commonState.camera->hasControl ( baseControl );
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
    commonState.camera->setControl ( control, 1 );
    if ( control == OA_CAM_CTRL_RESTORE_USER ||
        control == OA_CAM_CTRL_RESTORE_FACTORY ) {

      for ( c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
        switch ( controlType [ c ] ) {

          case OA_CTRL_TYPE_BOOLEAN:
            v = commonState.camera->readControl ( c );
            cameraConf.CONTROL_VALUE( c ) = v;
            SET_PROFILE_CONTROL( c, v );
            controlCheckbox[ c ]->setChecked ( v );
            break;

          case OA_CTRL_TYPE_INT32:
          case OA_CTRL_TYPE_INT64:
            v = commonState.camera->readControl ( c );
            cameraConf.CONTROL_VALUE( c ) = v;
            SET_PROFILE_CONTROL( c, v );
            controlSpinbox[ c ]->setValue ( v );
            break;

          case OA_CTRL_TYPE_MENU:
            v = commonState.camera->readControl ( c );
            cameraConf.CONTROL_VALUE( c ) = v;
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
          commonState.camera->controlRange ( c, &min, &max, &step, &def );
          controlCheckbox[ c ]->setChecked ( def );
          cameraConf.CONTROL_VALUE( c ) = def;
          SET_PROFILE_CONTROL( c, def );
          break;

        case OA_CTRL_TYPE_INT32:
        case OA_CTRL_TYPE_INT64:
          commonState.camera->controlRange ( c, &min, &max, &step, &def );
          controlSpinbox[ c ]->setValue ( def );
          cameraConf.CONTROL_VALUE( c ) = def;
          SET_PROFILE_CONTROL( c, def );
          break;

        case OA_CTRL_TYPE_MENU:
          commonState.camera->controlRange ( c, &min, &max, &step, &def );
          controlMenu[ c ]->setCurrentIndex ( def );
          cameraConf.CONTROL_VALUE( c ) = def;
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
	// FIX ME -- there's an implicit assumption here that menus will have
	// the same value sequence as the items in the menu. (ie. starting at 0
	// and incrementing by 1
	if ( controlType [ control ] == OA_CTRL_TYPE_DISC_MENU ) {
		int32_t count;
		int64_t *values;
		commonState.camera->controlDiscreteSet ( control, &count, &values );
		if ( value < count ) {
			value = values[value];
		} else {
			qWarning() << "Invalid menu value for discrete menu";
		}
	}
  commonState.camera->setControl ( control, value );
	if ( control == OA_CAM_CTRL_FRAME_FORMAT && commonState.camera ) {
		state.viewWidget->setVideoFramePixelFormat ( value );
	}
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

  if ( commonState.camera->isInitialised()) {
    commonState.camera->setFrameInterval ( config.frameRateNumerator,
        config.frameRateDenominator );
  }
}


void
CameraControls::updateFrameRateSlider ( void )
{
  if ( !commonState.camera->isInitialised()) {
    return;
  }
  if ( !commonState.camera->hasFrameRateSupport()) {
    frameRateLabel->hide();
    frameRateSlider->hide();
    frameRateMenu->hide();
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
  if ( commonState.camera->hasControl ( OA_CAM_CTRL_GAIN )) {
    return cameraConf.CONTROL_VALUE( OA_CAM_CTRL_GAIN );
  }
  return 0;
}


unsigned int
CameraControls::getCurrentExposure ( void )
{
  if ( commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) {
    return cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
  } else {
    if ( commonState.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_UNSCALED )) {
      return cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED );
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
CameraControls::updateExposureUnits ( int index )
{
  int64_t	min, max, step, def;
  int		setting;

  config.intervalMenuOption = index + minRangeIndex;

  commonState.camera->controlRange ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE, &min, &max,
    &step, &def );

  setting = cameraConf.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );

  switch ( config.intervalMenuOption ) {
    case MENU_RANGE_USEC:
			// don't need to change these values
      break;

    case MENU_RANGE_MSEC:
      // The settings from liboacam are in units of 1us so we need
      // to convert those to milliseconds by dividing by 1000.
      min /= 1000;
      max /= 1000;
      step /= 1000;
      setting /= 1000;
      break;

    case MENU_RANGE_SECS:
      // The settings from liboacam are in units of 1us so we need
      // to convert those to seconds by dividing by 1000000.
      min /= 1000000;
      max /= 1000000;
      step /= 1000000;
      setting /= 1000000;
      break;

    case MENU_RANGE_MINS:
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

  ignoreExposureChanges = 1;
  controlSlider[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ]->setRange ( min, max );
  controlSpinbox[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ]->setRange ( min, max );
  controlSlider[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ]->setSingleStep ( step );
  controlSpinbox[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ]->setSingleStep ( step );
  if ( state.settingsWidget ) {
    state.settingsWidget->reconfigureControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
  }
  ignoreExposureChanges = 0;
  controlSlider[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ]->setValue ( setting );
}


void
CameraControls::setBatteryLevel ( void )
{
	int		v;

  if ( controlType[ OA_CAM_CTRL_BATTERY_LEVEL ] == OA_CTRL_TYPE_READONLY ) {
		v = commonState.camera->readControl ( OA_CAM_CTRL_BATTERY_LEVEL );
		batteryLevel->setValue ( v );
	}
}
