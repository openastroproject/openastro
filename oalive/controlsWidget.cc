/*****************************************************************************
 *
 * controlsWidget.cc -- the tab block for the controls
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

#include "version.h"
#include "configuration.h"
#include "state.h"
#include "controlsWidget.h"

#include "outputTIFF.h"
#include "outputFITS.h"


ControlsWidget::ControlsWidget ( QWidget* parent )
{
  mainBox = new QVBoxLayout;
  topButtonBox = new QHBoxLayout;
  bottomButtonBox = new QHBoxLayout;

  tabSet = new QTabWidget ( this );
  camera = new CameraControls ( this );
  stacking = new StackingControls ( this );
  save = new SaveControls ( this );

  startButton = new QPushButton ( "Start", this );
  stopButton = new QPushButton ( "Stop", this );
  restartButton = new QPushButton ( "Restart", this );

  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 0 );
  restartButton->setEnabled ( 0 );

  connect ( startButton, SIGNAL( clicked()), this, SLOT( startCapture()));
  connect ( stopButton, SIGNAL( clicked()), this, SLOT( stopCapture()));
  connect ( restartButton, SIGNAL( clicked()), this, SLOT( restartCapture()));

  topButtonBox->addWidget ( startButton );
  topButtonBox->addWidget ( stopButton );
  topButtonBox->addWidget ( restartButton );

/*
  general = new GeneralSettings ( this );
  capture = new CaptureSettings ( this );
  profiles = new ProfileSettings ( this );
  filters = new FilterSettings ( this );
  demosaic = new DemosaicSettings ( this );

  state.generalSettingsIndex = tabSet->addTab ( general, 
      QIcon ( ":/icons/cog.png" ), tr ( "General" ));
  state.captureSettingsIndex = tabSet->addTab ( capture,
      QIcon ( ":/icons/capture.png" ), tr ( "Capture" ));
  state.profileSettingsIndex = tabSet->addTab ( profiles,
      QIcon ( ":/icons/jupiter.png" ), tr ( "Profiles" ));
  state.filterSettingsIndex = tabSet->addTab ( filters,
      QIcon ( ":/icons/filter-wheel.png" ), tr ( "Filters" ));
  state.demosaicSettingsIndex = tabSet->addTab ( demosaic,
      QIcon ( ":/icons/mosaic.png" ), tr ( "Demosaic" ));
*/

  tabSet->addTab ( camera, tr ( "Camera" ));
  tabSet->addTab ( stacking, tr ( "Stacking" ));
  tabSet->addTab ( save, tr ( "Image Capture" ));
  tabSet->setTabPosition ( QTabWidget::East );
  tabSet->setUsesScrollButtons ( false );

#if 0
  lightsButton = new QPushButton ( "Lights", this );
  darksButton = new QPushButton ( "Darks", this );

  lightsButton->setEnabled ( 0 );
  darksButton->setEnabled ( 0 );

  bottomButtonBox->addWidget ( lightsButton );
  bottomButtonBox->addWidget ( darksButton );
#endif
  mainBox = new QVBoxLayout ( this );
  mainBox->addLayout ( topButtonBox );
  mainBox->addWidget ( tabSet );
#if 0
  mainBox->addLayout ( bottomButtonBox );
#endif
  setLayout ( mainBox );

  ignoreResolutionChanges = 0;
  frameOutputHandler = processedImageOutputHandler = 0;

  state.cameraControls = camera;
}


ControlsWidget::~ControlsWidget()
{
  state.mainWindow->destroyLayout (( QLayout* ) mainBox );
  state.cameraControls = 0;
}


void
ControlsWidget::setActiveTab ( int index )
{
  tabSet->setCurrentIndex ( index );
}


void
ControlsWidget::enableTab ( int index, int state )
{
  tabSet->setTabEnabled ( index, state );
}


void
ControlsWidget::configure ( void )
{
  camera->configure();
  configureResolution();
}


void
ControlsWidget::enableButtons ( int state )
{
  startButton->setEnabled ( state );
  stopButton->setEnabled ( !state );
  restartButton->setEnabled ( state );
#if 0
  lightsButton->setEnabled ( state );
  darksButton->setEnabled ( state );
#endif
}


void
ControlsWidget::disableAllButtons ( void )
{
  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 0 );
  restartButton->setEnabled ( 0 );
#if 0
  lightsButton->setEnabled ( 0 );
  darksButton->setEnabled ( 0 );
#endif
}

void
ControlsWidget::startCapture ( void )
{
  openOutputFiles();
  state.camera->start();
  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 1 );
}


