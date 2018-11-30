/*****************************************************************************
 *
 * filterSettings.h -- class declaration
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

#include "oa_common.h"

#ifdef HAVE_QT5
#include <QtWidgets>
#endif
#include <QtCore>
#include <QtGui>

extern "C" {
#include <openastro/filterwheel.h>
}

#include "trampoline.h"
#include "profile.h"

typedef struct {
	int							numFilters;
	QList<FILTER>		filters;
	int							filterSlots[MAX_FILTER_SLOTS];
	QList<int>			autorunFilterSequence;
	int							promptForFilterChange;
	int							interFilterDelay;
} filterConfig;

extern filterConfig filterConf;

class FilterSettings : public QWidget
{
  Q_OBJECT

  public:
    			FilterSettings ( QWidget*, trampolineFuncs* );
    			~FilterSettings();
    void		storeSettings ( void );
    QString		getSlotFilterName ( int );
    void		setSlotCount ( int );

  private:
    QListWidget*        list;
    QVBoxLayout*        vbox;
    QHBoxLayout*        hbox;
    QPushButton*        addButton;
    QPushButton*        removeButton;
    QLabel*             slotLabels[MAX_FILTER_SLOTS];
    QComboBox*          slotMenus[MAX_FILTER_SLOTS];
    QGridLayout*        slotGrid;
    QSignalMapper*	slotChangedMapper;
    int			listChanged;
    int			slotsChanged;
    int			filterWheelSlots;
		trampolineFuncs*		trampolines;

  public slots:
    void		addEntry ( void );
    void		removeEntry ( void );
    void		focusChanged ( QListWidgetItem*, QListWidgetItem* );
    void		itemChanged ( QListWidgetItem* );
    void		filterSlotChanged ( int );
};
