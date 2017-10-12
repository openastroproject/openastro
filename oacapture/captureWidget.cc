/*****************************************************************************
 *
 * captureWidget.cc -- class for the capture widget in the UI
 *
 * Copyright 2013,2014,2015,2016,2017
 * James Fidell (james@openastroproject.org)
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

#include <ctime>
#include <iostream>
#include <fstream>
#include <QtGui>

#include "configuration.h"
#include "version.h"
#include "state.h"
#include "captureWidget.h"
#include "outputFFMPEG.h"
#include "outputAVI.h"
#include "outputDIB.h"
#include "outputMOV.h"
#include "outputSER.h"
#ifdef HAVE_LIBCFITSIO
#include "outputFITS.h"
#endif
#include "outputTIFF.h"
#include "outputPNG.h"
#include "targets.h"

#ifdef HAVE_LIBCFITSIO
#define	MAX_FILE_FORMATS	7
static QString	fileFormats[MAX_FILE_FORMATS] = {
    "", "AVI", "SER", "TIFF", "PNG", "FITS", "MOV"
};
#else
#define	MAX_FILE_FORMATS	6
static QString	fileFormats[MAX_FILE_FORMATS] = {
    "", "AVI", "SER", "TIFF", "PNG", "MOV"
};
#endif

CaptureWidget::CaptureWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  box = new QVBoxLayout ( this );
  profile = new QHBoxLayout;
  file = new QHBoxLayout;
  type = new QHBoxLayout;
  controls = new QHBoxLayout;

  profileLabel = new QLabel ( tr ( "Profile:" ), this );
  profileMenu = new QComboBox ( this );
  QStringList profileNames;
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      profileNames << config.profiles[i].profileName;
    }
  }
  profileMenu->addItems ( profileNames );
  if ( config.numProfiles ) {
    profileMenu->setCurrentIndex ( config.profileOption );
  }
  profileMenu->setEnabled ( 0 );
  connect ( profileMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( profileTypeChanged ( int )));

#ifdef RELOAD_PROFILE
  restoreButton = new QPushButton (
      QIcon ( ":/qt-icons/arrow-redo.png" ), "", this );
  restoreButton->setToolTip ( tr ( "Restore settings for current profile" ));
  connect ( restoreButton, SIGNAL( clicked()), this,
      SLOT( updateSettingsFromProfile()));
  restoreButton->setEnabled ( 0 );
#endif

  filterLabel = new QLabel ( tr ( "Filter:" ), this );
  filterMenu = new QComboBox ( this );
  QStringList filterNames;
  for ( int i = 0; i < config.numFilters; i++ ) {
    filterNames << config.filters[i].filterName;
  }
  filterMenu->addItems ( filterNames );
  filterMenu->setCurrentIndex ( config.filterOption );
  connect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));

  profile->addWidget ( profileLabel );
  profile->addWidget ( profileMenu );
#ifdef RELOAD_PROFILE
  profile->addWidget ( restoreButton );
#endif
  profile->addWidget ( filterLabel );
  profile->addWidget ( filterMenu );

  fileLabel = new QLabel ( tr ( "File: " ), this );
  fileName = new QLineEdit ( this );
  fileName->setText ( config.fileNameTemplate );
  QRegExp rx( "[^/; <>]+" );
  fileNameValidator = new QRegExpValidator ( rx, this );
  fileName->setValidator ( fileNameValidator );
  connect ( fileName, SIGNAL( editingFinished()), this,
      SLOT( updateFileNameTemplate()));
  connect ( fileName, SIGNAL( textEdited ( const QString& )), this,
      SLOT( updateFileNameTemplate()));

  newFolderButton = new QPushButton (
      QIcon ( ":/qt-icons/folder-new-7.png" ), "", this );
  newFolderButton->setToolTip ( tr ( "Select a new capture directory" ));
  connect ( newFolderButton, SIGNAL( clicked()), this,
      SLOT( setNewCaptureDirectory()));

  deleteButton = new QPushButton (
      QIcon ( ":/qt-icons/user-trash.png" ), "", this );
  deleteButton->setToolTip ( tr ( "Delete last captured file" ));
  connect ( deleteButton, SIGNAL( clicked()), this,
      SLOT( deleteLastRecordedFile()));

  openFolderButton = new QPushButton (
      QIcon ( ":/qt-icons/folder-open-4.png" ), "", this );
  openFolderButton->setToolTip ( tr ( "View capture directory" ));
  connect ( openFolderButton, SIGNAL( clicked()), this,
      SLOT( openCaptureDirectory()));

  file->addWidget ( fileLabel );
  file->addWidget ( fileName );
#ifdef RELOAD_PROFILE
  file->addWidget ( newFolderButton );
  file->addWidget ( deleteButton );
  file->addWidget ( openFolderButton );
#else
  profile->addWidget ( newFolderButton );
  profile->addWidget ( deleteButton );
  profile->addWidget ( openFolderButton );
#endif	/* RELOAD_PROFILE */

  typeLabel = new QLabel ( tr ( "Type:" ), this );
  typeMenu = new QComboBox ( this );
  for ( int i = 1; i < MAX_FILE_FORMATS; i++ ) {
    QVariant v(i);
    typeMenu->addItem ( fileFormats[i], v );
  }
  typeMenu->setCurrentIndex ( config.fileTypeOption - 1 );
  haveFITS = haveTIFF = havePNG = haveSER = haveMOV = 1;
  connect ( typeMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( fileTypeChanged ( int )));

  limitCheckbox = new QCheckBox ( tr ( "Limit:" ), this );
  limitCheckbox->setToolTip ( tr ( "Set a capture time limit" ));
  limitCheckbox->setChecked ( config.limitEnabled );
  connect ( limitCheckbox, SIGNAL( stateChanged ( int )), this,
      SLOT( showLimitInputBox ( int )));

  QStringList limitTypeStrings;
  limitTypeStrings << tr ( "secs" ) << tr ( "frames" );
  limitTypeMenu = new QComboBox ( this );
  limitTypeMenu->addItems ( limitTypeStrings );
  limitTypeMenu->setCurrentIndex ( config.limitType );
  connect ( limitTypeMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( limitTypeChanged ( int )));

  countValidator = new QIntValidator ( 0, 999999, this );

  countSecondsMenu = new QComboBox ( this );
  QStringList countSecondsStrings;
  countSecondsStrings << "30" << "45" << "60" << "90" << "120" << "150" <<
      "180" << "240" << "300";
  countSecondsMenu->addItems ( countSecondsStrings );
  countSecondsMenu->setEditable ( true );
  countSecondsMenu->setValidator ( countValidator );
  countSecondsMenu->setInsertPolicy ( QComboBox::InsertAtBottom );
  secondsInputBox = countSecondsMenu->lineEdit();

  countFramesMenu = new QComboBox ( this );
  QStringList countFramesStrings;
  countFramesStrings << "500" << "1000" << "1500" << "2000" << "2500" <<
      "3000" << "4000" << "5000";
  countFramesMenu->addItems ( countFramesStrings );
  countFramesMenu->setEditable ( true );
  countFramesMenu->setValidator ( countValidator );
  countFramesMenu->setInsertPolicy ( QComboBox::InsertAtBottom );
  framesInputBox = countFramesMenu->lineEdit();

  QString countStr;
  if ( config.framesLimitValue > 0 ) {
    countStr = QString::number ( config.framesLimitValue );
  } else {
    countStr = "0";
  }
  countFramesMenu->setEditText ( countStr );
  if ( config.secondsLimitValue > 0 ) {
    countStr = QString::number ( config.secondsLimitValue );
  } else {
    countStr = "0";
  }
  countSecondsMenu->setEditText ( countStr );

  if ( config.limitType ) {
    if ( config.limitEnabled ) {
      countFramesMenu->show();
    } else {
      countFramesMenu->hide();
    }
    countSecondsMenu->hide();
  } else {
    countSecondsMenu->setEditText ( countStr );
    if ( config.limitEnabled ) {
      countSecondsMenu->show();
    } else {
      countSecondsMenu->hide();
    }
    countFramesMenu->hide();
  }

  connect ( framesInputBox, SIGNAL( editingFinished()),
      this, SLOT( changeFramesLimitText()));
  connect ( framesInputBox, SIGNAL( textEdited ( const QString& )),
      this, SLOT( changeFramesLimitText()));
  connect ( countFramesMenu, SIGNAL( currentIndexChanged ( int )),
      this, SLOT( changeFramesLimitText()));
  connect ( secondsInputBox, SIGNAL( editingFinished()),
      this, SLOT( changeSecondsLimitText()));
  connect ( secondsInputBox, SIGNAL( textEdited ( const QString& )),
      this, SLOT( changeSecondsLimitText()));
  connect ( countSecondsMenu, SIGNAL( currentIndexChanged ( int )),
      this, SLOT( changeSecondsLimitText()));

  type->addWidget ( typeLabel );
  type->addWidget ( typeMenu );
  type->addWidget ( limitCheckbox );
  type->addWidget ( countFramesMenu );
  type->addWidget ( countSecondsMenu );
  type->addWidget ( limitTypeMenu );
  type->addStretch ( 1 );

  startButton = new QPushButton (
      QIcon ( ":/qt-icons/media-playback-start-7.png" ), "", this );
  startButton->setToolTip ( tr ( "Start capturing data" ));
  connect ( startButton, SIGNAL( clicked()), this, SLOT( startRecording()));

  pauseButton = new QPushButton (
      QIcon ( ":/qt-icons/media-playback-pause-7.png" ), "", this );
  pauseButton->setToolTip ( tr ( "Pause in-process recording" ));
  pauseButton->setCheckable ( 1 );
  connect ( pauseButton, SIGNAL( clicked()), this, SLOT( pauseRecording()));

  stopButton = new QPushButton (
      QIcon ( ":/qt-icons/media-playback-stop-7.png" ), "", this );
  stopButton->setToolTip ( tr ( "Stop capturing data" ));
  connect ( stopButton, SIGNAL( clicked()), this, SLOT( stopRecording()));

