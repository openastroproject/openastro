/*****************************************************************************
 *
 * mainWindow.h -- class declaration
 *
 * Copyright 2015,2016,2018,2019 James Fidell (james@openastroproject.org)
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

extern "C" {
#include <openastro/camera.h>
#include <openastro/filterwheel.h>
} 

#include "captureSettings.h"
#include "timerSettings.h"
#include "demosaicSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "generalSettings.h"
#include "cameraSettings.h"

#include "viewWidget.h"
#include "focusOverlay.h"
#include "controlsWidget.h"

extern cameraConfig		cameraConf;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
			MainWindow ( QString );
			~MainWindow();
    void		clearTemperature ( void );
    void		resetTemperatureLabel ( void );
    void		showStatusMessage ( QString );
    void		destroyLayout ( QLayout* );
    void		setFlipX ( int );
    void		setFlipY ( int );
    void		configure ( void );
    void		createControlWidgets ( void );
    void		createViewWindow ( void );
    void		updateConfig ( void );

  private:
    QMenu*		fileMenu;
    QMenu*		cameraMenu;
    QMenu*		optionsMenu;
    QMenu*		settingsMenu;
    QMenu*		filterWheelMenu;
    QMenu*		advancedMenu;
    QMenu*		helpMenu;
    QAction*		disconnectCam;
    QAction*		rescanCam;
    QAction*		cameraMenuSeparator;
    QAction*		disconnectWheel;
    QAction*		rescanWheel;
    QAction*		warmResetWheel;
    QAction*		coldResetWheel;
    QAction*		filterWheelMenuSeparator;
    QAction*		fits;
/*
    QProgressBar*	progressBar;
    QLabel*		capturedValue;
    QLabel*		droppedValue;
    QLabel*		fpsMaxValue;
    QLabel*		fpsActualValue;
*/
		QLabel*		stackedLabel;
		QLabel*		stackedValue;

    QStatusBar*		statusLine;
    QLabel*		tempValue;
    int			updateTemperatureLabel;
    QLabel*		wheelStatus;
    QString   userConfigFile;

    void		readConfig ( QString );
    void		writeConfig ( QString );
    void		createStatusBar ( void );
    void		createMenus ( void );
    void		doDisconnectCam ( void );
    void		doDisconnectFilterWheel ( void );
    void		mosaicFlipWarning ( void );
    void		styleStatusBarTemp ( int );
    void		doAdvancedMenu ( void );
    void		createSettingsWidget ( void );

    const char*  	devTypes[9];
    QAction*		cameras[ OA_MAX_DEVICES ];
    QString		cameraMenuEntry[ OA_MAX_DEVICES ];
    oaCameraDevice**	cameraDevs;
    QAction*		filterWheels[ OA_MAX_DEVICES ];
    QString		filterWheelMenuEntry[ OA_MAX_DEVICES ];
    oaFilterWheelDevice** filterWheelDevs;

    QLabel*		tempLabel;
    QAction*		exit;
    QSignalMapper*	cameraSignalMapper;
    QSignalMapper*	filterWheelSignalMapper;
    QSignalMapper*	advancedFilterWheelSignalMapper;
    QAction*		reticle;
    QAction*		focusaid;
    QAction*		derotate;
    QAction*		general;
    QAction*		capture;
    QAction*		profiles;
    QAction*		filters;
    QAction*		demosaic;
    QAction*		about;
    QAction*		colourise;
    int			connectedCameras;
    int			cameraMenuCreated;
    int			connectedFilterWheels;
    int			filterWheelMenuCreated;
    int			doingQuit;

    ViewWidget*		viewWidget;
    QScrollArea*        viewScroller;
    QSplitter*		splitter;
    ControlsWidget*	controlsWidget;
    QList<QAction*>	advancedActions;

    FocusOverlay*       focusOverlay;
/*
    QVBoxLayout*	imageZoomBox;
    CameraWidget*       cameraWidget;
    ImageWidget*        imageWidget;
    ControlWidget*      controlWidget;
    CaptureWidget*      captureWidget;
    QWidget*		imageZoomWidget;
    ZoomWidget*         zoomWidget;
    QVBoxLayout*	vertControlsBox;
    QHBoxLayout*	horizControlsBox;
    QHBoxLayout*	horizBox2;
    QGridLayout*	gridBox;
    QMainWindow*	controlWindow;
*/
    QColorDialog*	colourDialog;

  public slots:
    void		connectCamera( int );
    void		disconnectCamera ( void );
    void		rescanCameras ( void );
    void		connectFilterWheel( int );
    void		disconnectFilterWheel ( void );
    void		warmResetFilterWheel ( void );
    void		coldResetFilterWheel ( void );
    void		rescanFilterWheels ( void );
    void		quit ( void );
    void		enableReticle ( void );
    void		enableFocusAid ( void );
    void		aboutDialog ( void );
    void		doGeneralSettings ( void );
    void		doCaptureSettings ( void );
    void		doProfileSettings ( void );
    void		doFilterSettings ( void );
    void		doDemosaicSettings ( void );
    void		doCameraMenu ( int );
    void		doFilterWheelMenu ( int );
    void		closeSettingsWindow ( void );
    void		settingsClosed ( void );
    void		advancedFilterWheelHandler ( int );
    void		closeAdvancedWindow ( void );
    void		advancedClosed ( void );
    void		doColouriseSettings ( void );
    void		setTemperature ( void );
    void		setStackedFrames ( void );
    void		reveal ( void );
    void		doFITSSettings ( void );
    void		frameWriteFailedPopup ( void );
		void    outputUnwritable ( void );
    int     outputExists ( void );
    void    outputExistsUnwritable ( void );
    void    createFileFailed ( void );
};
