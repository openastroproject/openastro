/*****************************************************************************
 *
 * cameraWidget.h -- class declaration
 *
 * Copyright 2013,2014,2016,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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

class CameraWidget : public QGroupBox
{
  Q_OBJECT

  public:
    			CameraWidget ( QWidget* parent = 0 );
    			~CameraWidget();
    void                configure ( void );
    void                enableBinningControl ( int );
    void                updateFromConfig ( void );
    void                clearTemperature ( void );
    void                resetTemperatureLabel ( void );
    void                showFPSMaxValue ( int );
    void                clearFPSMaxValue ( void );
    void		updateForceFrameFormat ( unsigned int, unsigned int );

  private:
    QCheckBox*		binning2x2;
    QGridLayout*	grid;
    QComboBox*		inputFormatMenu;
    QLabel*		inputFormatLabel;
    QList<int>		inputFormatList;
    QLabel*             tempLabel;
    QLabel*             fpsMaxLabel;
    QLabel*             fpsActualLabel;
    QLabel*             fpsMaxValue;
    QLabel*             fpsActualValue;
    QLabel*             tempValue;
    int                 updateTemperatureLabel;

  public slots:
    void		setBinning ( int );
    void		changeFrameFormat ( int );
    void                setActualFrameRate ( double );
    void                setTemperature ( void );
};
