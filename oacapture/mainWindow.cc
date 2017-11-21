/*****************************************************************************
 *
 * mainWindow.cc -- the main controlling window class
 *
 * Copyright 2013,2014,2015,2016,2017 James Fidell (james@openastroproject.org)
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
#include <unistd.h>
#include <pwd.h>

#include <openastro/filterwheel.h>
#include <openastro/demosaic.h>
}

#include "focusOverlay.h"
#include "mainWindow.h"
#include "version.h"
#include "configuration.h"
#include "controlWidget.h"
#include "cameraWidget.h"
#include "imageWidget.h"
#include "captureWidget.h"
#include "zoomWidget.h"
#include "previewWidget.h"
#include "settingsWidget.h"
#include "state.h"
#include "targets.h"

CONFIG		config;
STATE		state;

static const char* styleGroupBoxBorders =
  "QGroupBox { "
    "margin-top: 1ex;"
    "border: 2px groove grey;"
//  "border-radius: 5px;"
  "} "
  "QGroupBox::title { "
    "subcontrol-origin: margin;"
    "subcontrol-position: left top;"
    "left: 1em;"
  "}";


MainWindow::MainWindow()
{
  QString qtVer;
  unsigned int qtMajorVersion;
  int i;
  bool ok;

  cameraSignalMapper = filterWheelSignalMapper = 0;
  timerSignalMapper = 0;
  timerStatus = wheelStatus = 0;
  advancedFilterWheelSignalMapper = 0;
  rescanCam = disconnectCam = 0;
  rescanWheel = disconnectWheel = warmResetWheel = coldResetWheel = 0;
  rescanTimer = disconnectTimerDevice = resetTimerDevice = 0;
  connectedCameras = cameraMenuCreated = 0;
  connectedFilterWheels = filterWheelMenuCreated = 0;
  connectedTimers = timerMenuCreated = 0;
  doingQuit = 0;
  state.histogramOn = 0;
  state.histogramWidget = 0;
  state.needGroupBoxBorders = 0;
  state.cameraTempValid = 0;
  state.gpsValid = 0;
  state.binningValid = 0;

  // The gtk+ style doesn't enable group box borders by default, which makes
  // the display look confusing.
  //
  // Same thing with Qt5, so work out the version and add them if required

  qtVer = qVersion();
  if (( i = qtVer.indexOf ( '.' )) >= 0 ) {
    qtVer.truncate ( i );
  }
  qtMajorVersion = qtVer.toInt( &ok );

  QString currentStyle = QApplication::style()->objectName();
  if ( currentStyle.toLower() == "gtk+" || ( ok && qtMajorVersion > 4 )) {
    state.needGroupBoxBorders = 1;
    this->setStyleSheet ( styleGroupBoxBorders );
  }

  readConfig();
  createStatusBar();
  createMenus();
  setWindowTitle( APPLICATION_NAME " " VERSION_STR );

  state.mainWindow = this;
  state.controlWidget = 0;
  state.libavStarted = 0;
  state.camera = new Camera;
  state.filterWheel = new FilterWheel;
  state.timer = new Timer;
  oldHistogramState = -1;
  state.lastRecordedFile = "";
  updateTemperatureLabel = 0;
  state.captureIndex = 0;
  state.settingsWidget = 0;
  state.advancedSettings = 0;
  colourDialog = 0;

  // need to do this to prevent access attempts before creation
  previewWidget = 0;

  createControlWidgets();
  createPreviewWindow();

  connect ( state.previewWidget, SIGNAL( updateFrameCount ( unsigned int )),
      this, SLOT ( setCapturedFrames ( unsigned int )));
  connect ( state.previewWidget, SIGNAL( updateActualFrameRate (
      double )), this, SLOT ( setActualFrameRate ( double )));
  connect ( state.previewWidget, SIGNAL( updateTemperature ( void )),
      this, SLOT ( setTemperature ( void )));
  connect ( state.previewWidget, SIGNAL( updateDroppedFrames ( void )),
      this, SLOT ( setDroppedFrames ( void )));
  connect ( state.previewWidget, SIGNAL( updateProgress ( unsigned int )),
      this, SLOT ( setProgress ( unsigned int )));
  connect ( state.previewWidget, SIGNAL( stopRecording ( void )),
      state.captureWidget, SLOT ( doStopRecording ( void )));
  connect ( state.captureWidget, SIGNAL( writeStatusMessage ( QString )),
      this, SLOT ( showStatusMessage ( QString )));
  connect ( state.previewWidget, SIGNAL( frameWriteFailed ( void )),
      this, SLOT ( frameWriteFailedPopup ( void )));

  // update filters for matching filter wheels from config
  state.filterWheel->updateAllSearchFilters();

  char d[ PATH_MAX ];
#ifdef HAVE_QT4
  state.currentDirectory = QString::fromAscii ( getcwd ( d, PATH_MAX ));
#else
  state.currentDirectory = QString::fromLatin1 ( getcwd ( d, PATH_MAX ));
#endif

  if ( connectedCameras == 1 && config.connectSoleCamera ) {
    connectCamera ( 0 );
  }
  focusaid->setChecked ( config.showFocusAid );
}


MainWindow::~MainWindow()
{
  // FIX ME -- delete cameras[], filterWheels[]

  state.histogramOn = 0;

  delete about;
  delete histogram;
  delete autorun;
  delete demosaic;
  delete fits;
  delete filters;
  delete profiles;
  delete capture;
  delete cameraOpt;
  delete general;
  delete demosaicOpt;
  delete flipY;
  delete flipX;
  delete darkframe;
  delete focusaid;
  delete cutout;
  delete reticle;
  delete autoguide;
  delete alignbox;
  delete autoalign;
  delete histogramOpt;
  if ( cameraSignalMapper ) {
    delete cameraSignalMapper;
  }
  if ( filterWheelSignalMapper ) {
    delete filterWheelSignalMapper;
  }
  if ( advancedFilterWheelSignalMapper ) {
    delete advancedFilterWheelSignalMapper;
  }
  if ( timerSignalMapper ) {
    delete timerSignalMapper;
  }
  delete exit;
  if ( rescanCam ) {
    delete rescanCam;
  }
  if ( disconnectCam ) {
    delete disconnectCam;
  }
  if ( rescanWheel ) {
    delete rescanWheel;
  }
  if ( disconnectWheel ) {
    delete disconnectWheel;
  }
  if ( warmResetWheel ) {
    delete warmResetWheel;
  }
  if ( coldResetWheel ) {
    delete coldResetWheel;
  }
  if ( rescanTimer ) {
    delete rescanTimer;
  }
  if ( disconnectTimerDevice ) {
    delete disconnectTimerDevice;
  }
  if ( resetTimerDevice ) {
    delete resetTimerDevice;
  }
  // delete saveConfigAs;
  delete saveConfig;
  delete loadConfig;
  delete capturedValue;
  delete fpsActualValue;
  delete fpsMaxValue;
  delete pixelFormatValue;
  delete progressBar;
  delete capturedLabel;
  delete fpsActualLabel;
  delete fpsMaxLabel;
  delete pixelFormatLabel;
  if ( state.camera ) {
    delete state.camera;
  }
  if ( state.filterWheel ) {
    delete state.filterWheel;
  }
  if ( state.timer ) {
    delete state.timer;
  }
  if ( state.histogramWidget ) {
    delete state.histogramWidget;
  }
}


void
MainWindow::readConfig ( void )
{
  QSettings		settings ( ORGANISATION_NAME_SETTINGS,
			    APPLICATION_NAME );
  const char*		defaultDir = "";
#if USE_HOME_DEFAULT
  struct passwd*	pwd;

  pwd = getpwuid ( getuid());
  if ( pwd ) {
    defaultDir = pwd->pw_dir;
  }
#endif
  
  // -1 means we don't have a config file.  We change it to 1 later in the
  // function
  config.saveSettings = settings.value ( "saveSettings", -1 ).toInt();

  if ( !config.saveSettings ) {

    config.tempsInC = 1;
    config.reticleStyle = 1;
    config.displayFPS = 15;

    config.showHistogram = 0;
    config.autoAlign = 0;
    config.showReticle = 0;
    config.cutout = 0;
    config.showFocusAid = 0;
    config.darkFrame = 0;
    config.flipX = 0;
    config.flipY = 0;
    config.demosaic = 0;

    config.sixteenBit = 0;
    config.binning2x2 = 0;
    config.rawMode = 0;
    config.colourise = 0;

    config.useROI = 0;
    config.imageSizeX = 0;
    config.imageSizeY = 0;

    config.zoomButton1Option = 1;
    config.zoomButton2Option = 3;
    config.zoomButton3Option = 5;
    config.zoomValue = 100;

    config.CONTROL_VALUE( OA_CAM_CTRL_GAIN ) = 50;
    config.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED ) = 10;
    config.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 100;
    config.CONTROL_VALUE( OA_CAM_CTRL_GAMMA ) = -1;
    config.CONTROL_VALUE( OA_CAM_CTRL_BRIGHTNESS ) = -1;
    config.exposureMenuOption = 3;
    config.frameRateNumerator = 0;
    config.frameRateDenominator = 1;
    config.selectableControl[0] = OA_CAM_CTRL_GAMMA;
    config.selectableControl[1] = OA_CAM_CTRL_BRIGHTNESS;
    config.intervalMenuOption = 1;  // msec

    config.profileOption = 0;
    config.filterOption = 0;
    config.fileTypeOption = 1;
    config.limitEnabled = 0;
    config.framesLimitValue = 0;
    config.secondsLimitValue = 0;
    config.limitType = 0;
    config.fileNameTemplate = QString ( "oaCapture-%DATE-%TIME" );
    config.captureDirectory = QString ( defaultDir );

    config.autorunCount = 0;
    config.autorunDelay = 0;
    config.saveCaptureSettings = 1;
    config.windowsCompatibleAVI = 0;
    config.useUtVideo = 0;
    config.indexDigits = 6;

    config.preview = 1;
    config.nightMode = 0;

    config.splitHistogram = 0;
    config.histogramOnTop = 1;

    config.demosaicPreview = 0;
    config.demosaicOutput = 0;
    config.cfaPattern = OA_DEMOSAIC_AUTO;
    config.demosaicMethod = 1;

    config.numProfiles = 0;
    config.numFilters = 0;

    config.promptForFilterChange = 0;
    config.interFilterDelay = 0;

    config.currentColouriseColour.setRgb ( 255, 255, 255 );
    config.numCustomColours = 0;

    config.fitsObserver = "";
    config.fitsInstrument = "";
    config.fitsObject = "";
    config.fitsComment = "";
    config.fitsTelescope = "";
    config.fitsFocalLength = "";
    config.fitsApertureDia = "";
    config.fitsApertureArea = "";
    config.fitsPixelSizeX = "";
    config.fitsPixelSizeY = "";
    config.fitsSubframeOriginX = "";
    config.fitsSubframeOriginY = "";
    config.fitsSiteLatitude = "";
    config.fitsSiteLongitude = "";
    config.fitsFilter = "";

    config.timerMode = OA_TIMER_MODE_UNSET;
    config.timerEnabled = 0;
  } else {

    int version = settings.value ( "configVersion", CONFIG_VERSION ).toInt();

    restoreGeometry ( settings.value ( "geometry").toByteArray());

    // FIX ME -- how to handle this?
    // config.cameraDevice = settings.value ( "device/camera", -1 ).toInt();

    config.tempsInC = settings.value ( "tempsInCentigrade", 1 ).toInt();
    config.connectSoleCamera = settings.value ( "connectSoleCamera",
        0 ).toInt();
    config.dockableControls = settings.value ( "dockableControls", 0 ).toInt();
    config.controlsOnRight = settings.value ( "controlsOnRight", 1 ).toInt();
    config.separateControls = settings.value ( "separateControls", 0 ).toInt();
    config.saveCaptureSettings = settings.value ( "saveCaptureSettings",
        1 ).toInt();
    config.windowsCompatibleAVI = settings.value ( "windowsCompatibleAVI",
        0 ).toInt();
    config.useUtVideo = settings.value ( "useUtVideo", 0 ).toInt();
    config.indexDigits = settings.value ( "indexDigits", 6 ).toInt();

    config.showHistogram = settings.value ( "options/showHistogram",
        0 ).toInt();
    config.autoAlign = settings.value ( "options/autoAlign", 0 ).toInt();
    config.showReticle = settings.value ( "options/showReticle", 0 ).toInt();
    config.cutout = settings.value ( "options/cutout", 0 ).toInt();
    config.showFocusAid = settings.value ( "options/showFocusAid", 0 ).toInt();
    config.darkFrame = settings.value ( "options/darkFrame", 0 ).toInt();
    config.flipX = settings.value ( "options/flipX", 0 ).toInt();
    config.flipY = settings.value ( "options/flipY", 0 ).toInt();
    config.demosaic = settings.value ( "options/demosaic", 0 ).toInt();

    config.sixteenBit = settings.value ( "camera/sixteenBit", 0 ).toInt();
    config.binning2x2 = settings.value ( "camera/binning2x2", 0 ).toInt();
    config.rawMode = settings.value ( "camera/raw", 0 ).toInt();
    config.colourise = settings.value ( "camera/colourise", 0 ).toInt();
    // FIX ME -- reset these temporarily.  needs fixing properly
    config.sixteenBit = 0;
    config.binning2x2 = 0;
    config.rawMode = 0;
    config.colourise = 0;

    config.useROI = settings.value ( "image/useROI", 0 ).toInt();
    config.imageSizeX = settings.value ( "image/imageSizeX", 0 ).toInt();
    config.imageSizeY = settings.value ( "image/imageSizeY", 0 ).toInt();

    config.zoomButton1Option = settings.value ( "image/zoomButton1Option",
        1 ).toInt();
    config.zoomButton2Option = settings.value ( "image/zoomButton2Option",
        3 ).toInt();
    config.zoomButton3Option = settings.value ( "image/zoomButton3Option",
        5 ).toInt();
    config.zoomValue = settings.value ( "image/zoomValue", 100 ).toInt();

    if ( version < 3 ) {
      config.CONTROL_VALUE( OA_CAM_CTRL_GAIN ) = settings.value (
          "control/gainValue", 50 ).toInt();
      config.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_UNSCALED ) = settings.value (
          "control/exposureValue", 10 ).toInt();
      config.CONTROL_VALUE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = settings.value (
          "control/exposureAbsoluteValue", 10 ).toInt();
      config.CONTROL_VALUE( OA_CAM_CTRL_GAMMA ) = settings.value (
          "control/gammaValue", -1 ).toInt();
      config.CONTROL_VALUE( OA_CAM_CTRL_BRIGHTNESS ) = settings.value (
          "control/brightnessValue", -1 ).toInt();
    }

    config.exposureMenuOption = settings.value ( "control/exposureMenuOption",
        3 ).toInt();
    config.frameRateNumerator = settings.value ( "control/frameRateNumerator",
        0 ).toInt();
    config.frameRateDenominator = settings.value (
        "control/frameRateDenominator", 1 ).toInt();
    config.selectableControl[0] = settings.value (
        "control/selectableControl1", OA_CAM_CTRL_GAMMA ).toInt();
    config.selectableControl[1] = settings.value (
        "control/selectableControl2", OA_CAM_CTRL_BRIGHTNESS ).toInt();
    if ( config.selectableControl[1] == config.selectableControl[0] ) {
      config.selectableControl[1] = -1;
    }
    config.intervalMenuOption = settings.value (
        "control/intervalMenuOption", 1 ).toInt(); // default = msec

    config.profileOption = settings.value ( "control/profileOption",
        0 ).toInt();
    config.filterOption = settings.value ( "control/filterOption", 0 ).toInt();
    config.fileTypeOption = settings.value ( "control/fileTypeOption",
        1 ).toInt();
    config.limitEnabled = settings.value ( "control/limitEnabled", 0 ).toInt();
    config.framesLimitValue = settings.value ( "control/framesLimitValue",
        0 ).toInt();
    config.secondsLimitValue = settings.value ( "control/secondsLimitValue",
        0 ).toInt();
    config.limitType = settings.value ( "control/limitType", 0 ).toInt();
    config.fileNameTemplate = settings.value ( "control/fileNameTemplate",
        "oaCapture-%DATE-%TIME" ).toString();
    config.captureDirectory = settings.value ( "control/captureDirectory",
        defaultDir ).toString();

    config.autorunCount = settings.value ( "autorun/count", 0 ).toInt();
    config.autorunDelay = settings.value ( "autorun/delay", 0 ).toInt();
    config.promptForFilterChange = settings.value (
        "autorun/filterPrompt", 0 ).toInt();
    config.interFilterDelay = settings.value (
        "autorun/interFilterDelay", 0 ).toInt();

    config.preview = settings.value ( "display/preview", 1 ).toInt();
    config.nightMode = settings.value ( "display/nightMode", 0 ).toInt();
    config.displayFPS = settings.value ( "display/displayFPS", 15 ).toInt();
    // fix a problem with existing configs
    if ( !config.displayFPS ) { config.displayFPS = 15; }

    config.splitHistogram = settings.value ( "histogram/split", 0 ).toInt();
    config.histogramOnTop = settings.value ( "histogram/onTop", 1 ).toInt();

    config.demosaicPreview = settings.value ( "demosaic/preview", 0 ).toInt();
    config.demosaicOutput = settings.value ( "demosaic/output", 0 ).toInt();
    config.demosaicMethod = settings.value ( "demosaic/method", 1 ).toInt();
    config.cfaPattern = settings.value ( "demosaic/cfaPattern",
        OA_DEMOSAIC_AUTO ).toInt();

    config.reticleStyle = settings.value ( "reticle/style",
        RETICLE_CIRCLE ).toInt();

    // Give up on earlier versions of this data.  It's too complicated to
    // sort out
    if ( version >= 7 ) {
      int numControls = settings.beginReadArray ( "controls" );
      if ( numControls ) {
        for ( int j = 1; j <= numControls; j++ ) {
          settings.setArrayIndex ( j-1 );
          int numModifiers = settings.beginReadArray ( "modifiers" );
          if ( numModifiers )  {
            for ( int i = 0; i < numModifiers; i++ ) {
              settings.setArrayIndex ( i );
              config.controlValues[i][j] = settings.value ( "controlValue",
                0 ).toInt();
            }
          }
          settings.endArray();
        }
      }
      settings.endArray();
    }

    // For v3 config and earlier we may not have a "none" option here, or if
    // we do it may not be first, so it has to be added and the filter numbers
    // adjusted accordingly.

    if ( version > 3 ) {
      config.numFilters = settings.beginReadArray ( "filters" );
      if ( config.numFilters ) {
        for ( int i = 0; i < config.numFilters; i++ ) {
          settings.setArrayIndex ( i );
          FILTER f;
          f.filterName = settings.value ( "name", "" ).toString();
          config.filters.append ( f );
        }
      } else {
        // FIX ME -- these should probably be configured elsewhere
        QList<QString> defaults;
        defaults << "None" << "L" << "R" << "G" << "B" << "IR" << "UV" <<
            "Ha" << "Hb" << "S2" << "O3" << "CH4";
        config.numFilters = defaults.count();
        for ( int i = 0; i < config.numFilters; i++ ) {
          FILTER f;
          f.filterName = defaults[i];
          config.filters.append ( f );
        }
      }
      settings.endArray();
    } else {
      int numFilters = settings.beginReadArray ( "filters" );
      int totalFilters = 0, renumberFrom = -1, renumberTo = -1;
      if ( numFilters ) {
        for ( int i = 0; i < numFilters; i++ ) {
          settings.setArrayIndex ( i );
          FILTER f;
          f.filterName = settings.value ( "name", "" ).toString();
          // see if this one is called "none"
          if ( !QString::compare ( f.filterName, "none",
              Qt::CaseInsensitive )) {
            // it is.  is it the first?
            if ( i ) {
              // no, then update renumberTo and skip this one as we've
              // already added it.  renumberFrom should already be set
              renumberTo = i - 1;
              continue;
            } // else carry on as normal
          } else {
            if ( !i ) {
              // current string is not "none", so add it if this is our first
              // time through
              FILTER fn;
              fn.filterName = "none";
              config.filters.append ( fn );
              totalFilters++;
              renumberFrom = 0;
              renumberTo = numFilters;
            }
          }
          config.filters.append ( f );
          totalFilters++;
        }
      } else {
        FILTER fn;
        fn.filterName = "none";
        config.filters.append ( fn );
        totalFilters++;
      }
      settings.endArray();
      config.numFilters = totalFilters;
      if ( config.filterOption >= renumberFrom && config.filterOption <=
          renumberTo ) {
        if ( renumberTo < numFilters && config.filterOption ==
            ( renumberTo + 1 )) {
          // we saw "none" and were currently using it
          config.filterOption = 0;
        } else {
          config.filterOption++;
        }
      }
    }

    config.numProfiles = settings.beginReadArray ( "profiles" );
    if ( config.numProfiles ) {
      for ( int i = 0; i < config.numProfiles; i++ ) {
        settings.setArrayIndex ( i );
        PROFILE p;
        p.profileName = settings.value ( "name", "" ).toString();
        p.sixteenBit = settings.value ( "sixteenBit", 0 ).toInt();
        p.binning2x2 = settings.value ( "binning2x2", 0 ).toInt();
        p.rawMode = settings.value ( "raw", 0 ).toInt();
        p.colourise = settings.value ( "colourise", 0 ).toInt();
        p.useROI = settings.value ( "useROI", 0 ).toInt();
        p.imageSizeX = settings.value ( "imageSizeX", 0 ).toInt();
        p.imageSizeY = settings.value ( "imageSizeY", 0 ).toInt();
        if ( version < 3 ) {
          /*
           * This is too much of a mess to set out now.  We'll just let
           * the camera force reasonable defaults later.
           *
          p.controls[ OA_CAM_CTRL_GAIN ] = settings.value (
              "gainValue", 50 ).toInt();
          p.controls[ OA_CAM_CTRL_EXPOSURE_UNSCALED ] = settings.value (
              "exposureValue", 10 ).toInt();
          p.controls[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = settings.value (
              "exposureAbsoluteValue", 10 ).toInt();
          p.controls[ OA_CAM_CTRL_GAMMA ] = settings.value (
              "gammaValue", -1 ).toInt();
          p.controls[ OA_CAM_CTRL_BRIGHTNESS ] = settings.value (
              "brightnessValue", -1 ).toInt();
           */
        } else {
          // FIX ME -- this "if" is redundant
          if ( version > 3 ) {
            int numFilters = settings.beginReadArray ( "filters" );
            if ( numFilters ) {
              for ( int k = 0; k < numFilters; k++ ) {
                settings.setArrayIndex ( k );
                if ( numFilters <= config.numFilters ) {
                  FILTER_PROFILE fp;
                  fp.filterName = config.filters[k].filterName;
                  p.filterProfiles.append ( fp );
                }
                // Give up on anything before version 7.  It's just too
                // messy
                if ( version >= 7 ) {
                  int numControls = settings.beginReadArray ( "controls" );
                  for ( int j = 1; j <= numControls; j++ ) {
                    settings.setArrayIndex ( j-1 );
                    int numModifiers = settings.beginReadArray ( "modifiers" );
                    if ( numModifiers )  { 
                      for ( int i = 0; i < numModifiers; i++ ) {
                        settings.setArrayIndex ( i );
                        if ( numFilters <= config.numFilters ) {
                          p.filterProfiles[ k ].controls[ i ][ j ] =
                              settings.value ( "controlValue", 0 ).toInt();
                        }
                      }
                    }
                    settings.endArray();
                  }
                  settings.endArray();
                }
                p.filterProfiles[ k ].intervalMenuOption = settings.value (
                    "intervalMenuOption", 1 ).toInt(); // default = msec
              }
            }
            settings.endArray();

          } else {
            /*
             * Give up on this as of version 7
             *
            int numControls = settings.beginReadArray ( "controls" );
            if ( config.numFilters ) {
              for ( int k = 0; k < config.numFilters; k++ ) {
                FILTER_PROFILE fp;
                fp.filterName = config.filters[k].filterName;
                p.filterProfiles.append ( fp );
              }
            }
            for ( int j = 0; j < numControls; j++ ) {
              settings.setArrayIndex ( j );
              int controlValue = settings.value ( "controlValue", 0 ).toInt();
              if ( config.numFilters ) {
                for ( int k = 0; k < config.numFilters; k++ ) {
                  p.filterProfiles[ k ].controls[ j ] = controlValue;
                }
              }
            }
            settings.endArray();
             */
            if ( config.numFilters ) {
              for ( int k = 0; k < config.numFilters; k++ ) {
                p.filterProfiles[ k ].intervalMenuOption = 1; // msec
              }
            }
          }

          p.frameRateNumerator = settings.value ( "frameRateNumerator",
              0 ).toInt();
          p.frameRateDenominator = settings.value ( "frameRateDenominator",
              1 ).toInt();
          p.filterOption = settings.value ( "filterOption", 0 ).toInt();
          p.fileTypeOption = settings.value ( "fileTypeOption", 1 ).toInt();
          p.fileNameTemplate = settings.value ( "fileNameTemplate",
              "oaCapture-%DATE-%TIME" ).toString();
          p.limitEnabled = settings.value ( "limitEnabled", 0 ).toInt();
          p.framesLimitValue = settings.value ( "framesLimitValue",
              0 ).toInt();
          p.secondsLimitValue = settings.value ( "secondsLimitValue",
              0 ).toInt();
          p.limitType = settings.value ( "limitType", -1 ).toInt();
          p.target = settings.value ( "target", 0 ).toInt();
          config.profiles.append ( p );
        }
      }
      settings.endArray();

    } else {
      // if we have no profiles we create a default one

      PROFILE p;
      p.profileName = "default";
      p.sixteenBit = config.sixteenBit;
      p.binning2x2 = config.binning2x2;
      p.rawMode = config.rawMode;
      p.colourise = config.colourise;
      p.useROI = config.useROI;
      p.imageSizeX = config.imageSizeX;
      p.imageSizeY = config.imageSizeY;
      if ( config.numFilters ) {
        for ( int k = 0; k < config.numFilters; k++ ) {
          FILTER_PROFILE fp;
          fp.filterName = config.filters[k].filterName;
          fp.intervalMenuOption = 1; // msec
          p.filterProfiles.append ( fp );
        }
      }
      for ( int j = 1; j < OA_CAM_CTRL_LAST_P1; j++ ) {
        if ( config.numFilters ) {
	  for ( int k = 0; k < config.numFilters; k++ ) {
            for ( int i = 0; i < OA_CAM_CTRL_MODIFIERS_P1; i++ ) {
              p.filterProfiles[ k ].controls[ i ][ j ] =
                  config.controlValues[ i ][ j ];
            }
          }
        }
      }

      p.frameRateNumerator = config.frameRateNumerator;
      p.frameRateDenominator = config.frameRateDenominator;
      p.filterOption = config.filterOption;
      p.fileTypeOption = config.fileTypeOption;
      p.fileNameTemplate = config.fileNameTemplate;
      p.limitEnabled = config.limitEnabled;
      p.framesLimitValue = config.framesLimitValue;
      p.secondsLimitValue = config.secondsLimitValue;
      p.limitType = config.limitType;
      p.target = TGT_UNKNOWN;
      config.profiles.append ( p );
      config.numProfiles = 1;
    }

    if ( version > 4 ) {
      ( void ) settings.beginReadArray ( "filterSlots" );
      for ( int i = 0; i < MAX_FILTER_SLOTS; i++ ) {
        settings.setArrayIndex ( i );
        config.filterSlots[i] = settings.value ( "slot", -1 ).toInt();
      }
      settings.endArray();

      int numSeqs = settings.beginReadArray ( "filterSequence" );
      if ( numSeqs ) {
        for ( int i = 0; i < numSeqs; i++ ) {
          settings.setArrayIndex ( i );
          config.autorunFilterSequence.append ( settings.value (
              "slot", -1 ).toInt());
        }
      }
      settings.endArray();
    }

    config.currentColouriseColour = QColor ( 255, 255, 255 );
    config.numCustomColours = 0;
    config.customColours.clear();

    if ( version > 5 ) {
      int r = settings.value ( "colourise/currentColour/red", 255 ).toInt();
      int g = settings.value ( "colourise/currentColour/green", 255 ).toInt();
      int b = settings.value ( "colourise/currentColour/blue", 255 ).toInt();
      config.currentColouriseColour = QColor ( r, g, b );
      config.numCustomColours = settings.beginReadArray (
          "colourise/customColours" );
      if ( config.numCustomColours ) {
        for ( int i = 0; i < config.numCustomColours; i++ ) {
          settings.setArrayIndex ( i );
          r = settings.value ( "red", 255 ).toInt();
          b = settings.value ( "blue", 255 ).toInt();
          g = settings.value ( "green", 255 ).toInt();
          config.customColours.append ( QColor ( r, g, b ));
        }
      }
      settings.endArray();
    }
  }

  config.filterWheelConfig.clear();
  for ( int i = 0; i < OA_FW_IF_COUNT; i++ ) {
    userConfigList	conf;
    conf.clear();
    config.filterWheelConfig.append ( conf );
  }
  int numInterfaces = settings.beginReadArray ( "filterWheelUserConfig" );
  if ( numInterfaces ) {
    for ( int i = 0; i < numInterfaces; i++ ) {
      settings.setArrayIndex ( i );
      int numMatches = settings.beginReadArray ( "matches" );
      if ( numMatches ) {
        for ( int j = 0; j < numMatches; j++ ) {
          settings.setArrayIndex ( j );
          userDeviceConfig c;
          c.vendorId = settings.value ( "vendorId", 0 ).toInt();
          c.productId = settings.value ( "productId", 0 ).toInt();
          ( void ) strcpy ( c.manufacturer, settings.value ( "manufacturer",
              0 ).toString().toStdString().c_str());
          ( void ) strcpy ( c.product, settings.value ( "product",
              0 ).toString().toStdString().c_str());
          ( void ) strcpy ( c.serialNo, settings.value ( "serialNo",
              0 ).toString().toStdString().c_str());
          ( void ) strcpy ( c.filesystemPath, settings.value ( "fsPath",
              0 ).toString().toStdString().c_str());
          config.filterWheelConfig[i].append ( c );
        }
      }
      settings.endArray();
    }
    settings.endArray();
  }

  config.timerConfig.clear();
  for ( int i = 0; i < OA_TIMER_IF_COUNT; i++ ) {
    userConfigList      conf;
    conf.clear();
    config.timerConfig.append ( conf );
  }
  numInterfaces = settings.beginReadArray ( "ptrUserConfig" );
  if ( numInterfaces ) {
    for ( int i = 0; i < numInterfaces; i++ ) {
      settings.setArrayIndex ( i );
      int numMatches = settings.beginReadArray ( "matches" );
      if ( numMatches ) {
        for ( int j = 0; j < numMatches; j++ ) {
          settings.setArrayIndex ( j );
          userDeviceConfig c;
          c.vendorId = settings.value ( "vendorId", 0 ).toInt();
          c.productId = settings.value ( "productId", 0 ).toInt();
          ( void ) strcpy ( c.manufacturer, settings.value ( "manufacturer",
              0 ).toString().toStdString().c_str());
          ( void ) strcpy ( c.product, settings.value ( "product",
              0 ).toString().toStdString().c_str());
          ( void ) strcpy ( c.serialNo, settings.value ( "serialNo",
              0 ).toString().toStdString().c_str());
          ( void ) strcpy ( c.filesystemPath, settings.value ( "fsPath",
              0 ).toString().toStdString().c_str());
          config.timerConfig[i].append ( c );
        }
      }
      settings.endArray();
    }
    settings.endArray();
  }

  if ( !config.saveSettings || config.saveSettings == -1 ) {
    config.saveSettings = -config.saveSettings;
  }

  config.fitsObserver = settings.value ( "fits/observer", "" ).toString();
  config.fitsInstrument = settings.value ( "fits/instrument", "" ).toString();
  config.fitsObject = settings.value ( "fits/object", "" ).toString();
  config.fitsComment = settings.value ( "fits/comment", "" ).toString();
  config.fitsTelescope = settings.value ( "fits/telescope", "" ).toString();
  config.fitsFocalLength = settings.value (
      "fits/focalLength", "" ).toString();
  config.fitsApertureDia = settings.value (
      "fits/apertureDia", "" ).toString();
  config.fitsApertureArea = settings.value (
      "fits/apertureArea", "" ).toString();
  config.fitsPixelSizeX = settings.value ( "fits/pixelSizeX", "" ).toString();
  config.fitsPixelSizeY = settings.value ( "fits/pixelSizeY", "" ).toString();
  config.fitsSubframeOriginX = settings.value (
      "fits/subframeOriginX", "" ).toString();
  config.fitsSubframeOriginY = settings.value (
      "fits/subframeOriginY", "" ).toString();
  config.fitsSiteLatitude = settings.value (
      "fits/siteLatitude", "" ).toString();
  config.fitsSiteLongitude = settings.value (
      "fits/siteLongitude", "" ).toString();
  config.fitsFilter = settings.value ( "fits/filter", "" ).toString();

  config.timerMode = settings.value ( "timer/mode",
      OA_TIMER_MODE_UNSET ).toInt();
  config.timerEnabled = settings.value ( "timer/enabled", 0 ).toInt();
  config.triggerInterval = settings.value ( "timer/triggerInterval",
      1 ).toInt();
  config.userDrainDelayEnabled = settings.value ( "timer/drainDelayEnabled",
      0 ).toInt();
  config.drainDelay = settings.value ( "timer/drainDelay", 500 ).toInt();
  config.timestampDelay = settings.value ( "timer/timestampDelay",
      50 ).toInt();
  config.queryGPSForEachCapture = settings.value (
      "timer/queryGPSForEachCapture", 0 ).toInt();
}


