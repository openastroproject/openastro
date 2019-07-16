/*****************************************************************************
 *
 * stackingControls.h -- class declaration
 *
 * Copyright 2015,2016,2019 James Fidell (james@openastroproject.org)
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
#include <QtGui>

extern "C" {
#include <openastro/camera.h>
}

#define	OA_STACK_NONE					0
#define	OA_STACK_SUM					1
#define	OA_STACK_MEAN					2
#define	OA_STACK_MEDIAN				3
#define	OA_STACK_MAXIMUM			4
#define	OA_STACK_KAPPA_SIGMA	5


class StackingControls : public QWidget
{
  Q_OBJECT

  public:
    			StackingControls ( QWidget* );
    			~StackingControls();

  private:
    QLabel*							methodLabel;
    QComboBox*					stackingMethodMenu;
    QGridLayout*				grid;
		QLabel*							kappaLabel;
		QLineEdit*					kappaInput;
		QDoubleValidator*		kappaValidator;

  public slots:
    void		stackingMethodChanged ( int );
    void		updateKappaValue ( void );
};
