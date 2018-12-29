/*****************************************************************************
 *
 * brightstarfw.c -- Find Brightstar filter wheels
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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

#include <libudev.h>
#include <errno.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "brightstarfw.h"

int
oaBrightstarGetFilterWheels ( FILTERWHEEL_LIST* deviceList )
{
  // want to find all the usb serial devices and see which have
  // a VID:PID of 0403:6001 or VID:PID 0403:6015.  Further check
  // that the serial number for the device is either A4008T44 or
  // DA001CEZ

  struct udev*			udev;
  struct udev_enumerate*	enumerate;
  struct udev_list_entry*	devices;
  struct udev_list_entry*	dev_list_entry;
  struct udev_device*		dev;
  const char*			path;
  const char*			deviceNode;
  const char*			vid;
  const char*			pid;
  uint8_t			haveWheel;
  int				numFound = 0, ret;
  oaFilterWheelDevice*		wheel;
  char				buffer[200];
  DEVICE_INFO*			_private;

  if (!( udev = udev_new())) {
    fprintf ( stderr, "can't get connection to udev\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  enumerate = udev_enumerate_new ( udev );
  udev_enumerate_add_match_subsystem ( enumerate, "tty" );
  udev_enumerate_scan_devices ( enumerate );
  devices = udev_enumerate_get_list_entry ( enumerate );
  udev_list_entry_foreach ( dev_list_entry, devices ) {
		
    path = udev_list_entry_get_name ( dev_list_entry );
    dev = udev_device_new_from_syspath ( udev, path );
    deviceNode = udev_device_get_devnode ( dev );

    haveWheel = 0;

    if (( dev = udev_device_get_parent_with_subsystem_devtype ( dev, "usb",
        "usb_device" ))) {
      vid = udev_device_get_sysattr_value ( dev, "idVendor" );
      pid = udev_device_get_sysattr_value ( dev, "idProduct" );
      haveWheel = 0;
      if ( !strcmp ( "04d8", vid ) && !strcmp ( "f99f", pid )) {
        haveWheel = 1;
      }

      if ( haveWheel ) {
        if (!( wheel = malloc ( sizeof ( oaFilterWheelDevice )))) {
          udev_device_unref ( dev );
          udev_enumerate_unref ( enumerate );
          udev_unref ( udev );
          return -OA_ERR_MEM_ALLOC;
        }
        if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
          udev_device_unref ( dev );
          udev_enumerate_unref ( enumerate );
          udev_unref ( udev );
          ( void ) free (( void* ) wheel );
          return -OA_ERR_MEM_ALLOC;
        }
        _oaInitFilterWheelDeviceFunctionPointers ( wheel );
        wheel->interface = OA_FW_IF_BRIGHTSTAR;
        wheel->_private = _private;
        ( void ) strcpy ( buffer, "Brightstar Quantum" );
        ( void ) strcpy ( wheel->deviceName, buffer );
        _private->devIndex = numFound++;
        wheel->initFilterWheel = oaBrightstarInitFilterWheel;
        ( void ) strncpy ( _private->sysPath, deviceNode, PATH_MAX );
        if (( ret = _oaCheckFilterWheelArraySize ( deviceList )) < 0 ) {
          ( void ) free (( void* ) wheel );
          ( void ) free (( void* ) _private );
          udev_device_unref ( dev );
          udev_enumerate_unref ( enumerate );
          udev_unref ( udev );
          return ret;
        }
        deviceList->wheelList[ deviceList->numFilterWheels++ ] = wheel;
      }
      udev_device_unref ( dev );
    }
  }
  udev_enumerate_unref ( enumerate );
  udev_unref ( udev );

  return numFound;
}


const char*
oaBrightstarWheelGetName ( oaFilterWheel* wheel )
{
  return wheel->deviceName;
}
