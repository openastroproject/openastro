/*****************************************************************************
 *
 * mainWindow.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
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

#pragma once

#include <oa_common.h>

#if HAVE_QT5
#include <QtWidgets>
#endif
#include <QtCore>
#include <QtGui>

extern "C" {
#include <openastro/camera.h>
#include <openastro/filterwheel.h>
#include <openastro/timer.h>
} 

#include "focusOverlay.h"
#include "demosaicSettings.h"
#include "timerSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "histogramSettings.h"
#include "autorunSettings.h"
#include "generalSettings.h"
#include "cameraSettings.h"

#include "mainWindow.h"
#include "configuration.h"
#include "captureWidget.h"
#include "controlWidget.h"
#include "imageWidget.h"
#include "zoomWidget.h"
#include "cameraWidget.h"
#include "previewWidget.h"
#include "histogramWidget.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
			MainWindow ( QString );
			~MainWindow();
    void		clearDroppedFrames ( void );
    void		setNightStyleSheet ( QWidget* );
    void		clearNightStyleSheet ( QWidget* );
    void		destroyLayout ( QLayout* );
    void		setFlipX ( int );
    void		setFlipY ( int );
    void		configure ( void );
    void		createControlWidgets ( void );
    void		createPreviewWindow ( void );
    void    updateConfig ( void );

  private:
    QMenu*		fileMenu;
    QMenu*		cameraMenu;
    QMenu*		optionsMenu;
    QMenu*		settingsMenu;
    QMenu*		filterWheelMenu;
    QMenu*		timerMenu;
    QMenu*		advancedMenu;
    QMenu*		helpMenu;
    QAction*		disconnectCam;
    QAction*		resetCam;
    QAction*		rescanCam;
    QAction*		cameraMenuSeparator;
    QAction*		disconnectWheel;
    QAction*		rescanWheel;
    QAction*		warmResetWheel;
    QAction*		coldResetWheel;
    QAction*		filterWheelMenuSeparator;
    QAction*		disconnectTimerDevice;
    QAction*		rescanTimer;
    QAction*		resetTimerDevice;
    QAction*		timerMenuSeparator;
    QProgressBar*	progressBar;
    QLabel*		capturedValue;
    QLabel*		droppedValue;
    QStatusBar*		statusLine;
    QLabel*             timerStatus;
    QLabel*             wheelStatus;
    QLabel*             locationLabel;
    QString    userConfigFile;

    void		readConfig ( QString );
    void		writeConfig ( QString );
    void		createStatusBar ( void );
    void		createMenus ( void );
    void		doDisconnectCam ( void );
    void		doDisconnectFilterWheel ( void );
    void		doDisconnectTimer ( void );
    void		mosaicFlipWarning ( void );
    void		styleStatusBarTemp ( int );
    void		styleStatusBarDroppedFrames ( int );
    void		doAdvancedMenu ( void );

    const char*  	devTypes[9];
    QAction*		cameras[ OA_MAX_DEVICES ];
    QString		cameraMenuEntry[ OA_MAX_DEVICES ];
    oaCameraDevice**	cameraDevs;
    QAction*		filterWheels[ OA_MAX_DEVICES ];
    QString		filterWheelMenuEntry[ OA_MAX_DEVICES ];
    oaFilterWheelDevice** filterWheelDevs;
    QAction*		timers[ OA_MAX_DEVICES ];
    QString		timerMenuEntry[ OA_MAX_DEVICES ];
    oaTimerDevice**	timerDevs;

    QLabel*		capturedLabel;
    QLabel*		droppedLabel;

    QAction*		loadConfig;
    QAction*		saveConfig;
    QAction*		saveConfigAs;
    QAction*		exit;
    QSignalMapper*	cameraSignalMapper;
    QSignalMapper*	filterWheelSignalMapper;
    QSignalMapper*	timerSignalMapper;
    QSignalMapper*	advancedFilterWheelSignalMapper;
    QAction*		histogramOpt;
    QAction*		ephems;
    QAction*		autoalign;
    QAction*		alignbox;
    QAction*		autoguide;
    QAction*		reticle;
    QAction*		cutout;
    QAction*		focusaid;
    QAction*		darkframe;
    QAction*		derotate;
    QAction*		flipX;
    QAction*		flipY;
    QAction*		demosaicOpt;
    QAction*		hideControls;
    QAction*		general;
    QAction*		capture;
    QAction*		cameraOpt;
    QAction*		profiles;
    QAction*		filters;
    QAction*		demosaic;
    QAction*		fits;
    QAction*		timer;
    QAction*		autorun;
    QAction*		histogram;
    QAction*		about;
    QAction*		falseColour;
    QAction*		nightMode;
    QAction*		colourise;
    QAction*		preview;
    int			oldHistogramState;
    int			connectedCameras;
    int			cameraMenuCreated;
    int			connectedFilterWheels;
    int			filterWheelMenuCreated;
    int			connectedTimers;
    int			timerMenuCreated;
    int			doingQuit;
    void		createSettingsWidget ( void );
    QDockWidget*	cameraDock;
    QDockWidget*	imageZoomDock;
    QDockWidget*	controlDock;
    QDockWidget*	captureDock;
    QVBoxLayout*	imageZoomBox;
    CameraWidget*       cameraWidget;
    ImageWidget*        imageWidget;
    ControlWidget*      controlWidget;
    CaptureWidget*      captureWidget;
    PreviewWidget*      previewWidget;
    QScrollArea*        previewScroller;
    FocusOverlay*       focusOverlay;
    QWidget*		imageZoomWidget;
    ZoomWidget*         zoomWidget;
    QVBoxLayout*	vertControlsBox;
    QHBoxLayout*	horizControlsBox;
    QHBoxLayout*	horizBox2;
    QGridLayout*	gridBox;
    QWidget*		dummyWidget;
    QMainWindow*	controlWindow;
    QList<QAction*>	advancedActions;
    QSplitter*		splitter;
    QColorDialog*	colourDialog;

  public slots:
    void		connectCamera( int );
    void		disconnectCamera ( void );
    void		resetCamera ( void );
    void		rescanCameras ( void );
    void		connectFilterWheel( int );
    void		disconnectFilterWheel ( void );
    void		warmResetFilterWheel ( void );
    void		coldResetFilterWheel ( void );
    void		rescanFilterWheels ( void );
    void		connectTimer( int );
    void		disconnectTimer ( void );
    void		resetTimer ( void );
    void		rescanTimers ( void );
    void		quit ( void );
    void		enableHistogram ( void );
    void		histogramClosed ( void );
    void		enableReticle ( void );
    void		enableFocusAid ( void );
    void		enableFlipX ( void );
    void		enableFlipY ( void );
    void		enableDemosaic ( void );
    void		enableNightMode ( void );
    void		enableColouriseMode ( void );
    void                enablePreviewMode ( void );
    void		aboutDialog ( void );
    void		doGeneralSettings ( void );
    void		doCaptureSettings ( void );
    void		doCameraSettings ( void );
    void		doProfileSettings ( void );
    void		doFilterSettings ( void );
    void		doDemosaicSettings ( void );
    void		doFITSSettings ( void );
    void		doTimerSettings ( void );
    void		doAutorunSettings ( void );
    void		doHistogramSettings ( void );
    void		doCameraMenu ( int );
    void		doFilterWheelMenu ( int );
    void		doTimerMenu ( int );
    void		closeSettingsWindow ( void );
    void		settingsClosed ( void );
    void		advancedFilterWheelHandler ( int );
    void		advancedPTRHandler ( void );
    void		closeAdvancedWindow ( void );
    void		advancedClosed ( void );
    void		doColouriseSettings ( void );
    void		setCapturedFrames ( unsigned int );
    void		setDroppedFrames ( void );
    void		setProgress ( unsigned int );
    void		reveal ( void );
    void		showStatusMessage ( QString );
    void		frameWriteFailedPopup ( void );
    void		setLocation ( void );
		void		promptForFilterChange ( int );
		void		outputUnwritable ( void );
		int			outputExists ( void );
		void		outputExistsUnwritable ( void );
		void		createFileFailed ( void );
		void		enableTimerExternalLED ( int );
		int			getTimerExternalLEDState ( void );
};
