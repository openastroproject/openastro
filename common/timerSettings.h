/*****************************************************************************
 *
 * timerSettings.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
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

#include "trampoline.h"


typedef struct {
	int			timerEnabled;
	int			timerMode;
	int			triggerInterval;
	int			userDrainDelayEnabled;
	int			drainDelay;
	int			timestampDelay;
	int			queryGPSForEachCapture;
	int			externalLEDEnabled;
} timerConfig;

extern timerConfig timerConf;

class TimerSettings : public QWidget
{
  Q_OBJECT

  public:
    			TimerSettings ( QWidget*, QString, trampolineFuncs* );
    			~TimerSettings();
    void		storeSettings ( void );

  private:
    QLabel*             enableLabel;
    QCheckBox*		timerEnableBox;
    QLabel*             modeLabel;
    QButtonGroup*       modeButtons;
    QRadioButton*       strobeModeButton;
    QRadioButton*       triggerModeButton;
    QVBoxLayout*	lBox;
    QHBoxLayout*	box;
    QPushButton*        resetButton;
    QPushButton*        syncButton;
    QLabel*             intervalLabel;
    QLineEdit*		interval;
    QIntValidator*	intervalValidator;
    QCheckBox*		enableUserDrainBox;
    QLabel*             drainDelayLabel;
    QLineEdit*		drainDelay;
    QIntValidator*	drainDelayValidator;
    QLabel*             timestampDelayLabel;
    QLineEdit*		timestampDelay;
    QIntValidator*	timestampDelayValidator;
    QHBoxLayout*	intervalLayout;
    QHBoxLayout*	drainDelayLayout;
    QHBoxLayout*	timestampDelayLayout;
    QHBoxLayout*	timerModeLayout;
    QCheckBox*		checkGPSBox;
		trampolineFuncs*	trampolines;
		QString				applicationName;
		QCheckBox*		externalLEDEnabled;

  public slots:
    void		doTimerReset ( void );
    void		doTimerSync ( void );
		void		externalLEDCheckboxChanged ( void );
};
