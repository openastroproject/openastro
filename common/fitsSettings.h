/*****************************************************************************
 *
 * fitsSettings.h -- class declaration
 *
 * Copyright 2015,2016,2017,2018
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
  QString		observer;
  QString		telescope;
  QString		instrument;
  QString		object;
  QString		comment;
  QString		focalLength;
  QString		apertureDia;
  QString		apertureArea;
  QString		pixelSizeX;
  QString		pixelSizeY;
  QString		subframeOriginX;
  QString		subframeOriginY;
  QString		siteLatitude;
  QString		siteLongitude;
  QString		filter;
} fitsConfig;

extern fitsConfig fitsConf;

class FITSSettings : public QWidget
{
  Q_OBJECT

  public:
    			FITSSettings ( QWidget*, trampolineFuncs* );
    			~FITSSettings();
    void		storeSettings ( void );

  private:
    QGridLayout*	grid;
    QLabel*		commentLabel;
    QLabel*		instrumentLabel;
    QLabel*		objectLabel;
    QLabel*		observerLabel;
    QLabel*		telescopeLabel;
    QLabel*             focalLengthLabel;
    QLabel*             apertureDiaLabel;
    QLabel*             apertureAreaLabel;
    QLabel*             pixelSizeXLabel;
    QLabel*             pixelSizeYLabel;
    QLabel*             subframeOriginXLabel;
    QLabel*             subframeOriginYLabel;
    QLabel*             siteLatitudeLabel;
    QLabel*             siteLongitudeLabel;
    QLabel*             filterLabel;
    QLineEdit*		commentInput;
    QLineEdit*		instrumentInput;
    QLineEdit*		objectInput;
    QLineEdit*		observerInput;
    QLineEdit*		telescopeInput;
    QLineEdit*          focalLengthInput;
    QLineEdit*          apertureDiaInput;
    QLineEdit*          apertureAreaInput;
    QLineEdit*          pixelSizeXInput;
    QLineEdit*          pixelSizeYInput;
    QLineEdit*          subframeOriginXInput;
    QLineEdit*          subframeOriginYInput;
    QLineEdit*          siteLatitudeInput;
    QLineEdit*          siteLongitudeInput;
    QLineEdit*          filterInput;
		trampolineFuncs*		trampolines;
};
