/*****************************************************************************
 *
 * demosaicSettings.cc -- class for the demosaic settings in the settings UI
 *
 * Copyright 2013,2014,2015,2016,2018
 *     James Fidell (james@openastroproject.org)
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
#include <openastro/demosaic.h>
#include <openastro/video/formats.h>
}

#include "commonConfig.h"
#include "commonState.h"
#include "trampoline.h"
#include "captureSettings.h"
#include "demosaicSettings.h"
#include "fitsSettings.h"

// This is global.  All applications using this code share it.

demosaicConfig demosaicConf;

DemosaicSettings::DemosaicSettings ( QWidget* parent, int demosaic,
		trampolineFuncs* redirs ) : QWidget ( parent ),
		trampolines ( redirs ), demosaicOpts ( demosaic )
{
	if ( demosaicOpts ) {
    demosaicLabel = new QLabel ( tr ( "When demosaic is enabled:" ));
    previewBox = new QCheckBox ( tr ( "Demosaic preview image" ), this );
    previewBox->setChecked ( demosaicConf.demosaicPreview );
    outputBox = new QCheckBox ( tr ( "Demosaic output data" ), this );
    outputBox->setChecked ( demosaicConf.demosaicOutput );
	}

  cfaLabel = new QLabel ( tr ( "Bayer format" ));
  cfaButtons = new QButtonGroup ( this );
  rggbButton = new QRadioButton ( tr ( "RGGB" ));
  bggrButton = new QRadioButton ( tr ( "BGGR" ));
  grbgButton = new QRadioButton ( tr ( "GRBG" ));
  gbrgButton = new QRadioButton ( tr ( "GBRG" ));
  cmygButton = new QRadioButton ( tr ( "CMYG" ));
  mcgyButton = new QRadioButton ( tr ( "MCGY" ));
  ygcmButton = new QRadioButton ( tr ( "YGCM" ));
  gymcButton = new QRadioButton ( tr ( "GYMC" ));
  autoButton = new QRadioButton ( tr ( "Auto" ));
  rggbButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_RGGB ? 1 : 0 );
  bggrButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_BGGR ? 1 : 0 );
  grbgButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_GRBG ? 1 : 0 );
  gbrgButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_GBRG ? 1 : 0 );
  cmygButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_CMYG ? 1 : 0 );
  mcgyButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_MCGY ? 1 : 0 );
  ygcmButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_YGCM ? 1 : 0 );
  gymcButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_GYMC ? 1 : 0 );
  autoButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_AUTO ? 1 : 0 );
  rggbButton->setIcon ( QIcon ( ":/qt-icons/RGGB.png" ));
  bggrButton->setIcon ( QIcon ( ":/qt-icons/BGGR.png" ));
  grbgButton->setIcon ( QIcon ( ":/qt-icons/GRBG.png" ));
  gbrgButton->setIcon ( QIcon ( ":/qt-icons/GBRG.png" ));
  cmygButton->setIcon ( QIcon ( ":/qt-icons/CMYG.png" ));
  mcgyButton->setIcon ( QIcon ( ":/qt-icons/MCGY.png" ));
  ygcmButton->setIcon ( QIcon ( ":/qt-icons/YGCM.png" ));
  gymcButton->setIcon ( QIcon ( ":/qt-icons/GYMC.png" ));

  cfaButtons->addButton ( rggbButton );
  cfaButtons->addButton ( bggrButton );
  cfaButtons->addButton ( grbgButton );
  cfaButtons->addButton ( gbrgButton );
  cfaButtons->addButton ( cmygButton );
  cfaButtons->addButton ( mcgyButton );
  cfaButtons->addButton ( ygcmButton );
  cfaButtons->addButton ( gymcButton );
  cfaButtons->addButton ( autoButton );

  methodLabel = new QLabel ( tr ( "Demosaic method" ));
  methodButtons = new QButtonGroup ( this );
  nnButton = new QRadioButton ( tr ( "Nearest Neighbour" ));
  bilinearButton = new QRadioButton ( tr ( "Bilinear" ));
  smoothHueButton = new QRadioButton ( tr ( "Smooth Hue" ));
  vngButton = new QRadioButton ( tr ( "VNG" ));
  nnButton->setChecked ( demosaicConf.demosaicMethod ==
      OA_DEMOSAIC_NEAREST_NEIGHBOUR ? 1 : 0 );
  bilinearButton->setChecked ( demosaicConf.demosaicMethod ==
      OA_DEMOSAIC_BILINEAR ? 1 : 0 );
  smoothHueButton->setChecked ( demosaicConf.demosaicMethod ==
      OA_DEMOSAIC_SMOOTH_HUE ? 1 : 0 );
  vngButton->setChecked ( demosaicConf.demosaicMethod ==
      OA_DEMOSAIC_VNG ? 1 : 0 );

  methodButtons->addButton ( nnButton );
  methodButtons->addButton ( bilinearButton );
  methodButtons->addButton ( smoothHueButton );
  methodButtons->addButton ( vngButton );

  monoAsRaw = new QCheckBox ( tr ( "Treat mono frame as raw colour" ), this );
  monoAsRaw->setChecked ( demosaicConf.monoIsRawColour );

  hbox = new QHBoxLayout ( this );
  lbox = new QVBoxLayout();
  rbox = new QVBoxLayout();
	if ( demosaicOpts ) {
    lbox->addWidget ( demosaicLabel );
    lbox->addWidget ( previewBox );
    lbox->addWidget ( outputBox );
    lbox->addSpacing ( 25 );
	}
  rbox->addWidget ( cfaLabel );
  rbox->addWidget ( rggbButton );
  rbox->addWidget ( bggrButton );
  rbox->addWidget ( grbgButton );
  rbox->addWidget ( gbrgButton );
  rbox->addWidget ( cmygButton );
  rbox->addWidget ( mcgyButton );
  rbox->addWidget ( ygcmButton );
  rbox->addWidget ( gymcButton );
  rbox->addWidget ( autoButton );
  rbox->addSpacing ( 15 );
  rbox->addWidget ( monoAsRaw );
  rbox->addStretch ( 1 );
  lbox->addWidget ( methodLabel );
  lbox->addWidget ( nnButton );
  lbox->addWidget ( bilinearButton );
  lbox->addWidget ( smoothHueButton );
  lbox->addWidget ( vngButton );
  lbox->addStretch ( 1 );
  hbox->addLayout ( lbox );
  hbox->addStretch ( 1 );
  hbox->addLayout ( rbox );
  hbox->addStretch ( 1 );
  setLayout ( hbox );

	if ( demosaicOpts ) {
    connect ( previewBox, SIGNAL ( stateChanged ( int )), parent,
        SLOT ( dataChanged()));
    connect ( outputBox, SIGNAL ( stateChanged ( int )), parent,
        SLOT ( dataChanged()));
	}
  connect ( cfaButtons, SIGNAL ( buttonClicked ( int )), parent,
      SLOT ( dataChanged()));
  connect ( methodButtons, SIGNAL ( buttonClicked ( int )), parent,
      SLOT ( dataChanged()));
  connect ( monoAsRaw, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
}


DemosaicSettings::~DemosaicSettings()
{
  trampolines->destroyLayout (( QLayout* ) hbox );
}


void
DemosaicSettings::storeSettings ( void )
{
	if ( demosaicOpts ) {
    demosaicConf.demosaicPreview = previewBox->isChecked() ? 1 : 0;
    demosaicConf.demosaicOutput = outputBox->isChecked() ? 1 : 0;
	}
  if ( rggbButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_RGGB;
  }
  if ( bggrButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_BGGR;
  }
  if ( grbgButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_GRBG;
  }
  if ( gbrgButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_GBRG;
  }
  if ( cmygButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_CMYG;
  }
  if ( mcgyButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_MCGY;
  }
  if ( ygcmButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_YGCM;
  }
  if ( gymcButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_GYMC;
  }
  if ( autoButton->isChecked()) {
    demosaicConf.cfaPattern = OA_DEMOSAIC_AUTO;
  }
  if ( nnButton->isChecked()) {
    demosaicConf.demosaicMethod = OA_DEMOSAIC_NEAREST_NEIGHBOUR;
  }
  if ( bilinearButton->isChecked()) {
    demosaicConf.demosaicMethod = OA_DEMOSAIC_BILINEAR;
  }
  if ( smoothHueButton->isChecked()) {
    demosaicConf.demosaicMethod = OA_DEMOSAIC_SMOOTH_HUE;
  }
  if ( vngButton->isChecked()) {
    demosaicConf.demosaicMethod = OA_DEMOSAIC_VNG;
  }
  demosaicConf.monoIsRawColour = monoAsRaw->isChecked() ? 1 : 0;

  if ( commonState.camera->isInitialised()) {
    int format = commonState.camera->videoFramePixelFormat();
		int enabled = commonConfig.demosaic;
    trampolines->setVideoFramePixelFormat ( format );
		if ( demosaicOpts ) {
			trampolines->enableTIFFCapture (
					( !oaFrameFormats[ format ].rawColour ||
					( enabled && demosaicConf.demosaicOutput )) ? 1 : 0 );
			trampolines->enablePNGCapture (
					( !oaFrameFormats[ format ].rawColour ||
					( enabled && demosaicConf.demosaicOutput )) ? 1 : 0 );
			trampolines->enableMOVCapture (( QUICKTIME_OK( format ) || 
					( oaFrameFormats[ format ].rawColour && enabled &&
					demosaicConf.demosaicOutput )) ? 1 : 0 );
			trampolines->enableNamedPipeCapture (
					( !oaFrameFormats[ format ].rawColour ||
					( enabled && demosaicConf.demosaicOutput )) ? 1 : 0 );
		}
  }
}


void
DemosaicSettings::updateCFASetting ( void )
{
  rggbButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_RGGB ? 1 : 0 );
  bggrButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_BGGR ? 1 : 0 );
  grbgButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_GRBG ? 1 : 0 );
  gbrgButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_GBRG ? 1 : 0 );
  cmygButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_CMYG ? 1 : 0 );
  mcgyButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_MCGY ? 1 : 0 );
  ygcmButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_YGCM ? 1 : 0 );
  gymcButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_GYMC ? 1 : 0 );
  autoButton->setChecked ( demosaicConf.cfaPattern ==
			OA_DEMOSAIC_AUTO ? 1 : 0 );
}
