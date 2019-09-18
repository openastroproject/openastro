/*****************************************************************************
 *
 * stackingControlss.cc -- class for the stacking tab in the settings dialog
 *
 * Copyright 2015,2018,2019 James Fidell (james@openastroproject.org)
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

#if HAVE_CSTRING
#include <cstring>
#endif

#include "captureSettings.h"
#include "fitsSettings.h"

#include "stackingControls.h"
#include "state.h"


StackingControls::StackingControls ( QWidget* parent ) : QWidget ( parent )
{
  methodLabel = new QLabel ( tr ( "Stacking Method" ), this );
  QStringList methodStrings;
  // This must be in the same order as in the header file
  methodStrings << tr ( "None" ) << tr ( "Sum" ) << tr ( "Mean" ) <<
			tr ( "Median" ) << tr ( "Maximum" ) << tr ( "Kappa Sigma" );
  stackingMethodMenu = new QComboBox ( this );
  stackingMethodMenu->addItems ( methodStrings );
  state.stackingMethod = OA_STACK_NONE;
  stackingMethodMenu->setCurrentIndex ( OA_STACK_NONE );

	kappaLabel = new QLabel ( tr ( "Kappa" ), this );
	kappaInput = new QLineEdit ( this );
	kappaValidator = new QDoubleValidator ( 0.1, 3.0, 3, this );
	kappaInput->setValidator ( kappaValidator );
	kappaInput->setFixedWidth ( 100 );
	kappaInput->setText ( QString::number ( config.stackKappa ));

  connect ( stackingMethodMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( stackingMethodChanged ( int )));
  connect ( kappaInput, SIGNAL( textEdited( const QString& )), this,
      SLOT( updateKappaValue()));

  grid = new QGridLayout;
	grid->addWidget ( methodLabel, 0, 0 );
	grid->addWidget ( stackingMethodMenu, 0, 1 );
	grid->addWidget ( kappaLabel, 1, 0 );
	grid->addWidget ( kappaInput, 1, 1 );

  grid->setRowStretch ( 2, 1 );

  setLayout ( grid );
}


StackingControls::~StackingControls()
{
  if ( grid ) {
    state.mainWindow->destroyLayout (( QLayout* ) grid );
  }
}


void
StackingControls::stackingMethodChanged ( int index )
{
  state.stackingMethod = index;
  // state.viewWidget->restart();
}


void
StackingControls::updateKappaValue ( void )
{
	QString k = kappaInput->text();
	config.stackKappa = k.toDouble();
}
