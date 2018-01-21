/*****************************************************************************
 *
 * atikSerialIO-ftdi.c -- Atik serial camera IO routines (libftdi1)
 *
 * Copyright 2014,2016 James Fidell (james@openastroproject.org)
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

#include <errno.h>
#if HAVE_FTDI_H
#include <ftdi.h>
#else
#include <libftdi1/ftdi.h>
#endif
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "atikSerial.h"
#include "atikSerialoacam.h"


int
_atikFTDISerialCamWrite ( AtikSerial_STATE* cameraInfo,
    const unsigned char* buffer, int len )
{
  struct ftdi_context* ftdiContext = cameraInfo->ftdiContext;

  if ( ftdi_usb_purge_buffers ( ftdiContext )) {
    fprintf ( stderr, "FTDI buffer purge failed\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if ( ftdi_write_data ( ftdiContext, buffer, len ) != len ) {
    return -OA_ERR_CAMERA_IO;
  }
  return OA_ERR_NONE;
}


int
_atikFTDISerialCamRead ( AtikSerial_STATE* cameraInfo,
    unsigned char* buffer, int maxlen )
{
  struct ftdi_context* ftdiContext = cameraInfo->ftdiContext;
  unsigned char *p = (unsigned char*) buffer;
  int done = 0, len = 0, tries, bytes;

  // libftdi1 doesn't have a blocking read, so for the time being we
  // loop for up to ten seconds (actually 100 tries) attempting to read
  // from the target device with a 100ms delay between each attempt.  Any
  // valid read resets the loop timer.
  // It's a bit scabby, but it appears to do the job for the time being.
  //
  // FIX ME
  // Should really replace this with select() or similar?
  do {
    tries = 0;
    do {
      if (!( bytes = ftdi_read_data ( ftdiContext, p, 1 ))) {
        tries++;
        usleep ( 100000 );
      }
    } while ( !bytes && tries < 100 );
    if ( bytes != 1 ) {
      return -OA_ERR_CAMERA_IO;
    }
    len++;
    p++;
  } while ( !done && len < maxlen );

  return len;
}


int
_atikFTDISerialCamReadToZero ( AtikSerial_STATE* cameraInfo,
    unsigned char* buffer, unsigned int max )
{
  struct ftdi_context* ftdiContext = cameraInfo->ftdiContext;
  unsigned char *p = (unsigned char*) buffer;
  int done = 0, tries, bytes;
  unsigned int len = 1;

  // libftdi1 doesn't have a blocking read, so for the time being we
  // loop for up to ten seconds (actually 100 tries) attempting to read
  // from the target device with a 100ms delay between each attempt.  Any
  // valid read resets the loop timer.
  // It's a bit scabby, but it appears to do the job for the time being.
  //
  // FIX ME
  // Should really replace this with select() or similar?
  do {
    tries = 0;
    do {
      if (!( bytes = ftdi_read_data ( ftdiContext, p, 1 ))) {
        tries++;
        usleep ( 100000 );
      }
    } while ( !bytes && tries < 100 );
    if ( bytes != 1 ) {
      return -OA_ERR_CAMERA_IO;
    }
    if ( !*p++ ) {
      done = 1;
    }
    len++;
  } while ( !done && len < max );

  if ( !done )
    return -1;

  return len - 1;
}


int
_atikFTDISerialCamReadBlock ( AtikSerial_STATE* cameraInfo,
    unsigned char* buffer, int len )
{
  return ( _atikFTDISerialCamRead ( cameraInfo, buffer, len ));
}

