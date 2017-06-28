/*****************************************************************************
 *
 * settingsWidget.h -- class declaration
 *
 * Copyright 2015, 2016 James Fidell (james@openastroproject.org)
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
#include <QtGui>

#include "generalSettings.h"
#include "captureSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "demosaicSettings.h"

class SettingsWidget : public QWidget
{
  Q_OBJECT

  public:
    			SettingsWidget();
    			~SettingsWidget();
    void		setActiveTab ( int );
    void		enableTab ( int, int );
    void		updateCFASetting ( void );
    QString		getSlotFilterName ( int );
    void		setSlotCount ( int );

  private:
    GeneralSettings*	general;
    CaptureSettings*	capture;
    ProfileSettings*	profiles;
    FilterSettings*	filters;
    DemosaicSettings*	demosaic;
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
