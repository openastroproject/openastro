/*****************************************************************************
 *
 * histogramWidget.h -- class declaration
 *
 * Copyright 2013,2014,2016,2017,2019
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


class HistogramWidget : public QWidget
{
	Q_OBJECT

  public:
    			HistogramWidget ( const char*, QWidget* parent = 0 );
    			~HistogramWidget();
    void		process ( void*, unsigned int, unsigned int,
			    unsigned int, int );
    void		updateLayout();
    void		resetStats();
    void		stopStats();
		QSize		sizeHint() const;

    static int			histogramMin;
    static int			histogramMax;
    static int			fullIntensity;

  protected:
    void		paintEvent ( QPaintEvent* );

  private:
    static int			red[256];
    static int			green[256];
    static int			blue[256];
    static int			*grey;
    static int			colours;
    static int			maxIntensity;
    static int			minIntensity;
    static int			maxRedIntensity;
    static int			maxGreenIntensity;
    static int			maxBlueIntensity;
    static int			currentLayoutIsSplit;
    static int			newLayoutIsSplit;
    static int			showingThreeGraphs;
    static int			doneProcess;
    static int			statsEnabled;
		int							windowSizeX;
		int							windowSizeY;

    void		_processRGBHistogram ( void*, unsigned int,
			    unsigned int, unsigned int, int );
    void		_processGreyscaleHistogram ( void*, unsigned int,
			    unsigned int, unsigned int, int );
    void		_processMosaicHistogram ( void*, unsigned int,
			    unsigned int, unsigned int, int );
};
