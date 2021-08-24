/*****************************************************************************
 *
 * occultationWidget.cc -- class declaration
 *
 * Copyright 2021
 *     Dave Tucker (dave@dtucker.co.uk), James Fidell (james@openastroject.org)
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

#include <QtGui>

#include "commonConfig.h"

#include "timerSettings.h"
#include "state.h"
#include "commonState.h"
#include "occultationWidget.h"
#include "configuration.h"

OccultationWidget::OccultationWidget(const char *appName, QWidget *parent) :
		QWidget(parent) {
	windowSizeX = 300;
	windowSizeY = 180;
	resize(windowSizeX, windowSizeY);
	if (appName) {
		char str[256];
		(void) strncpy(str, appName, 255);
		(void) strncat(str, " Occultation Capture Controls", 255);
		setWindowTitle(str);
		setWindowIcon(QIcon(":/qt-icons/occultation.png"));
	}

	intervalMultipliers << 1 << 1000 << 1000000 << 60000000;
	intervalsList << "usec" << "msec" << "sec" << "min";

	reticle = new QCheckBox(tr("Display Reticle"), this);
	reticle->setChecked(config.showReticle);
	if (state.mainWindow) {
		connect(reticle, SIGNAL(stateChanged( int )), state.mainWindow,
			SLOT(enableReticle( int )));
	}

	externalLEDEnabled = new QCheckBox(tr("Enable external LED"), this);
	externalLEDEnabled->setChecked(timerConf.externalLEDEnabled);
	connect(externalLEDEnabled, SIGNAL(stateChanged( int )), this,
			SLOT(externalLEDCheckboxChanged( int )));

	resetCaptureCounterButton = new QPushButton(tr("Reset Capture Counter"),
			this);
	connect(resetCaptureCounterButton, SIGNAL(clicked()), this,
			SLOT(resetCaptureCounter()));

	captureTenFrames = new QPushButton(tr("Capture 10 Frames"), this);
	captureTenFrames->setEnabled(false);

	interframeIntervalLabel = new QLabel(tr("Interframe Interval"), this);
	interframeInterval = new QLineEdit(this);
	interframeInterval->setEnabled(false);
	interframeIntervalInterval = new QComboBox(this);
	interframeIntervalInterval->addItems(intervalsList);
	interframeIntervalInterval->setEnabled(false);
	triggerIntervalLabel = new QLabel(tr("Trigger Interval"), this);
	triggerIntervalInterval = new QComboBox(this);
	triggerIntervalInterval->addItems(intervalsList);
	triggerIntervalInterval->setEnabled(false);
	triggerInterval = new QLineEdit(this);
	triggerInterval->setEnabled(false);

	grid = new QGridLayout();

	grid->addWidget(reticle, 0, 0);
	grid->addWidget(externalLEDEnabled, 1, 0);
	grid->addWidget(resetCaptureCounterButton, 0, 2);
	grid->addWidget(captureTenFrames, 1, 2);
	grid->addWidget(interframeIntervalLabel, 2, 0);
	grid->addWidget(interframeIntervalInterval, 2, 1);
	grid->addWidget(interframeInterval, 2, 2);
	grid->addWidget(triggerIntervalLabel, 3, 0);
	grid->addWidget(triggerIntervalInterval, 3, 1);
	grid->addWidget(triggerInterval, 3, 2);

	setLayout(grid);
}

OccultationWidget::~OccultationWidget() {
}

QSize
OccultationWidget::sizeHint() const {
	QSize size(windowSizeX, windowSizeY);
	return size;
}

void
OccultationWidget::externalLEDCheckboxChanged(int value) {
	QMetaObject::invokeMethod(state.mainWindow, "enableTimerExternalLED",
			Qt::DirectConnection, Q_ARG(int, value));
}

void
OccultationWidget::configure(void) {
	return;
}

void
OccultationWidget::resetCaptureCounter(void) {
	// FIX ME -- this might not be good in the middle of a capture run
	commonState.captureIndex = 0;
}
