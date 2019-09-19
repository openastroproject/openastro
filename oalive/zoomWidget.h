/*****************************************************************************
 *
 * zoomWidget.h -- class declaration
 *
 * Copyright 2013,2014,2016,2019 James Fidell (james@openastroproject.org)
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

class ZoomWidget : public QGroupBox
{
  Q_OBJECT

  public:
    			ZoomWidget ( QWidget* parent = 0 );
    			~ZoomWidget();
    int			getZoomFactor ( void );

  public slots:
    void		changeZoomLevel ( int );
    void		zoomOnButton1 ( void );
    void		zoomOnButton2 ( void );
    void		zoomOnButton3 ( void );
    void		zoom1Action0 ( void );
    void		zoom1Action1 ( void );
    void		zoom1Action2 ( void );
    void		zoom1Action3 ( void );
    void		zoom1Action4 ( void );
    void		zoom1Action5 ( void );
    void		zoom1Action6 ( void );
    void		zoom1Action7 ( void );
    void		zoom1Action8 ( void );
    void		zoom2Action0 ( void );
    void		zoom2Action1 ( void );
    void		zoom2Action2 ( void );
    void		zoom2Action3 ( void );
    void		zoom2Action4 ( void );
    void		zoom2Action5 ( void );
    void		zoom2Action6 ( void );
    void		zoom2Action7 ( void );
    void		zoom2Action8 ( void );
    void		zoom3Action0 ( void );
    void		zoom3Action1 ( void );
    void		zoom3Action2 ( void );
    void		zoom3Action3 ( void );
    void		zoom3Action4 ( void );
    void		zoom3Action5 ( void );
    void		zoom3Action6 ( void );
    void		zoom3Action7 ( void );
    void		zoom3Action8 ( void );

  private:
    void		updateZoom1Button ( int );
    void		updateZoom2Button ( int );
    void		updateZoom3Button ( int );
    QSlider*		zoomSlider;
    QVBoxLayout*	box;
    QHBoxLayout*	menuBox;
    QHBoxLayout*	sliderBox;
    QToolButton* 	menu1Button;
    QToolButton*	menu2Button;
    QToolButton*	menu3Button;
    QMenu*		zoom1Menu;
    QMenu*		zoom2Menu;
    QMenu*		zoom3Menu;
    QAction*		zoomActions[3][9];
};
