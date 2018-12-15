/*****************************************************************************
 *
 * cameraSettings.h -- class declaration
 *
 * Copyright 2014,2015,2016,2018
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

extern "C" {
#include <openastro/camera.h>
}

#include "trampoline.h"
#include "cameraSettings.h"


typedef struct {
	int64_t				controlValues[OA_CAM_CTRL_MODIFIERS_P1][ OA_CAM_CTRL_LAST_P1 ];
	int						forceInputFrameFormat;
} cameraConfig;

extern cameraConfig cameraConf;


class CameraSettings : public QWidget
{
  Q_OBJECT

  public:
    			CameraSettings ( QWidget*, QWidget*, trampolineFuncs* );
    			~CameraSettings();
    void		configure ( void );
    void		storeSettings ( void );
    void		enableFlipX ( int );
    void		enableFlipY ( int );
    void		updateControl ( int, int );
    void		updateFrameRate ( int );
    void		reconfigureControl ( int );

  private:
    QVBoxLayout*	layout;
    QGridLayout*	sliderGrid;
    QGridLayout*	checkboxGrid;
    QGridLayout*	buttonGrid;
    QGridLayout*	menuGrid;
    QGridLayout*	unhandledGrid;
    QSignalMapper*	sliderSignalMapper;
    QSignalMapper*	checkboxSignalMapper;
    QSignalMapper*	buttonSignalMapper;
    QSignalMapper*	menuSignalMapper;
    QLabel*		controlLabel[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
    QSlider*		controlSlider[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
    QSpinBox*		controlSpinbox[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
    QCheckBox*		controlCheckbox[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
    QPushButton*	controlButton[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
    QComboBox*		controlMenu[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
    QPushButton*	defaultButton;
    QLabel*		autoLabel1;
    QLabel*		autoLabel2;
    QLabel*		onOffLabel1;
    QLabel*		onOffLabel2;
    int			controlType[ OA_CAM_CTRL_MODIFIERS_P1 ][ OA_CAM_CTRL_LAST_P1 ];
    QLabel*		frameRateLabel;
    QSlider*		frameRateSlider;
    QComboBox*		frameRateMenu;
    QCheckBox*		forceFrameFormat;
    QComboBox*		selectedFrameFormat;
    QHBoxLayout*	frameHBoxLayout;
		QWidget*					topWidget;
		trampolineFuncs*	trampolines;

  public slots:
    void		updateSliderControl ( int );
    void		updateCheckboxControl ( int );
    void		buttonPushed ( int );
    void		menuChanged ( int );
    void		frameRateChanged ( void );
    void		forceFrameFormatChanged ( int );
    void		selectedFrameFormatChanged ( int );
};


#define CONTROL_VALUE(c)  controlValues[OA_CAM_CTRL_MODIFIER(c)][OA_CAM_CTRL_MODE_BASE(c)]
