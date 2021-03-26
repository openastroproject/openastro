/*****************************************************************************
 *
 * occulationWidget.h -- class declaration
 *
 * Copyright 2021
 *     Dave Tucker (dave@dtucker.co.uk)
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
#include "occulationWidget.h"
#include "configuration.h"

OcculationWidget::OcculationWidget(const char *appName, QWidget *parent) :
		QWidget(parent) {
	windowSizeX = 300;
	windowSizeY = 300;
	resize(windowSizeX, windowSizeY);
	if (appName) {
		char str[256];
		(void) strncpy(str, appName, 255);
		(void) strncat(str, " Targeted Capture Controls", 255);
		setWindowTitle(str);
		setWindowIcon(QIcon(":/qt-icons/occulation.png"));
	}

	intervalMultipliers << 1 << 1000 << 1000000 << 60000000;
	intervalsList << "usec" << "msec" << "sec" << "min";

	binning2x2 = new QCheckBox(tr("2x2 Binning"), this);
	binning2x2->setChecked(commonConfig.binning2x2);
	if (state.cameraWidget) {
		connect(binning2x2, SIGNAL(stateChanged ( int )), state.cameraWidget,
			SLOT(setBinning (int)));
	}
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
	exposureTimeLabel = new QLabel(tr("Exposure Time"), this);
	exposureTime = new QLineEdit(this);
	exposureTime->setEnabled(false);
	exposureTimeInterval = new QComboBox(this);
	exposureTimeInterval->addItems(intervalsList);
	exposureTimeInterval->setEnabled(false);

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

	grid->addWidget(binning2x2, 0, 0);
	grid->addWidget(reticle, 1, 0);
	grid->addWidget(externalLEDEnabled, 2, 0);
	grid->addWidget(resetCaptureCounterButton, 0, 2);
	grid->addWidget(captureTenFrames, 1, 2);
	grid->addWidget(exposureTimeLabel, 3, 0);
	grid->addWidget(exposureTimeInterval, 3, 1);
	grid->addWidget(exposureTime, 3, 2);
	grid->addWidget(interframeIntervalLabel, 4, 0);
	grid->addWidget(interframeIntervalInterval, 4, 1);
	grid->addWidget(interframeInterval, 4, 2);
	grid->addWidget(triggerIntervalLabel, 5, 0);
	grid->addWidget(triggerIntervalInterval, 5, 1);
	grid->addWidget(triggerInterval, 5, 2);

	setLayout(grid);
}

OcculationWidget::~OcculationWidget() {
}

QSize OcculationWidget::sizeHint() const {
	QSize size(windowSizeX, windowSizeY);
	return size;
}

void OcculationWidget::externalLEDCheckboxChanged(int value) {
	QMetaObject::invokeMethod(state.mainWindow, "enableTimerExternalLED",
			Qt::DirectConnection, Q_ARG(int, value));
}

void OcculationWidget::enableBinningControl(int value) {
	binning2x2->setEnabled(value);
}

void OcculationWidget::configure(void) {
	if (commonState.camera) {
		binning2x2->setEnabled(commonState.camera->hasBinning(2) ? 1 : 0);
	} else {
		binning2x2->setEnabled(true);
	}
}

void OcculationWidget::setBinning(int value) {
	binning2x2->setChecked(value);
}

void OcculationWidget::resetCaptureCounter(void) {
	// FIX ME -- this might not be good in the middle of a capture run
	commonState.captureIndex = 0;
}
