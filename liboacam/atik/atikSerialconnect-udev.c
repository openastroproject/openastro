/*****************************************************************************
 *
 * atikSerialconnect-udev.c -- Initialise Atik serial cameras via udev
 *
 * Copyright 2014,2015,2016,2018,2019
 *   James Fidell (james@openastroproject.org)
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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "atikSerial.h"
#include "atikSerialoacam.h"
#include "atikSerialstate.h"


static void _atikSerialInitFunctionPointers ( oaCamera* );

#define BUFFER_LEN	64

/**
 * Initialise a given camera device
 */

oaCamera*
oaAtikSerialInitCamera ( oaCameraDevice* device )
{
  oaCamera*		camera;
  int                   camDesc;
  struct termios        tio;
  int			numRead, i, j, useFIFO;
  DEVICE_INFO*		devInfo;
  AtikSerial_STATE*	cameraInfo;
  COMMON_INFO*		commonInfo;
  unsigned char		buffer[ BUFFER_LEN ];
  unsigned char		capsCmd[4] = { 'C', 'M', 'D', ATIK_CMD_QUERY_CAPS };
  unsigned char		pingCmd[4] = { 'C', 'M', 'D', ATIK_CMD_PING };
  unsigned char		serialCmd[4] = { 'C', 'M', 'D',
                            ATIK_CMD_QUERY_SERIAL_NO };
  unsigned char		fifoCmd[4] = { 'C', 'M', 'D', ATIK_CMD_QUERY_FIFO };
  unsigned char		extCmd[8] = { 'C', 'M', 'D', ATIK_CMD_SEND_EXTERNAL,
			    0xc9, 0xf4, 0xa9, 0xdb };

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( AtikSerial_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  devInfo = device->_private;

  _atikSerialInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->index = -1;

  cameraInfo->write = _atikUdevSerialCamWrite;
  cameraInfo->read = _atikUdevSerialCamRead;
  cameraInfo->readToZero = _atikUdevSerialCamReadToZero;
  cameraInfo->readBlock = _atikUdevSerialCamReadBlock;

  if (( camDesc = open ( devInfo->sysPath, O_RDWR | O_NOCTTY )) < 0 ) {
    fprintf ( stderr, "Can't open %s read-write, errno = %d\n",
        devInfo->sysPath, errno );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( ioctl ( camDesc, TIOCEXCL )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( camDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't get lock on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( tcgetattr ( camDesc, &tio )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( camDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't get termio on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    FREE_DATA_STRUCTS;
    return 0;
  }

  tio.c_cflag &= ~PARENB; // no parity
  tio.c_cflag &= ~CSTOPB; // 1 stop bit
  tio.c_cflag |= CLOCAL | CREAD | CS8;
  cfsetospeed ( &tio, B115200 );
  cfsetispeed ( &tio, B115200 );
  tio.c_oflag = 0;
  tio.c_iflag = IGNPAR;
  tio.c_lflag = 0;
  tio.c_cc[VTIME] = 1;
  tio.c_cc[VMIN] = 0;
  tcflush ( camDesc, TCIFLUSH );
  if ( tcsetattr ( camDesc, TCSANOW, &tio )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( camDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't set termio on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    FREE_DATA_STRUCTS;
    return 0;
  }

  cameraInfo->fd = camDesc;
  camera->interface = device->interface;
  cameraInfo->index = devInfo->devIndex;
  cameraInfo->cameraType = devInfo->devType;

  // Send a PING command to the PIC.  No idea if this is really required.
  if ( cameraInfo->write ( cameraInfo, pingCmd, 4 )) {
    fprintf ( stderr, "%s: write error on ping\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }
  usleep ( 100000 );
  if (( numRead = cameraInfo->read ( cameraInfo, buffer, 1 ) != 1 )) {
    fprintf ( stderr, "%s: read error on ping\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( cameraInfo->write ( cameraInfo, capsCmd, 4 )) {
    fprintf ( stderr, "%s: write error on query caps\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if (( numRead = cameraInfo->read ( cameraInfo, buffer, 2 )) != 2 ) {
    fprintf ( stderr, "%s: read error 1 on query caps\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }

  /*
  fprintf ( stderr, "%s: camera protocol version %hhu.%hhu\n", __FUNCTION__,
      buffer[1], buffer[0] );
   */

  if (( numRead = cameraInfo->readToZero ( cameraInfo, buffer,
      BUFFER_LEN )) < 1 ) {
    fprintf ( stderr, "%s: read error 2 on query caps\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }

  // fprintf ( stderr, "%s: camera id '%s'\n", __FUNCTION__, buffer );

  if (( numRead = cameraInfo->readToZero ( cameraInfo, buffer,
      BUFFER_LEN )) < 1 ) {
    fprintf ( stderr, "%s: read error 3 on query caps\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }

  // fprintf ( stderr, "%s: manufacturer '%s'\n", __FUNCTION__, buffer );

  if (( numRead = cameraInfo->read ( cameraInfo, buffer, 16 ) != 16 )) {
    fprintf ( stderr, "%s: read error 4 on query caps\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }

  cameraInfo->cameraFlags = buffer[ CAPS_FLAGS_LO ] |
      ( buffer[ CAPS_FLAGS_HI ] << 8 );
  cameraInfo->maxResolutionX = buffer[ CAPS_TOTAL_PIXELS_X_LO ] |
      ( buffer[ CAPS_TOTAL_PIXELS_X_HI ] << 8 );
  cameraInfo->maxResolutionY = buffer[ CAPS_TOTAL_PIXELS_Y_LO ] |
      ( buffer[ CAPS_TOTAL_PIXELS_Y_HI ] << 8 );
  cameraInfo->pixelSizeX = buffer[ CAPS_PIXEL_SIZE_X_LO ] |
      ( buffer[ CAPS_PIXEL_SIZE_X_HI ] << 8 );
  cameraInfo->pixelSizeY = buffer[ CAPS_PIXEL_SIZE_Y_LO ] |
      ( buffer[ CAPS_PIXEL_SIZE_Y_HI ] << 8 );
  cameraInfo->maxBinningX = buffer[ CAPS_MAX_BIN_X_LO ] |
      ( buffer[ CAPS_MAX_BIN_X_HI ] << 8 );
  cameraInfo->maxBinningY = buffer[ CAPS_MAX_BIN_Y_LO ] |
      ( buffer[ CAPS_MAX_BIN_Y_HI ] << 8 );
  cameraInfo->wellDepth = buffer[ CAPS_WELL_DEPTH_LO ] |
      ( buffer[ CAPS_WELL_DEPTH_HI ] << 8 );

  camera->features.pixelSizeX = cameraInfo->pixelSizeX * 10;
  camera->features.pixelSizeY = cameraInfo->pixelSizeY * 10;

  /*
  fprintf ( stderr, "flags: %04x\n", cameraInfo->cameraFlags );
  fprintf ( stderr, "pixels X: %d\n", cameraInfo->maxResolutionX );
  fprintf ( stderr, "pixels Y: %d\n", cameraInfo->maxResolutionY );
  fprintf ( stderr, "pixel width: %d\n", cameraInfo->pixelSizeX );
  fprintf ( stderr, "pixel height: %d\n", cameraInfo->pixelSizeY );
  fprintf ( stderr, "max X binning: %d\n", cameraInfo->maxBinningX );
  fprintf ( stderr, "max Y binning: %d\n", cameraInfo->maxBinningY );
  fprintf ( stderr, "well depth: %d\n", cameraInfo->wellDepth * 1000 );
  */

  if ( cameraInfo->write ( cameraInfo, serialCmd, 4 )) {
    fprintf ( stderr, "%s: write error on query serial no\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if (( numRead = cameraInfo->read ( cameraInfo, buffer, 7 ) != 7 )) {
    fprintf ( stderr, "%s: read error on query serial no\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }
  /*
  fprintf ( stderr, "serial no: %.6s\n", buffer );
  fprintf ( stderr, "hardware type: %s\n", buffer[6] ? "production" : "beta" );
  */
  cameraInfo->hardwareType = buffer[6];

  if ( cameraInfo->write ( cameraInfo, fifoCmd, 4 )) {
    fprintf ( stderr, "%s: write error on query fifo\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if (( numRead = cameraInfo->read ( cameraInfo, buffer, 1 ) != 1 )) {
    fprintf ( stderr, "%s: read error on query fifo\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }
  // fprintf ( stderr, "have fifo: %s\n", buffer[0] ? "yes" : "no" );
  cameraInfo->haveFIFO = buffer[0];

  // Send some external port data.  Not sure what this does, but my
  // camera won't work without it.
  if ( cameraInfo->write ( cameraInfo, extCmd, 8 )) {
    fprintf ( stderr, "%s: write error on ext port\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }
  usleep ( 100000 );
  if (( numRead = cameraInfo->read ( cameraInfo, buffer, 1 ) != 1 )) {
    fprintf ( stderr, "%s: read error on ext port\n", __FUNCTION__ );
    close ( camDesc );
    FREE_DATA_STRUCTS;
    return 0;
  }

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      OA_CTRL_TYPE_INT64;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  // made up
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1800000000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      DEFAULT_EXPOSURE * 1000;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED_RESET ) = OA_CTRL_TYPE_BUTTON;

  if (!( cameraInfo->frameSizes[1].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
    return 0;
  }
  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  cameraInfo->frameSizes[1].numSizes = 1;
  camera->features.flags |= OA_CAM_FEATURE_FIXED_FRAME_SIZES;

  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;
  cameraInfo->droppedFrames = 0;

  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * 2;
  if (!( cameraInfo->xferBuffer = malloc ( cameraInfo->imageBufferLength ))) {
    fprintf ( stderr, "malloc of transfer buffer failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if (!( cameraInfo->buffers = calloc ( OA_CAM_BUFFERS,
      sizeof ( struct atikSerialbuffer )))) {
    fprintf ( stderr, "malloc of buffer array failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->xferBuffer );
    FREE_DATA_STRUCTS;
    return 0;
  }

  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      fprintf ( stderr, "%s malloc failed\n", __FUNCTION__ );
      if ( i ) {
        for ( j = 0; j < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
      free (( void* ) cameraInfo->buffers );
      free (( void* ) cameraInfo->xferBuffer );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  cameraInfo->buffersFree = cameraInfo->configuredBuffers;
  cameraInfo->nextBuffer = 0;

  cameraInfo->currentExposure = DEFAULT_EXPOSURE;

  switch ( cameraInfo->cameraType ) {
    case CAM_ATK16C:
    case CAM_ATK16HRC:
    case CAM_ATK16ICC:
    case CAM_ATK16ICSC:
      cameraInfo->colour = 1;
      camera->frameFormats [ OA_PIX_FMT_GBRG16LE ] = 1;
      break;
    default:
      cameraInfo->colour = 0;
      camera->frameFormats [ OA_PIX_FMT_GREY16LE ] = 1;
      break;
  }

  cameraInfo->binMode = cameraInfo->horizontalBinMode =
      cameraInfo->verticalBinMode = OA_BIN_MODE_NONE;
  cameraInfo->ccdReadFlags = ATIK_SERIAL_READ_FLAGS_CTP_BOTH |
      ATIK_SERIAL_READ_FLAGS_IPCS_MODE;
  if ( cameraInfo->cameraFlags & ATIK_SERIAL_FLAGS_INTERLACED ) {
    cameraInfo->ccdReadFlags |= ATIK_SERIAL_READ_FLAGS_DEINTERLACE;
  }

  useFIFO = 0;
  /*
   * Even though my camera claims to have a FIFO, Artemis Capture doesn't
   * appear to use it.  Not that it means we shouldn't, I guess :)
   */
  if ( cameraInfo->cameraFlags & ATIK_SERIAL_FLAGS_HAVE_FIFO ) {
    useFIFO = 1;
  } else {
    if ( cameraInfo->hardwareType ) {
      useFIFO = 1;
    } else {
      if ( cameraInfo->haveFIFO ) {
        useFIFO = 1;
      }
    }
  }
  if ( useFIFO ) {
    cameraInfo->ccdReadFlags |= ATIK_SERIAL_READ_FLAGS_USE_FIFO;
  }

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamAtikSerialcontroller, ( void* ) camera )) {
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->xferBuffer );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamAtikSerialcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->xferBuffer );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  return camera;
}


static void
_atikSerialInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaAtikSerialInitCamera;
  camera->funcs.closeCamera = oaAtikSerialCloseCamera;

  camera->funcs.setControl = oaAtikSerialCameraSetControl;
  camera->funcs.readControl = oaAtikSerialCameraReadControl;
  camera->funcs.testControl = oaAtikSerialCameraTestControl;
  camera->funcs.getControlRange = oaAtikSerialCameraGetControlRange;

  camera->funcs.startStreaming = oaAtikSerialCameraStartStreaming;
  camera->funcs.stopStreaming = oaAtikSerialCameraStopStreaming;
  camera->funcs.isStreaming = oaAtikSerialCameraIsStreaming;

  camera->funcs.setResolution = oaAtikSerialCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaAtikSerialCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaAtikSerialCameraGetFramePixelFormat;
}


int
oaAtikSerialCloseCamera ( oaCamera* camera )
{
  int			j;
  void*			dummy;
  AtikSerial_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    close ( cameraInfo->fd );

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    free (( void* ) cameraInfo->frameSizes[1].sizes );

    if ( cameraInfo->buffers ) {
      for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
        if ( cameraInfo->buffers[j].start ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
    }
    free (( void* ) cameraInfo->xferBuffer );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo );
    free (( void* ) camera->_common );
    free (( void* ) camera );
  } else {
    return -OA_ERR_INVALID_CAMERA;
  }

  return OA_ERR_NONE;
}
