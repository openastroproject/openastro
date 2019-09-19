/*****************************************************************************
 *
 * controlsWidget.h -- class declaration
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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

#pragma once

#include <oa_common.h>

#if HAVE_QT5
#include <QtWidgets>
#endif
#include <QtGui>

#include "cameraControls.h"
#include "stackingControls.h"
#include "saveControls.h"
#include "processingControls.h"
#include "outputHandler.h"

class ControlsWidget : public QWidget
{
  Q_OBJECT

  public:
    			ControlsWidget ( QWidget* );
    			~ControlsWidget();
    void		setActiveTab ( int );
    void		enableTab ( int, int );
    void		configure ( void );
    void		enableButtons ( int );
    void		disableAllButtons ( void );
    OutputHandler*	getProcessedOutputHandler ( void );
    void		closeOutputHandlers ( void );
    void		connectSignals ( void );
		int			getZoomFactor ( void );

  private:
    QVBoxLayout*	mainBox;
    QHBoxLayout*	topButtonBox;
    QHBoxLayout*	bottomButtonBox;

    QPushButton*	startButton;
    QPushButton*	stopButton;
    QPushButton*	restartButton;
    QPushButton*	darksButton;
    QPushButton*	lightsButton;

    QTabWidget*		tabSet;
    CameraControls*	camera;
    StackingControls*	stacking;
    SaveControls*	save;
    ProcessingControls*	processing;

    OutputHandler*	frameOutputHandler;
    OutputHandler*	processedImageOutputHandler;

    QList<unsigned int>	XResolutions;
    QList<unsigned int>	YResolutions;
    int			ignoreResolutionChanges;

    void		configureResolution ( void );
    void		openOutputFiles ( void );

  public slots:
    void		startCapture ( void );
    void		stopCapture ( void );
    void		restartCapture ( void );
    void		startNextExposure ( void );
    void		resolutionChanged ( int );
    void		doResolutionChange ( int );	// for DSLR callbacks
};
