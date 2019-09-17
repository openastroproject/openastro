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
  blackLevelSlider->setTracking ( 0 );
  blackLevelValue = new QLabel ( "0" );
  whiteLevelSlider = new QSlider ( Qt::Horizontal, this );
  whiteLevelSlider->setFocusPolicy ( Qt::TabFocus );
  whiteLevelSlider->setMinimumWidth ( 100 );
  whiteLevelSlider->setRange ( 1, 65535 );
  whiteLevelSlider->setValue ( 65535 );
  whiteLevelSlider->setTracking ( 0 );
  whiteLevelValue = new QLabel ( "65535" );
  brightnessSlider = new QSlider ( Qt::Horizontal, this );
  brightnessSlider->setFocusPolicy ( Qt::TabFocus );
  brightnessSlider->setMinimumWidth ( 100 );
  brightnessSlider->setRange ( 0, 100 );
  brightnessSlider->setValue ( 0 );
  brightnessSlider->setTracking ( 0 );
  brightnessValue = new QLabel ( "0" );
  contrastSlider = new QSlider ( Qt::Horizontal, this );
  contrastSlider->setFocusPolicy ( Qt::TabFocus );
  contrastSlider->setMinimumWidth ( 100 );
  contrastSlider->setRange ( 0, 100 );
  contrastSlider->setValue ( 50 );
  contrastSlider->setTracking ( 0 );
  contrastValue = new QLabel ( "50" );
  saturationSlider = new QSlider ( Qt::Horizontal, this );
  saturationSlider->setFocusPolicy ( Qt::TabFocus );
  saturationSlider->setMinimumWidth ( 100 );
  saturationSlider->setRange ( 0, 200 );
  saturationSlider->setValue ( 50 );
  saturationSlider->setTracking ( 0 );
  saturationValue = new QLabel ( "50" );
  gammaSlider = new QSlider ( Qt::Horizontal, this );
  gammaSlider->setFocusPolicy ( Qt::TabFocus );
  gammaSlider->setMinimumWidth ( 100 );
  gammaSlider->setRange ( 1, 255 );
  gammaSlider->setValue ( 100 );
  gammaSlider->setTracking ( 0 );
  gammaValue = new QLabel ( "100" );

	zoom = new ZoomWidget ( this );
	histogram = new HistogramWidget ( 0, this );

  controlBox = new QVBoxLayout();
	blackLevelBox = new QHBoxLayout();
  blackLevelBox->addWidget ( blackLevelLabel );
  blackLevelBox->addStretch ( 1 );
  blackLevelBox->addWidget ( blackLevelValue );
  controlBox->addLayout ( blackLevelBox );
  controlBox->addWidget ( blackLevelSlider );
	whiteLevelBox = new QHBoxLayout();
  whiteLevelBox->addWidget ( whiteLevelLabel );
  whiteLevelBox->addStretch ( 1 );
  whiteLevelBox->addWidget ( whiteLevelValue );
  controlBox->addLayout ( whiteLevelBox );
  controlBox->addWidget ( whiteLevelSlider );
	brightnessBox = new QHBoxLayout();
  brightnessBox->addWidget ( brightnessLabel );
  brightnessBox->addStretch ( 1 );
  brightnessBox->addWidget ( brightnessValue );
  controlBox->addLayout ( brightnessBox );
  controlBox->addWidget ( brightnessSlider );
	contrastBox = new QHBoxLayout();
  contrastBox->addWidget ( contrastLabel );
  contrastBox->addStretch ( 1 );
  contrastBox->addWidget ( contrastValue );
  controlBox->addLayout ( contrastBox );
  controlBox->addWidget ( contrastSlider );
	saturationBox = new QHBoxLayout();
  saturationBox->addWidget ( saturationLabel );
  saturationBox->addStretch ( 1 );
  saturationBox->addWidget ( saturationValue );
  controlBox->addLayout ( saturationBox );
  controlBox->addWidget ( saturationSlider );
	gammaBox = new QHBoxLayout();
  gammaBox->addWidget ( gammaLabel );
  gammaBox->addStretch ( 1 );
  gammaBox->addWidget ( gammaValue );
  controlBox->addLayout ( gammaBox );
  controlBox->addWidget ( gammaSlider );
  controlBox->addWidget ( zoom );
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
	int			blackValue, whiteValue;
	QString	label;

	blackValue = blackLevelSlider->value();
	whiteValue = whiteLevelSlider->value();
	if ( whiteValue <= blackValue ) {
		whiteLevelSlider->setValue ( blackValue + 1 );
		label.setNum ( blackValue + 1 );
		whiteLevelValue->setText ( label );
	}
	label.setNum ( blackValue );
	blackLevelValue->setText ( label );
	state.viewWidget->setBlackLevel ( blackValue );
	emit redrawImage();
}


void
ProcessingControls::whiteLevelChanged ( void )
{
	int		blackValue, whiteValue;
	QString	label;

	blackValue = blackLevelSlider->value();
	whiteValue = whiteLevelSlider->value();
	if ( blackValue >= whiteValue ) {
		blackLevelSlider->setValue ( whiteValue - 1 );
		label.setNum ( whiteValue - 1 );
		blackLevelValue->setText ( label );
	}
	label.setNum ( whiteValue );
	whiteLevelValue->setText ( label );
	state.viewWidget->setWhiteLevel ( whiteValue );
	emit redrawImage();
}


void
ProcessingControls::brightnessChanged ( void )
{
	int		value = brightnessSlider->value();
	QString	label;

	label.setNum ( value );
	brightnessValue->setText ( label );
	state.viewWidget->setBrightness ( value );
	emit redrawImage();
}


void
ProcessingControls::contrastChanged ( void )
{
	int		value = contrastSlider->value();
	QString	label;

	label.setNum ( value );
	contrastValue->setText ( label );
	state.viewWidget->setContrast ( value );
	emit redrawImage();
}


void
ProcessingControls::saturationChanged ( void )
{
	int		value = saturationSlider->value();
	QString	label;

	label.setNum ( value );
	saturationValue->setText ( label );
	state.viewWidget->setSaturation ( value );
	emit redrawImage();
}


void
ProcessingControls::gammaChanged ( void )
{
	int		value = gammaSlider->value();
	QString	label;

	label.setNum ( value );
	gammaValue->setText ( label );
	state.viewWidget->setGamma ( value );
	emit redrawImage();
}
