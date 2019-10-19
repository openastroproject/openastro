/*****************************************************************************
 *
 * timer.c -- camera library timer
 *
 * Copyright 2019
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

#include <oa_common.h>

#include <pthread.h>
#include <stdint.h>

#include <openastro/util.h>

#include "oacamprivate.h"
#include "sharedState.h"


void*		_oacamTimerThread ( void* param );


int
oacamStartTimer ( uint64_t delayusec, void* param )
{
	SHARED_STATE*		cameraInfo = param;
	struct timeval	now;
	uint64_t				delaySec;
	uint64_t				nanoSecs;
	uint64_t				delayNanoSecs;

	gettimeofday ( &now, 0 );
	delaySec = delayusec / 1000000;
	nanoSecs = ( delayusec - delaySec * 1000000 ) * 1000;
	cameraInfo->timerEnd.tv_sec = now.tv_sec + delaySec;
	delayNanoSecs = now.tv_usec * 1000 + nanoSecs;
	if ( delayNanoSecs > 1000000000 ) {
		cameraInfo->timerEnd.tv_sec++;
		delayNanoSecs -= 1000000000;
	}
	cameraInfo->timerEnd.tv_nsec = delayNanoSecs;

	if ( !cameraInfo->stopControllerThread ) {
		if ( pthread_create ( &( cameraInfo->timerThread ), 0, _oacamTimerThread,
				( void* ) cameraInfo )) {
			return -OA_ERR_SYSTEM_ERROR;
		}
		cameraInfo->timerActive = 1;
	}

	return OA_ERR_NONE;
}


void*
_oacamTimerThread ( void* param )
{
	SHARED_STATE*		cameraInfo = param;
	int							ret;

	pthread_mutex_lock ( &( cameraInfo->timerMutex ));
	ret = pthread_cond_timedwait ( &( cameraInfo->timerState ),
			&( cameraInfo->timerMutex ), &( cameraInfo->timerEnd ));
	pthread_mutex_unlock ( &( cameraInfo->timerMutex ));

	if ( ret ) {
		if ( ret == ETIMEDOUT && !cameraInfo->stopControllerThread ) {
			cameraInfo->timerCallback ( cameraInfo );
		} else {
			fprintf ( stderr, "timer condition failed with error %d\n", ret );
		}
	}

	cameraInfo->timerActive = 0;
	return 0;
}


void
oacamAbortTimer ( void* param )
{
	SHARED_STATE*		cameraInfo = param;

	pthread_cond_signal ( &( cameraInfo->timerState ));
}
