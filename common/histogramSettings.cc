/*****************************************************************************
 *
 * histogramSettings.cc -- class for the histogram settings in the settings UI
 *
 * Copyright 2013,2014,2017,2018 James Fidell (james@openastroproject.org)
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

#include "histogramSettings.h"


HistogramSettings::HistogramSettings ( QWidget* parent,
		histogramConfig* hConf, trampolineFuncs* redirs ) : QWidget ( parent ),
		pHistConf ( hConf ), trampolines ( redirs )
{
  rawRGBBox = new QCheckBox ( tr ( "Show raw colour as RGB histogram" ), this );
  rawRGBBox->setChecked ( pHistConf->rawRGBHistogram );
  splitBox = new QCheckBox ( tr ( "Split RGB histogram" ), this );
  splitBox->setChecked ( pHistConf->splitHistogram );
  onTopBox = new QCheckBox ( tr ( "Keep histogram window on top" ), this );
  onTopBox->setChecked ( pHistConf->histogramOnTop );
  box = new QVBoxLayout ( this );
  box->addWidget ( rawRGBBox );
  box->addWidget ( splitBox );
  box->addWidget ( onTopBox );
  box->addStretch ( 1 );
  setLayout ( box );
  connect ( splitBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
  connect ( onTopBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
  connect ( rawRGBBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
}


HistogramSettings::~HistogramSettings()
{
  trampolines->destroyLayout (( QLayout* ) box );
}


void
HistogramSettings::storeSettings ( void )
{
  pHistConf->splitHistogram = splitBox->isChecked() ? 1 : 0;
  trampolines->updateHistogramLayout();
  pHistConf->histogramOnTop = onTopBox->isChecked() ? 1 : 0;
  pHistConf->rawRGBHistogram = rawRGBBox->isChecked() ? 1 : 0;
}
