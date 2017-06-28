/*****************************************************************************
 *
 * sxIO.c -- Starlight Xpress filter wheel IO routines
 *
 * Copyright 2014,2015 James Fidell (james@openastroproject.org)
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

#include <hidapi.h>

#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "sxfw.h"

#define	wheelState		wheel->_sx


int
_sxWheelWrite ( PRIVATE_INFO* wheelInfo, unsigned char* buffer )
{
  int		res;

  if (( res = hid_write ( wheelInfo->hidHandle, buffer, 2 )) != 2 ) {
    return -1;
  }
  return 0;
}


int
_sxWheelRead ( PRIVATE_INFO* wheelInfo, unsigned char* buffer )
{
  int		res;

  buffer[0] = 0;
  buffer[1] = 0;
  if (( res = hid_read ( wheelInfo->hidHandle, buffer, 2 )) != 2 ) {
    return -1;
  }

  return 0;
}
