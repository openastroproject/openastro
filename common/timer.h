/*****************************************************************************
 *
 * timer.h -- class declaration
 *
 * Copyright 2015,2016,2017,2018,2019
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
#include "QtGui"

extern "C" {
#include <openastro/timer.h>
#include <openastro/ptr.h>
}

#include "trampoline.h"


class Timer
{
  public:
    			Timer ( trampolineFuncs* );
    			~Timer();
    int			listConnected ( oaTimerDevice*** );
    void		releaseInfo ( oaTimerDevice** );
    int			initialise ( oaTimerDevice* );
    void		disconnect ( void );
    void		reset ( void );
    int			start ( void );
    const char*		name ( void );
    int			isInitialised ( void );
    int			isRunning ( void );
    int			hasReset ( void );
    int			hasSync ( void );
    int			hasGPS ( void );
    int			hasControl ( int );
    void		updateSearchFilters ( int );
    void		updateAllSearchFilters ( void );
    oaTimerStamp*	readTimestamp ( void );
    int			readGPS ( double*, double*, double*, int );

    void		populateControlValue ( oaControlValue*, uint32_t,
				int64_t );
    int64_t		unpackControlValue ( oaControlValue* );
    int			setControl ( int, int64_t );
		int			readControl ( int );
  private:
    int			initialised;
    oaTimer*		timerContext;
		trampolineFuncs*	trampolines;
};