void
MainWindow::writeConfig ( void )
{
  if ( !config.saveSettings ) {
    return;
  }

  QSettings settings ( ORGANISATION_NAME_SETTINGS, APPLICATION_NAME );
  settings.clear();

  settings.setValue ( "saveSettings", config.saveSettings );

  settings.setValue ( "configVersion", CONFIG_VERSION );
  settings.setValue ( "geometry", geometry());

  settings.setValue ( "tempsInCentigrade", config.tempsInC );
  settings.setValue ( "connectSoleCamera", config.connectSoleCamera );
  settings.setValue ( "dockableControls", config.dockableControls );
  settings.setValue ( "controlsOnRight", config.controlsOnRight );
  settings.setValue ( "separateControls", config.separateControls );
  settings.setValue ( "saveCaptureSettings", config.saveCaptureSettings );
  settings.setValue ( "windowsCompatibleAVI", config.windowsCompatibleAVI );
  settings.setValue ( "useUtVideo", config.useUtVideo );
  settings.setValue ( "indexDigits", config.indexDigits );

  // FIX ME -- how to handle this?
  // settings.setValue ( "device/camera", -1 ).toInt();

  settings.setValue ( "options/showHistogram", config.showHistogram );
  settings.setValue ( "options/autoAlign", config.autoAlign );
  settings.setValue ( "options/showReticle", config.showReticle );
  settings.setValue ( "options/cutout", config.cutout );
  settings.setValue ( "options/showFocusAid", config.showFocusAid );
  settings.setValue ( "options/darkFrame", config.darkFrame );
  settings.setValue ( "options/flipX", config.flipX );
  settings.setValue ( "options/flipY", config.flipY );
  settings.setValue ( "options/demosaic", config.demosaic );

  settings.setValue ( "camera/sixteenBit", config.sixteenBit );
  settings.setValue ( "camera/binning2x2", config.binning2x2 );
  settings.setValue ( "camera/raw", config.rawMode );
  settings.setValue ( "camera/colourise", config.colourise );

  settings.setValue ( "image/useROI", config.useROI );
  settings.setValue ( "image/imageSizeX", config.imageSizeX );
  settings.setValue ( "image/imageSizeY", config.imageSizeY );

  settings.setValue ( "image/zoomButton1Option", config.zoomButton1Option );
  settings.setValue ( "image/zoomButton2Option", config.zoomButton2Option );
  settings.setValue ( "image/zoomButton3Option", config.zoomButton3Option );
  settings.setValue ( "image/zoomValue", config.zoomValue );

  settings.setValue ( "control/exposureMenuOption", config.exposureMenuOption );
  settings.setValue ( "control/frameRateNumerator", config.frameRateNumerator );
  settings.setValue ( "control/frameRateDenominator",
      config.frameRateDenominator );
  settings.setValue ( "control/selectableControl1",
      config.selectableControl[0] );
  settings.setValue ( "control/selectableControl2",
      config.selectableControl[1] );
  settings.setValue ( "control/intervalMenuOption",
      config.intervalMenuOption );

  settings.setValue ( "control/profileOption", config.profileOption );
  settings.setValue ( "control/filterOption", config.filterOption );
  settings.setValue ( "control/fileTypeOption", config.fileTypeOption );
  settings.setValue ( "control/limitEnabled", config.limitEnabled );
  settings.setValue ( "control/framesLimitValue", config.framesLimitValue );
  settings.setValue ( "control/secondsLimitValue", config.secondsLimitValue );
  settings.setValue ( "control/limitType", config.limitType );
  settings.setValue ( "control/fileNameTemplate", config.fileNameTemplate );
  settings.setValue ( "control/captureDirectory", config.captureDirectory );

  settings.setValue ( "autorun/count", config.autorunCount );
  settings.setValue ( "autorun/delay", config.autorunDelay );
  settings.setValue ( "autorun/filterPrompt", config.promptForFilterChange );
  settings.setValue ( "autorun/interFilterDelay",
      config.interFilterDelay );

  settings.setValue ( "display/preview", config.preview );
  settings.setValue ( "display/nightMode", config.nightMode );
  settings.setValue ( "display/displayFPS", config.displayFPS );

  settings.setValue ( "histogram/split", config.splitHistogram );
  settings.setValue ( "histogram/onTop", config.histogramOnTop );

  settings.setValue ( "demosaic/preview", config.demosaicPreview );
  settings.setValue ( "demosaic/output", config.demosaicOutput );
  settings.setValue ( "demosaic/method", config.demosaicMethod );
  settings.setValue ( "demosaic/cfaPattern", config.cfaPattern );

  settings.setValue ( "reticle/style", config.reticleStyle );

  settings.beginWriteArray ( "controls" );
  for ( int i = 1; i < OA_CAM_CTRL_LAST_P1; i++ ) {
    settings.setArrayIndex ( i-1 );
    // don't particularly like this cast, but it seems to be the only way
    // to do it
    settings.setValue ( "controlValue",
        ( qlonglong ) config.controlValues[i] );
  }
  settings.endArray();

  settings.beginWriteArray ( "filters" );
  if ( config.numFilters ) {
    for ( int i = 0; i < config.numFilters; i++ ) {
      settings.setArrayIndex ( i );
      settings.setValue ( "name", config.filters[i].filterName );
    }
  }
  settings.endArray();

  settings.beginWriteArray ( "profiles" );
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      settings.setArrayIndex ( i );
      settings.setValue ( "name", config.profiles[i].profileName );
      settings.setValue ( "sixteenBit", config.profiles[i].sixteenBit );
      settings.setValue ( "binning2x2", config.profiles[i].binning2x2 );
      settings.setValue ( "raw", config.profiles[i].rawMode );
      settings.setValue ( "colourise", config.profiles[i].colourise );
      settings.setValue ( "useROI", config.profiles[i].useROI );
      settings.setValue ( "imageSizeX", config.profiles[i].imageSizeX );
      settings.setValue ( "imageSizeY", config.profiles[i].imageSizeY );

      if ( config.numFilters &&
          !config.profiles[ i ].filterProfiles.isEmpty()) {
        settings.beginWriteArray ( "filters" );
        for ( int j = 0; j < config.numFilters; j++ ) {
          settings.setArrayIndex ( j );
          settings.setValue ( "intervalMenuOption",
              config.profiles[ i ].filterProfiles[ j ].intervalMenuOption );
          settings.beginWriteArray ( "controls" );
          for ( int k = 1; k < OA_CAM_CTRL_LAST_P1; k++ ) {
            settings.setArrayIndex ( k );
            settings.beginWriteArray ( "modifiers" );
            for ( int l = 0; l < OA_CAM_CTRL_MODIFIERS_P1; l++ ) {
              settings.setArrayIndex ( l );
              settings.setValue ( "controlValue",
                  config.profiles[ i ].filterProfiles[ j ].controls[ l ][ k ]);
            }
            settings.endArray();
          }
          settings.endArray();
        }
        settings.endArray();
      }
      settings.setValue ( "frameRateNumerator",
          config.profiles[i].frameRateNumerator );
      settings.setValue ( "frameRateDenominator",
          config.profiles[i].frameRateDenominator );
      settings.setValue ( "filterOption", config.profiles[i].filterOption );
      settings.setValue ( "fileTypeOption", config.profiles[i].fileTypeOption );
      settings.setValue ( "fileNameTemplate",
          config.profiles[i].fileNameTemplate );
      settings.setValue ( "limitEnabled", config.profiles[i].limitEnabled );
      settings.setValue ( "framesLimitValue",
          config.profiles[i].framesLimitValue );
      settings.setValue ( "secondsLimitValue",
          config.profiles[i].secondsLimitValue );
      settings.setValue ( "limitType", config.profiles[i].limitType );
      settings.setValue ( "target", config.profiles[i].target );
    }
  }
  settings.endArray();

  settings.beginWriteArray ( "filterSlots" );
  for ( int i = 0; i < MAX_FILTER_SLOTS; i++ ) {
    settings.setArrayIndex ( i );
    settings.setValue ( "slot", config.filterSlots[i] );
  }
  settings.endArray();

  settings.beginWriteArray ( "filterSequence" );
  int numSeqs;
  if (( numSeqs = config.autorunFilterSequence.count())) {
    for ( int i = 0; i < numSeqs; i++ ) {
      settings.setArrayIndex ( i );
      settings.setValue ( "slot", config.autorunFilterSequence[i] );
    }
  }
  settings.endArray();

  settings.beginWriteArray ( "filterWheelUserConfig" );
  int numInterfaces = config.filterWheelConfig.count();
  for ( int i = 0; i < numInterfaces; i++ ) {
    settings.setArrayIndex ( i );
    settings.beginWriteArray ( "matches" );
    int numMatches = config.filterWheelConfig[i].count();
    userConfigList confList = config.filterWheelConfig[i];
    for ( int j = 0; j < numMatches; j++ ) {
      settings.setArrayIndex ( j );
      settings.setValue ( "vendorId", confList[j].vendorId );
      settings.setValue ( "productId", confList[j].productId );
      settings.setValue ( "manufacturer", confList[j].manufacturer );
      settings.setValue ( "product", confList[j].product );
      settings.setValue ( "serialNo", confList[j].serialNo );
      settings.setValue ( "fsPath", confList[j].filesystemPath );
    }
    settings.endArray();
  }
  settings.endArray();

  settings.beginWriteArray ( "ptrUserConfig" );
  numInterfaces = config.timerConfig.count();
  for ( int i = 0; i < numInterfaces; i++ ) {
    settings.setArrayIndex ( i );
    settings.beginWriteArray ( "matches" );
    int numMatches = config.timerConfig[i].count();
    userConfigList confList = config.timerConfig[i];
    for ( int j = 0; j < numMatches; j++ ) {
      settings.setArrayIndex ( j );
      settings.setValue ( "vendorId", confList[j].vendorId );
      settings.setValue ( "productId", confList[j].productId );
      settings.setValue ( "manufacturer", confList[j].manufacturer );
      settings.setValue ( "product", confList[j].product );
      settings.setValue ( "serialNo", confList[j].serialNo );
      settings.setValue ( "fsPath", confList[j].filesystemPath );
    }
    settings.endArray();
  }
  settings.endArray();

  int r = config.currentColouriseColour.red();
  int g = config.currentColouriseColour.green();
  int b = config.currentColouriseColour.blue();
  settings.setValue ( "colourise/currentColour/red", r );
  settings.setValue ( "colourise/currentColour/green", g );
  settings.setValue ( "colourise/currentColour/blue", b );
  settings.beginWriteArray ( "colourise/customColours" );
  for ( int i = 0; i < config.numCustomColours; i++ ) {
    settings.setArrayIndex ( i );
    r = config.customColours[i].red();
    g = config.customColours[i].green();
    b = config.customColours[i].blue();
    settings.setValue ( "red", r );
    settings.setValue ( "green", g );
    settings.setValue ( "blue", b );
  }
  settings.endArray();

  settings.setValue ( "fits/observer", config.fitsObserver );
  settings.setValue ( "fits/instrument", config.fitsInstrument );
  settings.setValue ( "fits/object", config.fitsObject );
  settings.setValue ( "fits/comment", config.fitsComment );
  settings.setValue ( "fits/telescope", config.fitsTelescope );
  settings.setValue ( "fits/focalLength", config.fitsFocalLength );
  settings.setValue ( "fits/apertureDia", config.fitsApertureDia );
  settings.setValue ( "fits/apertureArea", config.fitsApertureArea );
  settings.setValue ( "fits/pixelSizeX", config.fitsPixelSizeX );
  settings.setValue ( "fits/pixelSizeY", config.fitsPixelSizeY );
  settings.setValue ( "fits/subframeOriginX", config.fitsSubframeOriginX );
  settings.setValue ( "fits/subframeOriginY", config.fitsSubframeOriginY );
  settings.setValue ( "fits/siteLatitude", config.fitsSiteLatitude );
  settings.setValue ( "fits/siteLongitude", config.fitsSiteLongitude );
  settings.setValue ( "fits/filter", config.fitsFilter );

  settings.setValue ( "timer/mode", config.timerMode );
  settings.setValue ( "timer/enabled", config.timerEnabled );
  settings.setValue ( "timer/triggerInterval", config.triggerInterval );
  settings.setValue ( "timer/drainDelayEnabled", config.userDrainDelayEnabled );
  settings.setValue ( "timer/drainDelay", config.drainDelay );
  settings.setValue ( "timer/timestampDelay", config.timestampDelay );
  settings.setValue ( "timer/queryGPSForEachCapture",
      config.queryGPSForEachCapture );
}