#ifdef SESSION_BROWSER
  fileListButton = new QPushButton (
      QIcon ( ":/qt-icons/format-list-unordered.png" ), "", this );
#endif

  autorunButton = new QPushButton (
      QIcon ( ":/qt-icons/clicknrun_grey.png" ), "", this );
  autorunButton->setToolTip ( tr ( "Reset autorun settings" ));
  autorunLabel = new QLabel ( "   " );

  startButton->setEnabled ( 0 );
  pauseButton->setEnabled ( 0 );
  stopButton->setEnabled ( 0 );
  if ( config.autorunCount && config.limitEnabled &&
    (( config.framesLimitValue && config.limitType ) ||
    ( config.secondsLimitValue && !config.limitType ))) {
    autorunButton->setEnabled ( 1 );
  } else {
    autorunButton->setEnabled ( 0 );
  }
  connect ( autorunButton, SIGNAL( clicked()), this, SLOT( resetAutorun()));

  controls->addWidget ( startButton );
  controls->addWidget ( pauseButton );
  controls->addWidget ( stopButton );
#ifdef SESSION_BROWSER
  controls->addWidget ( fileListButton );
#endif
  controls->addWidget ( autorunButton );
  controls->addWidget ( autorunLabel );

  box->addLayout ( profile );
  box->addLayout ( file );
  box->addLayout ( type );
  box->addLayout ( controls );

  if ( !config.dockableControls ) {
    setTitle ( tr ( "Capture" ));
  }
  setLayout ( box );

  outputHandler = 0;

  // Final setup for signals to avoid crossing threads when widgets need to
  // be updated

  connect ( this, SIGNAL( changeAutorunLabel ( QString )), this,
      SLOT ( updateAutorunLabel ( QString )));
  connect ( this, SIGNAL( enableStopButton ( int )), this,
      SLOT ( changeStopButtonState ( int )));
  connect ( this, SIGNAL( configureButtonsForStart ( int )), this,
      SLOT ( setButtonsForBeginRecording ( int )));
  connect ( this, SIGNAL( configureButtonsAfterStop ( void )), this,
      SLOT ( setButtonsForRecordingStopped ( void )));
}


CaptureWidget::~CaptureWidget()
{
  state.mainWindow->destroyLayout (( QLayout* ) box );
}


void
CaptureWidget::showLimitInputBox ( int state )
{
  if ( state == Qt::Unchecked ) {
    config.limitEnabled = 0;
    limitTypeMenu->hide();
    countFramesMenu->hide();
    countSecondsMenu->hide();
  } else {
    config.limitEnabled = 1;
    limitTypeMenu->show();
    if ( config.limitType ) {
      countFramesMenu->show();
      countSecondsMenu->hide();
    } else {
      countSecondsMenu->show();
      countFramesMenu->hide();
    }
  }
  SET_PROFILE_CONFIG( limitEnabled, config.limitEnabled );
}


void
CaptureWidget::fileTypeChanged ( int index )
{
  QVariant v = typeMenu->itemData ( index );
  config.fileTypeOption = v.toInt();
  SET_PROFILE_CONFIG( fileTypeOption, config.fileTypeOption );
  if ( CAPTURE_TIFF == config.fileTypeOption ||
      CAPTURE_PNG == config.fileTypeOption ||
      CAPTURE_FITS == config.fileTypeOption ) {
    if ( !config.fileNameTemplate.contains ( "%INDEX" ) &&
        !config.fileNameTemplate.contains ( "%I" )) {
      QMessageBox::warning ( this, APPLICATION_NAME,
          tr ( "The " ) + fileFormats[ config.fileTypeOption ] +
          tr ( " file format is selected, but the filename template "
          "does not contain either the \"%INDEX\" or \"%I\" pattern.  Output "
          "files may therefore overwrite each other" ));
    }
  }
}


