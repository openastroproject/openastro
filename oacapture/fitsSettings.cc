/*****************************************************************************
 *
 * fitsSettings.cc -- class for the FITS data tab in the settings dialog
 *
 * Copyright 2015, 2016 James Fidell (james@openastroproject.org)
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

extern "C" {
#ifdef HAVE_FITSIO_H 
#include "fitsio.h"
#else
#ifdef HAVE_CFITSIO_FITSIO_H
#include "cfitsio/fitsio.h" 
#endif
#endif
}


#include "fitsSettings.h"
#include "state.h"
#ifdef HAVE_LIBCFITSIO
#include "outputFITS.h"
#endif


FITSSettings::FITSSettings ( QWidget* parent ) : QWidget ( parent )
{
  observerLabel = new QLabel ( tr ( "Observer" ), this );
  telescopeLabel = new QLabel ( tr ( "Telescope" ), this );
  instrumentLabel = new QLabel ( tr ( "Instrument" ), this );
  objectLabel = new QLabel ( tr ( "Object" ), this );
  commentLabel = new QLabel ( tr ( "Comments" ), this );

  observerInput = new QLineEdit ( this );
  observerInput->setMaxLength ( FLEN_VALUE );
  observerInput->setText ( config.fitsObserver );

  telescopeInput = new QLineEdit ( this );
  telescopeInput->setMaxLength ( FLEN_VALUE );
  telescopeInput->setText ( config.fitsTelescope );

  instrumentInput = new QLineEdit ( this );
  instrumentInput->setMaxLength ( FLEN_VALUE );
  instrumentInput->setText ( config.fitsInstrument );

  objectInput = new QLineEdit ( this );
  objectInput->setMaxLength ( FLEN_VALUE );
  objectInput->setText ( config.fitsObject );

  commentInput = new QLineEdit ( this );
  commentInput->setMaxLength ( FLEN_VALUE );
  commentInput->setText ( config.fitsComment );

  grid = new QGridLayout;
  grid->addWidget ( observerLabel, 0, 0 );
  grid->addWidget ( observerInput, 0, 1 );
  grid->addWidget ( telescopeLabel, 1, 0 );
  grid->addWidget ( telescopeInput, 1, 1 );
  grid->addWidget ( instrumentLabel, 2, 0 );
  grid->addWidget ( instrumentInput, 2, 1 );
  grid->addWidget ( objectLabel, 3, 0 );
  grid->addWidget ( objectInput, 3, 1 );
  grid->addWidget ( commentLabel, 4, 0 );
  grid->addWidget ( commentInput, 4, 1 );
  grid->setColumnStretch ( 2, 1 );
  grid->setRowStretch ( 5, 1 );

  setLayout ( grid );

  connect ( observerInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( telescopeInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( instrumentInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( objectInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( commentInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
}


FITSSettings::~FITSSettings()
{
  if ( grid ) {
    state.mainWindow->destroyLayout (( QLayout* ) grid );
  }
}


void
FITSSettings::storeSettings ( void )
{
  config.fitsObserver = observerInput->text();
  config.fitsTelescope = telescopeInput->text();
  config.fitsInstrument = instrumentInput->text();
  config.fitsObject = objectInput->text();
  config.fitsComment = commentInput->text();
}
