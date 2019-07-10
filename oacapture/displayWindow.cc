/*****************************************************************************
 *
 * displayWindow.cc -- managing class for the main display area
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
#include "controlWidget.h"
#include "cameraWidget.h"
#include "imageWidget.h"
#include "captureWidget.h"
#include "zoomWidget.h"
#include "previewWidget.h"
#include "settingsWidget.h"
#include "displayWindow.h"
#include "state.h"


DisplayWindow::DisplayWindow ( QWidget* parent ) : QWidget ( parent )
{
  outerBox = new QVBoxLayout ( this );
  topBox = new QHBoxLayout();
  bottomBox = new QHBoxLayout();
  topLeftBox = new QVBoxLayout();
  
  // ugly, but need to do this to prevent access attempts before creation
  previewWidget = nullptr;
  cameraWidget = new CameraWidget ( this );
  imageWidget = new ImageWidget ( this );
  zoomWidget = new ZoomWidget ( this );

  controlWidget = new ControlWidget ( this );
  state.controlWidget = controlWidget;
  state.imageWidget = imageWidget;
  state.zoomWidget = zoomWidget;
  state.cameraWidget = cameraWidget;
  state.captureWidget = new CaptureWidget ( this );

  topLeftBox->addWidget ( imageWidget );
  topLeftBox->addWidget ( zoomWidget );

  topBox->addWidget ( cameraWidget );
  topBox->addLayout ( topLeftBox );
  topBox->addWidget ( controlWidget );
  topBox->addWidget ( state.captureWidget );
  topBox->addStretch ( 1 );

  previewScroller = new QScrollArea ( this );
  previewWidget = new PreviewWidget ( previewScroller );
  state.previewWidget = previewWidget;
  commonState.viewerWidget = ( QWidget* ) previewWidget;
  // These figures are a bit arbitrary, but give a size that should work
  // initially on small displays
  // previewScroller->setMinimumSize ( 640, 240 );
  previewScroller->setSizePolicy( QSizePolicy::Expanding,
      QSizePolicy::Expanding );    
  previewScroller->setFocusPolicy( Qt::NoFocus );
  previewScroller->setContentsMargins( 0, 0, 0, 0 );
  previewScroller->setWidget ( previewWidget );

  if ( !config.preview ) {
    previewScroller->hide();
  }

  bottomBox->addWidget ( previewScroller );

  outerBox->setMargin ( 0 );
  outerBox->addLayout ( topBox );
  outerBox->addLayout ( bottomBox );

  setLayout ( outerBox );
}


DisplayWindow::~DisplayWindow()
{
  state.mainWindow->destroyLayout (( QLayout* ) outerBox );
}


void
DisplayWindow::changePreviewState ( int newState )
{
  if ( newState == Qt::Unchecked ) {
    config.preview = 0;
    previewScroller->hide();
    if ( previewWidget ) {
      previewWidget->setEnabled ( 0 );
    }
  } else {
    config.preview = 1;
    previewScroller->show();
    if ( previewWidget ) {
      previewWidget->setEnabled ( 1 );
    }
  }
}


void
DisplayWindow::configure ( void )
{
  cameraWidget->configure();
  imageWidget->configure();
  controlWidget->configure();
  previewWidget->configure();
}
