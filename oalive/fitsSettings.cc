/*****************************************************************************
 *
 * fitsSettings.cc -- class for the FITS data tab in the settings dialog
 *
 * Copyright 2015,2016,2017 James Fidell (james@openastroproject.org)
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


#include "captureSettings.h"
#include "outputHandler.h"
#include "fitsSettings.h"
#ifdef HAVE_LIBCFITSIO
#include "outputFITS.h"
#endif

#include "state.h"


FITSSettings::FITSSettings ( QWidget* parent ) : QWidget ( parent )
{
  // FIX ME -- these labels should come from the FITS data in liboavideo?

  observerLabel = new QLabel ( tr ( "Observer" ), this );
  siteLatitudeLabel = new QLabel ( tr ( "Site latitude" ), this );
  siteLongitudeLabel = new QLabel ( tr ( "Site longitude" ), this );
  commentLabel = new QLabel ( tr ( "Comments" ), this );

  telescopeLabel = new QLabel ( tr ( "Telescope" ), this );
  focalLengthLabel = new QLabel ( tr ( "Focal Length (mm)" ), this );
  apertureDiaLabel = new QLabel ( tr ( "Aperture Diameter (mm)" ), this );
  apertureAreaLabel = new QLabel ( tr ( "Aperture Area (mm)" ), this );

  instrumentLabel = new QLabel ( tr ( "Instrument" ), this );
  pixelSizeXLabel = new QLabel ( tr ( "Pixel width (um)" ), this );
  pixelSizeYLabel = new QLabel ( tr ( "Pixel height (um)" ), this );
  subframeOriginXLabel = new QLabel ( tr ( "X subframe origin" ), this );
  subframeOriginYLabel = new QLabel ( tr ( "Y subframe origin" ), this );

  objectLabel = new QLabel ( tr ( "Object" ), this );
  filterLabel = new QLabel ( tr ( "Filter" ), this );

  observerInput = new QLineEdit ( this );
  observerInput->setMaxLength ( FLEN_VALUE );
  observerInput->setText ( config.fitsObserver );

  siteLatitudeInput = new QLineEdit ( this );
  siteLatitudeInput->setMaxLength ( FLEN_VALUE );
  siteLatitudeInput->setText ( config.fitsSiteLatitude );

  siteLongitudeInput = new QLineEdit ( this );
  siteLongitudeInput->setMaxLength ( FLEN_VALUE );
  siteLongitudeInput->setText ( config.fitsSiteLongitude );

  commentInput = new QLineEdit ( this );
  commentInput->setMaxLength ( FLEN_VALUE );
  commentInput->setText ( config.fitsComment );

  telescopeInput = new QLineEdit ( this );
  telescopeInput->setMaxLength ( FLEN_VALUE );
  telescopeInput->setText ( config.fitsTelescope );

  focalLengthInput = new QLineEdit ( this );
  focalLengthInput->setMaxLength ( FLEN_VALUE );
  focalLengthInput->setText ( config.fitsFocalLength );

  apertureDiaInput = new QLineEdit ( this );
  apertureDiaInput->setMaxLength ( FLEN_VALUE );
  apertureDiaInput->setText ( config.fitsApertureDia );

  apertureAreaInput = new QLineEdit ( this );
  apertureAreaInput->setMaxLength ( FLEN_VALUE );
  apertureAreaInput->setText ( config.fitsApertureArea );

  instrumentInput = new QLineEdit ( this );
  instrumentInput->setMaxLength ( FLEN_VALUE );
  instrumentInput->setText ( config.fitsInstrument );

  pixelSizeXInput = new QLineEdit ( this );
  pixelSizeXInput->setMaxLength ( FLEN_VALUE );
  pixelSizeXInput->setText ( config.fitsPixelSizeX );

  pixelSizeYInput = new QLineEdit ( this );
  pixelSizeYInput->setMaxLength ( FLEN_VALUE );
  pixelSizeYInput->setText ( config.fitsPixelSizeY );

  subframeOriginXInput = new QLineEdit ( this );
  subframeOriginXInput->setMaxLength ( FLEN_VALUE );
  subframeOriginXInput->setText ( config.fitsSubframeOriginX );

  subframeOriginYInput = new QLineEdit ( this );
  subframeOriginYInput->setMaxLength ( FLEN_VALUE );
  subframeOriginYInput->setText ( config.fitsSubframeOriginY );

  objectInput = new QLineEdit ( this );
  objectInput->setMaxLength ( FLEN_VALUE );
  objectInput->setText ( config.fitsObject );

  filterInput = new QLineEdit ( this );
  filterInput->setMaxLength ( FLEN_VALUE );
  filterInput->setText ( config.fitsFilter );


  grid = new QGridLayout;

  grid->setRowStretch ( 0, 1 );

  grid->addWidget ( observerLabel, 1, 0 );
  grid->addWidget ( observerInput, 1, 1 );
  grid->addWidget ( siteLatitudeLabel, 2, 0 );
  grid->addWidget ( siteLatitudeInput, 2, 1 );
  grid->addWidget ( siteLongitudeLabel, 3, 0 );
  grid->addWidget ( siteLongitudeInput, 3, 1 );
  grid->addWidget ( commentLabel, 4, 0 );
  grid->addWidget ( commentInput, 4, 1 );

  grid->addWidget ( instrumentLabel, 6, 0 );
  grid->addWidget ( instrumentInput, 6, 1 );
  grid->addWidget ( pixelSizeXLabel, 7, 0 );
  grid->addWidget ( pixelSizeXInput, 7, 1 );
  grid->addWidget ( pixelSizeYLabel, 8, 0 );
  grid->addWidget ( pixelSizeYInput, 8, 1 );
  grid->addWidget ( subframeOriginXLabel, 9, 0 );
  grid->addWidget ( subframeOriginXInput, 9, 1 );
  grid->addWidget ( subframeOriginYLabel, 10, 0 );
  grid->addWidget ( subframeOriginYInput, 10, 1 );

  grid->setRowStretch ( 5, 1 );

  grid->addWidget ( telescopeLabel, 1, 3 );
  grid->addWidget ( telescopeInput, 1, 4 );
  grid->addWidget ( focalLengthLabel, 2, 3 );
  grid->addWidget ( focalLengthInput, 2, 4 );
  grid->addWidget ( apertureDiaLabel, 3, 3 );
  grid->addWidget ( apertureDiaInput, 3, 4 );
  grid->addWidget ( apertureAreaLabel, 4, 3 );
  grid->addWidget ( apertureAreaInput, 4, 4 );

  grid->addWidget ( objectLabel, 6, 3 );
  grid->addWidget ( objectInput, 6, 4 );
  grid->addWidget ( filterLabel, 7, 3 );
  grid->addWidget ( filterInput, 7, 4 );

  grid->setColumnStretch ( 2, 1 );
  grid->setColumnStretch ( 5, 1 );
  grid->setRowStretch ( 11, 1 );

  setLayout ( grid );

  connect ( observerInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( instrumentInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( objectInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( commentInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( telescopeInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( focalLengthInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( apertureDiaInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( apertureAreaInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( pixelSizeXInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( pixelSizeYInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( subframeOriginXInput, SIGNAL ( textEdited ( const QString& )),
      parent, SLOT ( dataChanged()));
  connect ( subframeOriginYInput, SIGNAL ( textEdited ( const QString& )),
      parent, SLOT ( dataChanged()));
  connect ( siteLatitudeInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( siteLongitudeInput, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( filterInput, SIGNAL ( textEdited ( const QString& )), parent,
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
  config.fitsInstrument = instrumentInput->text();
  config.fitsObject = objectInput->text();
  config.fitsComment = commentInput->text();
  config.fitsTelescope = telescopeInput->text();
  config.fitsFocalLength = focalLengthInput->text();
  config.fitsApertureDia = apertureDiaInput->text();
  config.fitsApertureArea = apertureAreaInput->text();
  config.fitsPixelSizeX = pixelSizeXInput->text();
  config.fitsPixelSizeY = pixelSizeYInput->text();
  config.fitsSubframeOriginX = subframeOriginXInput->text();
  config.fitsSubframeOriginY = subframeOriginYInput->text();
  config.fitsSiteLatitude = siteLatitudeInput->text();
  config.fitsSiteLongitude = siteLongitudeInput->text();
  config.fitsFilter = filterInput->text();
}
