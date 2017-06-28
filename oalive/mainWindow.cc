/*****************************************************************************
 *
 * mainWindow.cc -- the main controlling window class
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

#include <QtGui>

#include <openastro/filterwheel.h>
#include <openastro/demosaic.h>

#include "focusOverlay.h"
#include "mainWindow.h"
#include "version.h"
#include "configuration.h"
#include "cameraControls.h"
#include "settingsWidget.h"
#include "state.h"
#include "targets.h"

CONFIG		config;
STATE		state;


MainWindow::MainWindow()
{
  cameraSignalMapper = filterWheelSignalMapper = 0;
  advancedFilterWheelSignalMapper = 0;
  rescanCam = disconnectCam = 0;
  rescanWheel = disconnectWheel = warmResetWheel = coldResetWheel = 0;
  connectedCameras = cameraMenuCreated = 0;
  connectedFilterWheels = filterWheelMenuCreated = 0;
  doingQuit = 0;

  readConfig();
  createStatusBar();
  createMenus();
  setWindowTitle( APPLICATION_NAME " " VERSION_STR );

  state.mainWindow = this;
  // state.controlWidget = 0;
  state.libavStarted = 0;
  state.camera = new Camera;
  state.filterWheel = new FilterWheel;
  state.lastRecordedFile = "";
  updateTemperatureLabel = 0;
  state.captureIndex = 0;
  state.settingsWidget = 0;
  state.advancedSettings = 0;
  state.cameraControls = 0;
  colourDialog = 0;

  // need to do this to prevent access attempts before creation
  viewWidget = 0;

  createControlWidgets();
  createViewWindow();

#if USE_APP_PATH
  QString path = QCoreApplication::applicationDirPath();
  if ( path.endsWith ( "/MacOS" )) {
    path.chop ( 6 );
  }
  oaSetRootPath ( path.toStdString().c_str());
#endif

  connect ( state.viewWidget, SIGNAL( updateTemperature ( void )),
      this, SLOT ( setTemperature ( void )));

  // FIX ME - need to add dropped frames slot?
/*
  connect ( state.viewWidget, SIGNAL( updateDroppedFrames ( void )),
      this, SLOT ( setDroppedFrames ( void )));
*/

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

  if ( cameraSignalMapper ) {
    delete cameraSignalMapper;
  }
  if ( filterWheelSignalMapper ) {
    delete filterWheelSignalMapper;
  }
  if ( advancedFilterWheelSignalMapper ) {
    delete advancedFilterWheelSignalMapper;
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
  if ( state.camera ) {
    delete state.camera;
  }
  if ( state.filterWheel ) {
    delete state.filterWheel;
  }
}


