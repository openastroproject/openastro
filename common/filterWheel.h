/*****************************************************************************
 *
 * filterwheel.h -- class declaration
 *
 * Copyright 2014,2016,2018,2019
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

extern "C" {
#include <openastro/filterwheel.h>
}

#include "trampoline.h"


class FilterWheel // : public QObject
{
  public:
    			FilterWheel ( trampolineFuncs* );
    			~FilterWheel();
    int			listConnected ( oaFilterWheelDevice*** );
    void    releaseInfo ( oaFilterWheelDevice** );
    int			initialise ( oaFilterWheelDevice* );
    void		disconnect ( void );
    void		warmReset ( void );
    void		coldReset ( void );
    const char*		name ( void );
    int			isInitialised ( void );
    int			hasWarmReset ( void );
    int			hasColdReset ( void );
    int			hasSpeedControl ( void );
    int			selectFilter ( int );
    int			numSlots ( void );
    int			getSpeed ( unsigned int* );
    int			setSpeed ( unsigned int, int );
    void		updateSearchFilters ( int );
    void		updateAllSearchFilters ( void );

    void		populateControlValue ( oaControlValue*, uint32_t,
				int64_t );
    int64_t		unpackControlValue ( oaControlValue* );

  private:
    int			initialised;
    oaFilterWheel*	wheelContext;
		trampolineFuncs*	trampolines;
};