void
CaptureWidget::limitTypeChanged ( int index )
{
  config.limitType = index;
  SET_PROFILE_CONFIG( limitType, config.limitType );
  if ( index ) {
    countSecondsMenu->hide();
    countFramesMenu->show();
  } else {
    countSecondsMenu->show();
    countFramesMenu->hide();
  }
}


void
CaptureWidget::pauseRecording ( void )
{
  struct timeval t;
  unsigned long now, timeLeft;

  ( void ) gettimeofday ( &t, 0 );
  now = ( unsigned long ) t.tv_sec * 1000 + ( unsigned long ) t.tv_usec / 1000;

  if ( pauseButton->isChecked()) {
    state.pauseEnabled = 1;
    lastPauseTime = now;
    state.captureWasPaused = 1;
  } else {
    if ( config.limitEnabled && 0 == config.limitType ) {
      // now we need to calculate the new stop time based on how much time
      // had was left between the end time and the pause time at the point
      // at which the capture was paused
      timeLeft = recordingEndTime - lastPauseTime;
      recordingEndTime = timeLeft + now;
      // And this is to give us something slightly nicer to display in the
      // percentage complete display (so it doesn't go over 100%)
      totalTimePaused += now - lastPauseTime;
    }
    state.pauseEnabled = 0;
  }
}


void
CaptureWidget::startRecording ( void )
{
  if ( config.timerEnabled && state.timer && state.timer->isInitialised()) {
    if ( CAPTURE_FITS != config.fileTypeOption || !config.limitEnabled || 
        !config.limitType ) {
      QString msg = tr ( "\n\nWhen using timer mode the image capture type "
          "should be FITS and a frame-based capture limit should be set.\n\n"
          "Capture run abandoned" );
      QMessageBox::warning ( this, APPLICATION_NAME, msg );
      return;
    }
  }

  // Warn if we're using an index in the filename and a single image per
  // file capture format and the number of runs multiplied by the expected
  // number of frames would exceed the size of the index.

  if ( config.limitEnabled && ( CAPTURE_TIFF == config.fileTypeOption ||
      CAPTURE_PNG == config.fileTypeOption ||
      CAPTURE_FITS == config.fileTypeOption ) &&
      ( config.fileNameTemplate.contains ( "%INDEX" ) ||
      config.fileNameTemplate.contains ( "%I" ))) {

    int numRuns, numDigits;
    unsigned long long numFrames, maxFrames;

    numRuns = state.autorunEnabled ? config.autorunCount : 1;
    switch ( config.limitType ) {
      case 0:
        numFrames = config.secondsLimitValue * state.currentFPS;
        break;
      case 1:
        numFrames = config.framesLimitValue;
        break;
    }
    maxFrames = state.captureIndex + numRuns * numFrames;
    numDigits = 0;
    do {
      maxFrames /= 10;
      numDigits++;
    } while ( maxFrames );
    if ( numDigits > config.indexDigits ) {
      if ( QMessageBox::critical ( this, APPLICATION_NAME,
          tr ( "The number of frames in the currently configured capture run "
          "is likely to exceed the size of the index in the filename.  "
          "Continue?" ), QMessageBox::Ok | QMessageBox::Cancel ) != 
          QMessageBox::Ok ) {
        return;
      }
    }
  }

  doStartRecording ( 0 );
  if ( state.autorunEnabled ) {
    emit changeAutorunLabel ( "1 of " +
        QString::number ( config.autorunCount ));
  }
}


void
CaptureWidget::doStartRecording ( int autorunFlag )
{
  OutputHandler*	out = 0;
  int			format;
  int			pauseButtonState = 1;
  int64_t		exposureTime;

  state.pauseEnabled = 0;
  state.captureWasPaused = 0;
  totalTimePaused = 0;

  state.previewWidget->setFirstFrameTime();

  if ( state.autorunEnabled ) {
    if ( !autorunFlag ) {
      autorunFilter = 0;
    }
    if ( config.autorunFilterSequence.count()) {
      int newFilterNum = config.autorunFilterSequence [ autorunFilter ];
      // FIX ME -- this may cross threads: don't cross the threads!
      if ( config.promptForFilterChange && !( state.filterWheel &&
          state.filterWheel->isInitialised())) {
        QMessageBox* changeFilter = new QMessageBox ( QMessageBox::NoIcon,
          APPLICATION_NAME, tr ( "Change to next filter: " ) +
          config.filters[ newFilterNum ].filterName,
          QMessageBox::Ok, state.mainWindow );
        changeFilter->exec();
        delete changeFilter;
      }
      filterTypeChanged ( newFilterNum );
    }
  }

  format = state.camera->videoFramePixelFormat();
  if ( OA_ISBAYER ( format ) && config.demosaicOutput ) {
    format = OA_DEMOSAIC_FMT ( format );
  }

  if ( config.queryGPSForEachCapture && state.timer && state.timer->hasGPS()) {
    if ( state.timer->readGPS ( &state.latitude, &state.longitude,
        &state.altitude ) == OA_ERR_NONE ) {
      state.gpsValid = 1;
    }
  }

  switch ( config.fileTypeOption ) {
    case CAPTURE_AVI:
      if ( config.windowsCompatibleAVI && WINDIB_OK( format )) {

        // This is a bit messy.  If we can write a UtVideo frame that's
        // nice and easy.  If we can write a reliable WinDIB format that's
        // also cool (that pretty much comes down to GREY8 or BGR24).
        // Otherwise to get something usable on Windows mosaic images have
        // to be demosaicked (at which point it makes sense to use UtVideo)
        // and if you have 16-bit greyscale well, you're up a smelly waterway
        // with no means of propulsion

        out = new OutputDIB ( config.imageSizeX, config.imageSizeY,
            state.controlWidget->getFPSNumerator(),
            state.controlWidget->getFPSDenominator());
      } else {
        if ( config.useUtVideo && UTVIDEO_OK( format )) {
          out = new OutputAVI ( config.imageSizeX, config.imageSizeY,
              state.controlWidget->getFPSNumerator(),
              state.controlWidget->getFPSDenominator(), format );
        } else {
          out = new OutputAVI ( config.imageSizeX, config.imageSizeY,
              state.controlWidget->getFPSNumerator(),
              state.controlWidget->getFPSDenominator(), format );
        }
      }
      break;

    case CAPTURE_MOV:
      out = new OutputMOV ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator(), format );
      break;

    case CAPTURE_SER:
      out = new OutputSER ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator(), format );
      break;

    case CAPTURE_TIFF:
      out = new OutputTIFF ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator(), format );
      break;

    case CAPTURE_PNG:
      out = new OutputPNG ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator(), format );
      break;

