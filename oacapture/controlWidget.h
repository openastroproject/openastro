/*****************************************************************************
 *
 * controlWidget.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2019
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

#if HAVE_QT5
#include <QtWidgets>
#endif
#include <QtCore>
#include <QtGui>

extern "C" {
#include <openastro/camera.h>
}

#include "configuration.h"

class ControlWidget : public QGroupBox
{
  Q_OBJECT

  public:
    			ControlWidget ( QWidget* parent = 0 );
    			~ControlWidget();
    void		configure();
    void		updateFrameRates ( void );
    void		enableFPSControl ( int );
    int			getFPSNumerator ( void );
    int			getFPSDenominator ( void );
    void		updateFromConfig ( void );
    int			getSpinboxMinimum ( int );
    int			getSpinboxMaximum ( int );
    int			getSpinboxValue ( int );
    int			getSpinboxStep ( int );
    void		updateSpinbox ( int, int );
    void		updateCheckbox ( int, int );
    QStringList		getFrameRates ( void );
    unsigned int	getFrameRateIndex ( void );
    void		updateFrameRate ( int );
    void		disableAutoControls ( void );
    unsigned int	getCurrentGain ( void );
    unsigned int	getCurrentExposure ( void );
    QString		exposureIntervalString ( void );
		void		resetCamera ( void );

  public slots:
    void		exposureMenuChanged ( int );
    void		intervalMenuChanged ( int );
    void		frameRateChanged ( void );
    void		updateExposure ( int );
    void		updateGain ( int );
    void		updateSelectableControl ( int );
    void		updateSelectableCheckbox ( int );
    void		setSelectableControl1 ( int );
    void		setSelectableControl2 ( int );
    void		doAutoControlUpdate ( void );

  private:
    QSlider*		exposureSlider;
    QSlider*		framerateSlider;
    QSpinBox*		exposureSpinbox;
    QLabel*		exposureLabel;
    QLabel*		expRangeLabel;
    QLabel*		framerateLabel;
    QLabel*		autoLabel;
    QComboBox*		expMenu;
    QComboBox*		framerateMenu;
    int			minSettings[8];
    int			maxSettings[8];
    int			usingAbsoluteExposure; // avoids checking camera
    int			ignoreExposureMenuChanges;
    int			ignoreFrameRateChanges;
    int			ignoreSelectableControlChanges;
    int			ignoreGainChanges;
    int			ignoreIntervalChanges;
    int			ignoreExposureChanges;
    QList<int>		frameRateNumerator;
    QList<int>		frameRateDenominator;
    QLabel*		gainLabel;
    QGridLayout*	grid;
    int			theoreticalFPSNumerator;
    int			theoreticalFPSDenominator;
    float		theoreticalFPS;
    int			useExposureDropdown;
    QSlider*		selectableControlSlider[ OA_CAM_CTRL_LAST_P1 ];
    QSpinBox*		selectableControlSpinbox[ OA_CAM_CTRL_LAST_P1 ];
    QCheckBox*		selectableControlCheckbox[ OA_CAM_CTRL_LAST_P1 ];
    QComboBox*		selectableControlMenu[2];
    int			selectableControlsAllowed[2][ OA_CAM_CTRL_LAST_P1 ];
    int			selectableControlIndexes[2][ OA_CAM_CTRL_LAST_P1 ];
    QSignalMapper*	sliderSignalMapper;
    QSignalMapper*	checkboxSignalMapper;
    QComboBox*          intervalSizeMenu;
    QHBoxLayout*        intervalBox;
    QStringList		intervalsList;
    QList<int>		intervalMultipliers;
    QList<int>		enabledIntervals;
    int64_t		intervalMultiplier;

    void		_setSelectableControl ( int, int );
    void		_doFrameRateChange ( int, int );
};
