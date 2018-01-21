/*****************************************************************************
 *
 * V4L2ioctl.c -- ioctl interface for libv4l2
 *
 * Copyright 2013,2014 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBV4L2

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <linux/videodev2.h>
#include <linux/sysctl.h>
#include <libv4l2.h>

#include "V4L2ioctl.h"

int
v4l2ioctl ( int fd, int req, void *arg )
{
  int r;

  do {
    r = v4l2_ioctl ( fd, req, arg );
  } while ( -1 == r && ( EINTR == errno || EAGAIN == errno ));
  return ( -1 == r ) ? -1 : 0;
}

#endif /* HAVE_LIBV4L2 */
