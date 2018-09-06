/*****************************************************************************
 *
 * xagylfw-ftdi.c -- Find Xagyl filter wheels using libftdi
 *
 * Copyright 2014,2015,2018 James Fidell (james@openastroproject.org)
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
#include <libusb-1.0/libusb.h>
#include <libftdi1/ftdi.h>
#include <hidapi.h>

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

  int numFound = 0, numUSBDevices, i, ret;
  int matchedVidPid, matchedUSBStrings, entry;
  libusb_context*                       ctx = 0;
  libusb_device**                       devlist;
  libusb_device*                        device;
  libusb_device_handle*			handle;
  struct libusb_device_descriptor       desc;
  oaFilterWheelDevice*                  wheel;
  unsigned short                        busNum, addr;
  char					serialBuffer[20];
  char					manufacturerBuffer[80];
  char					productBuffer[80];
  char					buffer[100];
  struct ftdi_context*			ftdiCtx;
  DEVICE_INFO*				_private;

  libusb_init ( &ctx );
  // libusb_set_debug ( ctx, LIBUSB_LOG_LEVEL_DEBUG );
  numUSBDevices = libusb_get_device_list ( ctx, &devlist );
  if ( numUSBDevices < 1 ) {
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( ctx );
    if ( numUSBDevices ) {
      return -OA_ERR_SYSTEM_ERROR;
    }
    return 0;
  }

  if (!( ftdiCtx = ftdi_new())) {
    fprintf ( stderr, "can't connect to ftdi\n" );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( ctx );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( ret = ftdi_init ( ftdiCtx ))) {
    fprintf ( stderr, "can't initialise ftdi context, err = %d\n", ret );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( ctx );
    return -OA_ERR_SYSTEM_ERROR;
  }

  ftdi_set_interface ( ftdiCtx, INTERFACE_ANY );

  for ( i = 0; i < numUSBDevices; i++ ) {
    device = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( device, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( ctx );
      return -OA_ERR_SYSTEM_ERROR;
    }
    busNum = libusb_get_bus_number ( device );
    addr = libusb_get_device_address ( device );
    matchedVidPid = 0;
    if ( desc.idVendor == XAGYL_VID &&
        ( desc.idProduct == XAGYL_FILTERWHEEL_PID1 ||
        desc.idProduct == XAGYL_FILTERWHEEL_PID2 )) {
      matchedVidPid = 1;
    }
    if ( !matchedVidPid && xagylConfigEntries ) {
      userDeviceConfig* confp = xagylConfig;
      for ( entry = 0; entry < xagylConfigEntries && !matchedVidPid; entry++ ) {
        if ( confp->vendorId && confp->productId && confp->vendorId ==
            desc.idVendor && confp->productId == desc.idProduct ) {
          matchedVidPid = 1;
        }
        confp += sizeof ( userDeviceConfig );
      }
    }

    if ( matchedVidPid ) {

      if ( LIBUSB_SUCCESS != libusb_open ( device, &handle )) {
        fprintf ( stderr, "libusb_open for Xagyl FTDI filterwheel failed\n" );
        libusb_free_device_list ( devlist, 1 );
        libusb_exit ( ctx );
        ftdi_free ( ftdiCtx );
        return -OA_ERR_SYSTEM_ERROR;
      }

      serialBuffer[0] = manufacturerBuffer[0] = productBuffer[0] = 0;
      if ( desc.iSerialNumber ) {
        libusb_get_string_descriptor_ascii ( handle, desc.iSerialNumber,
            (unsigned char*) serialBuffer, 20 );
      }
      if ( desc.iManufacturer ) {
        libusb_get_string_descriptor_ascii ( handle, desc.iManufacturer,
            (unsigned char*) manufacturerBuffer, 80 );
      }
      if ( desc.iProduct ) {
        libusb_get_string_descriptor_ascii ( handle, desc.iProduct,
            (unsigned char*) productBuffer, 80 );
      }
      libusb_close ( handle );

      matchedUSBStrings = 0;
      if ( !strcmp ( serialBuffer, "A4008T44" ) ||
          !strcmp ( serialBuffer, "DA001CEZ" ) ||
          !strcmp ( serialBuffer, "DA00BT4MA" )) {
        matchedUSBStrings = 1;
      }

      if ( !matchedUSBStrings && xagylConfigEntries ) {
        userDeviceConfig* confp = xagylConfig;
        for ( entry = 0; entry < xagylConfigEntries && !matchedUSBStrings;
            entry++ ) {
          matchedUSBStrings = 1;
          if (( *( confp->manufacturer ) &&
              strcasecmp ( confp->manufacturer, manufacturerBuffer )) ||
              ( *( confp->product ) &&
              strcasecmp ( confp->product, productBuffer )) ||
              ( *( confp->serialNo ) &&
              strcasecmp ( confp->serialNo, serialBuffer ))) {
            matchedUSBStrings = 0;
          }
          confp += sizeof ( userDeviceConfig );
        }
      }

      if ( matchedUSBStrings ) {

        // Looks like we've got one

/*
        if ( ftdi_usb_open_dev ( ftdiCtx, device ) < 0 ) {
          fprintf ( stderr, "FTDI open of device serial '%s' failed\n",
              serialBuffer );
          continue;
        }

        if ( ftdi_set_baudrate ( ftdiCtx, 9600 ) < 0 ) {
          fprintf ( stderr, "set baud rate for device serial '%s' failed\n",
              serialBuffer );
          ftdi_usb_close ( ftdiCtx );
          continue;
        }

        if ( ftdi_set_line_property ( ftdiCtx, BITS_8, STOP_BIT_1,
            NONE ) < 0 ) {
          fprintf ( stderr, "set 8N1 for device serial '%s' failed\n",
              serialBuffer );
          ftdi_usb_close ( ftdiCtx );
          continue;
        }

        cmd[0] = 'i'; cmd[1] = '0'; cmd[2] = 0;
        if ( ftdi_write_data ( ftdiCtx, cmd, 2 ) != 2 ) {
          fprintf ( stderr, "write i0 for device serial '%s' failed\n",
              serialBuffer );
          ftdi_usb_close ( ftdiCtx );
          continue;
        }

        usleep ( 200000 );
        numRead = ftdi_read_data ( ftdiCtx, (unsigned char* ) buffer, 100 );

        ftdi_usb_close ( ftdiCtx );
        if ( numRead > 0 ) {
          buffer[numRead] = 0;
          numRead--;
          while ( buffer[numRead] == '\r' || buffer[numRead] == '\n' ) {
            buffer[numRead--] = 0;
          }
        } else {
          fprintf ( stderr, "%s: failed to read name from device serial %s, "
              "error %d\n", __FUNCTION__, serialBuffer, numRead );
          continue;
        }
*/
        ( void ) strcpy ( buffer, "Xagyl Filter Wheel" );


        if (!( wheel = malloc ( sizeof ( oaFilterWheelDevice )))) {
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          ftdi_free ( ftdiCtx );
          return -OA_ERR_MEM_ALLOC;
        }
        if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          ( void ) free (( void* ) wheel );
          return -OA_ERR_MEM_ALLOC;
        }


/*
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
*/
        _private->devType = XAGYL_5125;

        if ( !_private->devType ) {
          fprintf ( stderr, "%s: Unrecognised filter wheel '%s'\n",
              __FUNCTION__, buffer );
        } else {
          _oaInitFilterWheelDeviceFunctionPointers ( wheel );
          wheel->interface = OA_FW_IF_XAGYL;
          wheel->_private = _private;
          ( void ) strcpy ( wheel->deviceName, buffer );
          _private->devIndex = ( busNum << 8 ) | addr;
          _private->vendorId = XAGYL_VID;
          _private->productId = desc.idProduct;
          wheel->initFilterWheel = oaXagylInitFilterWheel;
          if (( ret = _oaCheckFilterWheelArraySize ( deviceList )) < 0 ) {
            ( void ) free (( void* ) wheel );
            ( void ) free (( void* ) _private );
            return ret;
          }
          deviceList->wheelList[ deviceList->numFilterWheels++ ] = wheel;
          numFound++;
        }
      }
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

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
