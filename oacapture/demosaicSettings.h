/*****************************************************************************
 *
 * demosaicSettings.h -- class declaration
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


class DemosaicSettings : public QWidget
{
  public:
    			DemosaicSettings ( QWidget* );
    			~DemosaicSettings();
    void		storeSettings ( void );
    void		updateCFASetting ( void );

  private:
    QLabel*             demosaicLabel;
    QCheckBox*		previewBox;
    QCheckBox*		outputBox;
    QVBoxLayout*	box;
    QLabel*             cfaLabel;
    QButtonGroup*       cfaButtons;
    QRadioButton*       rggbButton;
    QRadioButton*       bggrButton;
    QRadioButton*       grbgButton;
    QRadioButton*       gbrgButton;
    QRadioButton*       autoButton;
    QLabel*             methodLabel;
    QButtonGroup*       methodButtons;
    QRadioButton*       nnButton;
    QRadioButton*       bilinearButton;
    QRadioButton*       smoothHueButton;
    QRadioButton*       vngButton;
};
