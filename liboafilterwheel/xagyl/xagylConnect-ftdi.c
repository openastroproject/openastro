/*****************************************************************************
 *
 * xagylInit-ftdi.c -- Initialise Xagyl filter wheels (libftdi)
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
#include <pthread.h>
#include <hidapi.h>
#include <libusb-1.0/libusb.h>
#include <libftdi1/ftdi.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "xagylfw.h"


static void _xagylInitFunctionPointers ( oaFilterWheel* );
static int  _getNumSlots ( oaFilterWheel* );

/**
 * Initialise a given filter wheel device
 */

oaFilterWheel*
oaXagylInitFilterWheel ( oaFilterWheelDevice* device )
{
  int                                   deviceAddr, deviceBus;
  libusb_context*			usbContext;
  libusb_device**                       devlist;
  libusb_device*                        usbDevice;
  oaFilterWheel*			wheel;
  DEVICE_INFO*				devInfo;
  PRIVATE_INFO*				privateInfo;
  int                                   i, matched, ret, numUSBDevices;
  struct libusb_device_descriptor       desc;

  devInfo = device->_private;

  if (!( wheel = ( oaFilterWheel* ) malloc ( sizeof ( oaFilterWheel )))) {
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }
  if (!( privateInfo = ( PRIVATE_INFO* ) malloc ( sizeof ( PRIVATE_INFO )))) {
    free (( void* ) wheel );
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }

  OA_CLEAR ( *wheel );
  OA_CLEAR ( *privateInfo );
  // OA_CLEAR ( wheel->features );
  OA_CLEAR ( wheel->controls );

  wheel->_private = privateInfo;

  pthread_mutex_init ( &privateInfo->ioMutex, 0 );

  ( void ) strcpy ( wheel->deviceName, device->deviceName );

  _oaInitFilterWheelFunctionPointers ( wheel );
  _xagylInitFunctionPointers ( wheel );

  privateInfo->initialised = 0;
  privateInfo->index = -1;

  deviceAddr = devInfo->devIndex & 0xff;
  deviceBus = devInfo->devIndex >> 8;

  libusb_init ( &usbContext );
  numUSBDevices = libusb_get_device_list ( usbContext, &devlist );
  if ( numUSBDevices < 1 ) {
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( usbContext );
    if ( numUSBDevices ) {
      fprintf ( stderr, "Can't see any USB devices now (list returns -1)\n" );
      free ( wheel );
      return 0;
    }
    fprintf ( stderr, "Can't see any USB devices now\n" );
    free ( wheel );
    return 0;
  }

  matched = 0;
  for ( i = 0; i < numUSBDevices && !matched; i++ ) {
    usbDevice = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( usbDevice, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( usbContext );
      fprintf ( stderr, "get device descriptor failed\n" );
      free ( wheel );
      return 0;
    }
    if ( desc.idVendor == XAGYL_VID &&
        ( desc.idProduct == XAGYL_FILTERWHEEL_PID1 ||
        desc.idProduct == XAGYL_FILTERWHEEL_PID2 ) &&
        libusb_get_bus_number ( usbDevice ) == deviceBus &&
        libusb_get_device_address ( usbDevice ) == deviceAddr ) {
      // this looks like the one!
      matched = 1;
    }
  }

  if ( matched ) {
    if (!( privateInfo->ftdiContext = ftdi_new())) {
      fprintf ( stderr, "ftdi_new() failed\n" );
      matched = 0;
    } else {
      if (( ret = ftdi_init ( privateInfo->ftdiContext ))) {
        fprintf ( stderr, "can't initialise ftdi context, err = %d\n", ret );
        matched = 0;
      } else {
        ftdi_set_interface ( privateInfo->ftdiContext, INTERFACE_ANY );
        if ( ftdi_usb_open_dev ( privateInfo->ftdiContext, usbDevice ) < 0 ) {
          fprintf ( stderr, "FTDI open of serial device failed\n" );
          matched = 0;
        }
      }
    }
  }

  // now the libusb stuff can be released

  libusb_free_device_list ( devlist, 0 );
  // libusb_exit ( usbContext );

  if ( !matched ) {
    fprintf ( stderr, "no matching filter wheel\n" );
    free ( wheel );
    ftdi_free ( privateInfo->ftdiContext );
    return 0;
  }

  if (( ret = ftdi_set_baudrate ( privateInfo->ftdiContext, 9600 )) < 0 ) {
    fprintf ( stderr, "set baud rate for FTDI serial device failed, "
        "err = %d\n", ret );
    ftdi_usb_close ( privateInfo->ftdiContext );
    ftdi_free ( privateInfo->ftdiContext );
    return 0;
  }

  if ( ftdi_set_line_property ( privateInfo->ftdiContext, BITS_8, STOP_BIT_1,
      NONE ) < 0 ) {
    fprintf ( stderr, "set 8N1 for device FTDI serial device failed\n" );
    ftdi_usb_close ( privateInfo->ftdiContext );
    ftdi_free ( privateInfo->ftdiContext );
    return 0;
  }

  wheel->interface = device->interface;
  privateInfo->index = devInfo->devIndex;
  privateInfo->wheelType = devInfo->devType;
  privateInfo->currentPosition = 1;

  pthread_mutex_init ( &privateInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &privateInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &privateInfo->callbackQueued, 0 );
  pthread_cond_init ( &privateInfo->commandQueued, 0 );
  pthread_cond_init ( &privateInfo->commandComplete, 0 );

  privateInfo->stopControllerThread = privateInfo->stopCallbackThread = 0;
  privateInfo->commandQueue = oaDLListCreate();
  privateInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( privateInfo->controllerThread ), 0,
      oafwXagylcontroller, ( void* ) wheel )) {
    free (( void* ) wheel->_private );
    free (( void* ) wheel );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }

  if ( pthread_create ( &( privateInfo->callbackThread ), 0,
      oafwCallbackHandler, ( void* ) wheel )) {

    void* dummy;
    privateInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &privateInfo->commandQueued );
    pthread_join ( privateInfo->controllerThread, &dummy );
    free (( void* ) wheel->_private );
    free (( void* ) wheel );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }

  oaXagylWheelWarmReset ( privateInfo, 0 );

  if (( wheel->numSlots = _getNumSlots ( wheel )) < 1 ) {
    fprintf ( stderr, "%s: invalid number of slots in filter wheel %s\n",
        __FUNCTION__, devInfo->sysPath );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }

  wheel->controls [ OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC ] = OA_CTRL_TYPE_INT32;
  wheel->controls [ OA_FW_CTRL_SPEED ] = OA_CTRL_TYPE_INT32;
  wheel->controls [ OA_FW_CTRL_WARM_RESET ] = OA_CTRL_TYPE_BUTTON;
  wheel->controls [ OA_FW_CTRL_COLD_RESET ] = OA_CTRL_TYPE_BUTTON;

  return wheel;
}


