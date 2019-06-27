/*****************************************************************************
 *
 * processingControls.cc -- class for the processing tab in the settings dialog
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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

#include "captureSettings.h"
#include "fitsSettings.h"

#include "processingControls.h"
#include "state.h"


ProcessingControls::ProcessingControls ( QWidget* parent ) : QWidget ( parent )
{
  blackLevelLabel = new QLabel ( tr ( "Black Level" ));
  whiteLevelLabel = new QLabel ( tr ( "White Level" ));
  brightnessLabel = new QLabel ( tr ( "Brightness" ));
  contrastLabel = new QLabel ( tr ( "Contrast" ));
  saturationLabel = new QLabel ( tr ( "Saturation" ));
  gammaLabel = new QLabel ( tr ( "Gamma" ));
  blackLevelSlider = new QSlider ( Qt::Horizontal, this );
  blackLevelSlider->setFocusPolicy ( Qt::TabFocus );
  blackLevelSlider->setMinimumWidth ( 100 );
  blackLevelSlider->setRange ( 0, 65534 );
  blackLevelSlider->setValue ( 0 );
  whiteLevelSlider = new QSlider ( Qt::Horizontal, this );
  whiteLevelSlider->setFocusPolicy ( Qt::TabFocus );
  whiteLevelSlider->setMinimumWidth ( 100 );
  whiteLevelSlider->setRange ( 1, 65535 );
  whiteLevelSlider->setValue ( 65535 );
  brightnessSlider = new QSlider ( Qt::Horizontal, this );
  brightnessSlider->setFocusPolicy ( Qt::TabFocus );
  brightnessSlider->setMinimumWidth ( 100 );
  brightnessSlider->setRange ( 0, 100 );
  brightnessSlider->setValue ( 0 );
  contrastSlider = new QSlider ( Qt::Horizontal, this );
  contrastSlider->setFocusPolicy ( Qt::TabFocus );
  contrastSlider->setMinimumWidth ( 100 );
  contrastSlider->setRange ( 0, 100 );
  contrastSlider->setValue ( 50 );
  saturationSlider = new QSlider ( Qt::Horizontal, this );
  saturationSlider->setFocusPolicy ( Qt::TabFocus );
  saturationSlider->setMinimumWidth ( 100 );
  saturationSlider->setRange ( 0, 200 );
  saturationSlider->setValue ( 50 );
  gammaSlider = new QSlider ( Qt::Horizontal, this );
  gammaSlider->setFocusPolicy ( Qt::TabFocus );
  gammaSlider->setMinimumWidth ( 100 );
  gammaSlider->setRange ( 1, 255 );
  gammaSlider->setValue ( 100 );

	histogram = new HistogramWidget ( 0, this );

  controlBox = new QVBoxLayout();
  controlBox->addWidget ( blackLevelLabel );
  controlBox->addWidget ( blackLevelSlider );
  controlBox->addWidget ( whiteLevelLabel );
  controlBox->addWidget ( whiteLevelSlider );
  controlBox->addWidget ( brightnessLabel );
  controlBox->addWidget ( brightnessSlider );
  controlBox->addWidget ( contrastLabel );
  controlBox->addWidget ( contrastSlider );
  controlBox->addWidget ( saturationLabel );
  controlBox->addWidget ( saturationSlider );
  controlBox->addWidget ( gammaLabel );
  controlBox->addWidget ( gammaSlider );
  controlBox->addStretch ( 2 );
  controlBox->addWidget ( histogram );

  setLayout ( controlBox );

	connect ( blackLevelSlider, SIGNAL ( valueChanged ( int )), this,
			SLOT ( blackLevelChanged()));
	connect ( whiteLevelSlider, SIGNAL ( valueChanged ( int )), this,
			SLOT ( whiteLevelChanged()));
	connect ( brightnessSlider, SIGNAL ( valueChanged ( int )), this,
			SLOT ( brightnessChanged()));
	connect ( contrastSlider, SIGNAL ( valueChanged ( int )), this,
			SLOT ( contrastChanged()));
	connect ( saturationSlider, SIGNAL ( valueChanged ( int )), this,
			SLOT ( saturationChanged()));
	connect ( gammaSlider, SIGNAL ( valueChanged ( int )), this,
			SLOT ( gammaChanged()));

	connectHistogramSignal();
}


void
ProcessingControls::connectHistogramSignal ( void )
{
	if ( state.viewWidget && !state.histogramProcessingSignalConnected ) {
		connect ( state.viewWidget, SIGNAL( updateHistogram ( void )),
				histogram, SLOT( update ( void )));
		state.histogramProcessingSignalConnected = 1;
	}
}


ProcessingControls::~ProcessingControls()
{
  if ( controlBox ) {
    state.mainWindow->destroyLayout (( QLayout* ) controlBox );
  }
}


void
ProcessingControls::configure ( void )
{
}


void
ProcessingControls::blackLevelChanged ( void )
{
	int		blackValue, whiteValue;

	blackValue = blackLevelSlider->value();
	whiteValue = whiteLevelSlider->value();
	if ( whiteValue <= blackValue ) {
		whiteLevelSlider->setValue ( blackValue + 1 );
	}
	state.viewWidget->setBlackLevel ( blackValue );
}


void
ProcessingControls::whiteLevelChanged ( void )
{
	int		blackValue, whiteValue;

	blackValue = blackLevelSlider->value();
	whiteValue = whiteLevelSlider->value();
	if ( blackValue >= whiteValue ) {
		blackLevelSlider->setValue ( whiteValue - 1 );
	}
	state.viewWidget->setWhiteLevel ( whiteLevelSlider->value());
}


void
ProcessingControls::brightnessChanged ( void )
{
	state.viewWidget->setBrightness ( brightnessSlider->value());
}


void
ProcessingControls::contrastChanged ( void )
{
	state.viewWidget->setContrast ( contrastSlider->value());
}


void
ProcessingControls::saturationChanged ( void )
{
	state.viewWidget->setSaturation ( saturationSlider->value());
}


void
ProcessingControls::gammaChanged ( void )
{
	state.viewWidget->setGamma ( gammaSlider->value());
}
