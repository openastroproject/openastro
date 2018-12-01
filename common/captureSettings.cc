/*****************************************************************************
 *
 * captureSettings.cc -- class for the capture tab in the settings dialog
 *
 * Copyright 2013,2014,2018
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

#include "captureSettings.h"
#include "commonState.h"

// This is global.  All applications using this code share it.

captureConfig captureConf;


CaptureSettings::CaptureSettings ( QWidget* parent, int formats,
		trampolineFuncs* redirs ) :
		QWidget ( parent ), videoFormats ( formats ), trampolines ( redirs )
{
  indexResetButton = new QPushButton ( tr ( "Reset capture counter" ), this );

	if ( videoFormats ) {
    winAVIBox = new QCheckBox (
        tr ( "Use Windows-compatible format for AVI files "
        "(8-bit mono/raw colour)" ), this );
    winAVIBox->setChecked ( captureConf.windowsCompatibleAVI );

    utVideoBox = new QCheckBox (
        tr ( "Use UtVideo lossless compression for AVI files where possible" ),
        this );
    utVideoBox->setChecked ( captureConf.useUtVideo );
	}

  indexSizeLabel = new QLabel ( tr ( "Filename capture index size" ));
  indexSizeSpinbox = new QSpinBox ( this );
  indexSizeSpinbox->setMinimum ( 3 );
  indexSizeSpinbox->setMaximum ( 10 );
  indexSizeSpinbox->setValue ( captureConf.indexDigits );

  hLayout = new QHBoxLayout ( this );
  spinboxLayout = new QHBoxLayout();
  vLayout = new QVBoxLayout();
  vLayout->addWidget ( indexResetButton );
	if ( videoFormats ) {
    vLayout->addWidget ( winAVIBox );
    vLayout->addWidget ( utVideoBox );
	}
  spinboxLayout->addWidget ( indexSizeLabel );
  spinboxLayout->addWidget ( indexSizeSpinbox );
  vLayout->addLayout ( spinboxLayout );

  vLayout->addStretch ( 1 );
  hLayout->addLayout ( vLayout );
  hLayout->addStretch ( 1 );

  setLayout ( hLayout );
  connect ( indexResetButton, SIGNAL ( clicked()), this,
      SLOT ( resetIndex()));
	if ( videoFormats ) {
    connect ( winAVIBox, SIGNAL ( stateChanged ( int )), parent,
        SLOT ( dataChanged()));
    connect ( utVideoBox, SIGNAL ( stateChanged ( int )), parent,
        SLOT ( dataChanged()));
	}
  connect ( indexSizeSpinbox, SIGNAL ( valueChanged ( int )), parent,
      SLOT ( dataChanged()));
}


CaptureSettings::~CaptureSettings()
{
  trampolines->destroyLayout (( QLayout* ) vLayout );
}


void
CaptureSettings::storeSettings ( void )
{
	if ( videoFormats ) {
    captureConf.windowsCompatibleAVI = winAVIBox->isChecked() ? 1 : 0;
    captureConf.useUtVideo = utVideoBox->isChecked() ? 1 : 0;
	}
  captureConf.indexDigits = indexSizeSpinbox->value();
}


void
CaptureSettings::resetIndex ( void )
{
  // FIX ME -- this might not be good in the middle of a capture run
	commonState.captureIndex = 0;
}
