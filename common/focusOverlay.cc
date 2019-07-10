/*****************************************************************************
 *
 * focusOverlay.cc -- class for the focus overlay
 *
 * Copyright 2015,2019 James Fidell (james@openastroproject.org)
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

#include <QtGui>

#include "focusOverlay.h"


FocusOverlay::FocusOverlay ( QWidget* parent ) : QWidget ( parent )
{
  setPalette  ( Qt::transparent );
  setAttribute ( Qt::WA_TransparentForMouseEvents );
  setMinimumSize ( 512, 220 );
  startOfBuffer = endOfBuffer = -1;

  currentMaximum = -1;
  currentMinimum = 0x7fffffff;
  currentRange = 1;

  connect ( this, SIGNAL( updateFocus ( void )),
      this, SLOT( update ( void )));
}
 

void
FocusOverlay::paintEvent ( QPaintEvent* event __attribute__((unused)))
{
  QPainter	painter ( this );
  int		numVals;
  int		i, n, x, c, y;
  float		val;

  if ( startOfBuffer == -1 ) {
    return;
  }

  numVals = ( endOfBuffer - startOfBuffer + 256 ) % 256 + 1;
  x = 512 - numVals * 2;
  for ( i = 0, n = startOfBuffer; i < numVals; i++ ) {
    val = ( values[n] - currentMinimum ) / currentRange;
    c = val * 255.0;
    y = 220 - val * 200;
    painter.setPen ( QColor ( 255 - c, c, 0 ));
    painter.drawLine ( x, y, x + 1, y );
    n = ( n + 1 ) % 256;
    x += 2;
  }
}


void
FocusOverlay::addScore ( int score )
{
  if ( score > currentMaximum ) {
    currentMaximum = score;
  }
  if ( score < currentMinimum ) {
    currentMinimum = score;
  }
  if (( currentRange = currentMaximum - currentMinimum ) < 1 ) {
    currentRange = 1;
  }

  if ( startOfBuffer == -1 ) {
    startOfBuffer = endOfBuffer = 0;
    values[ 0 ] = score;
  } else {
    endOfBuffer = ( endOfBuffer + 1 ) % 256;
    values[ endOfBuffer ] = score;
    if ( endOfBuffer == startOfBuffer ) {
      startOfBuffer = ( startOfBuffer + 1 ) % 256;
    }
  }
  // emit updateFocus();
}


void
FocusOverlay::reset ( void )
{
  startOfBuffer = endOfBuffer = -1;

  currentMaximum = -1;
  currentMinimum = 0x7fffffff;
}
