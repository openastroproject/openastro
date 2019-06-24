/*****************************************************************************
 *
 * processingControls.h -- class declaration
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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
#include <QtGui>

#include "histogramWidget.h"


class ProcessingControls : public QWidget
{
  Q_OBJECT

  public:
    			ProcessingControls ( QWidget* );
    			~ProcessingControls();
    void		configure ( void );
		void		connectHistogramSignal ( void );

  private:
    QVBoxLayout*	controlBox;
    QLabel*		blackLevelLabel;
    QLabel*		whiteLevelLabel;
    QLabel*		brightnessLabel;
    QLabel*		contrastLabel;
    QLabel*		saturationLabel;
    QLabel*		gammaLabel;
    QSlider*		blackLevelSlider;
    QSlider*		whiteLevelSlider;
    QSlider*		brightnessSlider;
    QSlider*		contrastSlider;
    QSlider*		saturationSlider;
    QSlider*		gammaSlider;
		HistogramWidget*	histogram;

  public slots:
		void				blackLevelChanged ( void );
		void				whiteLevelChanged ( void );
		void				brightnessChanged ( void );
		void				contrastChanged ( void );
		void				saturationChanged ( void );
		void				gammaChanged ( void );
};
