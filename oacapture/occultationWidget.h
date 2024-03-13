/*****************************************************************************
 *
 * occultationWidget.h -- class for occultation tools
 *
 * Copyright 2021,2023
 *		Dave Tucker (dave@dtucker.co.uk),
 *		James Fidell (james@openastroproject.org)
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

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QtCore>
#include <QtGui>

class OccultationWidget : public QWidget
{
  Q_OBJECT
  public:
    OccultationWidget ( const char*, QWidget* parent = 0 );
    ~OccultationWidget();
    void                enableBinningControl ( int );
    void configure(void);
		void	updateTriggerInterval ( int );
  private slots:
    void externalLEDCheckboxChanged ( int );
    void resetCaptureCounter ( void );
    void triggerIntervalChanged ( void );
  private:
    QGridLayout*	grid;
    QCheckBox*		reticle;
    QCheckBox*		externalLEDEnabled;
    QPushButton*	resetCaptureCounterButton;
    QPushButton*	captureTenFrames;
    QLabel*         interframeIntervalLabel;
    QLineEdit*		interframeInterval;
    QLabel*			triggerIntervalLabel;
    QLineEdit*		triggerInterval;
		QIntValidator*	intervalValidator;
	QSize			sizeHint() const;
	int				windowSizeX;
	int				windowSizeY;
};
