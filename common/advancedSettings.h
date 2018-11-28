/*****************************************************************************
 *
 * advancedSettings.h -- class declaration
 *
 * Copyright 2014,2016 James Fidell (james@openastroproject.org)
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

extern "C" {
#include <openastro/userConfig.h>
}

#include "trampoline.h"


class AdvancedSettings : public QWidget
{
  Q_OBJECT

  public:
    			AdvancedSettings ( QWidget*, int, int, trampolineFuncs* );
    			~AdvancedSettings();

  private:
    QGridLayout*	grid;
    QHBoxLayout*	inputBoxes;
    QVBoxLayout*	vbox;
    QHBoxLayout*        buttonBox;
    QPushButton*        cancelButton;
    QPushButton*        saveButton;
    QPushButton*        addButton;
    QLabel*		vidPidTitle1;
    QLabel*		vidPidTitle2;
    QLabel*		manufacturerTitle1;
    QLabel*		manufacturerTitle2;
    QLabel*		productTitle1;
    QLabel*		productTitle2;
    QLabel*		serialTitle1;
    QLabel*		serialTitle2;
    QLabel*		fsPathTitle1;
    QLabel*		fsPathTitle2;
    QLineEdit*		vidPidInput;
    QLineEdit*		manufacturerInput;
    QLineEdit*		productInput;
    QLineEdit*		serialInput;
    QLineEdit*		fsPathInput;
    unsigned int	configFlags;
    int			interfaceType;
    int			deviceType;
    QList<userDeviceConfig> editedList;
    QList<int>		rowList;
    int			addedRows;
    QSignalMapper*	deleteMapper;
		trampolineFuncs*	trampolines;

  public slots:
    void		inputBoxesChanged ( void );
    void		addFilterToGrid ( void );
    void		deleteFilter ( int );
    void		saveFilters ( void );
};
