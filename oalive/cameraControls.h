/*****************************************************************************
 *
 * cameraControls.h -- class declaration
 *
 * Copyright 2015,2016,2019 James Fidell (james@openastroproject.org)
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
#include <QtGui>

extern "C" {
#include <openastro/camera.h>
}

#include "histogramWidget.h"


class CameraControls : public QWidget
{
  Q_OBJECT

  public:
    			CameraControls ( QWidget* );
    			~CameraControls();
    void		configure ( void );
    void		disableAutoControls ( void );
    unsigned int	getCurrentGain ( void );
    unsigned int	getCurrentExposure ( void );
		void		connectHistogramSignal ( void );

		HistogramWidget*	histogram;

  private:
    QVBoxLayout*	layout;
    QGridLayout*	sliderGrid;
    QGridLayout*	checkboxGrid;
    QGridLayout*	buttonGrid;
    QGridLayout*	menuGrid;
    QGridLayout*	statusGrid;
    QGridLayout*	unhandledGrid;
    QSignalMapper*	sliderSignalMapper;
    QSignalMapper*	checkboxSignalMapper;
    QSignalMapper*	buttonSignalMapper;
    QSignalMapper*	menuSignalMapper;
    QLabel*		controlLabel[ OA_CAM_CTRL_LAST_P1 ];
    QSlider*		controlSlider[ OA_CAM_CTRL_LAST_P1 ];
    QSpinBox*		controlSpinbox[ OA_CAM_CTRL_LAST_P1 ];
    QCheckBox*		controlCheckbox[ OA_CAM_CTRL_LAST_P1 ];
    QPushButton*	controlButton[ OA_CAM_CTRL_LAST_P1 ];
    QComboBox*		controlMenu[ OA_CAM_CTRL_LAST_P1 ];
    QLabel*		autoLabel1;
    int			controlType [ OA_CAM_CTRL_LAST_P1 ];
    QLabel*		frameRateLabel;
    QSlider*		frameRateSlider;
    QComboBox*		frameRateMenu;
    QList<int>		frameRateNumerator;
    QList<int>		frameRateDenominator;
    int			ignoreFrameRateChanges;
    int			theoreticalFPSNumerator;
    int			theoreticalFPSDenominator;
    QComboBox*		exposureRangeMenu;
    QLabel*		exposureRangeLabel;

    void		_doFrameRateChange ( int );
		int			minRangeIndex;
		int			maxRangeIndex;
		int			ignoreExposureChanges;
		QProgressBar*		batteryLevel;
		QLabel*	powerSource;

  public slots:
    void		updateSliderControl ( int );
    void		updateCheckboxControl ( int );
    void		updateFrameRateSlider ( void );
    void		frameRateChanged ( void );
    int			getFPSNumerator ( void );
    int			getFPSDenominator ( void );

    void		buttonPushed ( int );
    void		menuChanged ( int );
    void		updateExposureUnits ( int );
		void		setBatteryLevel ( void );
};