#ifdef HAVE_LIBCFITSIO
    case CAPTURE_FITS:
      out = new OutputFITS ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator(), format );
      break;
#endif
  }

  if ( out && ( CAPTURE_TIFF == config.fileTypeOption ||
      CAPTURE_PNG == config.fileTypeOption ||
      CAPTURE_FITS == config.fileTypeOption )) {
    if ( !out->outputWritable()) {
      // FIX ME -- this may cross threads: don't cross the threads!
      QMessageBox::warning ( this, tr ( "Start Recording" ),
        tr ( "Output is not writable" ));
      delete out;
      out = 0;
      return;
    }
  } else {
    if ( out && out->outputExists()) {
      if ( out->outputWritable()) {
        // FIX ME -- this may cross threads: don't cross the threads!
        if ( QMessageBox::question ( this, tr ( "Start Recording" ),
            tr ( "Output file exists.  OK to overwrite?" ), QMessageBox::No |
            QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No ) {
          delete out;
          out = 0;
          return;
        }
      } else {
        // FIX ME -- this may cross threads: don't cross the threads!
        QMessageBox::warning ( this, tr ( "Start Recording" ),
          tr ( "Output file exists and is not writable" ));
        delete out;
        out = 0;
        return;
      }
    }
  }

  if ( !out || out->openOutput()) {
    // FIX ME -- this may cross threads: don't cross the threads!
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Unable to create file for output" ));
    if ( state.autorunEnabled ) {
      state.autorunRemaining = 1; // force timeout
      filterSequenceRemaining = 1;
      singleAutorunFinished();
      autorunFlag = 0;
    }
    startButton->setEnabled ( 1 );
    pauseButton->setEnabled ( 0 );
    stopButton->setEnabled ( 0 );
    state.imageWidget->enableAllControls ( 1 );
    state.controlWidget->enableFPSControl ( 1 );
    return;
  }

  if ( state.histogramOn ) {
    state.histogramWidget->resetStats();
  }

  if ( config.timerEnabled && state.timer && state.timer->isInitialised()) {
    if ( state.timer->hasControl ( OA_TIMER_CTRL_MODE )) {
      state.timer->setControl ( OA_TIMER_CTRL_MODE, config.timerMode );
    }
    if ( state.timer->hasControl ( OA_TIMER_CTRL_COUNT )) {
      state.timer->setControl ( OA_TIMER_CTRL_COUNT, config.framesLimitValue );
    }
    if ( config.timerMode == OA_TIMER_MODE_TRIGGER ) {
      if ( state.timer->hasControl ( OA_TIMER_CTRL_INTERVAL )) {
        state.timer->setControl ( OA_TIMER_CTRL_INTERVAL,
            config.triggerInterval );
      }
      // FIX ME
      qWarning() << "Need to check camera trigger is armed?";
      // To ensure that the camera is ready to be triggered, poll the
      // SOFTWARE_TRIGGER register 0x62C.
      // bit 0 should be 0 to be ready to trigger
      // do this with a liboaPTR function?
    }

    // The camera should already be running, but we don't know when it
    // is going to start sending the frames that correspond to the sync
    // pulses.  So, temporarily we stop the camera, allow the frames
    // to drain (we hope), then enable strobe mode on the timer device
    // and restart the camera
    emit writeStatusMessage ( tr ( "Pausing camera" ));
    state.camera->stop();
    // Default is to stop for ten times the exposure interval, up to a
    // maximum of one second.  If we don't have an absolute exposure
    // value then guess at half a second total
    exposureTime = 500000; // 500ms
    if ( state.camera->hasControl ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) {
      exposureTime = state.controlWidget->getCurrentExposure() * 10;
      if ( exposureTime > 1000000 ) {
        exposureTime = 1000000;
      }
    }
    if ( config.userDrainDelayEnabled ) {
      exposureTime = config.drainDelay * 1000;
    }
    usleep ( exposureTime );
    emit writeStatusMessage ( tr ( "Starting timer" ));
    state.timer->start();
    emit writeStatusMessage ( tr ( "Restarting camera" ));
    state.camera->start();
    pauseButtonState = 0;
  }

  outputHandler = out;
  emit writeStatusMessage ( tr ( "Recording started" ));
  state.lastRecordedFile = out->getRecordingFilename();

  if ( config.limitEnabled && config.secondsLimitValue ) {
    struct timeval t;
    ( void ) gettimeofday ( &t, 0 );
    unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
        ( unsigned long ) t.tv_usec / 1000;
    recordingStartTime = now;
    recordingEndTime = now + config.secondsLimitValue * 1000;
  }

  if ( state.autorunEnabled ) {
    if ( !autorunFlag ) {
      state.autorunRemaining = config.autorunCount;
      filterSequenceRemaining = config.autorunFilterSequence.count();
    }
  }

  emit configureButtonsForStart ( pauseButtonState );
  state.previewWidget->beginRecording();
}


void
CaptureWidget::stopRecording ( void )
{
  if ( state.autorunEnabled ) {
    state.autorunEnabled = 0;
    state.autorunStartNext = 0;
    state.autorunRemaining = 0;
  }
  state.previewWidget->forceRecordingStop();
  if ( config.autorunCount && config.limitEnabled &&
      (( config.framesLimitValue  && config.limitType ) ||
      ( config.secondsLimitValue && !config.limitType ))) {
    autorunButton->setEnabled ( 1 );
  }
  // shouldn't need this as it should get done in
  // setButtonsForRecordingStopped()
  // stopButton->setEnabled ( 0 );
}


void
CaptureWidget::doStopRecording ( void )
{
  emit writeStatusMessage ( tr ( "Recording stopped" ));
  if ( state.histogramOn ) {
    state.histogramWidget->stopStats();
  }
  if ( config.saveCaptureSettings && outputHandler ) {
    writeSettings ( outputHandler );
  }
  closeOutputHandler();
  emit configureButtonsAfterStop();
}


void
CaptureWidget::enableStartButton ( int state )
{
  startButton->setEnabled ( state );
}


void
CaptureWidget::enableProfileSelect ( int state )
{
  profileMenu->setEnabled ( state );
#ifdef RELOAD_PROFILE
  restoreButton->setEnabled ( config.numProfiles ? state : 0 );
#endif
}


void
CaptureWidget::closeOutputHandler ( void )
{
  if ( outputHandler ) {
    outputHandler->closeOutput();
    delete outputHandler;
    outputHandler = 0;
  }
}


OutputHandler*
CaptureWidget::getOutputHandler ( void )
{
  return outputHandler;
}