void
MainWindow::createStatusBar ( void )
{
  statusLine = statusBar();
  setStatusBar ( statusLine );

  tempLabel = new QLabel();
  if ( config.tempsInC ) {
    tempLabel->setText ( tr ( "Temp (C)" ));
  } else {
    tempLabel->setText ( tr ( "Temp (F)" ));
  }
  tempLabel->setFixedWidth ( 60 );
  pixelFormatLabel = new QLabel ( tr ( "Pixel format" ));
  pixelFormatLabel->setFixedWidth ( 80 );
  fpsMaxLabel = new QLabel ( tr ( "FPS (max)" ));
  fpsMaxLabel->setFixedWidth ( 65 );
  fpsActualLabel = new QLabel ( tr ( "FPS (actual)" ));
  fpsActualLabel->setFixedWidth ( 80 );
  capturedLabel = new QLabel ( tr ( "Captured" ));
  capturedLabel->setFixedWidth ( 60 );
  droppedLabel = new QLabel ( tr ( "Dropped" ));
  droppedLabel->setFixedWidth ( 55 );
  progressBar = new QProgressBar;
  progressBar->setFixedWidth ( 200 );
  progressBar->setRange ( 0, 100 );
  progressBar->setTextVisible ( true );

  tempValue = new QLabel ( "" );
  tempValue->setFixedWidth ( 30 );
  pixelFormatValue = new QLabel ( "" );
  pixelFormatValue->setFixedWidth ( 70 );
  fpsMaxValue = new QLabel ( "0" );
  fpsMaxValue->setFixedWidth ( 30 );
  fpsActualValue = new QLabel ( "0" );
  fpsActualValue->setFixedWidth ( 50 );
  capturedValue = new QLabel ( "0" );
  capturedValue->setFixedWidth ( 40 );
  droppedValue = new QLabel ( "0" );
  droppedValue->setFixedWidth ( 40 );

  statusLine->addPermanentWidget ( tempLabel );
  statusLine->addPermanentWidget ( tempValue );
  statusLine->addPermanentWidget ( pixelFormatLabel );
  statusLine->addPermanentWidget ( pixelFormatValue );
  statusLine->addPermanentWidget ( fpsMaxLabel );
  statusLine->addPermanentWidget ( fpsMaxValue );
  statusLine->addPermanentWidget ( fpsActualLabel );
  statusLine->addPermanentWidget ( fpsActualValue );
  statusLine->addPermanentWidget ( capturedLabel );
  statusLine->addPermanentWidget ( capturedValue );
  statusLine->addPermanentWidget ( droppedLabel );
  statusLine->addPermanentWidget ( droppedValue );
  statusLine->addPermanentWidget ( progressBar );
  
  statusLine->showMessage ( tr ( "started" ));
}


