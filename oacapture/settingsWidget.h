/*****************************************************************************
 *
 * settingsWidget.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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
#include "cameraSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "autorunSettings.h"
#include "histogramSettings.h"
#include "demosaicSettings.h"
#include "fitsSettings.h"
#include "timerSettings.h"


class SettingsWidget : public QWidget
{
  Q_OBJECT

  public:
    			SettingsWidget();
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

  private:
    GeneralSettings*	general;
    CaptureSettings*	capture;
    CameraSettings*	cameras;
    ProfileSettings*	profiles;
    FilterSettings*	filters;
    AutorunSettings*	autorun;
    HistogramSettings*	histogram;
    DemosaicSettings*	demosaic;
    FITSSettings*	fits;
    TimerSettings*	timer;
    QVBoxLayout*	vbox;
    QTabWidget*		tabSet;
    QHBoxLayout*	buttonBox;
    QPushButton*	cancelButton;
    QPushButton*	saveButton;
    int                 haveUnsavedData;

  public slots:
    void		storeSettings ( void );
    void		dataChanged ( void );

};