static void
_xagylInitFunctionPointers ( oaFilterWheel* wheel )
{
  wheel->funcs.initWheel = oaXagylInitFilterWheel;
  wheel->funcs.closeWheel = oaXagylWheelClose;
  wheel->funcs.readControl = oaWheelReadControl;
  wheel->funcs.setControl = oaWheelSetControl;
  // wheel->funcs.testControl = XXXX;
}


int
_getNumSlots ( oaFilterWheel* wheel )
{
  PRIVATE_INFO*	privateInfo = wheel->_private;
  char		buffer[50];
  int		numRead, numSlots;

  pthread_mutex_lock ( &privateInfo->ioMutex );

  if ( _xagylWheelWrite ( privateInfo->ftdiContext, "i8", 2 )) {
    fprintf ( stderr, "%s: write error on i8 command\n", __FUNCTION__ );
    return 0;
  }

  numRead = _xagylWheelRead ( privateInfo->ftdiContext, buffer, 50 );
  pthread_mutex_unlock ( &privateInfo->ioMutex );

  if ( numRead > 0 ) {
    if ( strncmp ( buffer, "FilterSlots ", 12 )) {
      fprintf ( stderr, "%s: failed to match expecting string 'FilterSlots '"
           ", got '%40s'\n", __FUNCTION__, buffer );
      return 0;
    }
    if ( sscanf ( buffer, "FilterSlots %d", &numSlots ) != 1 ) {
      fprintf ( stderr, "%s: Failed to match number of slots in '%s'\n",
          __FUNCTION__, buffer );
      return 0;
    }
    return numSlots;
  }

  fprintf ( stderr, "%s: no data read from wheel interface\n",
      __FUNCTION__ );
  return 0;
}


int
oaXagylWheelClose ( oaFilterWheel* wheel )
{
  PRIVATE_INFO*	privateInfo = wheel->_private;

  privateInfo->initialised = 0;
  ftdi_usb_close ( privateInfo->ftdiContext );
  ftdi_free ( privateInfo->ftdiContext );
  privateInfo->ftdiContext = 0;
  return 0;
}
