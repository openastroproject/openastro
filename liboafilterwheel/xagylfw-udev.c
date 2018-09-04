/*****************************************************************************
 *
 * xagylfw-udev.c -- Find Xagyl filter wheels using (Linux) udev
 *
 * Copyright 2014,2015,2017,2018 James Fidell (james@openastroproject.org)
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
#include <hidapi.h>
#ifdef XAGYL_READ_FOUND_WHEEL
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#endif

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "xagylfw.h"

int
oaXagylGetFilterWheels ( FILTERWHEEL_LIST* deviceList )
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
  const char*			serialNo;
  const char*			manufacturer;
  const char*			product;
  const char*			path;
  const char*			deviceNode;
  const char*			vid;
  const char*			pid;
  uint8_t			haveWheel;
  int				numFound = 0, entry, ret;
  oaFilterWheelDevice*		wheel;
  char				buffer[200];
  DEVICE_INFO*			_private;
#ifdef XAGYL_READ_FOUND_WHEEL
  struct termios		tio;
  int				fwDesc, numRead;
#endif

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
      serialNo = udev_device_get_sysattr_value ( dev, "serial" );
      haveWheel = 0;
      if ( !strcmp ( "0403", vid ) && ( !strcmp ( "6001", pid ) ||
          !strcmp ( "6015", pid ))) {
        if ( !strcmp ( serialNo, "A4008T44" ) ||
            !strcmp ( serialNo, "DA001CEZ" ) ||
            !strcmp ( serialNo, "DA00BT4MA" )) {
            haveWheel = 1;
        }
      }

      // Check the user-defined entries if required.  The sense here is to
      // accept a match wherever the configured values match the USB data.
      if ( !haveWheel && xagylConfigEntries ) {
        manufacturer = udev_device_get_sysattr_value ( dev, "manufacturer" );
        product = udev_device_get_sysattr_value ( dev, "product" );
        userDeviceConfig* confp = xagylConfig;
        for ( entry = 0; entry < xagylConfigEntries && !haveWheel; entry++ ) {
          char vidStr[8], pidStr[8];
          if ( confp->vendorId && confp->productId ) {
            sprintf ( vidStr, "%04x", confp->vendorId );
            sprintf ( pidStr, "%04x", confp->productId );
          } else {
            vidStr[0] = pidStr[0] = 0;
          }
          haveWheel = 1;
          if (( *vidStr && strcasecmp ( vidStr, vid )) ||
              ( *pidStr && strcasecmp ( pidStr, pid )) ||
              ( *( confp->manufacturer ) &&
              strcasecmp ( confp->manufacturer, manufacturer )) ||
              ( *( confp->product ) &&
              strcasecmp ( confp->product, product )) ||
              ( *( confp->serialNo ) &&
              strcasecmp ( confp->serialNo, serialNo ))) {
            haveWheel = 0;
          }
          confp += sizeof ( userDeviceConfig );
        }
      }

      if ( haveWheel ) {

#ifdef XAGYL_READ_FOUND_WHEEL
        // At this point we want to lock the serial device, open it
        // and run inquiries to get the product name and firmware version

        if (( fwDesc = open ( deviceNode, O_RDWR | O_NOCTTY )) < 0 ) {
          fprintf ( stderr, "Can't open %s read-write, errno = %d\n",
              deviceNode, errno );
        } else {
          if ( ioctl ( fwDesc, TIOCEXCL )) {
            int errnoCopy = errno;
            errno = 0;
            while (( close ( fwDesc ) < 0 ) && EINTR == errno );
            fprintf ( stderr, "%s: can't get lock on %s, errno = %d\n",
              __FUNCTION__, deviceNode, errnoCopy );
            continue;
          }

          if ( tcgetattr ( fwDesc, &tio )) {
            int errnoCopy = errno;
            errno = 0;
            while (( close ( fwDesc ) < 0 ) && EINTR == errno );
            fprintf ( stderr, "%s: can't get termio on %s, errno = %d\n",
              __FUNCTION__, deviceNode, errnoCopy );
            continue;
          }

          tio.c_iflag = IXON | IGNPAR | BRKINT;
          tio.c_oflag = 0;
          tio.c_cflag = CLOCAL | CREAD | CS8 | B9600;
          tio.c_lflag = IEXTEN | ECHOKE | ECHOCTL | ECHOK | ECHOE;

          if ( tcsetattr ( fwDesc, TCSAFLUSH, &tio )) {
            int errnoCopy = errno;
            errno = 0;
            while (( close ( fwDesc ) < 0 ) && EINTR == errno );
            fprintf ( stderr, "%s: can't set termio on %s, errno = %d\n",
              __FUNCTION__, deviceNode, errnoCopy );
            continue;
          }
#endif

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
          wheel->interface = OA_FW_IF_XAGYL;
          wheel->_private = _private;
#ifdef XAGYL_READ_FOUND_WHEEL
          if ( write ( fwDesc, "i0", 2 ) != 2 ) {
            fprintf ( stderr, "%s: write error on %s\n", __FUNCTION__,
                deviceNode );
            close ( fwDesc );
            continue;
          }
          numRead = read ( fwDesc, buffer, 50 );
          if ( numRead > 0 ) {
            buffer[numRead] = 0;
            numRead--;
            while ( buffer[numRead] == '\r' || buffer[numRead] == '\n' ) {
              buffer[numRead--] = 0;
            }
          } else {
            fprintf ( stderr, "%s: failed to read name from %s\n",
                __FUNCTION__, deviceNode );
            close ( fwDesc );
            continue;
          }

          if ( strstr ( buffer, "FW5125" )) {
            wheel->_devType = XAGYL_5125;
          } else {
            if ( strstr ( buffer, "FW8125" )) {
              wheel->_devType = XAGYL_8125;
            } else {
              if ( strstr ( buffer, "FW5200" )) {
                wheel->_devType = XAGYL_5200;
              }
            }
          }
#else
          ( void ) strcpy ( buffer, "Xagyl Filter Wheel" );
          _private->devType = XAGYL_5125;
#endif
          if ( _private->devType ) {
            ( void ) strcpy ( wheel->deviceName, buffer );
            _private->devIndex = numFound++;
            // _private->vendorId = vid;
            // _private->productId = pid;
            wheel->initFilterWheel = oaXagylInitFilterWheel;
            ( void ) strncpy ( _private->sysPath, deviceNode, PATH_MAX );
            if (( ret = _oaCheckFilterWheelArraySize ( deviceList )) < 0 ) {
              ( void ) free (( void* ) wheel );
              ( void ) free (( void* ) _private );
#ifdef XAGYL_READ_FOUND_WHEEL
              close ( fwDesc );
#endif
              udev_device_unref ( dev );
              udev_enumerate_unref ( enumerate );
              udev_unref ( udev );
              return ret;
            }
            deviceList->wheelList[ deviceList->numFilterWheels++ ] = wheel;
          } else {
            fprintf ( stderr, "%s: Unrecognised filter wheel '%s'\n",
                __FUNCTION__, buffer );
          }
#ifdef XAGYL_READ_FOUND_WHEEL
          close ( fwDesc );
        }
#endif
      }
      udev_device_unref ( dev );
    }
  }
  udev_enumerate_unref ( enumerate );
  udev_unref ( udev );

  return numFound;
}


const char*
oaXagylWheelGetName ( oaFilterWheel* wheel )
{
  return wheel->deviceName;
}


/*
 * From the Xagyl manual:

As of firmware version 3.1.5, here are the current commands:
Serial = 9600, 8, n, 1
Command format = CV where C=command and V=value

"B", "b"
Move backward Value number of filters - Value = 0 to F Hex (0 to 15 Decimal)

"C", "c"
Deprecated – use “(“ and “)”
Removed from version 2.0.0
Calibrate Value where Value is
1 - Shift the carousel counterclockwise
2 - Shift the carousel clockwise

"F", "f"
Move forward Value number of filters - Value = 0 to F Hex (0 to 15 Decimal)

"G", "g"
Move to Value position - Value = 1 to X, carousel will turn in direction of shortest path
where X is the highest filter position

"I", "i"
Inquire Value where Value is:
0 - Display product name "Xagyl FW5125VX"
1 - Display firmware version "FW3.1.5"
2 - Display current filter position - "PX"
3 - Display the serial number - "S/N: XXXXXX"
4 - Display the maximum rotation speed - "MaxSpeed XXX%"
5 - Display the jitter value - "Jitter XX"
6 - Display sensor position offset for current filter position - "PX Offset XX"
7 - Display filter position sensor threshold value - "Threshold XX"
8 - Display the number of available filter slots - "FilterSlots X"
9 - Display the Pulse Width value - "Pulse Width XXXXXuS"

9 appears not to work in 2.x (my note)

“M”, “m”
Increase pulse width by 100uS
Value is ignored
Adjust the length of the “ON” pulse during the centering phase of a filter change. This
may be required when the filter wheel is used in extreme cold where some binding
can occur.
Pulse width values range from 100uS to 10000uS. Default value is 1500uS.
Displays “Pulse Width XXXXXuS”

“N”, “n”
Decrease pulse width by 100uS
Value is ignored
Adjust the length of the “ON” pulse during the centering phase of a filter change. This
may be required when the filter wheel is used in extreme cold where some binding
can occur.
Pulse width values range from 100uS to 10000uS. Default value is 1500uS.
Displays “Pulse Width XXXXXuS”

“O”, “o”
Display sensor position offset for filter position Value - "PX Offset XX"

“R”, ”r”
Reset Value where Value is:
0 - Hard reboot
1 - Initialize - restarts and moves to filter position 1
2 - Reset all calibration values to 0, displays "Calibration Removed"
3 - Reset Jitter value to 1, displays "Jitter 1"
4 - Reset maximum carousel rotation speed to 100%, displays "MaxSpeed 100%"
5 - Reset Threshold value to 30, displays "Threshold 30"
6 - Calibrate - performs a sensor calibration and stores the results in flash

"S", "s"
Set the maximum carousel rotation speed during normal operation at Value x 10
percent speed
if Value >= A Hex (10 decimal) then maximum carousel rotation speed = 100%
if Value = 0 then carousel rotation is stopped
Displays "Speed=XXX%"

"T", "t"
Test (Debug) Value where Value is:
0 - Display current sensor values, displays "Sensors LL RR" where LL is left sensor
and RR is right sensor. Numbers are absolute deviations from Midrange value
1 - Display current sensor values, displays "Sensors LLL RRR" where LLL is left
sensor and RRR is right sensor. Numbers are actual measured values
2 - Display median left sensor value, displays "MidRange XXX"
3 - Display right sensor calibration value, displays "RightCal XX" which is the
hardware deviation between right and left sensor

"<"
Runs motor backward at Value x 10 percent speed - if Value >= A Hex (10 decimal) then
run at 100% speed
if Value = 0 then carousel rotation is stopped
Do "R1" to stop and reset to index position
Displays "Backward XXX%"

">"
Runs motor forward at Value x 10 percent speed - if Value >= A Hex (10 decimal) then
run at 100% speed
if Value = 0 then carousel rotation is stopped
Do "R1" to stop and reset to index position
Displays "Forward XXX%"

“[“
Decrease jitter window by 1
Value is ignored
This feature is to compensate for noisy 5V power from the USB port
Jitter values range from 1 to 10 – use the smallest value which provides a jitter-free
filter center position
Displays “Jitter X”

“]“
Increase jitter window by 1
Value is ignored
This feature is to compensate for noisy 5V power from the USB port
Jitter values range from 1 to 10 – use the smallest value which provides a jitter-free
filter center position
Displays “Jitter X”

“(“
Position calibration of current filter
Value is ignored
Shift the carousel counterclockwise
Displays "PX Offset XX"

“)”
Position calibration of current filter
Value is ignored
Shift the carousel clockwise
Displays "PX Offset XX"

“{“
Decrease filter position threshold value
Value is ignored
Carousel will stop earlier in rotation and start fine positioning.
Too low of a value can cause carousel to stop prematurely when changing positions
Displays "Threshold XX"

“}”
Increase filter position threshold value
Value is ignored
Carousel will stop later in rotation and start fine positioning.
Too high of a value can cause the wheel to fail to recognize filter positions
Displays "Threshold XX"

*/
