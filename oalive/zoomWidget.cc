/*****************************************************************************
 *
 * zoomWidget.cc -- class for the zoom control widget in the main UI
 *
 * Copyright 2013,2014,2019 James Fidell (james@openastroproject.org)
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

#include "configuration.h"
#include "state.h"
#include "zoomWidget.h"

static QString	labels[] = { "25%", "50%", "75%", "100%", "125%", "150%",
    "200%", "250%", "300%" };
static int	labelValues[] = { 25, 50, 75, 100, 125, 150, 200, 250, 300 };


ZoomWidget::ZoomWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  box = new QVBoxLayout ( this );
  menuBox = new QHBoxLayout;
  sliderBox = new QHBoxLayout;

  menu1Button = new QToolButton( this );
  menu2Button = new QToolButton( this );
  menu3Button = new QToolButton( this );
  menu1Button->setPopupMode ( QToolButton::MenuButtonPopup );
  menu1Button->setToolButtonStyle ( Qt::ToolButtonTextOnly );
  menu2Button->setPopupMode ( QToolButton::MenuButtonPopup );
  menu2Button->setToolButtonStyle ( Qt::ToolButtonTextOnly );
  menu3Button->setPopupMode ( QToolButton::MenuButtonPopup );
  menu3Button->setToolButtonStyle ( Qt::ToolButtonTextOnly );
  menu1Button->setToolTip ( tr ( "Scale view to size" ));
  menu2Button->setToolTip ( tr ( "Scale view to size" ));
  menu3Button->setToolTip ( tr ( "Scale view to size" ));
  connect ( menu1Button, SIGNAL( clicked()), this, SLOT ( zoomOnButton1()));
  connect ( menu2Button, SIGNAL( clicked()), this, SLOT ( zoomOnButton2()));
  connect ( menu3Button, SIGNAL( clicked()), this, SLOT ( zoomOnButton3()));

  zoom1Menu = new QMenu ( this );
  zoom2Menu = new QMenu ( this );
  zoom3Menu = new QMenu ( this );

  // I can't at the moment find a less long-winded way of doing this
  // That can be a problem for another time
  zoomActions[0][0] = zoom1Menu->addAction ( labels[0] );
  connect ( zoomActions[0][0], SIGNAL( triggered()), this,
    SLOT( zoom1Action0()));
  zoomActions[0][1] = zoom1Menu->addAction ( labels[1] );
  connect ( zoomActions[0][1], SIGNAL( triggered()), this,
    SLOT( zoom1Action1()));
  zoomActions[0][2] = zoom1Menu->addAction ( labels[2] );
  connect ( zoomActions[0][2], SIGNAL( triggered()), this,
    SLOT( zoom1Action2()));
  zoomActions[0][3] = zoom1Menu->addAction ( labels[3] );
  connect ( zoomActions[0][3], SIGNAL( triggered()), this,
    SLOT( zoom1Action3()));
  zoomActions[0][4] = zoom1Menu->addAction ( labels[4] );
  connect ( zoomActions[0][4], SIGNAL( triggered()), this,
    SLOT( zoom1Action4()));
  zoomActions[0][5] = zoom1Menu->addAction ( labels[5] );
  connect ( zoomActions[0][5], SIGNAL( triggered()), this,
    SLOT( zoom1Action5()));
  zoomActions[0][6] = zoom1Menu->addAction ( labels[6] );
  connect ( zoomActions[0][6], SIGNAL( triggered()), this,
    SLOT( zoom1Action6()));
  zoomActions[0][7] = zoom1Menu->addAction ( labels[7] );
  connect ( zoomActions[0][7], SIGNAL( triggered()), this,
    SLOT( zoom1Action7()));
  zoomActions[0][8] = zoom1Menu->addAction ( labels[8] );
  connect ( zoomActions[0][8], SIGNAL( triggered()), this,
    SLOT( zoom1Action8()));

  zoomActions[1][0] = zoom2Menu->addAction ( labels[0] );
  connect ( zoomActions[1][0], SIGNAL( triggered()), this,
    SLOT( zoom2Action1()));
  zoomActions[1][1] = zoom2Menu->addAction ( labels[1] );
  connect ( zoomActions[1][1], SIGNAL( triggered()), this,
    SLOT( zoom2Action1()));
  zoomActions[1][2] = zoom2Menu->addAction ( labels[2] );
  connect ( zoomActions[1][2], SIGNAL( triggered()), this,
    SLOT( zoom2Action2()));
  zoomActions[1][3] = zoom2Menu->addAction ( labels[3] );
  connect ( zoomActions[1][3], SIGNAL( triggered()), this,
    SLOT( zoom2Action3()));
  zoomActions[1][4] = zoom2Menu->addAction ( labels[4] );
  connect ( zoomActions[1][4], SIGNAL( triggered()), this,
    SLOT( zoom2Action4()));
  zoomActions[1][5] = zoom2Menu->addAction ( labels[5] );
  connect ( zoomActions[1][5], SIGNAL( triggered()), this,
    SLOT( zoom2Action5()));
  zoomActions[1][6] = zoom2Menu->addAction ( labels[6] );
  connect ( zoomActions[1][6], SIGNAL( triggered()), this,
    SLOT( zoom2Action6()));
  zoomActions[1][7] = zoom2Menu->addAction ( labels[7] );
  connect ( zoomActions[1][7], SIGNAL( triggered()), this,
    SLOT( zoom2Action7()));
  zoomActions[1][8] = zoom2Menu->addAction ( labels[8] );
  connect ( zoomActions[1][8], SIGNAL( triggered()), this,
    SLOT( zoom2Action8()));

  zoomActions[2][0] = zoom3Menu->addAction ( labels[0] );
  connect ( zoomActions[2][0], SIGNAL( triggered()), this,
    SLOT( zoom3Action1()));
  zoomActions[2][1] = zoom3Menu->addAction ( labels[1] );
  connect ( zoomActions[2][1], SIGNAL( triggered()), this,
    SLOT( zoom3Action1()));
  zoomActions[2][2] = zoom3Menu->addAction ( labels[2] );
  connect ( zoomActions[2][2], SIGNAL( triggered()), this,
    SLOT( zoom3Action2()));
  zoomActions[2][3] = zoom3Menu->addAction ( labels[3] );
  connect ( zoomActions[2][3], SIGNAL( triggered()), this,
    SLOT( zoom3Action3()));
  zoomActions[2][4] = zoom3Menu->addAction ( labels[4] );
  connect ( zoomActions[2][4], SIGNAL( triggered()), this,
    SLOT( zoom3Action4()));
  zoomActions[2][5] = zoom3Menu->addAction ( labels[5] );
  connect ( zoomActions[2][5], SIGNAL( triggered()), this,
    SLOT( zoom3Action5()));
  zoomActions[2][6] = zoom3Menu->addAction ( labels[6] );
  connect ( zoomActions[2][6], SIGNAL( triggered()), this,
    SLOT( zoom3Action6()));
  zoomActions[2][7] = zoom3Menu->addAction ( labels[7] );
  connect ( zoomActions[2][7], SIGNAL( triggered()), this,
    SLOT( zoom3Action7()));
  zoomActions[2][8] = zoom3Menu->addAction ( labels[8] );
  connect ( zoomActions[2][8], SIGNAL( triggered()), this,
    SLOT( zoom3Action8()));


  menu1Button->setMenu ( zoom1Menu );
  menu2Button->setMenu ( zoom2Menu );
  menu3Button->setMenu ( zoom3Menu );
  menu1Button->setText ( labels[ config.zoomButton1Option ] );
  menu2Button->setText ( labels[ config.zoomButton2Option ] );
  menu3Button->setText ( labels[ config.zoomButton3Option ] );

  menuBox->addWidget ( menu1Button );
  menuBox->addWidget ( menu2Button );
  menuBox->addWidget ( menu3Button );

  zoomSlider = new QSlider ( Qt::Horizontal, this );
  zoomSlider->setToolTip ( tr ( "Slide to rescale view image" ));
  zoomSlider->setMinimum ( 25 );
  zoomSlider->setMaximum ( 300 );
  zoomSlider->setValue ( config.zoomValue );
  connect ( zoomSlider, SIGNAL( valueChanged ( int )), this,
      SLOT( changeZoomLevel ( int )));

  sliderBox->addWidget ( zoomSlider );

  box->addLayout ( menuBox );
  box->addLayout ( sliderBox );

  setTitle ( tr ( "Zoom" ));
  setLayout ( box );
}


ZoomWidget::~ZoomWidget()
{
  state.mainWindow->destroyLayout (( QLayout* ) box );
}


void
ZoomWidget::changeZoomLevel ( int size )
{
  config.zoomValue = size;
  if ( state.viewWidget ) {
    state.viewWidget->zoomUpdated ( size );
  }
}


void
ZoomWidget::zoomOnButton1 ( void )
{
  zoomSlider->setValue ( labelValues [ config.zoomButton1Option ] );
}


void
ZoomWidget::zoomOnButton2 ( void )
{
  zoomSlider->setValue ( labelValues [ config.zoomButton2Option ] );
}


void
ZoomWidget::zoomOnButton3 ( void )
{
  zoomSlider->setValue ( labelValues [ config.zoomButton3Option ] );
}


int
ZoomWidget::getZoomFactor ( void )
{
  return config.zoomValue;
}


void
ZoomWidget::zoom1Action0 ( void )
{
  updateZoom1Button(0);
}


void
ZoomWidget::zoom1Action1 ( void )
{
  updateZoom1Button(1);
}


void
ZoomWidget::zoom1Action2 ( void )
{
  updateZoom1Button(2);
}


void
ZoomWidget::zoom1Action3 ( void )
{
  updateZoom1Button(3);
}


void
ZoomWidget::zoom1Action4 ( void )
{
  updateZoom1Button(4);
}


void
ZoomWidget::zoom1Action5 ( void )
{
  updateZoom1Button(5);
}


void
ZoomWidget::zoom1Action6 ( void )
{
  updateZoom1Button(6);
}


void
ZoomWidget::zoom1Action7 ( void )
{
  updateZoom1Button(7);
}


void
ZoomWidget::zoom1Action8 ( void )
{
  updateZoom1Button(8);
}


void
ZoomWidget::zoom2Action0 ( void )
{
  updateZoom2Button(0);
}


void
ZoomWidget::zoom2Action1 ( void )
{
  updateZoom2Button(1);
}


void
ZoomWidget::zoom2Action2 ( void )
{
  updateZoom2Button(2);
}


void
ZoomWidget::zoom2Action3 ( void )
{
  updateZoom2Button(3);
}


void
ZoomWidget::zoom2Action4 ( void )
{
  updateZoom2Button(4);
}


void
ZoomWidget::zoom2Action5 ( void )
{
  updateZoom2Button(5);
}


void
ZoomWidget::zoom2Action6 ( void )
{
  updateZoom2Button(6);
}


void
ZoomWidget::zoom2Action7 ( void )
{
  updateZoom2Button(7);
}


void
ZoomWidget::zoom2Action8 ( void )
{
  updateZoom2Button(8);
}


void
ZoomWidget::zoom3Action0 ( void )
{
  updateZoom3Button(0);
}


void
ZoomWidget::zoom3Action1 ( void )
{
  updateZoom3Button(1);
}


void
ZoomWidget::zoom3Action2 ( void )
{
  updateZoom3Button(2);
}


void
ZoomWidget::zoom3Action3 ( void )
{
  updateZoom3Button(3);
}


void
ZoomWidget::zoom3Action4 ( void )
{
  updateZoom3Button(4);
}


void
ZoomWidget::zoom3Action5 ( void )
{
  updateZoom3Button(5);
}


void
ZoomWidget::zoom3Action6 ( void )
{
  updateZoom3Button(6);
}


void
ZoomWidget::zoom3Action7 ( void )
{
  updateZoom3Button(7);
}


void
ZoomWidget::zoom3Action8 ( void )
{
  updateZoom3Button(8);
}


void
ZoomWidget::updateZoom1Button ( int item )
{
  config.zoomButton1Option = item;
  menu1Button->setText ( labels[ item ] );
}


void
ZoomWidget::updateZoom2Button ( int item )
{
  config.zoomButton2Option = item;
  menu2Button->setText ( labels[ item ] );
}


void
ZoomWidget::updateZoom3Button ( int item )
{
  config.zoomButton3Option = item;
  menu3Button->setText ( labels[ item ] );
}