void
CaptureWidget::enableSERCapture ( int state )
{
  if ( haveSER && !state ) {
    typeMenu->removeItem ( CAPTURE_SER - 1 );
  }
  if ( !haveSER && state ) {
    QVariant v( CAPTURE_SER );
    typeMenu->insertItem ( CAPTURE_SER - 1, fileFormats[ CAPTURE_SER ], v );
  }
  haveSER = state;
}


void
CaptureWidget::enableTIFFCapture ( int state )
{
  int posn;

  posn = CAPTURE_TIFF - 1 - ( haveSER ? 0 : 1 );

  if ( haveTIFF && !state ) {
    if ( typeMenu->currentIndex() == CAPTURE_TIFF - 1 ) {
      QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "TIFF output format has been disabled" ));
    }
    typeMenu->removeItem ( posn );
  }
  if ( !haveTIFF && state ) {
    QVariant v( CAPTURE_TIFF );
    typeMenu->insertItem ( posn, fileFormats[ CAPTURE_TIFF ], v );
  }
  haveTIFF = state;
}


void
CaptureWidget::enablePNGCapture ( int state )
{
  int posn;

  posn = CAPTURE_PNG - 1 - ( haveSER ? 0 : 1 ) - ( haveTIFF ? 0 : 1 );

  if ( havePNG && !state ) {
    if ( typeMenu->currentIndex() == CAPTURE_PNG - 1 ) {
      QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "PNG output format has been disabled" ));
    }
    typeMenu->removeItem ( posn );
  }
  if ( !havePNG && state ) {
    QVariant v( CAPTURE_PNG );
    typeMenu->insertItem ( posn, fileFormats[ CAPTURE_PNG ], v );
  }
  havePNG = state;
}

void
CaptureWidget::enableFITSCapture ( int state )
{
#ifdef HAVE_LIBCFITSIO
  int posn;

  posn = CAPTURE_FITS - 1 - ( haveTIFF ? 0 : 1 ) - ( haveSER ? 0 : 1 ) -
      ( havePNG ? 0 : 1 );

  if ( haveFITS && !state ) {
    if ( typeMenu->currentIndex() == posn ) {
      QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "FITS output format has been disabled" ));
    }
    typeMenu->removeItem ( posn );
  }
  if ( !haveFITS && state ) {
    QVariant v( CAPTURE_FITS );
    typeMenu->insertItem ( posn, fileFormats[ CAPTURE_FITS ], v );
  }
  haveFITS = state;
#endif
  return;
}


void
CaptureWidget::enableMOVCapture ( int state )
{
  int posn;

  posn = CAPTURE_MOV - 1 - ( haveFITS ? 0 : 1 ) - ( haveTIFF ? 0 : 1 ) -
      ( haveSER ? 0 : 1 ) - ( havePNG ? 0 : 1 );

  if ( haveMOV && !state ) {
    if ( typeMenu->currentIndex() == posn ) {
      QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "MOV output format has been disabled" ));
    }
    typeMenu->removeItem ( posn );
  }
  if ( !haveMOV && state ) {
    QVariant v( CAPTURE_MOV );
    typeMenu->insertItem ( posn, fileFormats[ CAPTURE_MOV ], v );
  }
  haveMOV = state;
  return;
}


void
CaptureWidget::setNewCaptureDirectory ( void )
{
  QFileDialog dialog( this );

  dialog.setFileMode ( QFileDialog::Directory );
  dialog.setOption ( QFileDialog::ShowDirsOnly );
  if ( config.captureDirectory != "" ) {
    dialog.setDirectory ( config.captureDirectory );
  } else {
    dialog.setDirectory ( "." );
  }
  dialog.setWindowTitle ( tr ( "Select capture directory" ));
  int done = 0;
  while ( !done ) {
    if ( dialog.exec()) {
      QStringList names = dialog.selectedFiles();
      if ( !access ( names[0].toStdString().c_str(), W_OK | R_OK )) {
        config.captureDirectory = names[0];
        done = 1;
      } else {
        QMessageBox err;
        err.setText ( tr (
            "The selected directory is not writable/accessible" ));
        err.setInformativeText ( tr ( "Select another?" ));
        err.setStandardButtons ( QMessageBox::No | QMessageBox::Yes );
        err.setDefaultButton ( QMessageBox::Yes );
        int ret = err.exec();
        done = ( ret == QMessageBox::Yes ) ? 0 : 1;
      }
    } else {
      done = 1;
    }
  }
}


void
CaptureWidget::changeFramesLimitText ( void )
{
  QString countStr = framesInputBox->text();
  if ( countStr != "" ) {
    config.framesLimitValue = countStr.toInt();
    SET_PROFILE_CONFIG( framesLimitValue, config.framesLimitValue );
  }
}


void
CaptureWidget::changeSecondsLimitText ( void )
{
  QString countStr = secondsInputBox->text();
  if ( countStr != "" ) {
    config.secondsLimitValue = countStr.toInt();
    SET_PROFILE_CONFIG( secondsLimitValue, config.secondsLimitValue );
  }
}


void
CaptureWidget::changeFramesLimitCount ( int index )
{
  Q_UNUSED ( index )
  qWarning() << "CaptureWidget::changeFramesLimitCount called unexpectedly";
}


void
CaptureWidget::changeSecondsLimitCount ( int index )
{
  Q_UNUSED ( index )
  qWarning() << "CaptureWidget::changeSecondsLimitCount called unexpectedly";
}


void
CaptureWidget::enableAutorun ( void )
{
  autorunButton->setEnabled ( 1 );
  autorunButton->setIcon ( QIcon ( ":/qt-icons/clicknrun.png" ) );
  autorunLabel->setText ( "0 of " + QString::number ( config.autorunCount ));
  // set this to 0 to stop autorun being started automagicallly until
  // the first one is kicked off with the start button
  state.autorunStartNext = 0;
}


int
CaptureWidget::singleAutorunFinished ( void )
{
  int filterSequenceLength;

  filterSequenceLength = config.autorunFilterSequence.count();
  if ( filterSequenceLength ) {
    if ( !--filterSequenceRemaining ) {
      filterSequenceRemaining = filterSequenceLength;
      autorunFilter = 0;
    } else {
      autorunFilter++;
    }
  }
  // This can be called from the preview window thread, so it can't make
  // direct calls to change the labels
  if ( !--state.autorunRemaining ) {
    emit changeAutorunLabel ( "     " );
    state.autorunEnabled = 0;
    state.autorunStartNext = 0;
    emit enableStopButton ( 0 );
  }
  return state.autorunRemaining;
}


void
CaptureWidget::startNewAutorun ( void )
{
  doStartRecording ( 1 );
  emit changeAutorunLabel ( QString::number ( config.autorunCount -
      state.autorunRemaining + 1 ) + " of " +
      QString::number ( config.autorunCount ));
  state.autorunStartNext = 0;
}