void
MainWindow::readConfig ( void )
{
  QSettings settings ( ORGANISATION_NAME_SETTINGS, APPLICATION_NAME );

  // -1 means we don't have a config file.  We change it to 1 later in the
  // function
  config.saveSettings = settings.value ( "saveSettings", -1 ).toInt();

  if ( !config.saveSettings ) {

    config.tempsInC = 1;
    config.reticleStyle = 1;
    config.showReticle = 0;
    config.showFocusAid = 0;
    config.sixteenBit = 0;
    config.binning2x2 = 0;
    config.colourise = 0;
    config.useROI = 0;
    config.imageSizeX = 0;
    config.imageSizeY = 0;

    config.controlValues [ OA_CAM_CTRL_GAIN ] = 50;
    config.controlValues [ OA_CAM_CTRL_EXPOSURE ] = 10;
    config.controlValues [ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = 100;
    config.controlValues [ OA_CAM_CTRL_GAMMA ] = -1;
    config.controlValues [ OA_CAM_CTRL_BRIGHTNESS ] = -1;

    config.exposureMenuOption = 3;
    config.frameRateNumerator = 0;
    config.frameRateDenominator = 1;
    config.profileOption = 0;
    config.filterOption = 0;
    config.fileTypeOption = 1;
    config.frameFileNameTemplate = QString ( "oalive-%INDEX" );
    config.processedFileNameTemplate = QString ( "oalive-processsed-%INDEX" );
    config.saveEachFrame = 0;
    config.saveProcessedImage = 0;
    config.captureDirectory = QString ( "" );

    config.saveCaptureSettings = 1;
    config.windowsCompatibleAVI = 0;
    config.cfaPattern = 0;
    config.demosaicMethod = 1;

    config.numProfiles = 0;
    config.numFilters = 0;

    config.promptForFilterChange = 0;
    config.interFilterDelay = 0;

    config.currentColouriseColour.setRgb ( 255, 255, 255 );
    config.numCustomColours = 0;

    config.fitsObserver = "";
    config.fitsTelescope = "";
    config.fitsInstrument = "";
    config.fitsObject = "";
    config.fitsComment = "";

  } else {

    // int version = settings.value ( "configVersion", CONFIG_VERSION ).toInt();

    restoreGeometry ( settings.value ( "geometry").toByteArray());

    // FIX ME -- how to handle this?
    // config.cameraDevice = settings.value ( "device/camera", -1 ).toInt();

    config.tempsInC = settings.value ( "tempsInCentigrade", 1 ).toInt();
    config.connectSoleCamera = settings.value ( "connectSoleCamera",
        0 ).toInt();
    config.saveCaptureSettings = settings.value ( "saveCaptureSettings",
        1 ).toInt();
    config.windowsCompatibleAVI = settings.value ( "windowsCompatibleAVI",
        0 ).toInt();

    config.showReticle = settings.value ( "options/showReticle", 0 ).toInt();
    config.showFocusAid = settings.value ( "options/showFocusAid", 0 ).toInt();
    config.sixteenBit = settings.value ( "camera/sixteenBit", 0 ).toInt();
    config.binning2x2 = settings.value ( "camera/binning2x2", 0 ).toInt();
    config.colourise = settings.value ( "camera/colourise", 0 ).toInt();
    // FIX ME -- reset these temporarily.  needs fixing properly
    config.sixteenBit = 0;
    config.binning2x2 = 0;
    config.colourise = 0;
    config.useROI = settings.value ( "image/useROI", 0 ).toInt();
    config.imageSizeX = settings.value ( "image/imageSizeX", 0 ).toInt();
    config.imageSizeY = settings.value ( "image/imageSizeY", 0 ).toInt();

    config.exposureMenuOption = settings.value ( "control/exposureMenuOption",
        3 ).toInt();
    config.frameRateNumerator = settings.value ( "control/frameRateNumerator",
        0 ).toInt();
    config.frameRateDenominator = settings.value (
        "control/frameRateDenominator", 1 ).toInt();

    config.profileOption = settings.value ( "control/profileOption",
        0 ).toInt();
    config.filterOption = settings.value ( "control/filterOption", 0 ).toInt();

    config.fileTypeOption = settings.value ( "files/fileTypeOption",
        1 ).toInt();
    config.frameFileNameTemplate = settings.value (
        "files/frameFileNameTemplate", "oalive-%DATE-%TIME" ).toString();
    config.processedFileNameTemplate = settings.value (
        "files/processedFileNameTemplate",
        "oalive-processed-%DATE-%TIME" ).toString();
    config.saveEachFrame = settings.value ( "files/saveEachFrame", 0 ).toInt();
    config.saveProcessedImage = settings.value ( "files/saveProcessedImage",
        0 ).toInt();
    config.captureDirectory = settings.value ( "files/captureDirectory",
        "" ).toString();

    config.promptForFilterChange = settings.value (
        "autorun/filterPrompt", 0 ).toInt();
    config.interFilterDelay = settings.value (
        "autorun/interFilterDelay", 0 ).toInt();
    config.demosaicMethod = settings.value ( "demosaic/method", 1 ).toInt();
    config.cfaPattern = settings.value ( "demosaic/cfaPattern",
        OA_DEMOSAIC_AUTO ).toInt();

    config.reticleStyle = settings.value ( "reticle/style",
        RETICLE_CIRCLE ).toInt();

    int numControls = settings.beginReadArray ( "controls" );
    if ( numControls ) {
      for ( int i = 1; i <= numControls; i++ ) {
        settings.setArrayIndex ( i-1 );
        config.controlValues[ i ] = settings.value ( "controlValue",
            0 ).toInt();
      }
    }
    settings.endArray();

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

    config.numProfiles = settings.beginReadArray ( "profiles" );
    if ( config.numProfiles ) {
      for ( int i = 0; i < config.numProfiles; i++ ) {
        settings.setArrayIndex ( i );
        PROFILE p;
        p.profileName = settings.value ( "name", "" ).toString();
        p.sixteenBit = settings.value ( "sixteenBit", 0 ).toInt();
        p.binning2x2 = settings.value ( "binning2x2", 0 ).toInt();
        p.colourise = settings.value ( "colourise", 0 ).toInt();
        p.useROI = settings.value ( "useROI", 0 ).toInt();
        p.imageSizeX = settings.value ( "imageSizeX", 0 ).toInt();
        p.imageSizeY = settings.value ( "imageSizeY", 0 ).toInt();

        int numFilters = settings.beginReadArray ( "filters" );
        if ( numFilters ) {
          for ( int k = 0; k < numFilters; k++ ) {
            settings.setArrayIndex ( k );
            if ( numFilters <= config.numFilters ) {
              FILTER_PROFILE fp;
              fp.filterName = config.filters[k].filterName;
              p.filterProfiles.append ( fp );
            }
            int numControls = settings.beginReadArray ( "controls" );
            for ( int j = 0; j < numControls; j++ ) {
              settings.setArrayIndex ( j );
              if ( numFilters <= config.numFilters ) {
                p.filterProfiles[ k ].controls[ j ] = settings.value (
                    "controlValue", 0 ).toInt();
              }
            }
            settings.endArray();
          }
        }
        settings.endArray();

        p.frameRateNumerator = settings.value ( "frameRateNumerator",
             0 ).toInt();
        p.frameRateDenominator = settings.value ( "frameRateDenominator",
            1 ).toInt();
        p.filterOption = settings.value ( "filterOption", 0 ).toInt();
        p.fileTypeOption = settings.value ( "fileTypeOption", 1 ).toInt();
        p.frameFileNameTemplate = settings.value (
            "frameFileNameTemplate", "oalive-%DATE-%TIME" ).toString();
        p.processedFileNameTemplate = settings.value (
            "processedFileNameTemplate",
            "oalive-processed-%DATE-%TIME" ).toString();
        p.target = settings.value ( "target", 0 ).toInt();
        config.profiles.append ( p );
      }
      settings.endArray();

    } else {
      // if we have no profiles we create a default one

      PROFILE p;
      p.profileName = "default";
      p.sixteenBit = config.sixteenBit;
      p.binning2x2 = config.binning2x2;
      p.colourise = config.colourise;
      p.useROI = config.useROI;
      p.imageSizeX = config.imageSizeX;
      p.imageSizeY = config.imageSizeY;
      if ( config.numFilters ) {
        for ( int k = 0; k < config.numFilters; k++ ) {
          FILTER_PROFILE fp;
          fp.filterName = config.filters[k].filterName;
          p.filterProfiles.append ( fp );
        }
      }
      for ( int j = 0; j < OA_CAM_CTRL_LAST_P1; j++ ) {
        if ( config.numFilters ) {
                  for ( int k = 0; k < config.numFilters; k++ ) {
            p.filterProfiles[ k ].controls[ j ] = config.controlValues[ j ];
          }
        }
      }

      p.frameRateNumerator = config.frameRateNumerator;
      p.frameRateDenominator = config.frameRateDenominator;
      p.filterOption = config.filterOption;
      p.fileTypeOption = config.fileTypeOption;
      p.frameFileNameTemplate = config.frameFileNameTemplate;
      p.processedFileNameTemplate = config.processedFileNameTemplate;
      p.target = TGT_UNKNOWN;
      config.profiles.append ( p );
      config.numProfiles = 1;
    }

    ( void ) settings.beginReadArray ( "filterSlots" );
    for ( int i = 0; i < MAX_FILTER_SLOTS; i++ ) {
      settings.setArrayIndex ( i );
      config.filterSlots[i] = settings.value ( "slot", -1 ).toInt();
    }
    settings.endArray();

    // FIX ME -- Need to set filter sequence
/*
    int numSeqs = settings.beginReadArray ( "filterSequence" );
    if ( numSeqs ) {
      for ( int i = 0; i < numSeqs; i++ ) {
        settings.setArrayIndex ( i );
        config.autorunFilterSequence.append ( settings.value (
            "slot", -1 ).toInt());
      }
    }
    settings.endArray();
*/

    config.currentColouriseColour = QColor ( 255, 255, 255 );
    config.numCustomColours = 0;
    config.customColours.clear();

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

  config.filterWheelConfig.clear();
  for ( int i = 0; i < OA_FW_IF_COUNT; i++ ) {
    userConfigList      conf;
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

  config.experimentalASI2 = settings.value ( "experimental/ASI2",
      0 ).toInt();

  if ( !config.saveSettings || config.saveSettings == -1 ) {
    config.saveSettings = -config.saveSettings;
  }

  config.fitsObserver = settings.value ( "fits/observer", "" ).toString();
  config.fitsTelescope = settings.value ( "fits/telescope", "" ).toString();
  config.fitsInstrument = settings.value ( "fits/instrument", "" ).toString();
  config.fitsObject = settings.value ( "fits/object", "" ).toString();
  config.fitsComment = settings.value ( "fits/comment", "" ).toString();

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
  settings.setValue ( "saveCaptureSettings", config.saveCaptureSettings );
  settings.setValue ( "windowsCompatibleAVI", config.windowsCompatibleAVI );

  settings.setValue ( "options/showReticle", config.showReticle );
  settings.setValue ( "options/showFocusAid", config.showFocusAid );

  settings.setValue ( "camera/sixteenBit", config.sixteenBit );
  settings.setValue ( "camera/binning2x2", config.binning2x2 );
  settings.setValue ( "camera/colourise", config.colourise );

  settings.setValue ( "image/useROI", config.useROI );
  settings.setValue ( "image/imageSizeX", config.imageSizeX );
  settings.setValue ( "image/imageSizeY", config.imageSizeY );

  settings.setValue ( "control/exposureMenuOption", config.exposureMenuOption );
  settings.setValue ( "control/frameRateNumerator", config.frameRateNumerator );
  settings.setValue ( "control/frameRateDenominator",
      config.frameRateDenominator );

  settings.setValue ( "control/profileOption", config.profileOption );
  settings.setValue ( "control/filterOption", config.filterOption );

  settings.setValue ( "files/fileTypeOption", config.fileTypeOption );
  settings.setValue ( "files/frameFileNameTemplate",
      config.frameFileNameTemplate );
  settings.setValue ( "files/processedFileNameTemplate",
      config.processedFileNameTemplate );
  settings.setValue ( "files/saveEachFrame", config.saveEachFrame );
  settings.setValue ( "files/saveProcessedImage", config.saveProcessedImage );
  settings.setValue ( "files/captureDirectory", config.captureDirectory );

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
      settings.setValue ( "colourise", config.profiles[i].colourise );
      settings.setValue ( "useROI", config.profiles[i].useROI );
      settings.setValue ( "imageSizeX", config.profiles[i].imageSizeX );
      settings.setValue ( "imageSizeY", config.profiles[i].imageSizeY );

      if ( config.numFilters ) {
        settings.beginWriteArray ( "filters" );
        for ( int j = 0; j < config.numFilters; j++ ) {
          settings.setArrayIndex ( j );
          settings.beginWriteArray ( "controls" );
          for ( int k = 0; k < OA_CAM_CTRL_LAST_P1; k++ ) {
            settings.setArrayIndex ( k );
            settings.setValue ( "controlValue",
                config.profiles[ i ].filterProfiles[ j ].controls[ k ]);
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
      settings.setValue ( "frameFileNameTemplate",
          config.profiles[i].frameFileNameTemplate );
      settings.setValue ( "processedFileNameTemplate",
          config.profiles[i].processedFileNameTemplate );
      settings.setValue ( "target", config.profiles[i].target );
    }
    settings.endArray();
  }

  settings.beginWriteArray ( "filterSlots" );
  for ( int i = 0; i < MAX_FILTER_SLOTS; i++ ) {
    settings.setArrayIndex ( i );
    settings.setValue ( "slot", config.filterSlots[i] );
  }
  settings.endArray();

/*
  settings.beginWriteArray ( "filterSequence" );
  int numSeqs;
  if (( numSeqs = config.autorunFilterSequence.count())) {
    for ( int i = 0; i < numSeqs; i++ ) {
      settings.setArrayIndex ( i );
      settings.setValue ( "slot", config.autorunFilterSequence[i] );
    }
  }
  settings.endArray();
*/
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

  settings.setValue ( "experimental/ASI2", config.experimentalASI2 );

  settings.setValue ( "fits/observer", config.fitsObserver );
  settings.setValue ( "fits/telescope", config.fitsTelescope );
  settings.setValue ( "fits/instrument", config.fitsInstrument );
  settings.setValue ( "fits/object", config.fitsObject );
  settings.setValue ( "fits/comment", config.fitsComment );
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
  tempValue = new QLabel ( "" );
  tempValue->setFixedWidth ( 30 );

  statusLine->addPermanentWidget ( tempLabel );
  statusLine->addPermanentWidget ( tempValue );
}


void
MainWindow::createMenus ( void )
{
  exit = new QAction ( tr ( "&Quit" ), this );
  exit->setShortcut ( QKeySequence::Quit );
  connect ( exit, SIGNAL( triggered()), this, SLOT( quit()));

  fileMenu = menuBar()->addMenu( tr ( "&File" ));
  fileMenu->addAction ( exit );

  // Camera device menu

  cameraMenu = menuBar()->addMenu ( tr ( "&Camera" ));
  doCameraMenu(0);

  // Filter wheel menu

  filterWheelMenu = menuBar()->addMenu ( tr ( "&Filter Wheel" ));
  doFilterWheelMenu(0);

  // Options menu

  reticle = new QAction ( QIcon ( ":/icons/reticle.png" ),
      tr ( "Reticle" ), this );
  reticle->setStatusTip ( tr ( "Overlay a reticle on the preview image" ));
  reticle->setCheckable ( true );
  connect ( reticle, SIGNAL( changed()), this, SLOT( enableReticle()));
  reticle->setChecked ( config.showReticle );

  focusaid = new QAction ( tr ( "Focus Aid" ), this );
  focusaid->setCheckable ( true );
  connect ( focusaid, SIGNAL( changed()), this, SLOT( enableFocusAid()));

  optionsMenu = menuBar()->addMenu ( tr ( "&Options" ));
  optionsMenu->addAction ( reticle );
  optionsMenu->addAction ( focusaid );

  // settings menu

  general = new QAction ( QIcon ( ":/icons/cog.png" ),
      tr ( "General" ), this );
  general->setStatusTip ( tr ( "General configuration" ));
  connect ( general, SIGNAL( triggered()), this, SLOT( doGeneralSettings()));

  capture = new QAction ( QIcon ( ":/icons/capture.png" ),
      tr ( "Capture" ), this );
  connect ( capture, SIGNAL( triggered()), this, SLOT( doCaptureSettings()));

  profiles = new QAction ( QIcon ( ":/icons/jupiter.png" ),
      tr ( "Profiles" ), this );
  profiles->setStatusTip ( tr ( "Edit saved profiles" ));
  connect ( profiles, SIGNAL( triggered()), this, SLOT( doProfileSettings()));

  filters = new QAction ( QIcon ( ":/icons/filter-wheel.png" ),
      tr ( "Filters" ), this );
  filters->setStatusTip ( tr ( "Configuration for filters" ));
  connect ( filters, SIGNAL( triggered()), this, SLOT( doFilterSettings()));

  demosaic = new QAction ( QIcon ( ":/icons/mosaic.png" ),
      tr ( "Demosaic" ), this );
  demosaic->setStatusTip ( tr ( "Configuration for demosaicking" ));
  connect ( demosaic, SIGNAL( triggered()), this, SLOT( doDemosaicSettings()));

  colourise = new QAction ( QIcon ( ":/icons/sun.png" ),
      tr ( "False Colour" ), this );
  connect ( colourise, SIGNAL( triggered()), this,
      SLOT( doColouriseSettings()));

  settingsMenu = menuBar()->addMenu ( tr ( "&Settings" ));
  settingsMenu->addAction ( general );
  settingsMenu->addAction ( capture );
  settingsMenu->addAction ( profiles );
  settingsMenu->addAction ( filters );
  settingsMenu->addAction ( demosaic );
  settingsMenu->addAction ( colourise );

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

  doDisconnectCam();

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
      config.controlValues[ c ] = config.profiles[ config.profileOption ].
          filterProfiles[ config.filterOption ].controls[ c ];
    }
  }
  configure();
  statusLine->showMessage ( state.camera->name() + tr ( " connected" ), 5000 );
  clearTemperature();

  controlsWidget->enableButtons ( 1 );
  // FIX ME -- Enable profile selection
  // state.captureWidget->enableProfileSelect ( 1 );

  state.viewWidget->setVideoFramePixelFormat (
      state.camera->videoFramePixelFormat());
  // FIX ME -- Check binning enable
  // state.cameraWidget->enableBinningControl ( state.camera->hasBinning ( 2 ));
  v = state.camera->hasControl ( OA_CAM_CTRL_TEMPERATURE );
  state.viewWidget->enableTempDisplay ( v );
  styleStatusBarTemp ( v );

  if ( state.cameraControls ) {
    state.cameraControls->disableAutoControls();
  }

  format = state.camera->videoFramePixelFormat();
qWarning() << "Enable/disable capture types to" << format;
/*
  state.captureWidget->enableTIFFCapture (( !OA_ISBAYER( format ) ||
      ( config.demosaic && config.demosaicOutput )) ? 1 : 0 );
  state.captureWidget->enableFITSCapture ( OA_ISGREYSCALE( format ) ? 1 : 0 );
*/
}


void
MainWindow::disconnectCamera ( void )
{
  if ( state.controlsWidget ) {
    state.controlsWidget->disableAllButtons();
  }
  doDisconnectCam();
  statusLine->showMessage ( tr ( "Camera disconnected" ));
}


void
MainWindow::doDisconnectCam ( void )
{
  if ( state.camera && state.camera->isInitialised()) {
    if ( state.controlsWidget ) {
      state.controlsWidget->closeOutputHandlers();
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
  // display to the same?
}


void
MainWindow::disconnectFilterWheel ( void )
{
  doDisconnectFilterWheel();
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
MainWindow::setTemperature()
{
  float temp;
  QString stringVal;

  temp = state.camera->getTemperature();

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
MainWindow::quit ( void )
{
  doingQuit = 1;
  doDisconnectCam();
  doDisconnectFilterWheel();
  writeConfig();
  qApp->quit();
}


void
MainWindow::showStatusMessage ( QString message )
{
  statusLine->showMessage ( message );
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
MainWindow::mosaicFlipWarning ( void )
{
  int format = state.camera->videoFramePixelFormat();

  if ( OA_ISBAYER ( format )) {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Flipping a raw camera image may require a different colour mask to be used for demosaicking " ));
  }
}


void
MainWindow::aboutDialog ( void )
{
  QMessageBox::about ( this, tr ( "About " APPLICATION_NAME ),
      tr ( "<h2>" APPLICATION_NAME " " VERSION_STR "</h2>"
      "<p>Copyright &copy; " COPYRIGHT_YEARS " " AUTHOR_NAME "</p>"
      "<p>" APPLICATION_NAME " is an open source application for video "
      "astronomy/electronically enhanced astronomy.</p>"
      "<p>Thanks are due to numerous forum members for testing and "
      "encouragement, and to those manufacturers including ZW Optical, "
      "Celestron, The Imaging Source, QHY and Xagyl who have provided "
      "documentation, Linux SDKs and other help without which this "
      "application would have taken much longer to create.</p>"
      "<p>An honourable mention too to Chris Garry, author of PIPP, for "
      "the use of his code to create Windows-compatible AVI files.</p>"
      "<p>Kudos is also due to the FFmpeg project, the libusb project, "
      "libuvc and libhidapi, which I have hacked without mercy, as well as "
      "to many other open source projects that have provided inspiration, "
      "documentation and enlightenment where there was precious little "
      "otherwise.</p>" ));
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
MainWindow::doDemosaicSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.demosaicSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::createSettingsWidget ( void )
{
  if ( !state.settingsWidget ) {
    state.settingsWidget = new SettingsWidget();
    state.settingsWidget->setWindowFlags ( Qt::WindowStaysOnTopHint );
    state.settingsWidget->setAttribute ( Qt::WA_DeleteOnClose );
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
        if ( !config.experimentalASI2 && OA_CAM_IF_ZWASI2 ==
            cameraDevs[i]->interface ) {
          cameras[i]->setEnabled ( 0 );
        } else {
          cameraSignalMapper->setMapping ( cameras[i], i );
          connect ( cameras[i], SIGNAL( triggered()), cameraSignalMapper,
              SLOT( map()));
        }
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
MainWindow::createViewWindow()
{
  viewScroller = new QScrollArea ( this );
  focusOverlay = new FocusOverlay ( viewScroller );
  state.focusOverlay = focusOverlay;
  viewWidget = new ViewWidget ( viewScroller );
  state.viewWidget = viewWidget;
  viewScroller->setMinimumSize ( 800, 600 );
  viewScroller->setSizePolicy( QSizePolicy::Expanding,
      QSizePolicy::Expanding );
  viewScroller->setFocusPolicy( Qt::NoFocus );
  viewScroller->setContentsMargins( 0, 0, 0, 0 );
  viewScroller->setWidget ( viewWidget );

  splitter->addWidget ( viewScroller );
  splitter->addWidget ( controlsWidget );
}


void
MainWindow::configure ( void )
{
  viewWidget->configure();
  controlsWidget->configure();
}


void
MainWindow::createControlWidgets ( void )
{
  controlsWidget = new ControlsWidget ( this );
  state.controlsWidget = controlsWidget;
  splitter = new QSplitter ( this );
  setCentralWidget ( splitter );
}


void
MainWindow::doAdvancedMenu( void )
{
  int i, numActions;

  if ( advancedFilterWheelSignalMapper ) {
    numActions = advancedActions.count();
    if ( numActions ) {
      for ( i = 0; i < numActions; i++ ) {
        advancedMenu->removeAction ( advancedActions[i] );
      }
    }
    delete advancedFilterWheelSignalMapper;
    advancedFilterWheelSignalMapper = 0;
  }

  advancedActions.clear();
  advancedFilterWheelSignalMapper = new QSignalMapper ( this );

  numActions = 0;
  for ( i = 1; i < OA_FW_IF_COUNT; i++ ) {
    if ( oaFilterWheelInterfaces[ i ].userConfigFlags ) {
      QString label = QString ( oaFilterWheelInterfaces[ i ].name );
      label += " " + QString ( tr ( "filter wheels" ));
      advancedActions.append ( new QAction ( label, this ));
      advancedMenu->addAction ( advancedActions[ numActions ]);
      advancedFilterWheelSignalMapper->setMapping (
          advancedActions[numActions],
          oaFilterWheelInterfaces[ i ].interfaceType );
      connect ( advancedActions[numActions], SIGNAL( triggered()),
            advancedFilterWheelSignalMapper, SLOT( map()));
      numActions++;
    }
  }
  if ( numActions ) {
    connect ( advancedFilterWheelSignalMapper, SIGNAL( mapped ( int )),
        this, SLOT( advancedFilterWheelHandler ( int )));
  }
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
    if ( state.viewWidget ) {
      state.viewWidget->setMonoPalette ( config.currentColouriseColour );
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