void
ControlsWidget::stopCapture ( void )
{
  state.camera->stop();
  startButton->setEnabled ( 1 );
  stopButton->setEnabled ( 0 );
  if ( frameOutputHandler ) {
    frameOutputHandler->closeOutput();
    delete frameOutputHandler;
    frameOutputHandler = 0;
  }
  if ( processedImageOutputHandler ) {
    processedImageOutputHandler->closeOutput();
    delete processedImageOutputHandler;
    processedImageOutputHandler = 0;
  }
}


void
ControlsWidget::restartCapture ( void )
{
  state.camera->stop();
  if ( frameOutputHandler ) {
    frameOutputHandler->closeOutput();
    delete frameOutputHandler;
    frameOutputHandler = 0;
  }
  if ( processedImageOutputHandler ) {
    processedImageOutputHandler->closeOutput();
    delete processedImageOutputHandler;
    processedImageOutputHandler = 0;
  }
  openOutputFiles();
  state.viewWidget->restart();
  state.camera->start();
  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 1 );
}


void
ControlsWidget::configureResolution ( void )
{
  // FIX ME -- add controls for screen resolution
/* commented to prevent unused error
  int maxX, maxY;
  maxX = maxY = 0;
*/

  // This includes a particularly ugly way to sort the resolutions using
  // a QMap and some further QMap abuse to be able to find the X and Y
  // resolutions should we not have a setting that matches the config and
  // happen to choose the max size option
  const FRAMESIZES* sizeList = state.camera->frameSizes();
  QMap<int,QString> sortMap;
  QMap<int,int> xRes, yRes;
  QString showItemStr;
  int firstKey = -1, lastKey = -1;
  unsigned int i;

  for ( i = 0; i < sizeList->numSizes; i++ ) {
    int numPixels = sizeList->sizes[i].x * sizeList->sizes[i].y;
    QString resStr = QString::number ( sizeList->sizes[i].x ) + "x" +
        QString::number ( sizeList->sizes[i].y );
    if ( sizeList->sizes[i].x == config.imageSizeX &&
        sizeList->sizes[i].y == config.imageSizeY ) {
      showItemStr = resStr;
    }
    sortMap [ numPixels ] = resStr;
    xRes [ numPixels ] = sizeList->sizes[i].x;
    yRes [ numPixels ] = sizeList->sizes[i].y;
    if ( lastKey < numPixels ) {
      lastKey = numPixels;
    }
    if ( -1 == firstKey || firstKey > numPixels ) {
      firstKey = numPixels;
    }
  }

  int numItems = 0, showItem = -1, showXRes = -1, showYRes = -1;
  QStringList resolutions;
  QMap<int,QString>::iterator j;
  QMap<int,int>::iterator x, y;
  XResolutions.clear();
  YResolutions.clear();
  for ( j = sortMap.begin(), x = xRes.begin(), y = yRes.begin();
      j != sortMap.end(); j++, x++, y++ ) {
    resolutions << *j;
    XResolutions << *x;
    YResolutions << *y;
    if ( *j == showItemStr ) {
      showItem = numItems;
      showXRes = *x;
      showYRes = *y;
    }
    numItems++;
  }

/*
  ignoreResolutionChanges = 1;
  resMenu->clear();
  resMenu->addItems ( resolutions );
  ignoreResolutionChanges = 0;
*/

  if ( showItem >= 0 ) {
    config.imageSizeX = showXRes;
    config.imageSizeY = showYRes;
  } else {
    // FIX ME -- put back when there is a resolutions menu
    showItem = numItems - 1; // max->isChecked() ? numItems - 1: 0;
    if ( showItem ) {
      config.imageSizeX = xRes[ lastKey ];
      config.imageSizeY = yRes[ lastKey ];
    } else {
      config.imageSizeX = xRes[ firstKey ];
      config.imageSizeY = yRes[ firstKey ];
    }
  }
  // commented to prevent unused error
  //maxX = xRes[ lastKey ];
  //maxY = yRes[ lastKey ];

  // There's a gotcha here for cameras that only support a single
  // resolution, as the index won't actually change, and the slot
  // won't get called.  So, we have to get the current index and
  // if it is 0, call resolution changed manually

  // remove next line when there's a resolution menu
  resolutionChanged ( showItem );
/*
  if ( resMenu->currentIndex() == showItem ) {
    resolutionChanged ( showItem );
  } else {
    resMenu->setCurrentIndex ( showItem );
  }
*/
  // xSize->setText ( QString::number ( config.imageSizeX ));
  // ySize->setText ( QString::number ( config.imageSizeY ));
/*
  xSize->setEnabled ( 0 );
  ySize->setEnabled ( 0 );
  if ( 1 == numItems ) {
    max->setEnabled ( 0 );
  } else {
    max->setEnabled ( 1 );
  }
*/
/*
  if ( state.camera->hasROI()) {
    roi->setEnabled ( 1 );
    max->setEnabled ( 1 );
    roiButton->setEnabled(1);
    if ( !roiXValidator ) {
      roiXValidator = new QIntValidator ( 1, maxX, this );
      roiYValidator = new QIntValidator ( 1, maxY, this );
    } else {
      roiXValidator->setRange ( 1, maxX );
      roiYValidator->setRange ( 1, maxY );
    }
    xSize->setValidator ( roiXValidator );
    ySize->setValidator ( roiYValidator );
    xSize->setEnabled ( 1 );
    ySize->setEnabled ( 1 );
  } else {
    roi->setEnabled ( 0 );
    xSize->setEnabled ( 0 );
    ySize->setEnabled ( 0 );
    roiButton->setEnabled ( 0 );
  }
*/
}