void
CaptureWidget::resetAutorun ( void )
{
  if ( config.autorunCount ) {
    enableAutorun();
    state.autorunEnabled = 1;
    emit writeStatusMessage ( tr ( "Autorun Enabled" ));
  }
}


void
CaptureWidget::deleteLastRecordedFile ( void )
{
  if ( "" == state.lastRecordedFile ) {
    QMessageBox::warning ( this, tr ( "Delete File" ),
        tr ( "No last file to delete" ));
    return;
  }

  QString name = state.lastRecordedFile.section ( '/', -1 );
  if ( QMessageBox::question ( this, tr ( "Delete File" ),
      tr ( "Delete file " ) + name, QMessageBox::Cancel | QMessageBox::Yes,
      QMessageBox::Cancel ) == QMessageBox::Yes ) {
    if ( unlink ( state.lastRecordedFile.toStdString().c_str())) {
      QMessageBox::warning ( this, tr ( "Delete File" ),
      tr ( "Delete failed for" ) + name );
    }
    state.lastRecordedFile = "";
  }
}


void
CaptureWidget::openCaptureDirectory ( void )
{
  QFileDialog dialog( this );

  dialog.setFileMode ( QFileDialog::AnyFile );
  if ( config.captureDirectory != "" ) {
    dialog.setDirectory ( config.captureDirectory );
  } else {
    dialog.setDirectory ( "." );
  }
  dialog.setWindowTitle ( tr ( "Capture directory" ));
  dialog.exec();
}


void
CaptureWidget::updateFileNameTemplate ( void )
{
  config.fileNameTemplate = fileName->text();
  SET_PROFILE_CONFIG( fileNameTemplate, config.fileNameTemplate );
}


QString
CaptureWidget::getCurrentFilterName ( void )
{
  int f = -1;

  if ( state.filterWheel && state.filterWheel->isInitialised()) {
    f = config.filterSlots[ config.filterOption ];
  } else {
    if ( config.numFilters > 0 && config.filterOption < config.numFilters ) {
      f = config.filterOption;
    }
  }
  return ( f >= 0 ) ? config.filters[ f ].filterName : "";
}


void
CaptureWidget::filterTypeChanged ( int index )
{
  config.filterOption = index;
  if ( state.filterWheel->isInitialised()) {
    state.filterWheel->selectFilter ( index + 1 );
  }
  updateFilterSettingsFromProfile();
}


QString
CaptureWidget::getCurrentProfileName ( void )
{
  if ( config.numProfiles > 0 && config.profileOption < config.numProfiles ) {
    return config.profiles[ config.profileOption ].profileName;
  }
  return "";
}


QString
CaptureWidget::getCurrentTargetName ( void )
{
  if ( config.numProfiles > 0 && config.profileOption < config.numProfiles ) {
    return targetList [ config.profiles[ config.profileOption ].target ];
  }
  return "";
}


int
CaptureWidget::getCurrentTargetId ( void )
{
  if ( config.numProfiles > 0 && config.profileOption < config.numProfiles ) {
    return config.profiles[ config.profileOption ].target;
  }
  return -1;
}


void
CaptureWidget::profileTypeChanged ( int index )
{
  config.profileOption = index;
  updateSettingsFromProfile();
}


void
CaptureWidget::reloadFilters ( void )
{
  disconnect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));

  int oldCount = filterMenu->count();
  if ( !state.filterWheel || !state.filterWheel->isInitialised()) {
    if ( config.numFilters ) {
      for ( int i = 0; i < config.numFilters; i++ ) {
        if ( i < oldCount ) {
          filterMenu->setItemText ( i, config.filters[i].filterName );
        } else {
          filterMenu->addItem ( config.filters[i].filterName );
        }
      }
    }
    if ( config.numFilters < oldCount ) {
      for ( int i = config.numFilters; i < oldCount; i++ ) {
        filterMenu->removeItem ( config.numFilters );
      }
    }
  } else {
    int numSlots = state.filterWheel->numSlots();
    if ( numSlots ) {
      for ( int i = numSlots - 1; i >= 0; i-- ) {
        if ( config.filterSlots[ i ] >= 0 ) {
          if ( i < oldCount ) {
            filterMenu->setItemText ( i,
                config.filters[ config.filterSlots[ i ]].filterName );
          } else {
            filterMenu->addItem (
                config.filters[ config.filterSlots[ i ]].filterName );
          }
        } else {
          filterMenu->removeItem ( i );
        }
      }
    }
    if ( numSlots < oldCount ) {
      for ( int i = numSlots; i < oldCount; i++ ) {
        filterMenu->removeItem ( numSlots );
      }
    }
  }

  connect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));
}


void
CaptureWidget::reloadProfiles ( void )
{
  disconnect ( profileMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( profileTypeChanged ( int )));
  int oldCount = profileMenu->count();
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      if ( i < oldCount ) {
        profileMenu->setItemText ( i, config.profiles[i].profileName );
      } else {
        profileMenu->addItem ( config.profiles[i].profileName );
      }
    }
  }
  if ( config.numProfiles < oldCount ) {
    for ( int i = config.numProfiles; i < oldCount; i++ ) {
      profileMenu->removeItem ( config.numProfiles );
    }
  }
  connect ( profileMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( profileTypeChanged ( int )));
#ifdef RELOAD_PROFILE
  restoreButton->setEnabled ( config.numProfiles ? 1 : 0 );
#endif
}


void
CaptureWidget::updateSettingsFromProfile ( void )
{
  config.sixteenBit = config.profiles[ config.profileOption ].sixteenBit;
  config.binning2x2 = config.profiles[ config.profileOption ].binning2x2;
  config.rawMode = config.profiles[ config.profileOption ].rawMode;
  config.colourise = config.profiles[ config.profileOption ].colourise;
  config.useROI = config.profiles[ config.profileOption ].useROI;
  config.imageSizeX = config.profiles[ config.profileOption ].imageSizeX;
  config.imageSizeY = config.profiles[ config.profileOption ].imageSizeY;

  config.frameRateNumerator =
      config.profiles[ config.profileOption ].frameRateNumerator;
  config.frameRateDenominator =
      config.profiles[ config.profileOption ].frameRateDenominator;
  config.fileTypeOption =
      config.profiles[ config.profileOption ].fileTypeOption;
  config.fileNameTemplate =
      config.profiles[ config.profileOption ].fileNameTemplate;
  config.limitEnabled =
      config.profiles[ config.profileOption ].limitEnabled;
  config.framesLimitValue =
      config.profiles[ config.profileOption ].framesLimitValue;
  config.secondsLimitValue =
      config.profiles[ config.profileOption ].secondsLimitValue;

  for ( int i = 1; i < OA_CAM_CTRL_LAST_P1; i++ ) {
    for ( int j = 0; j < OA_CAM_CTRL_MODIFIERS_P1; j++ ) {
      config.controlValues[j][i] =
        config.profiles[ config.profileOption ].filterProfiles[
        config.filterOption ].controls[j][i];
    }
  }

  state.controlWidget->updateFromConfig();
  state.cameraWidget->updateFromConfig();
  state.imageWidget->updateFromConfig();
  updateFromConfig();
}


