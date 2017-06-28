/*****************************************************************************
 *
 * ptr-udev.c -- Find PTR devices using (Linux) udev
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

#include <oa_common.h>

#include <libudev.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>
#include <sys/select.h>

#include <openastro/util.h>
#include <openastro/ptr.h>

#include "oaptrprivate.h"
#include "unimplemented.h"
#include "ptr.h"

int
oaPTREnumerate ( PTR_LIST* deviceList )
{
  int				numFound = 0, entry, ret;

  // want to find all the usb serial devices and see which have
  // a VID:PID of 04d8:00df.  Further check that the first interface
  // has is USB class 2, subclass 2, protocol 1

  struct udev*			udev;
  struct udev_enumerate*	enumerate;
  struct udev_list_entry*	devices;
  struct udev_list_entry*	dev_list_entry;
  struct udev_device*		dev;
  const char*			serialNo;
  const char*			manufacturer;
  const char*			product;
  const char*			path;
  const char*			deviceNode;
  const char*			vid;
  const char*			pid;
  uint8_t			havePTR;
  oaPTRDevice*			ptr;
  char				buffer[512];
  char				nameBuffer[512];
  char*				namePtr;
  char*				endPtr;
  DEVICE_INFO*			_private;
  struct termios		tio;
  int				ptrDesc, numRead, result, i;
  fd_set			readable;
  struct timeval		timeout;


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

    havePTR = 0;

    if (( dev = udev_device_get_parent_with_subsystem_devtype ( dev, "usb",
        "usb_device" ))) {
      vid = udev_device_get_sysattr_value ( dev, "idVendor" );
      pid = udev_device_get_sysattr_value ( dev, "idProduct" );
      serialNo = udev_device_get_sysattr_value ( dev, "serial" );
      havePTR = 0;
      if ( !strcmp ( "04d8", vid ) && !strcmp ( "00df", pid )) {
        havePTR = 1;
      }

      // Check the user-defined entries if required.  The sense here is to
      // accept a match wherever the configured values match the USB data.
      if ( !havePTR && ptrConfigEntries ) {
        manufacturer = udev_device_get_sysattr_value ( dev, "manufacturer" );
        product = udev_device_get_sysattr_value ( dev, "product" );
        userDeviceConfig* confp = ptrConfig;
        for ( entry = 0; entry < ptrConfigEntries && !havePTR; entry++ ) {
          char vidStr[8], pidStr[8];
          if ( confp->vendorId && confp->productId ) {
            sprintf ( vidStr, "%04x", confp->vendorId );
            sprintf ( pidStr, "%04x", confp->productId );
          } else {
            vidStr[0] = pidStr[0] = 0;
          }
          havePTR = 1;
          if (( *vidStr && strcasecmp ( vidStr, vid )) ||
              ( *pidStr && strcasecmp ( pidStr, pid )) ||
              ( confp->manufacturer && *( confp->manufacturer ) &&
              strcasecmp ( confp->manufacturer, manufacturer )) ||
              ( confp->product && *( confp->product ) &&
              strcasecmp ( confp->product, product )) ||
              ( confp->serialNo && *( confp->serialNo ) &&
              strcasecmp ( confp->serialNo, serialNo ))) {
            havePTR = 0;
          }
          confp += sizeof ( userDeviceConfig );
        }
      }

      if ( havePTR ) {

        // At this point we want to lock the serial device, open it
        // and run inquiries to get the product name and firmware version

        if (( ptrDesc = open ( deviceNode, O_RDWR | O_NOCTTY )) < 0 ) {
          fprintf ( stderr, "Can't open %s read-write, errno = %d\n",
              deviceNode, errno );
        } else {
          if ( ioctl ( ptrDesc, TIOCEXCL )) {
            int errnoCopy = errno;
            errno = 0;
            while (( close ( ptrDesc ) < 0 ) && EINTR == errno );
            fprintf ( stderr, "%s: can't get lock on %s, errno = %d\n",
              __FUNCTION__, deviceNode, errnoCopy );
            continue;
          }

          if ( tcgetattr ( ptrDesc, &tio )) {
            int errnoCopy = errno;
            errno = 0;
            while (( close ( ptrDesc ) < 0 ) && EINTR == errno );
            fprintf ( stderr, "%s: can't get termio on %s, errno = %d\n",
              __FUNCTION__, deviceNode, errnoCopy );
            continue;
          }

          tio.c_iflag = IGNBRK | IGNPAR | CS8;
          tio.c_oflag |= CS8;
          tio.c_oflag &= ~( ONLRET | ONOCR );
          tio.c_lflag &= ~ICANON;
          tio.c_cc[VMIN] = 1;
          tio.c_cc[VTIME] = 4;
          tio.c_cflag &= ~PARENB; // no parity
          tio.c_cflag &= ~CSTOPB; // 1 stop bit
          cfsetispeed ( &tio, B38400 );
          cfsetospeed ( &tio, B38400 );
          if ( tcsetattr ( ptrDesc, TCSANOW, &tio )) {
            int errnoCopy = errno;
            errno = 0;
            while (( close ( ptrDesc ) < 0 ) && EINTR == errno );
            fprintf ( stderr, "%s: can't set termio on %s, errno = %d\n",
              __FUNCTION__, deviceNode, errnoCopy );
            continue;
          }

          // ctrl-C
          if ( _ptrWrite ( ptrDesc, "\003", 1 )) {
            fprintf ( stderr, "%s: failed to write ctrl-C to %s\n",
                __FUNCTION__, deviceNode );
            close ( ptrDesc );
            continue;
          }
          // we need to wait at least 5ms here for the PTR to respond
          usleep ( 10000 );

          // now flush the input buffer to get rid of the echoed "^C" and
          // the PTR prompt

          tcflush ( ptrDesc, TCIFLUSH );

          if ( _ptrWrite ( ptrDesc, "sysreset\r", 9 )) {
            fprintf ( stderr, "%s: failed to write sysreset to %s\n",
                __FUNCTION__, deviceNode );
            close ( ptrDesc );
            continue;
          }

          // After a reset we should get seven lines something like:
          //
          // sysreset
          // <blank line>
          // PTR-1.0 YYYY-MM-DD
          // ------------------
          // Copyright YYYY, ....
          // example@example.com
          // <blank line>
          //
          // and then a pause whilst we wait for the status message (which
          // can take at least a second):
          //
          // Internal clock synchronized: 20160531T212127.000
          // PTR-0.1 >

          result = 0;
          for ( i = 0; i < 7 && !result; i++ ) {
            numRead = _ptrRead ( ptrDesc, buffer, sizeof ( buffer ) - 1 );
            if ( numRead < 0 ) {
              perror(0);
              result = -1;
            } else {
              if ( !numRead ) {
                fprintf ( stderr, "%s: no characters read from PTR\n",
                    __FUNCTION__ );
              } else {
                if (( namePtr = strstr ( buffer, "PTR-" )) &&
                    isdigit ( namePtr[4] ) && namePtr[5] == '.' ) {
                  endPtr = namePtr + 5;
                  while ( *endPtr++ != ' ' );
                  sprintf ( endPtr, "(%s)", deviceNode );
                  ( void ) strncpy ( nameBuffer, namePtr, 63 );
                  result = 1;
                }
              }
            }
          }

          // Assuming we have found something useful, wait for the remaining
          // lines and clean up

          if ( result > 0 ) {
            FD_ZERO ( &readable );
            FD_SET ( ptrDesc, &readable );
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            if ( select ( ptrDesc + 1, &readable, 0, 0, &timeout ) == 0 ) {
              fprintf ( stderr, "%s: PTR select timed out\n", __FUNCTION__ );
              result = -1;
            } else {
              numRead = _ptrRead ( ptrDesc, buffer, sizeof ( buffer ) - 1 );
              if ( numRead > 0 ) {
                // ignore this -- we won't check it for now
                tcflush ( ptrDesc, TCIFLUSH );
              } else {
                fprintf ( stderr, "%s: PTR status message not found\n",
                    __FUNCTION__ );
                result = -1;
              }
            }
          }

          close ( ptrDesc );

          if ( result < 0 ) {
            fprintf ( stderr, "%s: Can't find PTR name from %s, buff = '%s'\n",
                __FUNCTION__, deviceNode, buffer );
            continue;
          }

          if (!( ptr = malloc ( sizeof ( oaPTRDevice )))) {
            udev_device_unref ( dev );
            udev_enumerate_unref ( enumerate );
            udev_unref ( udev );
            _oaFreePTRDeviceList ( deviceList );
            return -OA_ERR_MEM_ALLOC;
          }
          if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
            udev_device_unref ( dev );
            udev_enumerate_unref ( enumerate );
            udev_unref ( udev );
            free (( void* ) ptr );
            _oaFreePTRDeviceList ( deviceList );
            return -OA_ERR_MEM_ALLOC;
          }
          _oaInitPTRDeviceFunctionPointers ( ptr );
          ptr->_private = _private;
          _private->devType = 1;
          ( void ) strcpy ( ptr->deviceName, nameBuffer );
          _private->devIndex = numFound++;
          // _private->vendorId = vid;
          // _private->productId = pid;
          ptr->init = oaPTRInit;
          ( void ) strncpy ( _private->sysPath, deviceNode, PATH_MAX );
          if (( ret = _oaCheckPTRArraySize ( deviceList )) < 0 ) {
            free (( void* ) ptr );
            free (( void* ) _private );
            _oaFreePTRDeviceList ( deviceList );
            udev_device_unref ( dev );
            udev_enumerate_unref ( enumerate );
            udev_unref ( udev );
            return ret;
          }
          deviceList->ptrList[ deviceList->numPTRDevices++ ] = ptr;
        }
      }
      udev_device_unref ( dev );
    }
  }
  udev_enumerate_unref ( enumerate );
  udev_unref ( udev );
  return numFound;
}


const char*
oaPTRGetName ( oaPTRDevice* device )
{
  return device->deviceName;
}
