/*****************************************************************************
 *
 * common.h -- common liboacam header bits
 *
 * Copyright 2014,2018,2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_COMMON_H
#define OA_COMMON_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_CREAT64 || HAVE_LSEEK64
#define _LARGEFILE64_SOURCE
#endif

#include <stdio.h>
#include <sys/types.h>

#if STDC_HEADERS
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#elif HAVE_STRINGS_H
#include <strings.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif
#ifndef errno
extern int errno;
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#endif

#endif /* OA_COMMON_H */
