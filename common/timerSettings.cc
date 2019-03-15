/*****************************************************************************
 *
 * timerSettings.cc -- class for the timer settings in the settings UI
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

#include <oa_common.h>

extern "C" {
#include <openastro/timer.h>
}

#include "commonState.h"
#include "timerSettings.h"

// This is global.  All applications using this code share it.

timerConfig timerConf;

TimerSettings::TimerSettings ( QWidget* parent, QString appName,
		trampolineFuncs* redirs ) :
		QWidget ( parent ), trampolines ( redirs ), applicationName ( appName )
{
  resetButton = 0;
  syncButton = 0;

  timerEnableBox = new QCheckBox ( tr ( "Enable Timer" ));
  timerEnableBox->setChecked ( timerConf.timerEnabled );

  modeLabel = new QLabel ( tr ( "Timer mode" ));
  modeButtons = new QButtonGroup ( this );
  strobeModeButton = new QRadioButton ( tr ( "Strobe" ));
  triggerModeButton = new QRadioButton ( tr ( "Trigger" ));

  strobeModeButton->setChecked ( timerConf.timerMode ==
      OA_TIMER_MODE_STROBE ? 1 : 0 );
  triggerModeButton->setChecked ( timerConf.timerMode ==
      OA_TIMER_MODE_TRIGGER ?  1 : 0 );

  modeButtons->addButton ( strobeModeButton );
  modeButtons->addButton ( triggerModeButton );

  if ( commonState.timer && commonState.timer->isInitialised()) {
    if ( commonState.timer->hasReset()) {
      resetButton = new QPushButton ( tr ( "Reset Timer" ), this );
    }
    if ( commonState.timer->hasSync()) {
      syncButton = new QPushButton ( tr ( "Resync Timer" ), this );
    }
  }

  intervalLabel = new QLabel ( tr ( "Trigger interval (ms)" ), this );
  interval = new QLineEdit ( this );
  intervalValidator = new QIntValidator ( 0, 99999, this );
  interval->setValidator ( intervalValidator );
  if ( timerConf.triggerInterval ) {
    QString n = QString::number ( timerConf.triggerInterval );
    interval->setText ( n );
  }

  enableUserDrainBox = new QCheckBox ( tr (
      "Enable User-specified frame drain delay" ));
  enableUserDrainBox->setChecked ( timerConf.userDrainDelayEnabled );

  drainDelayLabel = new QLabel ( tr ( "Image Drain delay (ms)" ), this );
  drainDelay = new QLineEdit ( this );
  drainDelayValidator = new QIntValidator ( 1, 1000, this );
  drainDelay->setValidator ( drainDelayValidator );
  if ( timerConf.drainDelay ) {
    QString n = QString::number ( timerConf.drainDelay );
    drainDelay->setText ( n );
  }

  checkGPSBox = new QCheckBox ( tr ( "Read GPS for every capture run" ));
  checkGPSBox->setChecked ( timerConf.queryGPSForEachCapture );

	externalLEDEnabled = new QCheckBox ( tr ( "Enable external LED" ));
	externalLEDEnabled->setChecked ( timerConf.externalLEDEnabled );

  /*
   * Not sure we need this for the moment
   *
  timestampDelayLabel = new QLabel ( tr ( "Timestamp Delay (ms)" ), this );
  timestampDelay = new QLineEdit ( this );
  timestampDelayValidator = new QIntValidator ( 1, 5000, this );
  timestampDelay->setValidator ( timestampDelayValidator );
  if ( timerConf.timestampDelay ) {
    QString n = QString::number ( timerConf.timestampDelay );
    timestampDelay->setText ( n );
  }
   */

  intervalLayout = new QHBoxLayout();
  intervalLayout->addWidget ( intervalLabel );
  intervalLayout->addWidget ( interval );
  intervalLayout->addStretch ( 1 );

  drainDelayLayout = new QHBoxLayout();
  drainDelayLayout->addWidget ( drainDelayLabel );
  drainDelayLayout->addWidget ( drainDelay );
  drainDelayLayout->addStretch ( 1 );

  /*
  timestampDelayLayout = new QHBoxLayout();
  timestampDelayLayout->addWidget ( timestampDelayLabel );
  timestampDelayLayout->addWidget ( timestampDelay );
  timestampDelayLayout->addStretch ( 1 );
   */

	timerModeLayout = new QHBoxLayout();
	timerModeLayout->addWidget ( modeLabel );
	timerModeLayout->addWidget ( strobeModeButton );
	timerModeLayout->addWidget ( triggerModeButton );

  box = new QHBoxLayout ( this );
  lBox = new QVBoxLayout();
  lBox->addWidget ( timerEnableBox );
  if ( resetButton || syncButton ) {
    if ( resetButton ) {
      lBox->addWidget ( resetButton );
    }
    if ( syncButton ) {
      lBox->addWidget ( syncButton );
    }
  }
  lBox->addLayout ( timerModeLayout );
  lBox->addLayout ( intervalLayout );
  lBox->addWidget ( enableUserDrainBox );
  lBox->addLayout ( drainDelayLayout );
  lBox->addWidget ( checkGPSBox );
  lBox->addWidget ( externalLEDEnabled );
  lBox->addStretch ( 1 );

  /*
  lBox->addLayout ( timestampDelayLayout );
   */

	box->addLayout ( lBox );
	box->addStretch ( 1 );
  setLayout ( box );

  connect ( timerEnableBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
  connect ( modeButtons, SIGNAL ( buttonClicked ( int )), parent,
      SLOT ( dataChanged()));
  connect ( enableUserDrainBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
  connect ( interval, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( drainDelay, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
  connect ( checkGPSBox, SIGNAL ( stateChanged ( int )), parent,
      SLOT ( dataChanged()));
  connect ( externalLEDEnabled, SIGNAL ( stateChanged ( int )), this,
      SLOT ( externalLEDCheckboxChanged()));
  /*
  connect ( timestampDelay, SIGNAL ( textEdited ( const QString& )), parent,
      SLOT ( dataChanged()));
   */

  if ( resetButton ) {
    connect ( resetButton, SIGNAL ( clicked()), this, SLOT( doTimerReset()));
  }
  if ( syncButton ) {
    connect ( syncButton, SIGNAL ( clicked()), this, SLOT( doTimerSync()));
  }

}


TimerSettings::~TimerSettings()
{
  trampolines->destroyLayout (( QLayout* ) box );
}


void
TimerSettings::storeSettings ( void )
{
  QString msg;

  timerConf.timerMode = strobeModeButton->isChecked() ? OA_TIMER_MODE_STROBE :
      triggerModeButton->isChecked() ? OA_TIMER_MODE_TRIGGER :
      OA_TIMER_MODE_UNSET;
  if (( timerConf.timerEnabled = timerEnableBox->isChecked() ? 1 : 0 )) {
		if ( commonState.timer && commonState.timer->isInitialised()) {
			trampolines->checkTimerWarnings();
    }
  }

qDebug() << "Set external LED on/off";

  timerConf.userDrainDelayEnabled = enableUserDrainBox->isChecked() ? 1 : 0;

  QString intervalStr = interval->text();
  QString drainDelayStr = drainDelay->text();
  /*
  QString timestampDelayStr = timestampDelay->text();
   */
  if ( intervalStr != "" ) {
    timerConf.triggerInterval = intervalStr.toInt();
  }
  if ( drainDelayStr != "" ) {
    timerConf.drainDelay = drainDelayStr.toInt();
  }
  /*
  if ( timestampDelayStr != "" ) {
    timerConf.timestampDelay = timestampDelayStr.toInt();
  }
   */

  timerConf.queryGPSForEachCapture = checkGPSBox->isChecked() ? 1 : 0;
	timerConf.externalLEDEnabled = externalLEDEnabled->isChecked() ? 1 : 0;
}


void
TimerSettings::doTimerReset ( void )
{
  // FIX ME
  qWarning() << "Implement TimerSettings::doTimerReset";
}


void
TimerSettings::doTimerSync ( void )
{
  // FIX ME
  qWarning() << "Implement TimerSettings::doTimerSync";
}


void
TimerSettings::externalLEDCheckboxChanged ( void )
{
	trampolines->enableTimerExternalLED (
			externalLEDEnabled->isChecked() ? 1 : 0 );
}
