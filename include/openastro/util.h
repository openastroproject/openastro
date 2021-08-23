/*****************************************************************************
 *
 * util.h -- utility functions header
 *
 * Copyright 2015,2021 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_UTIL_H
#define OPENASTRO_UTIL_H

#include <stdarg.h>

#include <openastro/openastro.h>

/*
 * Double-linked list management
 */

struct  dl_list {
  void*			head;
  void*			tail;
  int			length;
  pthread_mutex_t	listMutex;
};

typedef struct dl_list*	DL_LIST;

extern DL_LIST		oaDLListCreate ( void );
extern int		oaDLListAddToHead ( DL_LIST, void* );
extern int		oaDLListAddToTail ( DL_LIST, void* );
extern void*		oaDLListRemoveFromHead ( DL_LIST );
extern void*		oaDLListRemoveFromTail ( DL_LIST );
extern void		oaDLListDelete ( DL_LIST, int );
extern int		oaDLListIsEmpty ( DL_LIST );
extern void*		oaDLListPeekAt ( DL_LIST, int );
extern void*		oaDLListRemoveAt ( DL_LIST, int );

/*
 * Logging management
 */

#define	OA_LOG_NONE					0

#define	OA_LOG_ERROR				1
#define	OA_LOG_WARN					2
#define	OA_LOG_INFO					3
#define	OA_LOG_DEBUG				4

#define	OA_LOG_APP					0x0001
#define	OA_LOG_CAMERA				0x0002
#define	OA_LOG_FILTERWHEEL	0x0004
#define	OA_LOG_TIMER				0x0008
#define	OA_LOG_DEMOSAIC			0x0010
#define	OA_LOG_IMGPROC			0x0020
#define	OA_LOG_VIDEO				0x0040
#define	OA_LOG_EPHEMERIS		0x0080
#define OA_LOG_NUM_TYPES		8
#define	OA_LOG_TYPE_ALL			0x00ff

extern void		oaSetLogLevel ( unsigned int );
extern int		oaSetLogType ( unsigned int );
extern int		oaAddLogType ( unsigned int );
extern int		oaRemoveLogType ( unsigned int );
extern int		oaSetLogFile ( const char* );
extern int		oaLogError ( unsigned int, const char*, ... );
extern int		oaLogErrorNoNL ( unsigned int, const char*, ... );
extern int		oaLogErrorCont ( unsigned int, const char*, ... );
extern int		oaLogErrorEndline ( unsigned int );
extern int		oaLogWarning ( unsigned int, const char*, ... );
extern int		oaLogInfo ( unsigned int, const char*, ... );
extern int		oaLogInfoNoNL ( unsigned int, const char*, ... );
extern int		oaLogInfoCont ( unsigned int, const char*, ... );
extern int		oaLogInfoEndline ( unsigned int );
extern int		oaLogDebug ( unsigned int, const char*, ... );
extern int		oaLogDebugNoNL ( unsigned int, const char*, ... );
extern int		oaLogDebugCont ( unsigned int, const char*, ... );
extern int		oaLogDebugEndline ( unsigned int );

#endif	/* OPENASTRO_UTIL_H */
