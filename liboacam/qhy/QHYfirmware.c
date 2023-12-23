/*****************************************************************************
 *
 * QHYfirmware.c -- QHY camera firmware management
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
#include <openastro/camera.h>
#include <openastro/errno.h>

#include "oacamprivate.h"
#include "QHYoacam.h"
#include "QHYfirmware.h"

typedef struct {
  unsigned short	vendorId;
  unsigned short	productId;
  unsigned short	twoStage;
  const char*		command;
} commandByUSBID;

commandByUSBID		firmwareLoadCommands[] = {
  { 0x1618, 0x0412, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY2.HEX" },
  { 0x1618, 0x0901, 1, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY5.HEX -s %s%s/QHY5LOADER.HEX" },
  { 0x1618, 0x1002, 1, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY5.HEX -s %s%s/QHY5LOADER.HEX" },
  { 0x0547, 0x1002, 1, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY5.HEX -s %s%s/QHY5LOADER.HEX" },
  { 0x04b4, 0x1002, 1, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY5.HEX -s %s%s/QHY5LOADER.HEX" },
  { 0x16c0, 0x081a, 1, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY5.HEX -s %s%s/QHY5LOADER.HEX" },
  { 0x1856, 0x0011, 1, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY5.HEX -s %s%s/QHY5LOADER.HEX" },
  { 0x1618, 0x0259, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY6.HEX" },
  { 0x16c0, 0x2980, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY6PRO.HEX" },
  { 0x1618, 0x4022, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY7.HEX" },
  { 0x1618, 0x6000, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY8.HEX" },
  { 0x1618, 0x6002, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY8PRO.HEX" },
  { 0x1618, 0x6004, 0, "%s%s -t fx2lp -q -p %d,%d -i %s%s/QHY8L.HEX" },
  { 0x1618, 0x8300, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY9.HEX" },
  { 0x1618, 0x8310, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY9L.HEX" },
  { 0x1618, 0x1000, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY10.HEX" },
  { 0x1618, 0x1100, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY11.HEX" },
  { 0x1618, 0x6740, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY21.HEX" },
  { 0x1618, 0x6940, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY22.HEX" },
  { 0x1618, 0x8140, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY23.HEX" },
  { 0x1618, 0x8613, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY16000.HEX" },
  { 0x1618, 0xb618, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/IMG0H.HEX" },
  { 0x1618, 0x2850, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/IMG2PRO.HEX" },
  { 0x1618, 0xb285, 0, "%s%s -t fx2lp -q -p %d,%d -i %s%s/IMG2S.HEX" },
  { 0x1618, 0x0005, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/SLAVEFIFO.HEX" },
  { 0x1618, 0x0920, 0, "%s%s -t fx2 -q -p %d,%d -i %s%s/QHY5II.HEX" }
};


int
oaQHYCameraDeviceLoadFirmware ( oaCameraDevice* device )
{
  int	       numCommands;
  unsigned int bus, addr;
  int          found, i, ret;
  char         cmd[ PATH_MAX+1 ];
  char*        path = "";
  DEVICE_INFO*	_private;

  *cmd = 0;
  if ( installPathRoot ) {
    path = installPathRoot;
    ( void ) strncpy ( cmd, installPathRoot, PATH_MAX );
  }
  ( void ) strncat ( cmd, FXLOAD_PATH, PATH_MAX );
  if ( access ( cmd, X_OK )) {
    return -OA_ERR_FXLOAD_NOT_FOUND;
  }

  numCommands = sizeof ( firmwareLoadCommands ) / ( sizeof ( commandByUSBID* ));
  _private = device->_private;
  bus = _private->devIndex >> 8;
  addr = _private->devIndex & 0xff;

  for ( i = 0, found = 0; i < numCommands && !found; i++ ) {
    if ( firmwareLoadCommands[i].vendorId == _private->vendorId &&
        firmwareLoadCommands[i].productId == _private->productId ) {
      found = 1;
      if ( firmwareLoadCommands[i].twoStage ) {
        ( void ) snprintf ( cmd, PATH_MAX, firmwareLoadCommands[i].command,
            path, FXLOAD_PATH, bus, addr, path, FIRMWARE_QHY_PATH, path,
            FIRMWARE_QHY_PATH );
      } else {
        ( void ) snprintf ( cmd, PATH_MAX, firmwareLoadCommands[i].command,
            path, FXLOAD_PATH, bus, addr, path, FIRMWARE_QHY_PATH );
      }
      ret = system ( cmd );
      sleep ( 2 ); // allow a little time for the firmware to load
    }
  }

  if ( !found ) {
    return -OA_ERR_FIRMWARE_UNKNOWN;
  }

  if ( ret ) {
    return -OA_ERR_FXLOAD_ERROR;
  }

  // Ok, so at this point we have a camera that has a different USB VID/PID
  // pair and almost certainly a different address on the USB bus as well.
  // Rescanning is probably the best we can sanely offer

  return -OA_ERR_RESCAN_BUS;
}
