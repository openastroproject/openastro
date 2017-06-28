/*****************************************************************************
 *
 * captureSettings.cc -- class for the capture tab in the settings dialog
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

#include "captureSettings.h"
#include "state.h"

CaptureSettings::CaptureSettings ( QWidget* parent ) : QWidget ( parent )
{
  indexResetButton = new QPushButton ( tr ( "Reset capture counter" ), this );
  winAVIBox = new QCheckBox (
    tr ( "Use Windows-compatible format for AVI files" ), this );
  winAVIBox->setChecked ( config.windowsCompatibleAVI );

  hLayout = new QHBoxLayout ( this );
  vLayout = new QVBoxLayout();
  vLayout->addWidget ( indexResetButton );
  vLayout->addWidget ( winAVIBox );
  vLayout->addStretch ( 1 );
  hLayout->addLayout ( vLayout );
  hLayout->addStretch ( 1 );

  setLayout ( hLayout );
  connect ( indexResetButton, SIGNAL ( clicked()), this,
      SLOT ( resetIndex()));
  connect ( winAVIBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
}


CaptureSettings::~CaptureSettings()
{
  state.mainWindow->destroyLayout (( QLayout* ) vLayout );
}


void
CaptureSettings::storeSettings ( void )
{
  config.windowsCompatibleAVI = winAVIBox->isChecked() ? 1 : 0;
}


void
CaptureSettings::resetIndex ( void )
{
  // FIX ME -- this might not be good in the middle of a capture run
  state.captureIndex = 0;
}
