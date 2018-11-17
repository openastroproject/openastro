/*****************************************************************************
 *
 * processingControls.cc -- class for the processing tab in the settings dialog
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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
  blackLevelSlider = new QSlider ( Qt::Horizontal, this );
  blackLevelSlider->setFocusPolicy ( Qt::TabFocus );
  blackLevelSlider->setMinimumWidth ( 100 );
  blackLevelSlider->setRange ( 0, 65535 );
  blackLevelSlider->setValue ( 0 );
  whiteLevelSlider = new QSlider ( Qt::Horizontal, this );
  whiteLevelSlider->setFocusPolicy ( Qt::TabFocus );
  whiteLevelSlider->setMinimumWidth ( 100 );
  whiteLevelSlider->setRange ( 0, 65535 );
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
  contrastSlider->setValue ( 0 );

  controlBox = new QVBoxLayout();
  controlBox->addWidget ( blackLevelLabel );
  controlBox->addWidget ( blackLevelSlider );
  controlBox->addWidget ( whiteLevelLabel );
  controlBox->addWidget ( whiteLevelSlider );
  controlBox->addWidget ( brightnessLabel );
  controlBox->addWidget ( brightnessSlider );
  controlBox->addWidget ( contrastLabel );
  controlBox->addWidget ( contrastSlider );
  controlBox->addStretch ( 1 );

  setLayout ( controlBox );
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
