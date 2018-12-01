/*****************************************************************************
 *
 * autorunSettings.h -- class declaration
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
	int			autorunCount;
	int			autorunDelay;
} autorunConfig;

extern autorunConfig autorunConf;

class AutorunSettings : public QWidget
{
  Q_OBJECT

  public:
    			AutorunSettings ( QWidget*, trampolineFuncs* );
    			~AutorunSettings();
    void		storeSettings ( void );
    void		setSlotName ( int, const QString& );
    void		setSlotCount ( int );

  private:
    QLabel*		numRunsLabel;
    QLabel*		delayLabel;
    QLabel*		delayLabel2;
    QLabel*		titleLabel;
    QLineEdit*		numRuns;
    QLineEdit*		delay;
    QIntValidator*	numRunsValidator;
    QIntValidator*	delayValidator;
    QGridLayout*	buttonGrid;
    QGridLayout*	filterGrid;
    QVBoxLayout*	mainBox;
    QPushButton*	enableButton;
    QHBoxLayout*	addFilterBox;
    QHBoxLayout*	addButtonBox;
    QHBoxLayout*	filterGridBox;
    QPushButton*	addButton;
    QList<QComboBox*>	filterMenus;
    QList<QPushButton*>	removeButtons;
    QList<QHBoxLayout*>	filterContainers;
    QSignalMapper*	removeButtonMapper;
    QCheckBox*		filterPromptOption;
    QLabel*		filterDelayLabel;
    QLineEdit*		filterDelay;
    QIntValidator*	filterDelayValidator;
    QHBoxLayout*	filterPromptBox;
    QHBoxLayout*	filterDelayBox;
    int			sequenceLength;
    int			filterWheelSlots;
		trampolineFuncs*	trampolines;
		QWidget*					parentWidget;

    void		addFilterWidgets ( int );

  public slots:
    void		enableAutorun ( void );
    void		addFilter ( void );
    void		removeFilter ( int );
};