void
ControlsWidget::resolutionChanged ( int index )
{
  // changes to this function may need to be replicated in
  // updateFromConfig()
  if ( !state.camera || ignoreResolutionChanges ||
      !state.camera->isInitialised()) {
    return;
  }
/*
  if ( index == ( resMenu->count() - 1 )) {
    // last item -- max size
    max->setChecked ( true );
  } else {
    // Neither should be checked in this instance
    buttonGroup->setExclusive ( false );
    max->setChecked ( false );
    roi->setChecked ( false );
    buttonGroup->setExclusive ( true );
  }
*/
  config.imageSizeX = XResolutions[ index ];
  config.imageSizeY = YResolutions[ index ];
  // xSize->setText ( QString::number ( config.imageSizeX ));
  // ySize->setText ( QString::number ( config.imageSizeY ));
  doResolutionChange ( 0 );
}


void
ControlsWidget::doResolutionChange ( int roiChanged )
{
  // state.camera->delayFrameRateChanges();

  camera->updateFrameRateSlider();

  if ( roiChanged ) {
    state.camera->setROI ( config.imageSizeX, config.imageSizeY );
  } else {
    state.camera->setResolution ( config.imageSizeX, config.imageSizeY );
  }
  if ( state.viewWidget ) {
    state.viewWidget->updateFrameSize();
  }
  if ( config.profileOption >= 0 ) {
    config.profiles[ config.profileOption ].imageSizeX = config.imageSizeX;
    config.profiles[ config.profileOption ].imageSizeY = config.imageSizeY;
  }
}


void
ControlsWidget::openOutputFiles ( void )
{
  OutputHandler*	out = 0;
  int			format;

  format = state.camera->videoFramePixelFormat();
  if ( OA_ISBAYER ( format )) {
    format = OA_DEMOSAIC_FMT ( format );
  }

  if ( config.saveEachFrame ) {
    switch ( config.fileTypeOption ) {
      case CAPTURE_TIFF:
        out = new OutputTIFF ( config.imageSizeX, config.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
            config.frameFileNameTemplate );
        break;
#ifdef HAVE_LIBCFITSIO
      case CAPTURE_FITS:
        out = new OutputFITS ( config.imageSizeX, config.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
            config.frameFileNameTemplate );
        break;
#endif
    }

    if ( out && ( CAPTURE_TIFF == config.fileTypeOption ||
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
          return;
        }
      }
    }
    if ( !out || out->openOutput()) {
      // FIX ME -- this may cross threads: don't cross the threads!
      QMessageBox::warning ( this, APPLICATION_NAME,
          tr ( "Unable to create file for output" ));
      return;
    }

qWarning() << "have frame save handler";
    frameOutputHandler = out;
  }

  if ( config.saveProcessedImage ) {
    switch ( config.fileTypeOption ) {
      case CAPTURE_TIFF:
        out = new OutputTIFF ( config.imageSizeX, config.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
            config.processedFileNameTemplate );
        break;
#ifdef HAVE_LIBCFITSIO
      case CAPTURE_FITS:
        out = new OutputFITS ( config.imageSizeX, config.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
            config.processedFileNameTemplate );
        break;
#endif
    }

    if ( out && ( CAPTURE_TIFF == config.fileTypeOption ||
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
          return;
        }
      }
    }
    if ( !out || out->openOutput()) {
      // FIX ME -- this may cross threads: don't cross the threads!
      QMessageBox::warning ( this, APPLICATION_NAME,
          tr ( "Unable to create file for output" ));
      return;
    }

    processedImageOutputHandler = out;
  }
}


OutputHandler*
ControlsWidget::getProcessedOutputHandler ( void )
{
  return processedImageOutputHandler;
}


void
ControlsWidget::closeOutputHandlers ( void )
{
  if ( frameOutputHandler ) {
    frameOutputHandler->closeOutput();
    delete frameOutputHandler;
    frameOutputHandler = 0;
  }
  if ( processedImageOutputHandler ) {
    processedImageOutputHandler->closeOutput();
    delete processedImageOutputHandler;
    processedImageOutputHandler = 0;
  }
}