void
MainWindow::createMenus ( void )
{
  // FIX ME -- add "restore program defaults" option

  // File menu
  loadConfig = new QAction ( tr ( "Re&load Config" ), this );
  loadConfig->setStatusTip ( tr ( "Load default configuration" ));
  // FIX ME - set up slots

  saveConfig = new QAction ( tr ( "&Save Config" ), this );
  saveConfig->setShortcut ( QKeySequence::Save );
  saveConfig->setStatusTip ( tr ( "Save default configuration" ));
  // FIX ME - set up slots

  exit = new QAction ( tr ( "&Quit" ), this );
  exit->setShortcut ( QKeySequence::Quit );
  connect ( exit, SIGNAL( triggered()), this, SLOT( quit()));

  fileMenu = menuBar()->addMenu( tr ( "&File" ));
  fileMenu->addAction ( loadConfig );
  fileMenu->addAction ( saveConfig );
  fileMenu->addSeparator();
  fileMenu->addAction ( exit );

  // Camera device menu

  cameraMenu = menuBar()->addMenu ( tr ( "&Camera" ));
  doCameraMenu(0);

  // Filter wheel menu

  filterWheelMenu = menuBar()->addMenu ( tr ( "&Filter Wheel" ));
  doFilterWheelMenu(0);

  // Timer menu

  timerMenu = menuBar()->addMenu ( tr ( "&Timer" ));
  doTimerMenu(0);

  // Options menu

  histogramOpt = new QAction ( QIcon ( ":/qt-icons/barchart.png" ),
      tr ( "Histogram" ), this );
  histogramOpt->setStatusTip ( tr ( "Open window for image histogram" ));
  histogramOpt->setCheckable ( true );
  connect ( histogramOpt, SIGNAL( changed()), this, SLOT( enableHistogram()));
  histogramOpt->setChecked ( config.showHistogram );

  autoalign = new QAction ( tr ( "Auto Align" ), this );
  autoalign->setCheckable ( true );
  // FIX ME - set up slots

  alignbox = new QAction ( tr ( "Align Box" ), this );
  alignbox->setCheckable ( true );
  // FIX ME - set up slots

  autoguide = new QAction ( tr ( "Auto Guide" ), this );
  autoguide->setCheckable ( true );
  // FIX ME - set up slots

  reticle = new QAction ( QIcon ( ":/qt-icons/reticle.png" ),
      tr ( "Reticle" ), this );
  reticle->setStatusTip ( tr ( "Overlay a reticle on the preview image" ));
  reticle->setCheckable ( true );
  connect ( reticle, SIGNAL( changed()), this, SLOT( enableReticle()));
  reticle->setChecked ( config.showReticle );

  cutout = new QAction ( tr ( "Cut Out" ), this );
  cutout->setCheckable ( true );
  // FIX ME - set up slots

  focusaid = new QAction ( tr ( "Focus Aid" ), this );
  focusaid->setCheckable ( true );
  connect ( focusaid, SIGNAL( changed()), this, SLOT( enableFocusAid()));

  darkframe = new QAction ( tr ( "Dark Frame" ), this );
  darkframe->setCheckable ( true );
  // FIX ME - set up slots

  flipX = new QAction ( QIcon ( ":/qt-icons/object-flip-horizontal.png" ), 
      tr ( "Flip X" ), this );
  flipX->setStatusTip ( tr ( "Flip image left<->right" ));
  flipX->setCheckable ( true );
  flipX->setChecked ( config.flipX );
  connect ( flipX, SIGNAL( changed()), this, SLOT( enableFlipX()));

  flipY = new QAction ( QIcon ( ":/qt-icons/object-flip-vertical.png" ),
      tr ( "Flip Y" ), this );
  flipY->setStatusTip ( tr ( "Flip image top<->bottom" ));
  flipY->setCheckable ( true );
  flipY->setChecked ( config.flipY );
  connect ( flipY, SIGNAL( changed()), this, SLOT( enableFlipY()));

  demosaicOpt = new QAction ( QIcon ( ":/qt-icons/mosaic.png" ),
      tr ( "Demosaic" ), this );
  demosaicOpt->setCheckable ( true );
  demosaicOpt->setChecked ( config.demosaic );
  connect ( demosaicOpt, SIGNAL( changed()), this, SLOT( enableDemosaic()));

  optionsMenu = menuBar()->addMenu ( tr ( "&Options" ));
  optionsMenu->addAction ( histogramOpt );
#ifdef ENABLE_AUTOALIGN
  optionsMenu->addAction ( autoalign );
#endif
#ifdef ENABLE_ALIGNBOX
  optionsMenu->addAction ( alignbox );
#endif
#ifdef ENABLE_AUTOGUIDE
  optionsMenu->addAction ( autoguide );
#endif
  optionsMenu->addAction ( reticle );
#ifdef ENABLE_CUTOUT
  optionsMenu->addAction ( cutout );
#endif
  optionsMenu->addAction ( focusaid );
#ifdef ENABLE_DARKFRAME
  optionsMenu->addAction ( darkframe );
#endif
  optionsMenu->addAction ( flipX );
  optionsMenu->addAction ( flipY );
  optionsMenu->addAction ( demosaicOpt );

  // settings menu

  general = new QAction ( QIcon ( ":/qt-icons/cog.png" ),
      tr ( "General" ), this );
  general->setStatusTip ( tr ( "General configuration" ));
  connect ( general, SIGNAL( triggered()), this, SLOT( doGeneralSettings()));

  capture = new QAction ( QIcon ( ":/qt-icons/capture.png" ),
      tr ( "Capture" ), this );
  connect ( capture, SIGNAL( triggered()), this, SLOT( doCaptureSettings()));

  cameraOpt = new QAction ( QIcon ( ":/qt-icons/planetary-camera.png" ),
      tr ( "Camera" ), this );
  connect ( cameraOpt, SIGNAL( triggered()), this, SLOT( doCameraSettings()));
  // not enabled until we connect a camera
  cameraOpt->setEnabled ( 0 );

  profiles = new QAction ( QIcon ( ":/qt-icons/jupiter.png" ),
      tr ( "Profiles" ), this );
  profiles->setStatusTip ( tr ( "Edit saved profiles" ));
  connect ( profiles, SIGNAL( triggered()), this, SLOT( doProfileSettings()));

  filters = new QAction ( QIcon ( ":/qt-icons/filter-wheel.png" ),
      tr ( "Filters" ), this );
  filters->setStatusTip ( tr ( "Configuration for filters" ));
  connect ( filters, SIGNAL( triggered()), this, SLOT( doFilterSettings()));

  demosaic = new QAction ( QIcon ( ":/qt-icons/mosaic.png" ),
      tr ( "Demosaic" ), this );
  demosaic->setStatusTip ( tr ( "Configuration for demosaicking" ));
  connect ( demosaic, SIGNAL( triggered()), this, SLOT( doDemosaicSettings()));

  fits = new QAction ( QIcon ( ":/qt-icons/fits.png" ),
      tr ( "FITS/SER Metadata" ), this );
  fits->setStatusTip ( tr ( "Configuration for FITS/SER metadata keywords" ));
  connect ( fits, SIGNAL( triggered()), this, SLOT( doFITSSettings()));

  autorun = new QAction ( QIcon ( ":/qt-icons/clicknrun.png" ),
      tr ( "Autorun" ), this );
  autorun->setStatusTip ( tr ( "Configuration for repeat captures" ));
  connect ( autorun, SIGNAL( triggered()), this, SLOT( doAutorunSettings()));

  histogram = new QAction ( QIcon ( ":/qt-icons/barchart.png" ),
      tr ( "Histogram" ), this );
  histogram->setStatusTip ( tr ( "Configuration for histogram" ));
  connect ( histogram, SIGNAL( triggered()), this,
      SLOT( doHistogramSettings()));

  colourise = new QAction ( QIcon ( ":/qt-icons/sun.png" ),
      tr ( "False Colour" ), this );
  connect ( colourise, SIGNAL( triggered()), this,
      SLOT( doColouriseSettings()));

  timer = new QAction ( QIcon ( ":/qt-icons/timer.png" ),
      tr ( "Timer" ), this );
  timer->setStatusTip ( tr ( "Configuration for Timer unit" ));
  connect ( timer, SIGNAL( triggered()), this, SLOT( doTimerSettings()));

  settingsMenu = menuBar()->addMenu ( tr ( "&Settings" ));
  settingsMenu->addAction ( general );
  settingsMenu->addAction ( capture );
  settingsMenu->addAction ( cameraOpt );
  settingsMenu->addAction ( profiles );
  settingsMenu->addAction ( filters );
  settingsMenu->addAction ( demosaic );
  settingsMenu->addAction ( fits );
  settingsMenu->addAction ( autorun );
  settingsMenu->addAction ( histogram );
  settingsMenu->addAction ( colourise );
  settingsMenu->addAction ( timer );

  // For the moment we only add the advanced menu if there are filter
  // wheels with user-configurable interfaces

  int requireAdvanced = 0;
  for ( int i = 1; i < OA_FW_IF_COUNT && !requireAdvanced; i++ ) {
    if ( oaFilterWheelInterfaces[ i ].userConfigFlags ) {
      requireAdvanced = 1;
    }
  }

  if ( requireAdvanced ) {
    advancedMenu = menuBar()->addMenu ( tr ( "&Advanced" ));
    doAdvancedMenu();
  }

  // help menu

  about = new QAction ( tr ( "About" ), this );
  connect ( about, SIGNAL( triggered()), this, SLOT( aboutDialog()));

  helpMenu = menuBar()->addMenu ( tr ( "&Help" ));
  helpMenu->addAction ( about );
}


