/*****************************************************************************
 *
 * stackingControlss.cc -- class for the stacking tab in the settings dialog
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

#include "stackingControls.h"
#include "state.h"
#include "strings.h"


StackingControls::StackingControls ( QWidget* parent ) : QWidget ( parent )
{
  methodLabel = new QLabel ( tr ( "Stacking Method" ));
  QStringList methodStrings;
  // This must be in the same order as in the header file
  methodStrings << tr ( "None" ) << tr ( "Sum" ) << tr ( "Mean" );
  stackingMethodMenu = new QComboBox;
  stackingMethodMenu->addItems ( methodStrings );
  state.stackingMethod = OA_STACK_NONE;
  stackingMethodMenu->setCurrentIndex ( OA_STACK_NONE );

  connect ( stackingMethodMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( stackingMethodChanged ( int )));

  hbox1 = new QHBoxLayout;
  hbox1->addWidget ( methodLabel );
  hbox1->addWidget ( stackingMethodMenu );

  layout = new QVBoxLayout;
  layout->addLayout ( hbox1 );
  layout->addStretch ( 1 );

  setLayout ( layout );
}


StackingControls::~StackingControls()
{
  if ( layout ) {
    state.mainWindow->destroyLayout (( QLayout* ) layout );
  }
}


void
StackingControls::stackingMethodChanged ( int index )
{
  state.stackingMethod = index;
  state.viewWidget->restart();
}
