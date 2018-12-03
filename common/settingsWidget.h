/*****************************************************************************
 *
 * settingsWidget.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2018
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

#ifdef HAVE_QT5
#include <QtWidgets>
#endif
#include <QtCore>
#include <QtGui>

#include "generalSettings.h"
#include "captureSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "autorunSettings.h"
#include "histogramSettings.h"
#include "cameraSettings.h"
#include "timerSettings.h"
#include "demosaicSettings.h"
#include "fitsSettings.h"

#define	SETTINGS_GENERAL		0x0001
#define	SETTINGS_CAPTURE		0x0002
#define	SETTINGS_CAMERA			0x0004
#define	SETTINGS_PROFILE		0x0008
#define	SETTINGS_FILTER			0x0010
#define	SETTINGS_AUTORUN		0x0020
#define	SETTINGS_HISTOGRAM	0x0040
#define	SETTINGS_TIMER			0x0080
#define	SETTINGS_DEMOSAIC		0x0100
#define	SETTINGS_FITS				0x0200

#define OACAPTURE_SETTINGS	0x03ff
#define	OALIVE_SETTINGS			0x031b


class SettingsWidget : public QWidget
{
  Q_OBJECT

  public:
    			SettingsWidget ( QWidget*, QWidget*, QString, unsigned int, int, int,
							trampolineFuncs* );
    			~SettingsWidget();
    void		setActiveTab ( int );
    void		enableTab ( int, int );
    void		updateCFASetting ( void );
    void		configureCameraSettings ( void );
    void		enableFlipX ( int );
    void		enableFlipY ( int );
    void		updateControl ( int, int );
    void		propagateNewSlotName ( int, const QString& );
    QString		getSlotFilterName ( int );
    void		setSlotCount ( int );
    void		updateFrameRate ( int );
    void		reconfigureControl ( int );
    QWidget*		getTabset ( void );

  private:
    GeneralSettings*	general;
    CaptureSettings*	capture;
    CameraSettings*	cameras;
    TimerSettings*	timer;
    AutorunSettings*	autorun;
    HistogramSettings*	histogram;
    ProfileSettings*	profiles;
    FilterSettings*	filters;
    DemosaicSettings*	demosaic;
    FITSSettings*	fits;
    QVBoxLayout*	vbox;
    QTabWidget*		tabSet;
    QHBoxLayout*	buttonBox;
    QPushButton*	cancelButton;
    QPushButton*	saveButton;
    int                 haveUnsavedData;
		QString				applicationName;
		unsigned int	reqdWindows;

  public slots:
    void		storeSettings ( void );
    void		dataChanged ( void );

};