void
MainWindow::connectCamera ( int deviceIndex )
{
  int v, ret, attempt, format;

  if ( -1 == oldHistogramState ) {
    oldHistogramState = state.histogramOn;
  }
  state.histogramOn = 0;
  doDisconnectCam();
  config.sixteenBit = 0;
  config.rawMode = 0;
  SET_PROFILE_CONFIG( sixteenBit, 0 );
  SET_PROFILE_CONFIG( rawMode, 0 );

  for ( attempt = 0, ret = 1; ret == 1 && attempt < 2; attempt++ ) {
    if (( ret = state.camera->initialise ( cameraDevs[ deviceIndex ] ))) {
      if ( !attempt && ret == 1 ) {
        if ( connectedCameras == 1 ) {
          // we think a rescan should be sufficient to identify the camera
          // as there's only one that we know about.  deviceIndex should
          // be 0 by definition for this
          int retries = 5, haveCamera = 0;
          do {
            doCameraMenu(1);
            if ( connectedCameras ) {
              haveCamera = 1;
            } else {
              sleep(1);
            }
            retries--;
          } while ( !connectedCameras && retries );
          if ( haveCamera && connectedCameras == 1 ) {
            continue;
          }
          QMessageBox::warning ( this, APPLICATION_NAME,
              tr ( "The firmware has loaded, but a rescan is required "
              "and the camera must be selected again." ));
        } else {
          QMessageBox::warning ( this, APPLICATION_NAME,
              tr ( "The firmware has loaded, but a rescan is required "
              "and the camera must be selected again." ));
        }
      } else {
        QMessageBox::warning ( this, APPLICATION_NAME,
            tr ( "Unable to connect camera" ));
      }
      state.histogramOn = oldHistogramState;
      return;
    }
  }

  disconnectCam->setEnabled( 1 );
  rescanCam->setEnabled( 0 );
  // Now it gets a bit messy.  The camera should get the settings from
  // the current profile, but the configure() functions take the current
  // values, set them in the camera and write them to the current
  // profile.
  if ( config.profileOption >= 0 && config.profileOption <
      config.numProfiles && config.filterOption >= 0 && config.filterOption <
      config.numFilters ) {
    for ( uint8_t c = 1; c < OA_CAM_CTRL_LAST_P1; c++ ) {
      for ( uint8_t m = 1; m < OA_CAM_CTRL_MODIFIERS_P1; m++ ) {
        config.controlValues[ m ][ c ] =
          config.profiles[ config.profileOption ].filterProfiles[
              config.filterOption ].controls[ m ][ c ];
      }
    }
    config.intervalMenuOption = config.profiles[ config.profileOption ].
        filterProfiles[ config.filterOption ].intervalMenuOption;
  }
  configure();
  statusLine->showMessage ( state.camera->name() + tr ( " connected" ), 5000 );
  clearTemperature();
  clearDroppedFrames();
  state.captureWidget->enableStartButton ( 1 );
  state.captureWidget->enableProfileSelect ( 1 );
  // FIX ME -- should these happen in the "configure" functions for each
  // widget?
  state.previewWidget->setVideoFramePixelFormat (
      state.camera->videoFramePixelFormat());
  state.cameraWidget->enableBinningControl ( state.camera->hasBinning ( 2 ));
  v = state.camera->hasControl ( OA_CAM_CTRL_TEMPERATURE );
  state.previewWidget->enableTempDisplay ( v );
  styleStatusBarTemp ( v );
  v = state.camera->hasControl ( OA_CAM_CTRL_DROPPED );
  state.previewWidget->enableDroppedDisplay ( v );
  styleStatusBarDroppedFrames ( v );
  if ( state.settingsWidget ) {
    state.settingsWidget->enableTab ( state.cameraSettingsIndex, 1 );
  }
  cameraOpt->setEnabled ( 1 );

  enableFlipX();
  enableFlipY();

  // start regardless of whether we're displaying or capturing the
  // data
  state.camera->start();
  state.controlWidget->disableAutoControls();
  state.histogramOn = oldHistogramState;
  oldHistogramState = -1;

  format = state.camera->videoFramePixelFormat();
  state.captureWidget->enableTIFFCapture (( !OA_ISBAYER( format ) ||
      ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
  state.captureWidget->enablePNGCapture (( !OA_ISBAYER( format ) ||
      ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
  state.captureWidget->enableMOVCapture (( QUICKTIME_OK( format ) || 
      ( OA_ISBAYER( format ) && config.demosaic &&
      config.demosaicOutput )) ? 1 : 0 );
}


void
MainWindow::disconnectCamera ( void )
{
  state.cameraTempValid = 0;
  state.binningValid = 0;
  if ( state.settingsWidget ) {
    state.settingsWidget->enableTab ( state.cameraSettingsIndex, 0 );
  }
  cameraOpt->setEnabled ( 0 );
  oldHistogramState = state.histogramOn;
  state.histogramOn = 0;
  state.captureWidget->enableStartButton ( 0 );
  state.captureWidget->enableProfileSelect ( 0 );
  doDisconnectCam();
  statusLine->showMessage ( tr ( "Camera disconnected" ));
}


void
MainWindow::doDisconnectCam ( void )
{
  if ( state.camera && state.camera->isInitialised()) {
    if ( state.captureWidget ) {
      state.captureWidget->closeOutputHandler();
    }
    state.camera->stop();
    state.camera->disconnect();
    disconnectCam->setEnabled( 0 );
    rescanCam->setEnabled( 1 );
  }
}


void
MainWindow::rescanCameras ( void )
{
  doCameraMenu(0);
}


void
MainWindow::connectFilterWheel ( int deviceIndex )
{
  doDisconnectFilterWheel();
  if ( state.filterWheel->initialise ( filterWheelDevs[ deviceIndex ] )) {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Unable to connect filter wheel" ));
    return;
  }

  disconnectWheel->setEnabled( 1 );
  warmResetWheel->setEnabled( state.filterWheel->hasWarmReset());
  coldResetWheel->setEnabled( state.filterWheel->hasColdReset());
  rescanWheel->setEnabled( 0 );
  if ( !wheelStatus ) {
    wheelStatus = new QLabel();
    wheelStatus->setPixmap ( QPixmap ( QString::fromUtf8 (
        ":/qt-icons/filter-wheel.png" )));
    statusLine->insertWidget( 0, wheelStatus );
  }
  statusLine->addWidget ( wheelStatus );
  wheelStatus->show();
  statusLine->showMessage ( state.filterWheel->name() +
      tr ( " connected" ), 5000 );
  if ( state.filterWheel->hasSpeedControl()) {
    unsigned int speed;
    state.filterWheel->getSpeed ( &speed );
    if ( !speed ) {
      state.filterWheel->setSpeed ( 100, 0 );
    }
  }

  // FIX ME
  // need to set filter wheel position and filter setting in
  // capture widget to the same?
}


void
MainWindow::disconnectFilterWheel ( void )
{
  doDisconnectFilterWheel();
  statusLine->removeWidget ( wheelStatus );
  statusLine->showMessage ( tr ( "Filter wheel disconnected" ));
}


void
MainWindow::warmResetFilterWheel ( void )
{
  if ( state.filterWheel && state.filterWheel->isInitialised()) {
    state.filterWheel->warmReset();
  }
  statusLine->showMessage ( tr ( "Filter wheel reset" ));
}


void
MainWindow::coldResetFilterWheel ( void )
{
  if ( state.filterWheel && state.filterWheel->isInitialised()) {
    state.filterWheel->coldReset();
  }
  statusLine->showMessage ( tr ( "Filter wheel reset" ));
}


void
MainWindow::rescanFilterWheels ( void )
{
  doFilterWheelMenu ( 0 );
}


void
MainWindow::doDisconnectFilterWheel ( void )
{
  if ( state.filterWheel && state.filterWheel->isInitialised()) {
    state.filterWheel->disconnect();
    disconnectWheel->setEnabled( 0 );
    warmResetWheel->setEnabled( 0 );
    coldResetWheel->setEnabled( 0 );
    rescanWheel->setEnabled( 1 );
  }
}


void
MainWindow::connectTimer ( int deviceIndex )
{
  doDisconnectTimer();
  if ( state.timer->initialise ( timerDevs[ deviceIndex ] )) {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Unable to connect timer" ));
    return;
  }

  disconnectTimerDevice->setEnabled( 1 );
  resetTimerDevice->setEnabled( state.timer->hasReset());
  rescanTimer->setEnabled( 0 );
  if ( !timerStatus ) {
    timerStatus = new QLabel();
    timerStatus->setPixmap ( QPixmap ( QString::fromUtf8 (
        ":/qt-icons/timer.png" )));
    statusLine->insertWidget( 0, timerStatus );
  }
  statusLine->addWidget ( timerStatus );
  timerStatus->show();
  if ( state.timer->hasGPS()) {
    if ( state.timer->readGPS ( &state.latitude, &state.longitude,
        &state.altitude ) == OA_ERR_NONE ) {
      state.gpsValid = 1;
    }
  }
  statusLine->showMessage ( state.timer->name() + tr ( " connected" ), 5000 );
}


void
MainWindow::disconnectTimer ( void )
{
  state.gpsValid = 0;
  doDisconnectTimer();
  statusLine->removeWidget ( timerStatus );
  statusLine->showMessage ( tr ( "Timer disconnected" ));
}


void
MainWindow::resetTimer ( void )
{
  if ( state.timer && state.timer->isInitialised()) {
    state.timer->reset();
  }
  statusLine->showMessage ( tr ( "Timer reset" ));
}


void
MainWindow::rescanTimers ( void )
{
  doTimerMenu ( 0 );
}


void
MainWindow::doDisconnectTimer ( void )
{
  if ( state.timer && state.timer->isInitialised()) {
    state.timer->disconnect();
    disconnectTimerDevice->setEnabled( 0 );
    resetTimerDevice->setEnabled( 0 );
    rescanTimer->setEnabled( 1 );
  }
}


void
MainWindow::setCapturedFrames ( unsigned int newVal )
{
  QString stringVal;

  stringVal.setNum ( newVal );
  capturedValue->setText ( stringVal );
}


void
MainWindow::setActualFrameRate ( double fps )
{
  QString stringVal;

  // precision, eg 100, 10.0, 1.00, 0.10, 0.01
  stringVal.setNum ( fps, 'f', std::min(2, std::max(0, 2-(int)log10(fps))) );
  fpsActualValue->setText ( stringVal );
  state.currentFPS = fps;
}


void
MainWindow::setTemperature()
{
  float temp;
  QString stringVal;

  temp = state.camera->getTemperature();
  state.cameraTempValid = 1;
  state.cameraTemp = temp;

  if ( updateTemperatureLabel == 1 ) {
    if ( config.tempsInC ) {
      tempLabel->setText ( tr ( "Temp (C)" ));
    } else {
      tempLabel->setText ( tr ( "Temp (F)" ));
    }
    updateTemperatureLabel = 0;
  }

  if ( !config.tempsInC ) {
    temp = temp * 9 / 5 + 32;
  }
  stringVal.setNum ( temp, 'g', 3 );
  tempValue->setText ( stringVal );
}


void
MainWindow::setDroppedFrames()
{
  uint64_t dropped;
  QString stringVal;

  dropped = state.camera->readControl ( OA_CAM_CTRL_DROPPED );
  stringVal.setNum ( dropped );
  droppedValue->setText ( stringVal );
}


void
MainWindow::resetTemperatureLabel()
{
  updateTemperatureLabel = 1;
} 
  

void
MainWindow::clearTemperature ( void )
{
  tempValue->setText ( "" );
}


void
MainWindow::clearDroppedFrames ( void )
{
  droppedValue->setText ( "" );
}

void
MainWindow::quit ( void )
{
  doingQuit = 1;
  doDisconnectCam();
  doDisconnectFilterWheel();
  writeConfig();
  qApp->quit();
}


void
MainWindow::setPixelFormatValue ( int format )
{
  pixelFormatValue->setText ( QString( OA_PIX_FMT_STRING( format )));
}


void
MainWindow::showFPSMaxValue ( int value )
{
  fpsMaxValue->setText ( QString::number ( value ));
}


void
MainWindow::clearFPSMaxValue ( void )
{
  fpsMaxValue->setText ( "" );
}


void
MainWindow::showStatusMessage ( QString message )
{
  statusLine->showMessage ( message );
}


void
MainWindow::setNightMode ( int mode )
{
  // FIX ME -- need to set flag so subwindows can be started with the
  // correct stylesheet
  if ( mode ) {
    setNightStyleSheet ( this );
    if ( state.histogramWidget ) {
      setNightStyleSheet ( state.histogramWidget );
    }
    config.nightMode = 1;
  } else {
    clearNightStyleSheet ( this );
    if ( state.histogramWidget ) {
      clearNightStyleSheet ( state.histogramWidget );
    }
    config.nightMode = 0;
  }
  update();
}


void
MainWindow::enableHistogram ( void )
{
  if ( histogramOpt->isChecked()) {
    if ( !state.histogramWidget ) {
      state.histogramWidget = new HistogramWidget();
      // need to do this to be able to uncheck the menu item on closing
      state.histogramWidget->setAttribute ( Qt::WA_DeleteOnClose );
      if ( config.histogramOnTop ) {
        state.histogramWidget->setWindowFlags ( Qt::WindowStaysOnTopHint );
      }
      connect ( state.histogramWidget, SIGNAL( destroyed ( QObject* )), this,
          SLOT ( histogramClosed()));
    }
    state.histogramWidget->show();
    state.histogramOn = 1;
    config.showHistogram = 1;
    oldHistogramState = -1;
  } else {
    if ( state.histogramWidget ) {
      state.histogramWidget->hide();
    }
    state.histogramOn = 0;
    config.showHistogram = 0;
  }
}


void
MainWindow::histogramClosed ( void )
{
  state.histogramWidget = 0;
  state.histogramOn = 0;
  if ( !doingQuit ) {
    histogramOpt->setChecked ( 0 );
  }
  oldHistogramState = 0;
  // We don't want to change this if the histogram window is closing because
  // we exited
  if ( !doingQuit ) {
    config.showHistogram = 0;
  }
}


void
MainWindow::setNightStyleSheet ( QWidget* window )
{
  QString style = QString (
      "background: rgb( 92, 0, 0 ); "
      "border-color: rgb( 192, 0, 0 ); "
  );

  if ( state.needGroupBoxBorders ) {
    style += styleGroupBoxBorders;
  }

  window->setStyleSheet ( style );
}


void
MainWindow::clearNightStyleSheet ( QWidget* window )
{
  window->setStyleSheet ( state.needGroupBoxBorders ?
      styleGroupBoxBorders : "" );
}


void
MainWindow::enableReticle ( void )
{
  config.showReticle = reticle->isChecked() ? 1 : 0;
}


void
MainWindow::enableFocusAid ( void )
{
  config.showFocusAid = focusaid->isChecked() ? 1 : 0;
  if ( config.showFocusAid ) {
    focusOverlay->show();
  } else {
    focusOverlay->hide();
  }
}


void
MainWindow::setFlipX ( int state )
{
  flipX->setChecked ( state );
}


void
MainWindow::setFlipY ( int state )
{
  flipY->setChecked ( state );
}


void
MainWindow::enableFlipX ( void )
{
  int flipState = flipX->isChecked() ? 1 : 0;

  config.flipX = flipState;
  if ( state.camera->isInitialised() &&
      state.camera->hasControl ( OA_CAM_CTRL_HFLIP )) {
    state.camera->setControl ( OA_CAM_CTRL_HFLIP, flipState );
    config.CONTROL_VALUE( OA_CAM_CTRL_HFLIP ) = flipState;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_HFLIP, flipState );
    if ( state.settingsWidget ) {
      state.settingsWidget->enableFlipX ( flipState );
    }
    return;
  }
  state.previewWidget->enableFlipX ( flipState );
  if ( flipState ) {
    mosaicFlipWarning();
  }
}


void
MainWindow::enableFlipY ( void )
{
  int flipState = flipY->isChecked() ? 1 : 0;

  config.flipY = flipState;
  if ( state.camera->isInitialised() &&
      state.camera->hasControl ( OA_CAM_CTRL_VFLIP )) {
    state.camera->setControl ( OA_CAM_CTRL_VFLIP, flipState );
    config.CONTROL_VALUE( OA_CAM_CTRL_VFLIP ) = flipState;
    SET_PROFILE_CONTROL( OA_CAM_CTRL_VFLIP, flipState );
    if ( state.settingsWidget ) {
      state.settingsWidget->enableFlipY ( flipState );
    }
    return;
  }
  state.previewWidget->enableFlipY ( flipState );
  if ( flipState ) {
    mosaicFlipWarning();
  }
}


void
MainWindow::mosaicFlipWarning ( void )
{
  int format = state.camera->videoFramePixelFormat();

  if ( OA_ISBAYER ( format )) {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Flipping a raw camera image may require a different colour mask to be used for demosaicking " ));
  }
}


void
MainWindow::enableDemosaic ( void )
{
  int demosaicState = demosaicOpt->isChecked() ? 1 : 0;
  int format;

  config.demosaic = demosaicState;
  state.previewWidget->enableDemosaic ( demosaicState );
  if ( state.camera->isInitialised()) {
    format = state.camera->videoFramePixelFormat ( 0 );
    state.captureWidget->enableTIFFCapture (( !OA_ISBAYER( format ) ||
        ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
    state.captureWidget->enablePNGCapture (( !OA_ISBAYER( format ) ||
        ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
    state.captureWidget->enableMOVCapture (( QUICKTIME_OK( format ) || 
        ( OA_ISBAYER( format ) && config.demosaic &&
        config.demosaicOutput )) ? 1 : 0 );
  }
}


void
MainWindow::aboutDialog ( void )
{
  QMessageBox::about ( this, tr ( "About " APPLICATION_NAME ),
      tr ( "<h2>" APPLICATION_NAME " " VERSION_STR "</h2>"
      "<p>Copyright &copy; " COPYRIGHT_YEARS " " AUTHOR_NAME "<br/>"
      "&lt;" AUTHOR_EMAIL "&gt;</p>"
      "<p>" APPLICATION_NAME " is an open source video capture application "
      "intended primarily for planetary imaging."
      "<p>Thanks are due to numerous forum members for testing and "
      "encouragement, and to those manufacturers including ZW Optical, "
      "Celestron, The Imaging Source, First Light Optics, QHY and Xagyl "
      "who have provided documentation, Linux SDKs and other help without "
      "which this application would have taken much longer to create.</p>"
      "<p>An honourable mention too to Chris Garry, author of PIPP, for "
      "the use of his code to create Windows-compatible AVI files.</p>"
      "<p>Kudos is also due to the FFmpeg project, the libusb project, "
      "libuvc and libhidapi, which I have hacked without mercy, as well as "
      "to many other open source projects that have provided inspiration, "
      "documentation and enlightenment where there was precious little "
      "otherwise.</p>" ));
}


void
MainWindow::setProgress ( unsigned int progress )
{
  progressBar->setValue ( progress );
}


void
MainWindow::doGeneralSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.generalSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doCaptureSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.captureSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doCameraSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.cameraSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doProfileSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.profileSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doFilterSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.filterSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doAutorunSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.autorunSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doHistogramSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.histogramSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doDemosaicSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.demosaicSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doFITSSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.fitsSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doTimerSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.timerSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::createSettingsWidget ( void )
{
  if ( !state.settingsWidget ) {
    state.settingsWidget = new SettingsWidget();
    state.settingsWidget->setWindowFlags ( Qt::WindowStaysOnTopHint );
    state.settingsWidget->setAttribute ( Qt::WA_DeleteOnClose );
    state.settingsWidget->enableTab ( state.cameraSettingsIndex,
        state.camera->isInitialised() ? 1 : 0 );
    connect ( state.settingsWidget, SIGNAL( destroyed ( QObject* )), this,
        SLOT ( settingsClosed()));
  }
}


void
MainWindow::settingsClosed ( void )
{
  state.settingsWidget = 0;
}


void
MainWindow::doCameraMenu ( int replaceSingleItem )
{
  int numDevs;
  int i;

  if ( rescanCam ) {
    rescanCam->setEnabled( 0 );
  }
  if ( connectedCameras && !replaceSingleItem ) {
    for ( i = 0; i < connectedCameras; i++ ) {
      cameraMenu->removeAction ( cameras[i] );
      delete cameras[i];
    }
    delete cameraSignalMapper;
  }

  numDevs = state.camera->listConnected ( &cameraDevs );

  if ( !replaceSingleItem ) {
    if ( numDevs > 0 ) {
      cameraSignalMapper = new QSignalMapper ( this );
      for ( i = 0; i < numDevs && cameraDevs[i]; i++ ) {
        QString iface (
            oaCameraInterfaces [ cameraDevs[i]->interface ].shortName );
        QString name ( cameraDevs[i]->deviceName );
        cameraMenuEntry[i] = "(" + iface + ") " + name;
        cameras[i] = new QAction ( cameraMenuEntry[i], this );
        if ( cameraMenuCreated ) {
          cameraMenu->insertAction ( cameraMenuSeparator, cameras[i] );
        } else {
          cameraMenu->addAction ( cameras[i] );
        }
        cameraSignalMapper->setMapping ( cameras[i], i );
        connect ( cameras[i], SIGNAL( triggered()), cameraSignalMapper,
            SLOT( map()));
      }
      connect ( cameraSignalMapper, SIGNAL( mapped ( int )), this,
          SLOT( connectCamera ( int )));
    }

    if ( !cameraMenuCreated ) {
      cameraMenuSeparator = cameraMenu->addSeparator();
      rescanCam = new QAction ( tr ( "Rescan" ), this );
      rescanCam->setStatusTip ( tr ( "Scan for newly connected devices" ));
      connect ( rescanCam, SIGNAL( triggered()), this, SLOT( rescanCameras() ));
      disconnectCam = new QAction ( tr ( "Disconnect" ), this );
      connect ( disconnectCam, SIGNAL( triggered()), this,
          SLOT( disconnectCamera()));
      disconnectCam->setEnabled( 0 );
      cameraMenu->addAction ( rescanCam );
      cameraMenu->addAction ( disconnectCam );
    }
  } else {
    if ( numDevs == 1 ) { // this should hopefully be true
      QString iface (
          oaCameraInterfaces [ cameraDevs[0]->interface ].shortName );
      QString name ( cameraDevs[0]->deviceName );
      cameraMenuEntry[0] = "(" + iface + ") " + name;
      cameras[0]->setText ( cameraMenuEntry[0] );
    } else {
      for ( i = 0; i < connectedCameras; i++ ) {
        cameras[i]->setEnabled(0);
      }
    }
  }

  cameraMenuCreated = 1;
  connectedCameras = numDevs;
  if ( rescanCam ) {
    rescanCam->setEnabled( 1 );
  }
}


void
MainWindow::doFilterWheelMenu ( int replaceSingleItem )
{
  int numFilterWheels = 0;
  int i;

  if ( rescanWheel ) {
    rescanWheel->setEnabled( 0 );
  }
  if ( connectedFilterWheels && !replaceSingleItem ) {
    for ( i = 0; i < connectedFilterWheels; i++ ) {
      filterWheelMenu->removeAction ( filterWheels[i] );
      delete filterWheels[i];
    }
    delete filterWheelSignalMapper;
  }

  numFilterWheels = state.filterWheel->listConnected ( &filterWheelDevs );

  if ( !replaceSingleItem ) {
    if ( numFilterWheels > 0 ) {
      filterWheelSignalMapper = new QSignalMapper ( this );
      for ( i = 0; i < numFilterWheels && filterWheelDevs[i]; i++ ) {
        QString name ( filterWheelDevs[i]->deviceName );
        filterWheelMenuEntry[i] = name;
        filterWheels[i] = new QAction ( filterWheelMenuEntry[i], this );
        if ( filterWheelMenuCreated ) {
          filterWheelMenu->insertAction ( filterWheelMenuSeparator,
              filterWheels[i] );
        } else {
          filterWheelMenu->addAction ( filterWheels[i] );
        }
        filterWheelSignalMapper->setMapping ( filterWheels[i], i );
        connect ( filterWheels[i], SIGNAL( triggered()),
            filterWheelSignalMapper, SLOT( map()));
      }
      connect ( filterWheelSignalMapper, SIGNAL( mapped ( int )), this,
          SLOT( connectFilterWheel ( int )));
    }

    if ( !filterWheelMenuCreated ) {
      filterWheelMenuSeparator = filterWheelMenu->addSeparator();

      warmResetWheel = new QAction ( tr ( "Warm Reset" ), this );
      warmResetWheel->setStatusTip ( tr (
          "Send a 'warm reset' command to the wheel" ));
      connect ( warmResetWheel, SIGNAL( triggered()), this, SLOT(
          warmResetFilterWheel()));
      warmResetWheel->setEnabled( 0 );

      coldResetWheel = new QAction ( tr ( "Cold Reset" ), this );
      coldResetWheel->setStatusTip ( tr (
          "Send a 'cold reset' command to the wheel" ));
      connect ( coldResetWheel, SIGNAL( triggered()), this, SLOT(
          coldResetFilterWheel()));
      coldResetWheel->setEnabled( 0 );

      rescanWheel = new QAction ( tr ( "Rescan" ), this );
      rescanWheel->setStatusTip ( tr ( "Scan for newly connected wheels" ));
      connect ( rescanWheel, SIGNAL( triggered()), this, SLOT(
          rescanFilterWheels()));

      disconnectWheel = new QAction ( tr ( "Disconnect" ), this );
      connect ( disconnectWheel, SIGNAL( triggered()), this,
          SLOT( disconnectFilterWheel()));
      disconnectWheel->setEnabled( 0 );

      filterWheelMenu->addAction ( warmResetWheel );
      filterWheelMenu->addAction ( coldResetWheel );
      filterWheelMenu->addAction ( rescanWheel );
      filterWheelMenu->addAction ( disconnectWheel );
    }
  } else {
    if ( numFilterWheels == 1 ) { // this should hopefully be true
      QString name ( filterWheelDevs[0]->deviceName );
      filterWheelMenuEntry[0] = name;
      filterWheels[0]->setText ( filterWheelMenuEntry[0] );
    } else {
      for ( i = 0; i < connectedFilterWheels; i++ ) {
        filterWheels[i]->setEnabled(0);
      }
    }
  }

  filterWheelMenuCreated = 1;
  connectedFilterWheels = numFilterWheels;
  if ( rescanWheel ) {
    rescanWheel->setEnabled( 1 );
  }
}


void
MainWindow::doTimerMenu ( int replaceSingleItem )
{
  int numTimers = 0;
  int i;

  if ( rescanTimer ) {
    rescanTimer->setEnabled( 0 );
  }
  if ( connectedTimers && !replaceSingleItem ) {
    for ( i = 0; i < connectedTimers; i++ ) {
      timerMenu->removeAction ( timers[i] );
      delete timers[i];
    }
    delete timerSignalMapper;
  }

  numTimers = state.timer->listConnected ( &timerDevs );

  if ( !replaceSingleItem ) {
    if ( numTimers > 0 ) {
      timerSignalMapper = new QSignalMapper ( this );
      for ( i = 0; i < numTimers && timerDevs[i]; i++ ) {
        QString name ( timerDevs[i]->deviceName );
        timerMenuEntry[i] = name;
        timers[i] = new QAction ( timerMenuEntry[i], this );
        if ( timerMenuCreated ) {
          timerMenu->insertAction ( timerMenuSeparator,
              timers[i] );
        } else {
          timerMenu->addAction ( timers[i] );
        }
        timerSignalMapper->setMapping ( timers[i], i );
        connect ( timers[i], SIGNAL( triggered()),
            timerSignalMapper, SLOT( map()));
      }
      connect ( timerSignalMapper, SIGNAL( mapped ( int )), this,
          SLOT( connectTimer ( int )));
    }

    if ( !timerMenuCreated ) {
      timerMenuSeparator = timerMenu->addSeparator();

      resetTimerDevice = new QAction ( tr ( "Reset" ), this );
      resetTimerDevice->setStatusTip ( tr ( "Reset the timer device" ));
      connect ( resetTimerDevice, SIGNAL( triggered()), this,
          SLOT( resetTimer()));
      resetTimerDevice->setEnabled( 0 );

      rescanTimer = new QAction ( tr ( "Rescan" ), this );
      rescanTimer->setStatusTip ( tr ( "Scan for newly-connected timers" ));
      connect ( rescanTimer, SIGNAL( triggered()), this, SLOT(
          rescanTimers()));

      disconnectTimerDevice = new QAction ( tr ( "Disconnect" ), this );
      connect ( disconnectTimerDevice, SIGNAL( triggered()), this,
          SLOT( disconnectTimer()));
      disconnectTimerDevice->setEnabled( 0 );

      timerMenu->addAction ( resetTimerDevice );
      timerMenu->addAction ( rescanTimer );
      timerMenu->addAction ( disconnectTimerDevice );
    }
  } else {
    if ( numTimers == 1 ) { // this should hopefully be true
      QString name ( timerDevs[0]->deviceName );
      timerMenuEntry[0] = name;
      timers[0]->setText ( timerMenuEntry[0] );
    } else {
      for ( i = 0; i < connectedTimers; i++ ) {
        timers[i]->setEnabled(0);
      }
    }
  }

  timerMenuCreated = 1;
  connectedTimers = numTimers;
  if ( rescanTimer ) {
    rescanTimer->setEnabled( 1 );
  }
}


void
MainWindow::closeSettingsWindow ( void )
{
  if ( state.settingsWidget ) {
    state.settingsWidget->close();
    state.settingsWidget = 0;
  }
}


void
MainWindow::destroyLayout ( QLayout* layout )
{
  QLayoutItem* item;
  QLayout*     sublayout;
  QWidget*     widget;

  if ( !layout ) { return; }

  while (( item = layout->takeAt(0))) {
    if (( sublayout = item->layout())) {
      destroyLayout ( sublayout );
    }
    else if (( widget = item->widget())) {
      widget->hide();
      delete widget;
    } else {
      delete item;
    }
  }

  delete layout;
}


void
MainWindow::styleStatusBarTemp ( int state )
{
  tempLabel->setEnabled ( state );
  tempValue->setEnabled ( state );
}


void
MainWindow::styleStatusBarDroppedFrames ( int state )
{
  droppedLabel->setEnabled ( state );
  droppedValue->setEnabled ( state );
}


void
MainWindow::createPreviewWindow()
{
  int height, width;
  int minHeight = 600, minWidth = 800;

  previewScroller = new QScrollArea ( this );
  focusOverlay = new FocusOverlay ( previewScroller );
  state.focusOverlay = focusOverlay;
  previewWidget = new PreviewWidget ( previewScroller );
  state.previewWidget = previewWidget;

  // These figures are a bit arbitrary, but give a size that should work
  // initially on small displays
  QRect rec = QApplication::desktop()->availableGeometry (
      QApplication::desktop()->primaryScreen());
  height = rec.height();
  width = rec.width();
  if ( height < 1024 || width < 1280 ) {
    if ( height < 600 || width < 800 ) {
      minWidth = 640;
      minHeight = 480;
    } else {
      minWidth = 800;
      minHeight = 600;
    }
  } else {
    height *= 2.0/3.0;
    width *= 2.0/3.0;
    if ( minHeight > height ) {
      minHeight = height;
    }
    if ( minWidth > width ) {
      minWidth = width;
    }
  }
  previewScroller->setMinimumSize ( minWidth, minHeight );
  previewScroller->setSizePolicy( QSizePolicy::Expanding,
      QSizePolicy::Expanding );
  previewScroller->setFocusPolicy( Qt::NoFocus );
  previewScroller->setContentsMargins( 0, 0, 0, 0 );
  previewScroller->setWidget ( previewWidget );

  if ( !config.preview ) {
    previewScroller->hide();
  }

  if ( config.dockableControls || config.separateControls ) {
    setCentralWidget ( previewScroller );
  } else {
    if ( config.controlsOnRight ) {
      dummyWidget = new QWidget ( this );
      dummyWidget->setLayout ( vertControlsBox );
      splitter->addWidget ( previewScroller );
      splitter->addWidget ( dummyWidget );
    } else {
      dummyWidget = new QWidget ( this );
      dummyWidget->setLayout ( horizControlsBox );
      splitter->setOrientation ( Qt::Vertical );
      splitter->addWidget ( dummyWidget );
      splitter->addWidget ( previewScroller );
    }
  }
}


void
MainWindow::changePreviewState ( int newState )
{
  if ( newState == Qt::Unchecked ) {
    config.preview = 0;
    previewScroller->hide();
    if ( previewWidget ) {
      previewWidget->setEnabled ( 0 );
    }
  } else {
    config.preview = 1;
    previewScroller->show();
    if ( previewWidget ) {
      previewWidget->setEnabled ( 1 );
    }
  }
}


void
MainWindow::configure ( void )
{
  cameraWidget->configure();
  imageWidget->configure();
  controlWidget->configure();
  previewWidget->configure();
}


void
MainWindow::createControlWidgets ( void )
{
  cameraWidget = new CameraWidget ( this );
  imageWidget = new ImageWidget ( this );
  zoomWidget = new ZoomWidget ( this );
  controlWidget = new ControlWidget ( this );
  captureWidget = new CaptureWidget ( this );

  imageZoomBox = new QVBoxLayout();
  imageZoomBox->addWidget ( imageWidget );
  imageZoomBox->addWidget ( zoomWidget );

  if ( config.dockableControls && !config.separateControls ) {
    cameraDock = new QDockWidget ( tr ( "Camera" ), this );
    cameraDock->setFeatures ( QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable );
    cameraDock->setWidget ( cameraWidget );
    cameraWidget->show();

    imageZoomDock = new QDockWidget ( tr ( "Image/Zoom" ), this );
    imageZoomDock->setFeatures ( QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable );
    imageZoomWidget = new QWidget ( this );
    imageZoomWidget->setLayout ( imageZoomBox );
    imageZoomDock->setWidget ( imageZoomWidget );
    imageZoomWidget->show();

    controlDock = new QDockWidget ( tr ( "Control" ), this );
    controlDock->setFeatures ( QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable );
    controlDock->setWidget ( controlWidget );
    controlWidget->show();

    captureDock = new QDockWidget ( tr ( "Capture" ), this );
    captureDock->setFeatures ( QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable );
    captureDock->setWidget ( captureWidget );
    captureWidget->show();

    enum Qt::DockWidgetArea where = config.controlsOnRight ?
        Qt::RightDockWidgetArea : Qt::TopDockWidgetArea;
    addDockWidget ( where, cameraDock );
    addDockWidget ( where, imageZoomDock );
    addDockWidget ( where, controlDock );
    addDockWidget ( where, captureDock );

  } else {

    // we can't set a MainWindow layout directly, so we need to add a
    // new widget as the central widget and add the layout to that

    if ( config.separateControls ) {

      controlWindow = new QMainWindow;
      dummyWidget = new QWidget ( controlWindow );
      controlWindow->setCentralWidget ( dummyWidget );
      controlWindow->setWindowTitle ( tr ( "oaCapture controls" ));
      Qt::WindowFlags flags = controlWindow->windowFlags();
      flags &= ~Qt::WindowCloseButtonHint;
      controlWindow->setWindowFlags ( flags );
      gridBox = new QGridLayout();
      gridBox->addWidget ( cameraWidget, 0, 0 );
      gridBox->addLayout ( imageZoomBox, 0, 1 );
      gridBox->addWidget ( controlWidget, 1, 0 );
      gridBox->addWidget ( captureWidget, 1, 1 );
      dummyWidget->setLayout ( gridBox );
      controlWindow->show();

    } else {

      splitter = new QSplitter ( this );
      setCentralWidget ( splitter );

      if ( config.controlsOnRight ) {
        vertControlsBox = new QVBoxLayout();
        vertControlsBox->addWidget ( cameraWidget );
        vertControlsBox->addLayout ( imageZoomBox );
        vertControlsBox->addWidget ( controlWidget );
        vertControlsBox->addWidget ( captureWidget );
        vertControlsBox->addStretch ( 1 );
      } else {
        horizControlsBox = new QHBoxLayout();
        horizControlsBox->addWidget ( cameraWidget );
        horizControlsBox->addLayout ( imageZoomBox );
        horizControlsBox->addWidget ( controlWidget );
        horizControlsBox->addWidget ( captureWidget );
        horizControlsBox->addStretch ( 1 );
      }
    }
  }

  state.controlWidget = controlWidget;
  state.imageWidget = imageWidget;
  state.zoomWidget = zoomWidget;
  state.cameraWidget = cameraWidget;
  state.captureWidget = captureWidget;
}


void
MainWindow::doAdvancedMenu( void )
{
  int i, numFilterWheelActions, totalActions;

  if ( advancedFilterWheelSignalMapper ) {
    totalActions = advancedActions.count();
    if ( totalActions ) {
      for ( i = 0; i < totalActions; i++ ) {
        advancedMenu->removeAction ( advancedActions[i] );
      }
    }
    delete advancedFilterWheelSignalMapper;
    advancedFilterWheelSignalMapper = 0;
  }

  advancedActions.clear();
  advancedFilterWheelSignalMapper = new QSignalMapper ( this );

  numFilterWheelActions = totalActions = 0;
  for ( i = 1; i < OA_FW_IF_COUNT; i++ ) {
    if ( oaFilterWheelInterfaces[ i ].userConfigFlags ) {
      QString label = QString ( oaFilterWheelInterfaces[ i ].name );
      label += " " + QString ( tr ( "filter wheels" ));
      advancedActions.append ( new QAction ( label, this ));
      advancedMenu->addAction ( advancedActions[ totalActions ]);
      advancedFilterWheelSignalMapper->setMapping (
          advancedActions[ totalActions ],
          oaFilterWheelInterfaces[ i ].interfaceType );
      connect ( advancedActions[ totalActions ], SIGNAL( triggered()),
            advancedFilterWheelSignalMapper, SLOT( map()));
      totalActions++;
      numFilterWheelActions++;
    }
  }

  if ( numFilterWheelActions ) {
    connect ( advancedFilterWheelSignalMapper, SIGNAL( mapped ( int )),
        this, SLOT( advancedFilterWheelHandler ( int )));
  }

  QString label = QString ( "PTR" );
  label += " " + QString ( tr ( "timers" ));
  advancedActions.append ( new QAction ( label, this ));
  advancedMenu->addAction ( advancedActions[ totalActions ]);
  connect ( advancedActions[ totalActions ], SIGNAL( triggered()),
      this, SLOT( advancedPTRHandler()));
}


void
MainWindow::advancedFilterWheelHandler ( int interfaceType )
{
  if ( !state.advancedSettings ) {
    state.advancedSettings = new AdvancedSettings ( OA_DEVICE_FILTERWHEEL,
        interfaceType );
    state.advancedSettings->setAttribute ( Qt::WA_DeleteOnClose );
    connect ( state.advancedSettings, SIGNAL( destroyed ( QObject* )), this,
        SLOT ( advancedClosed()));
  }

  state.advancedSettings->show();
}


void
MainWindow::advancedPTRHandler ( void )
{
  if ( !state.advancedSettings ) {
    state.advancedSettings = new AdvancedSettings ( OA_DEVICE_PTR, 0 );
    state.advancedSettings->setAttribute ( Qt::WA_DeleteOnClose );
    connect ( state.advancedSettings, SIGNAL( destroyed ( QObject* )), this,
        SLOT ( advancedClosed()));
  }

  state.advancedSettings->show();
}


void
MainWindow::advancedClosed ( void )
{
  state.advancedSettings = 0;
}


void
MainWindow::closeAdvancedWindow ( void )
{
  if ( state.advancedSettings ) {
    state.advancedSettings->close();
    state.advancedSettings = 0;
  }
}


void
MainWindow::doColouriseSettings ( void )
{
  if ( config.numCustomColours ) {
    for ( int i = 0; i < config.numCustomColours; i++ ) {
      QColorDialog::setCustomColor ( i, config.customColours[i].rgb());
    }
  }

  QColor chosenColour = QColorDialog::getColor();
  if ( chosenColour.isValid()) {
    config.currentColouriseColour = chosenColour;
    if ( state.previewWidget ) {
      state.previewWidget->setMonoPalette ( config.currentColouriseColour );
    }
  }
  config.numCustomColours = colourDialog->customCount();
  if ( config.numCustomColours ) {
    config.customColours.clear();
    for ( int i = 0; i < config.numCustomColours; i++ ) {
#ifdef HAVE_QT4
      QRgb custCol = colourDialog->customColor ( i );
      config.customColours.append ( QColor ( custCol ));
#else
      QColor custCol = colourDialog->customColor ( i );
      config.customColours.append ( custCol );
#endif
    }
  }
}


void
MainWindow::reveal ( void )
{
  show();
}


void
MainWindow::frameWriteFailedPopup ( void )
{
  QMessageBox::warning ( this, APPLICATION_NAME,
      tr ( "Error saving captured frame" ));
}