void
CaptureWidget::updateFilterSettingsFromProfile ( void )
{
  for ( int i = 1; i < OA_CAM_CTRL_LAST_P1; i++ ) {
    for ( int j = 0; j < OA_CAM_CTRL_MODIFIERS_P1; j++ ) {
      config.controlValues[j][i] =
        config.profiles[ config.profileOption ].filterProfiles[
        config.filterOption ].controls[j][i];
    }
  }

  state.controlWidget->updateFromConfig();
  updateFromConfig();
}


void
CaptureWidget::updateFromConfig ( void )
{
  disconnect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));
  if ( config.numFilters > config.filterOption ) {
    filterMenu->setCurrentIndex ( config.filterOption );
  }
  connect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));
  fileName->setText ( config.fileNameTemplate );
  if ( typeMenu->count() >= config.fileTypeOption ) {
    typeMenu->setCurrentIndex ( config.fileTypeOption - 1 );
  }
  limitCheckbox->setChecked ( config.limitEnabled );
  if ( config.framesLimitValue > 0 ) {
    framesInputBox->setText ( QString::number ( config.framesLimitValue ));
  }
  if ( config.secondsLimitValue > 0 ) {
    secondsInputBox->setText ( QString::number ( config.secondsLimitValue ));
  }
}


void
CaptureWidget::writeSettings ( OutputHandler* out )
{
  unsigned long duration;

  QString settingsFile = out->getRecordingBasename();
  settingsFile += ".txt";

  std::ofstream settings ( settingsFile.toStdString().c_str());
  if ( settings.is_open()) {
    time_t now = state.firstFrameTime / 1000;
    struct tm* tmp = localtime ( &now );
    QString timeStr;
    timeStr = QString ("%1-%2-%3 %4:%5:%6").
        arg (( int ) tmp->tm_year + 1900, 4 ).
        arg (( int ) tmp->tm_mon + 1, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_mday, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_hour, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_min, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_sec, 2, 10, QChar('0'));
    settings << APPLICATION_NAME << " " << VERSION_STR << std::endl;
    settings << std::endl;
    settings << tr ( "Camera: " ).toStdString().c_str() <<
        state.camera->name() << std::endl;
    settings << tr ( "Time: " ).toStdString().c_str() <<
        timeStr.toStdString() << std::endl;

    char zone[40];
    strftime ( zone, 40, "%Z (%z)", tmp );
    settings << tr ( "Timezone: " ).toStdString().c_str() << zone << std::endl;

    settings << tr ( "Profile: " ).toStdString().c_str() <<
        getCurrentProfileName().toStdString().c_str() << std::endl;
    settings << tr ( "Target: " ).toStdString().c_str() <<
        getCurrentTargetName().toStdString().c_str() << std::endl;
    
    for ( int baseVal = 1; baseVal < OA_CAM_CTRL_LAST_P1; baseVal++ ) {
      for ( int mod = 0; mod <= OA_CAM_CTRL_MODIFIER_AUTO; mod++ ) {
        int c = baseVal | ( mod ? OA_CAM_CTRL_MODIFIER_AUTO_MASK : 0 );
        int type;
        if (( type = state.camera->hasControl ( c ))) {
          int v = config.CONTROL_VALUE( c );
          switch ( c ) {
            case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
            {
              QString interval = state.controlWidget->exposureIntervalString();
              QString expTextStr = "Exposure (" + interval + "): ";
              settings << tr (
                  expTextStr.toStdString().c_str()).toStdString().c_str();
              break;
            }
            case OA_CAM_CTRL_TEMPERATURE:
              if ( config.tempsInC ) {
                settings << tr ( "Temp (C): " ).toStdString().c_str();
              } else {
                settings << tr ( "Temp (F): " ).toStdString().c_str();
              }
              break;
            default:
              if ( type != OA_CTRL_TYPE_BUTTON ) {
                if ( mod == OA_CAM_CTRL_MODIFIER_AUTO ) {
                  settings << tr ( "Auto" ).toStdString().c_str() << " ";
                }
                settings << tr ( oaCameraControlLabel[
                    baseVal ] ).toStdString().c_str() << ": ";
              }
              break;
          }

          if ( state.camera->hasControl ( OA_CAM_CTRL_MODE_ON_OFF (baseVal))) {
            v = config.CONTROL_VALUE ( OA_CAM_CTRL_MODE_ON_OFF ( baseVal ));
            settings << tr ( v ? "on" : "off" ).toStdString().c_str() <<
                std::endl;
          } else {

            switch ( type ) {

              case OA_CTRL_TYPE_INT32:
              case OA_CTRL_TYPE_INT64:
                settings << v << std::endl;
                break;

              case OA_CTRL_TYPE_BOOLEAN:
                if ( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED )
                    == c || OA_CAM_CTRL_MODE_AUTO(
                    OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) == c ) {
                  settings << tr ( OA_EXPOSURE_AUTO == v ? "on" :
                      "off" ).toStdString().c_str() << std::endl;
                } else {
                  settings << tr ( v ? "on" : "off" ).toStdString().c_str() <<
                      std::endl;
                }
                break;

              case OA_CTRL_TYPE_READONLY:
                switch ( c ) {

                  case OA_CAM_CTRL_TEMPERATURE:
                  {
                    QString stringVal;
                    float temp = state.camera->getTemperature();
                    state.cameraTempValid = 1;
                    state.cameraTemp = temp;
                    if ( !config.tempsInC ) {
                      temp = temp * 9 / 5 + 32;
                    }
                    stringVal.setNum ( temp, 'g', 3 );
                    settings << stringVal.toStdString().c_str() << std::endl;
                    break;
                  }

                  case OA_CAM_CTRL_DROPPED:
                    settings << state.camera->readControl( OA_CAM_CTRL_DROPPED )
                      << std::endl;
                    break;

                  default:
                    settings << tr ( "not recorded" ).toStdString().c_str() <<
                        std::endl;
                    break;
                }
                break;

              case OA_CTRL_TYPE_BUTTON:
                break;

              case OA_CTRL_TYPE_DISCRETE:
                switch ( c ) {

                  case OA_CAM_CTRL_BINNING:
                    if ( config.binning2x2 ) {
                      settings << "2x2" << std::endl;
                    } else {
                      settings << "1x1" << std::endl;
                    }
                    break;

                  case OA_CAM_CTRL_BIT_DEPTH:
                  {
                    int fmt = state.camera->videoFramePixelFormat();
                    int Bpp = OA_BYTES_PER_PIXEL( fmt );
                    int bpp = Bpp * 8;
                    settings << bpp << std::endl;
                    break;
                  }
                  default:
                    settings << tr ( "not recorded" ).toStdString().c_str() <<
                        std::endl;
                    break;
                }
                break;

              default:
                settings << tr ( "not recorded" ).toStdString().c_str() <<
                    std::endl;
                break;
            }
          }
        }
      }
    }

    settings << tr ( "Image size: " ).toStdString().c_str() <<
        config.imageSizeX << "x" << config.imageSizeY << std::endl;

    if ( state.camera->hasFrameRateSupport()) {
      settings << tr ( "Frame rate/sec: " ).toStdString().c_str();
      if ( state.captureWasPaused ) {
        settings << tr ( "n/a (capture paused)" ).toStdString().c_str();
      } else {
        settings << config.frameRateNumerator;
        if ( config.frameRateDenominator != 1 ) {
          settings << "/" << config.frameRateDenominator;
        }
      }
      settings << std::endl;
    }
    if ( config.numFilters > 0 && config.filterOption < config.numFilters ) {
      QString n = getCurrentFilterName();
      settings <<  tr ( "Filter: " ).toStdString().c_str() <<
          n.toStdString() << std::endl;
    }

    time_t t = state.firstFrameTime / 1000;
    tmp = localtime ( &t );
    timeStr = QString ("%1:%2:%3.%4").
        arg (( int ) tmp->tm_hour, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_min, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_sec, 2, 10, QChar('0')).
        arg ( state.firstFrameTime % 1000, 3, 10, QChar('0'));
    settings << tr ( "First frame: " ).toStdString().c_str() <<
        timeStr.toStdString() << std::endl;
    unsigned long midTime = ( state.firstFrameTime +
        state.lastFrameTime ) / 2;
    t = midTime / 1000;
    tmp = localtime ( &t );
    timeStr = QString ("%1:%2:%3.%4").
        arg (( int ) tmp->tm_hour, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_min, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_sec, 2, 10, QChar('0')).
        arg ( midTime % 1000, 3, 10, QChar('0') );
    settings << tr ( "Middle: " ).toStdString().c_str() <<
        timeStr.toStdString();
    if ( state.captureWasPaused ) {
      settings <<
          tr ( " (estimate -- recording paused)" ).toStdString().c_str();
    }
    settings << std::endl;
    t = state.lastFrameTime / 1000;
    tmp = localtime ( &t );
    timeStr = QString ("%1:%2:%3.%4").
        arg (( int ) tmp->tm_hour, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_min, 2, 10, QChar('0')).
        arg (( int ) tmp->tm_sec, 2, 10, QChar('0')).
        arg ( state.lastFrameTime % 1000, 3, 10, QChar('0') );
    settings << tr ( "Last frame: " ).toStdString().c_str() <<
        timeStr.toStdString() << std::endl;

    duration = state.lastFrameTime - state.firstFrameTime;
    timeStr = QString ("%1.%2").
        arg (( int ) duration / 1000, 1, 10, QChar('0')).
        arg (( int ) duration % 1000, 3, 10, QChar('0'));
    settings << tr ( "Duration (seconds): " ).toStdString().c_str() <<
        timeStr.toStdString() << std::endl;

    settings << tr ( "Frames captured: " ).toStdString().c_str() <<
        out->getFrameCount() << std::endl;

    float fps = ( float ) out->getFrameCount() / ( duration / 1000.0 );
    settings << tr ( "Frames per second (average): " ).toStdString().c_str()
        << ( int ) fps;
    if ( state.captureWasPaused ) {
      settings << tr ( " (recording paused)" ).toStdString().c_str();
    }
    settings << std::endl;

    if ( !out->writesDiscreteFiles ) {
      settings << tr ( "Filename: " ).toStdString().c_str() <<
          out->getRecordingFilename().section( QChar('/'), -1 ).toStdString().
          c_str() << std::endl;
    }

    settings << tr ( "Recording type: " ).toStdString().c_str() <<
        fileFormats[ config.fileTypeOption ].toStdString().c_str() <<
        std::endl;

    if ( config.limitEnabled && (( config.framesLimitValue &&
        config.limitType ) || ( config.secondsLimitValue &&
        !config.limitType ))) {
      settings << tr ( "Limit " ).toStdString().c_str();
      if ( config.limitType ) {
        settings << tr ( "(frames) : " ).toStdString().c_str() <<
            config.framesLimitValue << std::endl;
      } else {
        settings << tr ( "(seconds) : " ).toStdString().c_str() <<
            config.secondsLimitValue << std::endl;
      }
    }

    if ( state.histogramWidget ) {
      settings << tr ( "Histogram Min: " ).toStdString().c_str() <<
          state.histogramWidget->histogramMin << std::endl;
      settings << tr ( "Histogram Max: " ).toStdString().c_str() <<
          state.histogramWidget->histogramMax << std::endl;
      if ( state.histogramWidget->fullIntensity ) {
        int pc = 100 * state.histogramWidget->histogramMax /
            state.histogramWidget->fullIntensity;
        settings << tr ( "Histogram %: " ).toStdString().c_str() << pc <<
            "%" << std::endl;
      }
    }

    settings.close();
  } else {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Unable to create settings output file" ));
  }
}


