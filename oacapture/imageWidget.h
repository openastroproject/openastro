/*****************************************************************************
 *
 * imageWidget.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2018,2019
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

class ImageWidget : public QGroupBox
{
  Q_OBJECT

  public:
    			  ImageWidget ( QWidget* parent = 0 );
    			  ~ImageWidget();
    void		configure ( void );
    void		enableAllControls ( int );
    void    updateFromConfig ( void );

  public slots:
    void		cameraROIChanged ( int );
    void		setUserROI ( void );
    void		updateUserROI ( void );
    void		setCropSize ( void );
    void		updateFrameCrop ( void );
    void		resetResolution ( void );

  private:
    QGridLayout*        grid;
    QLabel*             cameraROILabel;
    QCheckBox*          userROI;
    QCheckBox*          cropRegion;
    QComboBox*	       	resMenu;
    QLineEdit*		      roiXSize;
    QLineEdit*		      roiYSize;
    QLabel*             roiBy;
    QPushButton*        roiButton;
    QHBoxLayout*        roiInputBox;
    QIntValidator*      roiXValidator;
    QIntValidator*      roiYValidator;
    QLineEdit*		      cropXSize;
    QLineEdit*		      cropYSize;
    QLabel*             cropBy;
    QPushButton*        cropButton;
    QHBoxLayout*        cropInputBox;
    QIntValidator*      cropXValidator;
    QIntValidator*      cropYValidator;
    QList<unsigned int>	XResolutions;
    QList<unsigned int>	YResolutions;
    int	            		ignoreResolutionChanges;
    void                doResolutionChange ( int );
};
