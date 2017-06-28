/*****************************************************************************
 *
 * util.h -- utility functions header
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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

#include <openastro/openastro.h>

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

#endif	/* OPENASTRO_UTIL_H */