void
CaptureWidget::setSlotCount ( int numSlots )
{
  Q_UNUSED ( numSlots )
  // Not actually sure we need to do anyting here as calls to reloadFilters
  // should take care of everything
  return;
}


void
CaptureWidget::updateAutorunLabel ( QString newText )
{
  autorunLabel->setText ( newText );
}


void
CaptureWidget::changeStopButtonState ( int newState )
{
  stopButton->setEnabled ( newState );
}

void
CaptureWidget::setButtonsForBeginRecording ( int pauseButtonState )
{
  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 1 );
  pauseButton->setEnabled ( pauseButtonState );
  autorunButton->setEnabled ( 0 );
  state.imageWidget->enableAllControls ( 0 );
  state.controlWidget->enableFPSControl ( 0 );
}


void
CaptureWidget::setButtonsForRecordingStopped ( void )
{
  startButton->setEnabled ( 1 );
  pauseButton->setEnabled ( 0 );
  stopButton->setEnabled ( 0 );
  if ( config.autorunCount && config.limitEnabled &&
      (( config.framesLimitValue  && config.limitType ) ||
      ( config.secondsLimitValue && !config.limitType ))) {
    autorunButton->setEnabled ( 1 );
  }
  state.imageWidget->enableAllControls ( 1 );
  state.controlWidget->enableFPSControl ( 1 );
}
