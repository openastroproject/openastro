/*****************************************************************************
 *
 * captureSettings.h -- class declaration
 *
 * Copyright 2013,2014,2016,2018
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

#include "trampoline.h"

typedef struct {
	int			useUtVideo;
	int			indexDigits;
	int			windowsCompatibleAVI;
} captureConfig;

extern captureConfig captureConf;

class CaptureSettings : public QWidget
{
  Q_OBJECT

  public:
    			CaptureSettings ( QWidget*, int, trampolineFuncs* );
    			~CaptureSettings();
    void		storeSettings ( void );

  private:
    QPushButton*	indexResetButton;
    QCheckBox*		winAVIBox;
    QCheckBox*		utVideoBox;
    QVBoxLayout*	vLayout;
    QHBoxLayout*	hLayout;
    QGridLayout*	grid;
    QHBoxLayout*	spinboxLayout;
    QLabel*		indexSizeLabel;
    QSpinBox*		indexSizeSpinbox;
		int						videoFormats;
		trampolineFuncs*	trampolines;

  public slots:
    void		resetIndex ( void );
};
