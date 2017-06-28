/*****************************************************************************
 *
 * histogramWidget.h -- class declaration
 *
 * Copyright 2013,2014,2016 James Fidell (james@openastroproject.org)
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
  public:
    			HistogramWidget();
    			~HistogramWidget();
    void		process ( void*, int, int );
    void		updateLayout();
    void		resetStats();
    void		stopStats();

    int			histogramMin;
    int			histogramMax;
    int			fullIntensity;

  protected:
    void		paintEvent ( QPaintEvent* );

  private:
    int			red[256];
    int			green[256];
    int			blue[256];
    int			*grey;
    int			colours;
    int			maxIntensity;
    int			maxRedIntensity;
    int			maxGreenIntensity;
    int			maxBlueIntensity;
    int			currentLayoutIsSplit;
    int			newLayoutIsSplit;
    int			showingThreeGraphs;
    int			doneProcess;
    int			statsEnabled;
    int			signalConnected;
};
