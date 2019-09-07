/*****************************************************************************
 *
 * captureWidget.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2017,2019
 *   James Fidell (james@openastroproject.org)
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

#include "outputHandler.h"

#define	CAPTURE_AVI	1
#define	CAPTURE_SER	2
#define	CAPTURE_TIFF	3
#define	CAPTURE_PNG	4
#define	CAPTURE_FITS	5
#define	CAPTURE_MOV	6
#define	CAPTURE_NAMED_PIPE	7

class CaptureWidget : public QGroupBox
{
  Q_OBJECT

  public:
    unsigned long	recordingStartTime;
    unsigned long	recordingEndTime;
    unsigned long	lastPauseTime;
    unsigned long	totalTimePaused;

    			CaptureWidget ( QWidget* parent = 0 );
    			~CaptureWidget();
    void		enableStartButton ( int );
    void		enableProfileSelect ( int );
    void		closeOutputHandler ( void );
    OutputHandler*	getOutputHandler ( void );
    void		enableSERCapture ( int );
    void		enableTIFFCapture ( int );
    void		enablePNGCapture ( int );
    void		enableFITSCapture ( int );
    void		enableMOVCapture ( int );
    void		enableNamedPipeCapture ( int );
    int			singleAutorunFinished ( void );
    void		enableAutorun ( void );
    QString		getCurrentFilterName ( void );
    QString		getCurrentProfileName ( void );
    QString		getCurrentTargetName ( void );
    int			getCurrentTargetId ( void );
    void		reloadFilters ( void );
    void		reloadProfiles ( void );
    void		updateFromConfig ( void );
    void		setSlotCount ( int );

  private:
    void		doStartRecording ( int );
    void		writeSettings ( OutputHandler* );
    QComboBox*		limitTypeMenu;
    QComboBox*		countFramesMenu;
    QComboBox*		countSecondsMenu;
    QPushButton*	startButton;
    QPushButton*	stopButton;
    QPushButton*	pauseButton;
    QPushButton*	autorunButton;
    QLabel*		autorunLabel;
    QVBoxLayout*	box;
    QHBoxLayout*	profile;
    QHBoxLayout*	file;
    QHBoxLayout*	type;
    QHBoxLayout*	controls;
    QLabel*		profileLabel;
    QComboBox*		profileMenu;
    QPushButton*	restoreButton;
    QLabel*		filterLabel;
    QComboBox*		filterMenu;
    QLabel*		fileLabel;
    QLineEdit*		fileName;
    QRegExpValidator*	fileNameValidator;
    QPushButton*	newFolderButton;
    QPushButton*	deleteButton;
    QPushButton*	openFolderButton;
    QLabel*		typeLabel;
    QComboBox*		typeMenu;
    QCheckBox*		limitCheckbox;
    QPushButton*	fileListButton;
    OutputHandler*	outputHandler;
    QIntValidator*	countValidator;
    int			haveTIFF;
    int			havePNG;
    int			haveSER;
    int			haveFITS;
    int			haveMOV;
    int			haveNamedPipe;
    QLineEdit*		framesInputBox;
    QLineEdit*		secondsInputBox;
    int			filterSequenceRemaining;
    int			autorunFilter;
    int			updateTemperatureLabel;

  signals:
    void		writeStatusMessage ( QString );
    void		changeAutorunLabel ( QString );
    void		enableStopButton ( int );
    void		configureButtonsForStart ( int );
    void		configureButtonsAfterStop ( void );
    void		updateLocation ( void );

  public slots:
    void		showLimitInputBox ( int );
    void		fileTypeChanged ( int );
    void		limitTypeChanged ( int );
    void		startRecording ( void );
    void		pauseRecording ( void );
    void		stopRecording ( void );
    void		setNewCaptureDirectory ( void );
    void		openCaptureDirectory ( void );
    void		resetAutorun ( void );
    void		deleteLastRecordedFile ( void );
    void		updateFileNameTemplate ( void );
    void		filterTypeChanged ( int );
    void		profileTypeChanged ( int );
    void		updateSettingsFromProfile ( void );
    void		updateFilterSettingsFromProfile ( void );
    void		changeFramesLimitText ( void );
    void		changeSecondsLimitText ( void );
    void		changeFramesLimitCount ( int );
    void		changeSecondsLimitCount ( int );
    void		doStopRecording ( void );
    void		updateAutorunLabel ( QString );
    void		changeStopButtonState ( int );
    void		setButtonsForBeginRecording ( int );
    void		setButtonsForRecordingStopped ( void );
    void		startNewAutorun ( void );
};
