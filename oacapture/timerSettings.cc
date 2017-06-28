/*****************************************************************************
 *
 * timerSettings.cc -- class for the timer settings in the settings UI
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

#include <oa_common.h>

extern "C" {
#include <openastro/timer.h>
}

#include "timerSettings.h"

#include "configuration.h"
#include "state.h"


TimerSettings::TimerSettings ( QWidget* parent ) : QWidget ( parent )
{
  resetButton = 0;
  syncButton = 0;

  timerEnableBox = new QCheckBox ( tr ( "Enable Timer" ));
  timerEnableBox->setChecked ( config.timerEnabled );

  modeLabel = new QLabel ( tr ( "Timer mode" ));
  modeButtons = new QButtonGroup ( this );
  strobeModeButton = new QRadioButton ( tr ( "Strobe" ));
  triggerModeButton = new QRadioButton ( tr ( "Trigger" ));

  strobeModeButton->setChecked ( config.timerMode ==
      OA_TIMER_MODE_STROBE ? 1 : 0 );
  triggerModeButton->setChecked ( config.timerMode ==
      OA_TIMER_MODE_TRIGGER ?  1 : 0 );

  modeButtons->addButton ( strobeModeButton );
  modeButtons->addButton ( triggerModeButton );

  if ( state.timer && state.timer->isInitialised()) {
    if ( state.timer->hasReset()) {
      resetButton = new QPushButton ( tr ( "Reset Timer" ), this );
    }
    if ( state.timer->hasSync()) {
      syncButton = new QPushButton ( tr ( "Resync Timer" ), this );
    }
  }

  intervalLabel = new QLabel ( tr ( "Trigger interval (ms)" ), this );
  interval = new QLineEdit ( this );
  intervalValidator = new QIntValidator ( 0, 99999, this );
  interval->setValidator ( intervalValidator );
  if ( config.triggerInterval ) {
    QString n = QString::number ( config.triggerInterval );
    interval->setText ( n );
  }

  enableUserDrainBox = new QCheckBox ( tr (
      "Enable User-specified frame drain delay" ));
  enableUserDrainBox->setChecked ( config.userDrainDelayEnabled );

  drainDelayLabel = new QLabel ( tr ( "Image Drain delay (ms)" ), this );
  drainDelay = new QLineEdit ( this );
  drainDelayValidator = new QIntValidator ( 1, 1000, this );
  drainDelay->setValidator ( drainDelayValidator );
  if ( config.drainDelay ) {
    QString n = QString::number ( config.drainDelay );
    drainDelay->setText ( n );
  }

  /*
   * Not sure we need this for the moment
   *
  timestampDelayLabel = new QLabel ( tr ( "Timestamp Delay (ms)" ), this );
  timestampDelay = new QLineEdit ( this );
  timestampDelayValidator = new QIntValidator ( 1, 5000, this );
  timestampDelay->setValidator ( timestampDelayValidator );
  if ( config.timestampDelay ) {
    QString n = QString::number ( config.timestampDelay );
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

  box = new QVBoxLayout ( this );
  box->addWidget ( timerEnableBox );
  box->addSpacing ( 15 );
  if ( resetButton || syncButton ) {
    if ( resetButton ) {
      box->addWidget ( resetButton );
    }
    if ( syncButton ) {
      box->addWidget ( syncButton );
    }
    box->addSpacing ( 15 );
  }

  box->addWidget ( modeLabel );
  box->addWidget ( strobeModeButton );
  box->addWidget ( triggerModeButton );
  box->addSpacing ( 15 );
  box->addLayout ( intervalLayout );
  box->addSpacing ( 15 );
  box->addWidget ( enableUserDrainBox );
  box->addLayout ( drainDelayLayout );
  /*
  box->addLayout ( timestampDelayLayout );
   */
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
  state.mainWindow->destroyLayout (( QLayout* ) box );
}


void
TimerSettings::storeSettings ( void )
{
  QString msg;

  config.timerMode = strobeModeButton->isChecked() ? OA_TIMER_MODE_STROBE :
      triggerModeButton->isChecked() ? OA_TIMER_MODE_TRIGGER :
      OA_TIMER_MODE_UNSET;
  if (( config.timerEnabled = timerEnableBox->isChecked() ? 1 : 0 )) {
    if ( state.timer && state.timer->isInitialised()) {
      if ( CAPTURE_FITS != config.fileTypeOption || !config.limitEnabled ||
          !config.limitType ) {
        msg = tr ( "\n\nWhen using timer mode the image capture type should "
            "be FITS and a frame-based capture limit should be set." );
        QMessageBox::warning ( this, APPLICATION_NAME, msg );
      }
      if ( state.camera && state.camera->isInitialised()) {
        if ( state.camera->hasControl ( OA_CAM_CTRL_TRIGGER_ENABLE ) &&
            config.timerMode == OA_TIMER_MODE_TRIGGER &&
            !state.camera->readControl ( OA_CAM_CTRL_TRIGGER_ENABLE )) {
          msg = tr ( "\n\nThe timer is in trigger mode but the camera is "
              "not.  These two settings should be the same." );
        }
        if ( state.camera->hasControl ( OA_CAM_CTRL_STROBE_ENABLE ) &&
            config.timerMode == OA_TIMER_MODE_STROBE &&
            !state.camera->readControl ( OA_CAM_CTRL_STROBE_ENABLE )) {
          msg = tr ( "\n\nThe timer is in strobe mode but the camera is "
              "not.  These two settings should be the same." );
        }
      }
    }
  }

  config.userDrainDelayEnabled = enableUserDrainBox->isChecked() ? 1 : 0;

  QString intervalStr = interval->text();
  QString drainDelayStr = drainDelay->text();
  /*
  QString timestampDelayStr = timestampDelay->text();
   */
  if ( intervalStr != "" ) {
    config.triggerInterval = intervalStr.toInt();
  }
  if ( drainDelayStr != "" ) {
    config.drainDelay = drainDelayStr.toInt();
  }
  /*
  if ( timestampDelayStr != "" ) {
    config.timestampDelay = timestampDelayStr.toInt();
  }
   */
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
