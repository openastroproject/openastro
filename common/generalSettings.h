/*****************************************************************************
 *
 * generalSettings.h -- class declaration
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

#include "commonState.h"
#include "trampoline.h"
 
typedef struct {
	// misc config
	int				saveSettings;
	int				tempsInC;
	int				saveCaptureSettings;
	int				connectSoleCamera;
	// window layout
	int				dockableControls;
	int				controlsOnRight;
	int				separateControls;
	// display config
	int				displayFPS;
	// reticle config
	int				reticleStyle;
} generalConfig;

extern generalConfig generalConf;

#define RETICLE_CIRCLE		1
#define RETICLE_CROSS			2
#define RETICLE_TRAMLINES	3


class GeneralSettings : public QWidget
{
  Q_OBJECT

  public:
    			GeneralSettings ( QWidget*, QWidget*, QString, int, int,
							trampolineFuncs* );
    			~GeneralSettings();
    void		storeSettings ( void );

  private:
    QCheckBox*		saveBox;
    QCheckBox*		saveCaptureSettings;
    QCheckBox*		connectSole;
    QCheckBox*		dockable;
    QCheckBox*		controlPosn;
    QCheckBox*		separateControls;
    QVBoxLayout*	box;
    QVBoxLayout*	leftBox;
    QVBoxLayout*	rightBox;
    QHBoxLayout*	topBox;
    QHBoxLayout*	fpsbox;
    QHBoxLayout*	reticlebox;
    QButtonGroup*	reticleButtons;
    QRadioButton*	circleButton;
    QRadioButton*	crossButton;
    QRadioButton*	tramlineButton;
    QButtonGroup*	tempButtons;
    QRadioButton*	degCButton;
    QRadioButton*	degFButton;
    QLabel*		fpsLabel;
    QLabel*		fpsCountLabel;
    QSlider*		fpsSlider;
    QPushButton*	recentreButton;
    QPushButton*	derotateButton;
		QWidget*			topWidget;
		QString				applicationName;
		int						splitControls;
		int						fpsControls;
		trampolineFuncs*	trampolines;
		QWidget*			parentWidget;

  public slots:
    void		updateFPSLabel ( int );
    void		showRestartWarning ( void );
};
