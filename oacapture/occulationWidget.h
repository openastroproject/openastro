/*****************************************************************************
 *
 * occulationWidget.h -- class for occulation tools
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

#pragma once

#include <oa_common.h>

#if HAVE_QT5
#include <QtWidgets>
#endif
#include <QtCore>
#include <QtGui>

class OcculationWidget : public QWidget
{
  Q_OBJECT
  public:
    OcculationWidget ( const char*, QWidget* parent = 0 );
    ~OcculationWidget();
    void                enableBinningControl ( int );
    void configure(void);
  public slots:
    void setBinning (int);
  private slots:
    void externalLEDCheckboxChanged ( int );
    void resetCaptureCounter ( void );
  private:
    QGridLayout*	grid;
    QCheckBox*		binning2x2;
    QCheckBox*		reticle;
    QCheckBox*		externalLEDEnabled;
    QPushButton*	resetCaptureCounterButton;
    QPushButton*	captureTenFrames;
    QLabel* 		exposureTimeLabel;
    QLineEdit* 		exposureTime;
    QComboBox*		exposureTimeInterval;
    QLabel*         interframeIntervalLabel;
    QComboBox*		interframeIntervalInterval;
    QLineEdit*		interframeInterval;
    QLabel*			triggerIntervalLabel;
    QComboBox*		triggerIntervalInterval;
    QLineEdit*		triggerInterval;
    QStringList		intervalsList;
	QList<int>		intervalMultipliers;
	int64_t			intervalMultiplier;
	QSize			sizeHint() const;
	int				windowSizeX;
	int				windowSizeY;
};
