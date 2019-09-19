/*****************************************************************************
 *
 * controlsWidget.cc -- the tab block for the controls
 *
 * Copyright 2015,2017,2018,2019
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

#include "fitsSettings.h"
#include "trampoline.h"
#include "outputHandler.h"
#include "outputTIFF.h"
#include "outputPNG.h"
#include "outputFITS.h"
#include "commonState.h"
#include "commonConfig.h"

#include "version.h"
#include "configuration.h"
#include "state.h"
#include "controlsWidget.h"


ControlsWidget::ControlsWidget ( QWidget* parent __attribute((unused)))
{
  mainBox = new QVBoxLayout;
  topButtonBox = new QHBoxLayout;
  bottomButtonBox = new QHBoxLayout;

  tabSet = new QTabWidget ( this );
  camera = new CameraControls ( this );
  stacking = new StackingControls ( this );
  save = new SaveControls ( this );
  processing = new ProcessingControls ( this );

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
  tabSet->addTab ( processing, tr ( "Display" ));
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
  state.processingControls = processing;
	state.cameraRunning = 0;
}


ControlsWidget::~ControlsWidget()
{
  state.mainWindow->destroyLayout (( QLayout* ) mainBox );
  state.cameraControls = 0;
  state.processingControls = 0;
}


void
ControlsWidget::connectSignals ( void )
{
  connect ( state.viewWidget, SIGNAL( startNextExposure ( void )),
      this, SLOT ( startNextExposure ( void )));
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
  processing->configure();
	if ( !commonState.camera->frameSizeUnknown()) {
		configureResolution();
	}
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
	if ( commonState.camera->isSingleShot()) {
		commonState.camera->startExposure ( 0, &ViewWidget::addImage,
				&commonState );
	} else {
		commonState.camera->startStreaming ( &ViewWidget::addImage, &commonState );
	}
  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 1 );
	state.cameraRunning = 1;
}


void
ControlsWidget::stopCapture ( void )
{
  commonState.camera->stop();
	state.cameraRunning = 0;
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
  commonState.camera->stop();
	state.cameraRunning = 0;
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
	if ( commonState.camera->isSingleShot()) {
		commonState.camera->startExposure ( 0, &ViewWidget::addImage,
				&commonState );
	} else {
		commonState.camera->startStreaming ( &ViewWidget::addImage, &commonState );
	}
	state.cameraRunning = 1;
  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 1 );
}


void
ControlsWidget::startNextExposure ( void )
{
	if ( state.cameraRunning ) {
		if ( commonState.camera->isSingleShot()) {
			commonState.camera->startExposure ( 0, &ViewWidget::addImage,
					&commonState );
		}
	}
}


void
ControlsWidget::configureResolution ( void )
{
  // FIX ME -- add controls for screen resolution
  int maxX, maxY;
  maxX = maxY = 0;

  // This includes a particularly ugly way to sort the resolutions using
  // a QMap and some further QMap abuse to be able to find the X and Y
  // resolutions should we not have a setting that matches the config and
  // happen to choose the max size option
  const FRAMESIZES* sizeList = commonState.camera->frameSizes();
  QMap<int,QString> sortMap;
  QMap<int,int> xRes, yRes;
  QString showItemStr;
  int firstKey = -1, lastKey = -1;
  unsigned int i;

  for ( i = 0; i < sizeList->numSizes; i++ ) {
    int numPixels = sizeList->sizes[i].x * sizeList->sizes[i].y;
    QString resStr = QString::number ( sizeList->sizes[i].x ) + "x" +
        QString::number ( sizeList->sizes[i].y );
    if ( sizeList->sizes[i].x == commonConfig.imageSizeX &&
        sizeList->sizes[i].y == commonConfig.imageSizeY ) {
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
    commonConfig.imageSizeX = showXRes;
    commonConfig.imageSizeY = showYRes;
  } else {
    // FIX ME -- put back when there is a resolutions menu
    showItem = numItems - 1; // max->isChecked() ? numItems - 1: 0;
    if ( showItem ) {
      commonConfig.imageSizeX = xRes[ lastKey ];
      commonConfig.imageSizeY = yRes[ lastKey ];
    } else {
      commonConfig.imageSizeX = xRes[ firstKey ];
      commonConfig.imageSizeY = yRes[ firstKey ];
    }
  }
  maxX = xRes[ lastKey ];
  maxY = yRes[ lastKey ];
	commonState.sensorSizeX = maxX;
	commonState.sensorSizeY = maxY;

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
  // xSize->setText ( QString::number ( commonConfig.imageSizeX ));
  // ySize->setText ( QString::number ( commonConfig.imageSizeY ));
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
  if ( commonState.camera->hasROI()) {
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
  if ( !commonState.camera || ignoreResolutionChanges ||
      !commonState.camera->isInitialised()) {
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
  commonConfig.imageSizeX = XResolutions[ index ];
  commonConfig.imageSizeY = YResolutions[ index ];
  // xSize->setText ( QString::number ( commonConfig.imageSizeX ));
  // ySize->setText ( QString::number ( commonConfig.imageSizeY ));
  doResolutionChange ( 0 );
}


void
ControlsWidget::doResolutionChange ( int roiChanged )
{
  // commonState.camera->delayFrameRateChanges();

  camera->updateFrameRateSlider();

  if ( roiChanged ) {
    commonState.camera->setROI ( commonConfig.imageSizeX,
				commonConfig.imageSizeY );
  } else {
		if ( !commonState.camera->hasUnknownFrameSize()) {
			commonState.camera->setResolution ( commonConfig.imageSizeX,
					commonConfig.imageSizeY );
		}
  }
  if ( state.viewWidget ) {
    state.viewWidget->updateFrameSize();
  }
  if ( commonConfig.profileOption >= 0 ) {
    profileConf.profiles[ commonConfig.profileOption ].imageSizeX =
				commonConfig.imageSizeX;
    profileConf.profiles[ commonConfig.profileOption ].imageSizeY =
				commonConfig.imageSizeY;
  }
}


void
ControlsWidget::openOutputFiles ( void )
{
  OutputHandler*	out = 0;
  int			format;

  format = commonState.camera->videoFramePixelFormat();
  if ( oaFrameFormats[ format ].rawColour ) {
    format = OA_DEMOSAIC_FMT ( format );
  }

  if ( config.saveEachFrame ) {
    switch ( commonConfig.fileTypeOption ) {
      case CAPTURE_TIFF:
        out = new OutputTIFF ( commonConfig.imageSizeX, commonConfig.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
						APPLICATION_NAME, VERSION_STR, config.frameFileNameTemplate,
						&trampolines );
        break;
      case CAPTURE_PNG:
        out = new OutputPNG ( commonConfig.imageSizeX, commonConfig.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
						APPLICATION_NAME, VERSION_STR, config.frameFileNameTemplate,
						&trampolines );
        break;
#if HAVE_LIBCFITSIO
      case CAPTURE_FITS:
        out = new OutputFITS ( commonConfig.imageSizeX, commonConfig.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
						APPLICATION_NAME, VERSION_STR, config.frameFileNameTemplate,
						&trampolines );
        break;
#endif
    }

    if ( out && ( CAPTURE_TIFF == commonConfig.fileTypeOption ||
        CAPTURE_FITS == commonConfig.fileTypeOption ||
				CAPTURE_PNG == commonConfig.fileTypeOption )) {
      if ( !out->outputWritable()) {
				// Have to do it this way rather than calling direct to ensure
				// thread-safety
				QMetaObject::invokeMethod ( state.mainWindow, "outputUnwritable",
						Qt::DirectConnection );
        delete out;
        out = 0;
        return;
      }
    } else {
      if ( out && out->outputExists()) {
        if ( out->outputWritable()) {
					int result;
					// Have to do it this way rather than calling direct to ensure
					// thread-safety
					QMetaObject::invokeMethod ( state.mainWindow, "outputExists",
							Qt::DirectConnection, Q_RETURN_ARG( int, result ));
					if ( result == QMessageBox::No ) {
            delete out;
            out = 0;
            return;
          }
        } else {
					// Have to do it this way rather than calling direct to ensure
					// thread-safety
					QMetaObject::invokeMethod ( state.mainWindow,
							"outputExistsUnwritable", Qt::DirectConnection );
          delete out;
          return;
        }
      }
    }
    if ( !out || out->openOutput()) {
			QMetaObject::invokeMethod ( state.mainWindow, "createFileFailed",
					Qt::DirectConnection );
      return;
    }

    frameOutputHandler = out;
  }

  if ( config.saveProcessedImage ) {
    switch ( commonConfig.fileTypeOption ) {
      case CAPTURE_TIFF:
        out = new OutputTIFF ( commonConfig.imageSizeX, commonConfig.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
						APPLICATION_NAME, VERSION_STR, config.processedFileNameTemplate,
            &trampolines );
        break;
      case CAPTURE_PNG:
        out = new OutputPNG ( commonConfig.imageSizeX, commonConfig.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
						APPLICATION_NAME, VERSION_STR, config.processedFileNameTemplate,
            &trampolines );
        break;
#if HAVE_LIBCFITSIO
      case CAPTURE_FITS:
        out = new OutputFITS ( commonConfig.imageSizeX, commonConfig.imageSizeY,
            state.cameraControls->getFPSNumerator(),
            state.cameraControls->getFPSDenominator(), format,
						APPLICATION_NAME, VERSION_STR, config.processedFileNameTemplate,
            &trampolines );
        break;
#endif
    }

    if ( out && ( CAPTURE_TIFF == commonConfig.fileTypeOption ||
        CAPTURE_FITS == commonConfig.fileTypeOption ||
				CAPTURE_PNG == commonConfig.fileTypeOption )) {
      if ( !out->outputWritable()) {
				QMetaObject::invokeMethod ( state.mainWindow, "outputUnwritable",
						Qt::DirectConnection );
        delete out;
        out = 0;
        return;
      }
    } else {
      if ( out && out->outputExists()) {
        if ( out->outputWritable()) {
					int result;
					// Have to do it this way rather than calling direct to ensure
					// thread-safety
					QMetaObject::invokeMethod ( state.mainWindow, "outputExists",
							Qt::DirectConnection, Q_RETURN_ARG( int, result ));
					if ( result == QMessageBox::No ) {
            delete out;
            out = 0;
            return;
          }
        } else {
					// Have to do it this way rather than calling direct to ensure
					// thread-safety
					QMetaObject::invokeMethod ( state.mainWindow,
							"outputExistsUnwritable", Qt::DirectConnection );
          delete out;
          return;
        }
      }
    }
    if ( !out || out->openOutput()) {
			QMetaObject::invokeMethod ( state.mainWindow, "createFileFailed",
					Qt::DirectConnection );
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


int
ControlsWidget::getZoomFactor ( void )
{
	return processing->zoom->getZoomFactor();
}
