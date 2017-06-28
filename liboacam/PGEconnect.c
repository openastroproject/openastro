/*****************************************************************************
 *
 * PGEconnect.c -- Initialise Point Grey Gig-E cameras
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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

#include <flycapture/C/FlyCapture2_C.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "PGEoacam.h"
#include "PGE.h"
#include "PGEstate.h"


static void _PGEInitFunctionPointers ( oaCamera* );

struct pgeCtrl pgeControls[] = {
  { FC2_BRIGHTNESS, OA_CAM_CTRL_BRIGHTNESS, OA_CAM_CTRL_AUTO_BRIGHTNESS },
  { FC2_AUTO_EXPOSURE, 0, 0 },
  { FC2_SHARPNESS, OA_CAM_CTRL_SHARPNESS, 0 },
  { FC2_WHITE_BALANCE, OA_CAM_CTRL_WHITE_BALANCE,
      OA_CAM_CTRL_AUTO_WHITE_BALANCE },
  { FC2_HUE, OA_CAM_CTRL_HUE, OA_CAM_CTRL_HUE_AUTO },
  { FC2_SATURATION, OA_CAM_CTRL_SATURATION, 0 },
  { FC2_GAMMA, OA_CAM_CTRL_GAMMA, OA_CAM_CTRL_AUTO_GAMMA },
  { FC2_IRIS, 0, 0 },
  { FC2_FOCUS, 0, 0 },
  { FC2_ZOOM, 0, 0 },
  { FC2_PAN, 0, 0 },
  { FC2_TILT, 0, 0 },
  { FC2_SHUTTER, -1, -1 },
  { FC2_GAIN, OA_CAM_CTRL_GAIN, OA_CAM_CTRL_AUTO_GAIN },
  { FC2_TRIGGER_MODE, OA_CAM_CTRL_TRIGGER_MODE, 0 },
  { FC2_TRIGGER_DELAY, OA_CAM_CTRL_TRIGGER_DELAY, 0 },
  { FC2_FRAME_RATE, -1, -1 },
  { FC2_TEMPERATURE, OA_CAM_CTRL_TEMPERATURE, 0 },
};

unsigned int numPGEControls = sizeof ( pgeControls ) /
    sizeof ( struct pgeCtrl );

struct pgeFrameRate pgeFrameRates[] = {
  { FC2_FRAMERATE_1_875, 8, 15 },
  { FC2_FRAMERATE_3_75, 4, 15 },
  { FC2_FRAMERATE_7_5, 2, 15 },
  { FC2_FRAMERATE_15, 1, 15 },
  { FC2_FRAMERATE_30, 1, 30 },
  { FC2_FRAMERATE_60, 1, 60 },
  { FC2_FRAMERATE_120, 1, 120 },
  { FC2_FRAMERATE_240, 1, 240 }
};

unsigned int numPGEFrameRates = sizeof ( pgeFrameRates ) /
    sizeof ( struct pgeFrameRate );

/**
 * Initialise a given camera device
 */

oaCamera*
oaPGEInitCamera ( oaCameraDevice* device )
{
  oaCamera*			camera;
  PGE_STATE*			cameraInfo;
  COMMON_INFO*			commonInfo;
  fc2Context			pgeContext;
  DEVICE_INFO*			devInfo;
  int				oaControl, oaAutoControl, mode, firstMode;
  fc2Property			property;
  fc2PropertyInfo		propertyInfo;
  fc2GigEImageSettings		settings;
  fc2GigEImageSettingsInfo	imageInfo;
  fc2TriggerModeInfo		triggerInfo;
  fc2TriggerMode		triggerMode;
  fc2TriggerDelayInfo		delayInfo;
  fc2TriggerDelay		triggerDelay;
  fc2StrobeInfo			strobeInfo;
  fc2StrobeControl		strobeControl;
  fc2CameraInfo			camInfo;
  unsigned int			i, j, numResolutions, found, xbin, ybin;
  unsigned int			firstBinMode;
  BOOL				supported;
  uint16_t			mask16;
  unsigned int			numberOfSources, numberOfModes;
  unsigned int			dataFormat;
  int				ret, checkDataFormat = 0;

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }

  if (!( cameraInfo = ( PGE_STATE* ) malloc ( sizeof ( PGE_STATE )))) {
    free (( void* ) camera );
    perror ( "malloc PGE_STATE failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  OA_CLEAR ( camera->controls );
  OA_CLEAR ( camera->features );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  _oaInitCameraFunctionPointers ( camera );
  _PGEInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;

  if (( *p_fc2CreateGigEContext )( &pgeContext ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get PGE context\n" );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  if (( *p_fc2Connect )( pgeContext, &devInfo->pgeGuid ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't connect to PGE GUID\n" );
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  if (( *p_fc2GetCameraInfo )( pgeContext, &camInfo ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get camera info for PGE camera\n" );
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  /*
  fprintf(
      stderr,
      "GigE major version - %u\n"
      "GigE minor version - %u\n"
      "User-defined name - %s\n"
      "Model name - %s\n"
      "XML URL1 - %s\n"
      "XML URL2 - %s\n"
      "Firmware version - %s\n"
      "IIDC version - %1.2f\n"
      "MAC address - %02X:%02X:%02X:%02X:%02X:%02X\n"
      "IP address - %u.%u.%u.%u\n"
      "Subnet mask - %u.%u.%u.%u\n"
      "Default gateway - %u.%u.%u.%u\n\n",
      camInfo.gigEMajorVersion,
      camInfo.gigEMinorVersion,
      camInfo.userDefinedName,
      camInfo.modelName,
      camInfo.xmlURL1,
      camInfo.xmlURL2,
      camInfo.firmwareVersion,
      camInfo.iidcVer / 100.0f,
      camInfo.macAddress.octets[0],
      camInfo.macAddress.octets[1],
      camInfo.macAddress.octets[2],
      camInfo.macAddress.octets[3],
      camInfo.macAddress.octets[4],
      camInfo.macAddress.octets[5],
      camInfo.ipAddress.octets[0],
      camInfo.ipAddress.octets[1],
      camInfo.ipAddress.octets[2],
      camInfo.ipAddress.octets[3],
      camInfo.subnetMask.octets[0],
      camInfo.subnetMask.octets[1],
      camInfo.subnetMask.octets[2],
      camInfo.subnetMask.octets[3],
      camInfo.defaultGateway.octets[0],
      camInfo.defaultGateway.octets[1],
      camInfo.defaultGateway.octets[2],
      camInfo.defaultGateway.octets[3]);
  */

  camera->interface = device->interface;

  cameraInfo->colour = devInfo->colour;
  cameraInfo->cfaPattern = devInfo->cfaPattern;

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  // FIX ME -- Frame rate is awkward because the maximum exposure time
  // varies with the frame rate.  For the time being if the frame rate
  // can be changed we'll turn it off and pretend it doesn't exist.

  OA_CLEAR ( propertyInfo );
  propertyInfo.type = FC2_FRAME_RATE;
  if (( *p_fc2GetPropertyInfo )( pgeContext, &propertyInfo ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get property info for PGR frame rate\n" );
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  if ( propertyInfo.present ) {
    OA_CLEAR ( property );
    property.type = FC2_FRAME_RATE;
    if (( *p_fc2GetProperty )( pgeContext, &property ) != FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get property for PGR frame rate\n" );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
    if ( propertyInfo.onOffSupported ) {
      property.onOff = 0;
      property.autoManualMode = 0;
      if (( *p_fc2SetProperty )( pgeContext, &property ) != FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't set property for PGR frame rate\n" );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
    } else {
      fprintf ( stderr, "PGE frame rate exists, but cannot be turned off\n" );
    }
  }

  // There's probably a lot of work still to be done here.

  for ( i = 0; i < FC2_UNSPECIFIED_PROPERTY_TYPE; i++ ) {
    OA_CLEAR ( propertyInfo );
    propertyInfo.type = i;
    if (( *p_fc2GetPropertyInfo )( pgeContext, &propertyInfo ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get property info %d for PGE GUID\n", i );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
    if ( !propertyInfo.present ) {
      continue;
    }
    OA_CLEAR ( property );
    property.type = i;
    if (( *p_fc2GetProperty )( pgeContext, &property ) != FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get property %d for PGE GUID\n", i );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
/*
fprintf ( stderr, "property %d, units: %s, abbrev: %s\n", i, propertyInfo.pUnits, propertyInfo.pUnitAbbr );
fprintf ( stderr, "  on/off: %d, value %d\n", propertyInfo.onOffSupported, property.onOff  );
fprintf ( stderr, "  min: %d, max %d\n", propertyInfo.min, propertyInfo.max );
fprintf ( stderr, "  abs: %d, absmin: %f, absmax: %f\n", propertyInfo.absValSupported, propertyInfo.absMin, propertyInfo.absMax );
fprintf ( stderr, "  auto: %d, manual %d, state: %d\n", propertyInfo.autoSupported, propertyInfo.manualSupported, property.autoManualMode );
*/
    if (( propertyInfo.onOffSupported && property.onOff ) ||
        !propertyInfo.onOffSupported ) {

      oaControl = pgeControls[ i ].oaControl;
      oaAutoControl = pgeControls[ i ].oaAutoControl;

      switch ( i + FC2_BRIGHTNESS ) {

        case FC2_BRIGHTNESS:
        case FC2_SHARPNESS:
        case FC2_HUE:
        case FC2_SATURATION:
        case FC2_GAMMA:
        case FC2_GAIN:
          if ( propertyInfo.manualSupported ) {
            camera->controls[ oaControl ] = OA_CTRL_TYPE_INT32;
            commonInfo->min[ oaControl ] = propertyInfo.min;
            commonInfo->max[ oaControl ] = propertyInfo.max;
            commonInfo->step[ oaControl ] = 1; // arbitrary
            commonInfo->def[ oaControl ] = property.valueA;
          }
          if ( propertyInfo.autoSupported ) {
            if ( oaAutoControl ) {
              camera->controls[ oaAutoControl ] = OA_CTRL_TYPE_BOOLEAN;
              commonInfo->min[ oaAutoControl ] = 0;
              commonInfo->max[ oaAutoControl ] = 1;
              commonInfo->step[ oaAutoControl ] = 1;
              commonInfo->def[ oaAutoControl ] =
                  ( property.autoManualMode ) ? 1 : 0;
            } else {
              fprintf ( stderr, "%s: have auto for control %d, but "
                  "liboacam does not\n", __FUNCTION__, oaControl );
            }
          }
          break;

        case FC2_SHUTTER:
        {
          unsigned int min, max, step, def;
          // shutter is actually exposure time.  exposure is something
          // else
          oaAutoControl = OA_CAM_CTRL_AUTO_EXPOSURE;
          if ( propertyInfo.absValSupported ) {
            oaControl = OA_CAM_CTRL_EXPOSURE_ABSOLUTE;
            camera->controls[ oaControl ] = OA_CTRL_TYPE_INT64;
            // On the Blackfly at least, these values appear to be in seconds
            // rather than as the units string suggests
            min = propertyInfo.absMin * 1000.0;
            max = propertyInfo.absMax * 1000.0;
            step = 1000; // arbitrary
            def = property.absValue * 1000.0;
          } else {
            oaControl = OA_CAM_CTRL_EXPOSURE;
            camera->controls[ oaControl ] = OA_CTRL_TYPE_INT32;
            min = propertyInfo.min;
            max = propertyInfo.max;
            step = 1; // arbitrary
            def = property.valueA;
          }
          commonInfo->min[ oaControl ] = min;
          commonInfo->max[ oaControl ] = max;
          commonInfo->step[ oaControl ] = step;
          commonInfo->def[ oaControl ] = def;
          if ( propertyInfo.autoSupported ) {
            camera->controls[ oaAutoControl ] = OA_CTRL_TYPE_BOOLEAN;
            commonInfo->min[ oaAutoControl ] = OA_EXPOSURE_AUTO;
            commonInfo->max[ oaAutoControl ] = OA_EXPOSURE_MANUAL;
            commonInfo->step[ oaAutoControl ] = 1;
            commonInfo->def[ oaAutoControl ] = ( property.autoManualMode ) ?
                OA_EXPOSURE_AUTO : OA_EXPOSURE_MANUAL;
          }
          break;
        }
        case FC2_WHITE_BALANCE:
          // This is more complex.
          // The manual white balance control is actually achieved by
          // setting the red and blue balances independently (though they
          // both have to be set at the same time).  Auto mode however
          // controls both at once.
          // So, we need manual red and blue balance controls and an auto
          // white balance control

          if ( propertyInfo.manualSupported ) {
            camera->controls[ OA_CAM_CTRL_BLUE_BALANCE ] =
                camera->controls[ OA_CAM_CTRL_RED_BALANCE ] =
                OA_CTRL_TYPE_INT32;
            commonInfo->min[ OA_CAM_CTRL_BLUE_BALANCE ] =
                commonInfo->min[ OA_CAM_CTRL_RED_BALANCE ] =
                propertyInfo.min;
            commonInfo->max[ OA_CAM_CTRL_BLUE_BALANCE ] =
                commonInfo->max[ OA_CAM_CTRL_RED_BALANCE ] =
                propertyInfo.max;
            commonInfo->step[ OA_CAM_CTRL_BLUE_BALANCE ] =
                commonInfo->step[ OA_CAM_CTRL_RED_BALANCE ] = 1;//arbitrary
            commonInfo->def[ OA_CAM_CTRL_RED_BALANCE ] =
                cameraInfo->currentRedBalance = property.valueA;
            commonInfo->def[ OA_CAM_CTRL_BLUE_BALANCE ] =
                cameraInfo->currentBlueBalance = property.valueB;
          }
          if ( propertyInfo.autoSupported ) {
            if ( oaAutoControl ) {
              camera->controls[ oaAutoControl ] = OA_CTRL_TYPE_BOOLEAN;
              commonInfo->min[ oaAutoControl ] = 0;
              commonInfo->max[ oaAutoControl ] = 1;
              commonInfo->step[ oaAutoControl ] = 1;
              commonInfo->def[ oaAutoControl ] =
                  ( property.autoManualMode ) ? 1 : 0;
            } else {
              fprintf ( stderr, "%s: have auto for control %d, but "
                  "liboacam does not\n", __FUNCTION__, oaControl );
            }
          }
          break;

        case FC2_FRAME_RATE:
          fprintf ( stderr, "Need to set up frame rates for PGE camera\n" );
          break;

        case FC2_TEMPERATURE:
          camera->controls[ OA_CAM_CTRL_TEMPERATURE ] = OA_CTRL_TYPE_READONLY;
          break;

        case FC2_TRIGGER_MODE:
          fprintf ( stderr, "%s: unsupported PGE TRIGGER_MODE control\n",
              __FUNCTION__ );
          break;

        case FC2_TRIGGER_DELAY:
          fprintf ( stderr, "%s: unsupported PGE TRIGGER_DELAY control\n",
              __FUNCTION__ );
          break;

        case FC2_AUTO_EXPOSURE:
        case FC2_IRIS:
        case FC2_FOCUS:
        case FC2_ZOOM:
        case FC2_PAN:
        case FC2_TILT:
          fprintf ( stderr, "%s: unsupported PGE control %d\n", __FUNCTION__,
              i + FC2_BRIGHTNESS );
          break;

        default:
          fprintf ( stderr, "%s: unknown PGE control %d\n", __FUNCTION__,
              i + FC2_BRIGHTNESS );
          break;
      }
    }
  }

  // Now sort out whether trigger mode is supported or not

  if (( p_fc2GetTriggerModeInfo )( pgeContext, &triggerInfo ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get trigger mode info %d for PGE GUID\n", i );
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  /*
  fprintf ( stderr, "trigger mode:\n" );
  fprintf ( stderr, "  present  : %d\n", triggerInfo.present ? 1 : 0 );
  fprintf ( stderr, "  readout  : %d\n", triggerInfo.readOutSupported ? 1 : 0 );
  fprintf ( stderr, "  on/off   : %d\n", triggerInfo.onOffSupported ? 1 : 0 );
  fprintf ( stderr, "  polarity : %d\n", triggerInfo.polaritySupported ? 1 : 0 );
  fprintf ( stderr, "  readable : %d\n", triggerInfo.valueReadable ? 1 : 0 );
  fprintf ( stderr, "  src mask : %08x\n", triggerInfo.sourceMask );
  fprintf ( stderr, "  sw trig  : %d\n", triggerInfo.softwareTriggerSupported ? 1 : 0 );
  fprintf ( stderr, "  mode mask: %08x\n", triggerInfo.modeMask );
   */

  camera->features.externalTrigger = triggerInfo.present ? 1 : 0;

  // FIX ME -- need to handle readOutSupported ?
  // FIX ME -- need to handle valueReadable ?
  // FIX ME -- need to handle softwareTriggerSupported ?

  if ( triggerInfo.present ) {

    cameraInfo->triggerEnable = triggerInfo.onOffSupported ? 1 : 0;
    if ( triggerInfo.onOffSupported ) {
      if ( !triggerInfo.valueReadable ) {
        fprintf ( stderr, "Trigger info is not readable. This will break\n" );
      }
      camera->controls[ OA_CAM_CTRL_TRIGGER_ENABLE ] = OA_CTRL_TYPE_BOOLEAN;
      commonInfo->min[ OA_CAM_CTRL_TRIGGER_ENABLE ] = 0;
      commonInfo->max[ OA_CAM_CTRL_TRIGGER_ENABLE ] = 1;
      commonInfo->step[ OA_CAM_CTRL_TRIGGER_ENABLE ] = 1;
      // off appears to be the case for the Blackfly.  I'll assume for all
      commonInfo->def[ OA_CAM_CTRL_TRIGGER_ENABLE ] = 0;
    }

    if ( triggerInfo.polaritySupported ) {
      camera->controls[ OA_CAM_CTRL_TRIGGER_POLARITY ] = OA_CTRL_TYPE_MENU;
      commonInfo->min[ OA_CAM_CTRL_TRIGGER_POLARITY ] = 0;
      commonInfo->max[ OA_CAM_CTRL_TRIGGER_POLARITY ] = 1;
      commonInfo->step[ OA_CAM_CTRL_TRIGGER_POLARITY ] = 1;
      // trailing edge appears to be the case for the Blackfly.  I'll
      // assume for all
      commonInfo->def[ OA_CAM_CTRL_TRIGGER_POLARITY ] = 0;
    }

    // We're going to asssume that the list of sources will be contiguous
    // otherwise it won't work as a menu and we'll have to swap to discrete
    // values.
    // My current interpretation of sourceMask is that bits 0 to 3
    // correspond to GPIO3 to GPIO0 (the camera appears to use a different
    // endianness to x86 as far as I can see).

    numberOfSources = 0;
    if ( triggerInfo.sourceMask ) {
      mask16 = triggerInfo.sourceMask & 0x0f; // only bits 0 to 3
      while ( mask16 ) {
        if ( mask16 & 0x8 ) {
          cameraInfo->triggerGPIO = numberOfSources;
          numberOfSources++;
        } else {
          if ( numberOfSources ) {
            // At this point we don't have a source for the current GPIO pin,
            // but there appear to be sources for higher-numbered pins.  This
            // is not going to lead to happiness at the moment
            fprintf ( stderr, "Available source GPIO pins appear to be "
                "non-contiguous.  This will lead to pain and needs fixing.\n" );
          }
        }
        mask16 = ( mask16 << 1 ) & 0x0f;
      }

      if ( numberOfSources > 1 ) {
        camera->controls[ OA_CAM_CTRL_TRIGGER_SOURCE ] = OA_CTRL_TYPE_MENU;
        commonInfo->min[ OA_CAM_CTRL_TRIGGER_SOURCE ] = 0;
        commonInfo->max[ OA_CAM_CTRL_TRIGGER_SOURCE ] = numberOfSources;
        commonInfo->step[ OA_CAM_CTRL_TRIGGER_SOURCE ] = 1;
        fprintf ( stderr, "Need to set default trigger source value\n" );
        commonInfo->def[ OA_CAM_CTRL_TRIGGER_SOURCE ] = 0;
      }
    }

    if ( triggerInfo.modeMask ) {
      numberOfModes = 0;
      mask16 = cameraInfo->modeMask = triggerInfo.modeMask;
      while ( mask16 ) {
        if ( mask16 & 0x8000 ) {
          numberOfModes++;
        }
        mask16 <<= 1;
      }

      if ( numberOfModes > 1 ) {
        camera->controls[ OA_CAM_CTRL_TRIGGER_MODE ] = OA_CTRL_TYPE_DISC_MENU;
        commonInfo->min[ OA_CAM_CTRL_TRIGGER_MODE ] = 0;
        commonInfo->max[ OA_CAM_CTRL_TRIGGER_MODE ] = numberOfModes;
        commonInfo->step[ OA_CAM_CTRL_TRIGGER_MODE ] = 1;
        // 0 appears to be the case for the Blackfly.  I'll assume for all
        commonInfo->def[ OA_CAM_CTRL_TRIGGER_MODE ] = 0;

        // Now we know how many values there are, cycle through them again to
        // create the discrete values list

        cameraInfo->triggerModeCount = numberOfModes;
        if (!( cameraInfo->triggerModes = calloc ( numberOfModes,
            sizeof ( int64_t )))) {
          fprintf ( stderr, "Can't calloc space for trigger mode list\n" );
          ( *p_fc2DestroyContext )( pgeContext );
          free (( void* ) commonInfo );
          free (( void* ) cameraInfo );
          free (( void* ) camera );
          return 0;
        }
        mask16 = cameraInfo->modeMask;
        int64_t* modep = cameraInfo->triggerModes;
        int modeNumber = 0;
        while ( mask16 ) {
          if ( mask16 & 0x8000 ) {
            *modep++ = modeNumber;
          }
          modeNumber++;
          mask16 <<= 1;
        }
      }
    }

    if ( numberOfSources ) {
      triggerMode.source = cameraInfo->triggerGPIO;
      if (( ret = ( *p_fc2GetTriggerMode )( pgeContext, &triggerMode )) !=
          FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't get trigger mode for PGE GUID\n" );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }

      /*
      fprintf ( stderr, "trigger %d:\n", cameraInfo->triggerGPIO );
      fprintf ( stderr, "  on/off   : %d\n", triggerMode.onOff ? 1 : 0 );
      fprintf ( stderr, "  polarity : %d\n", triggerMode.polarity ? 1 : 0 );
      fprintf ( stderr, "  mode     : %d\n", triggerMode.mode );
      fprintf ( stderr, "  param    : %d\n", triggerMode.parameter );
       */

      cameraInfo->triggerEnabled = triggerMode.onOff;
      cameraInfo->triggerCurrentPolarity = triggerMode.polarity;
      cameraInfo->triggerCurrentMode = triggerMode.mode;

      if (( ret = ( *p_fc2GetTriggerDelayInfo )( pgeContext, &delayInfo )) !=
          FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't get trigger delay info for PGE GUID\n" );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }

      /*
      fprintf ( stderr, "trigger delay info:\n" );
      fprintf ( stderr, "  present  : %d\n", delayInfo.present ? 1 : 0 );
      fprintf ( stderr, "  auto     : %d\n", delayInfo.autoSupported ? 1 : 0 );
      fprintf ( stderr, "  manual   : %d\n", delayInfo.manualSupported ? 1 : 0 );
      fprintf ( stderr, "  on/off   : %d\n", delayInfo.onOffSupported ? 1 : 0 );
      fprintf ( stderr, "  one push : %d\n", delayInfo.onePushSupported ? 1 : 0 );
      fprintf ( stderr, "  absolute : %d\n", delayInfo.absValSupported ? 1 : 0 );
      fprintf ( stderr, "  readout  : %d\n", delayInfo.readOutSupported ? 1 : 0 );
      fprintf ( stderr, "  min      : %d\n", delayInfo.min );
      fprintf ( stderr, "  max      : %d\n", delayInfo.max );
      fprintf ( stderr, "  min      : %f\n", delayInfo.absMin );
      fprintf ( stderr, "  max      : %f\n", delayInfo.absMax );
      fprintf ( stderr, "  units    : %s\n", delayInfo.pUnits );
      fprintf ( stderr, "  units    : %s\n", delayInfo.pUnitAbbr );
       */

      if ( delayInfo.present ) {
        if (( ret = ( *p_fc2GetTriggerDelay )( pgeContext, &triggerDelay )) !=
            FC2_ERROR_OK ) {
          fprintf ( stderr, "Can't get trigger delay for PGE GUID\n" );
          ( *p_fc2DestroyContext )( pgeContext );
          free (( void* ) commonInfo );
          free (( void* ) cameraInfo );
          free (( void* ) camera );
          return 0;
        }

        /*
        fprintf ( stderr, "trigger delay:\n" );
        fprintf ( stderr, "  present  : %d\n", triggerDelay.present ? 1 : 0 );
        fprintf ( stderr, "  absolute : %d\n", triggerDelay.absControl ? 1 : 0 );
        fprintf ( stderr, "  one push : %d\n", triggerDelay.onePush ? 1 : 0 );
        fprintf ( stderr, "  on/off   : %d\n", triggerDelay.onOff ? 1 : 0 );
        fprintf ( stderr, "  auto/man : %d\n", triggerDelay.autoManualMode ? 1 : 0 );
        fprintf ( stderr, "  valueA   : %d\n", triggerDelay.valueA ? 1 : 0 );
        fprintf ( stderr, "  absValue : %f\n", triggerDelay.absValue );
         */

        cameraInfo->triggerDelayEnable = delayInfo.onOffSupported ? 1 : 0;
        if ( delayInfo.onOffSupported ) {
          camera->controls[ OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ] =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ] = 0;
          commonInfo->max[ OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ] = 1;
          commonInfo->step[ OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ] = 1;
          commonInfo->def[ OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ] = 0;
        }

        camera->controls[ OA_CAM_CTRL_TRIGGER_DELAY ] = OA_CTRL_TYPE_INT64;
        commonInfo->min[ OA_CAM_CTRL_TRIGGER_DELAY ] = delayInfo.min * 1000000;
        commonInfo->max[ OA_CAM_CTRL_TRIGGER_DELAY ] = delayInfo.max * 1000000;
        commonInfo->step[ OA_CAM_CTRL_TRIGGER_DELAY ] = 1;
        commonInfo->def[ OA_CAM_CTRL_TRIGGER_DELAY ] = 0;
      }
    }
  }

  // And now it's the turn of strobe mode

  // Multiple pins can be configured as strobe outputs.  The documentation
  // suggests that there are only three bits available for the strobe source,
  // so hopefully we can assume that there can be no more than 8 GPIO lines
  // available.

  for ( i = 0; i < 8; i++ ) {

    strobeInfo.source = i;
    if (( ret = ( *p_fc2GetStrobeInfo )( pgeContext, &strobeInfo )) !=
        FC2_ERROR_OK ) {
      // not an error if this isn't a strobe line
      if ( ret != FC2_ERROR_INVALID_PARAMETER ) {
        fprintf ( stderr, "Can't get strobe mode info for PGE GUID\n" );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
    }

    if ( strobeInfo.present ) {
      /*
      fprintf ( stderr, "GPIO %d strobe mode:\n", i );
      fprintf ( stderr, "  present  : %d\n", strobeInfo.present ? 1 : 0 );
      fprintf ( stderr, "  on/off   : %d\n", strobeInfo.onOffSupported ?
          1 : 0 );
      fprintf ( stderr, "  polarity : %d\n", strobeInfo.polaritySupported ?
          1 : 0 );
      fprintf ( stderr, "  min val  : %f\n", strobeInfo.minValue );
      fprintf ( stderr, "  max val  : %f\n", strobeInfo.maxValue );
       */
      if ( camera->features.strobeOutput ) {
        fprintf ( stderr, "Looks like there is more than one strobe output\n"
            "This could get messy\n" );
      }
      if ( !i ) {
        fprintf ( stderr, "Looks like the strobe output may be the same as"
            "the trigger input.\nThis could get very messy\n" );
      }
 
      camera->features.strobeOutput = strobeInfo.present ? 1 : 0;
      cameraInfo->strobeGPIO = i;

      cameraInfo->strobeEnable = strobeInfo.onOffSupported ? 1 : 0;
      if ( strobeInfo.onOffSupported ) {
        camera->controls[ OA_CAM_CTRL_STROBE_ENABLE ] = OA_CTRL_TYPE_BOOLEAN;
        commonInfo->min[ OA_CAM_CTRL_STROBE_ENABLE ] = 0;
        commonInfo->max[ OA_CAM_CTRL_STROBE_ENABLE ] = 1;
        commonInfo->step[ OA_CAM_CTRL_STROBE_ENABLE ] = 1;
        // on appears to be the case for the Blackfly.  I'll assume for all
        commonInfo->def[ OA_CAM_CTRL_STROBE_ENABLE ] = 1;
      }

      if ( strobeInfo.polaritySupported ) {
        camera->controls[ OA_CAM_CTRL_STROBE_POLARITY ] = OA_CTRL_TYPE_MENU;
        commonInfo->min[ OA_CAM_CTRL_STROBE_POLARITY ] = 0;
        commonInfo->max[ OA_CAM_CTRL_STROBE_POLARITY ] = 1;
        commonInfo->step[ OA_CAM_CTRL_STROBE_POLARITY ] = 1;
        // trailing edge appears to be the case for the Blackfly.  I'll
        // assume for all
        commonInfo->def[ OA_CAM_CTRL_STROBE_POLARITY ] = 0;
      }

      camera->controls[ OA_CAM_CTRL_STROBE_DELAY ] = OA_CTRL_TYPE_INT64;
      commonInfo->min[ OA_CAM_CTRL_STROBE_DELAY ] =
          strobeInfo.minValue * 1000000;
      commonInfo->max[ OA_CAM_CTRL_STROBE_DELAY ] =
          strobeInfo.maxValue * 1000000;
      commonInfo->step[ OA_CAM_CTRL_STROBE_DELAY ] = 1;
      commonInfo->def[ OA_CAM_CTRL_STROBE_DELAY ] = 0;

      camera->controls[ OA_CAM_CTRL_STROBE_DURATION ] = OA_CTRL_TYPE_INT64;
      commonInfo->min[ OA_CAM_CTRL_STROBE_DURATION ] =
          strobeInfo.minValue * 1000000;
      commonInfo->max[ OA_CAM_CTRL_STROBE_DURATION ] =
          strobeInfo.maxValue * 1000000;
      commonInfo->step[ OA_CAM_CTRL_STROBE_DURATION ] = 1;
      commonInfo->def[ OA_CAM_CTRL_STROBE_DURATION ] = 0;
    }
  }

  if ( camera->features.strobeOutput ) {
    strobeControl.source = cameraInfo->strobeGPIO;
    if (( ret = ( *p_fc2GetStrobe )( pgeContext, &strobeControl )) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get strobe control for PGE GUID\n" );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }

    cameraInfo->strobeEnabled = strobeControl.onOff ? 1 : 0;
    cameraInfo->strobeCurrentPolarity = strobeControl.polarity ? 1 : 0;
    cameraInfo->strobeCurrentDelay = strobeControl.delay * 1000000;
    cameraInfo->strobeCurrentDuration = strobeControl.duration * 1000000;
    /*
    fprintf ( stderr, "source %d:\n", cameraInfo->strobeGPIO );
    fprintf ( stderr, "  on/off   : %d\n", strobeControl.onOff ? 1 : 0 );
    fprintf ( stderr, "  polarity : %d\n", strobeControl.polarity ? 1 : 0 );
    fprintf ( stderr, "  delay    : %f\n", strobeControl.delay );
    fprintf ( stderr, "  duration : %f\n", strobeControl.duration );
     */
  }

  // There are problems here if not all colour modes are supported in
  // all resolutions

  cameraInfo->videoRGB24 = 0;
  cameraInfo->videoRaw = 0;
  cameraInfo->videoGrey16 = 0;
  cameraInfo->videoGrey = 0;
  cameraInfo->currentVideoFormat = 0;
  cameraInfo->currentMode = 0;

  camera->features.rawMode = camera->features.demosaicMode = 0;

  numResolutions = 0;
  firstMode = -1;
  firstBinMode = OA_BIN_MODE_NONE;
  for ( mode = FC2_MODE_0; mode < FC2_NUM_MODES; mode++ ) {
    if (( *p_fc2QueryGigEImagingMode )( pgeContext, mode, &supported ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get mode info %d for PGE GUID\n", i );
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
    if ( supported ) {
      if (( *p_fc2SetGigEImagingMode )( pgeContext, mode ) != FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't set mode %d for PGE GUID\n", mode );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
      if (( *p_fc2GetGigEImageSettingsInfo )( pgeContext, &imageInfo ) !=
          FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't get image info %d for PGE GUID\n", i );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
      if (( *p_fc2GetGigEImageBinningSettings )( pgeContext, &xbin, &ybin ) !=
          FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't get binning info %d for PGE GUID\n", i );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }

      if ( xbin != ybin ) {
        fprintf ( stderr, "skipping PGE mode %d x bin %d != y bin %d\n",
            mode, xbin, ybin );
        continue;
      }

      if ( xbin > OA_MAX_BINNING ) {
        fprintf ( stderr, "skipping PGE mode %d binning %d > OA_MAX_BINNING\n",
            mode, xbin );
        continue;
      }

      if ( firstMode == -1 ) {
        firstMode = mode;
        firstBinMode = xbin;
      }
      if ( imageInfo.imageHStepSize || imageInfo.imageVStepSize ) {
        camera->features.ROI = 1;
      }

      found = 0;
      numResolutions = cameraInfo->frameSizes[xbin].numSizes;
      if ( numResolutions ) {
        for ( j = 0; j < numResolutions && !found; j++ ) {
          if ( cameraInfo->frameSizes[xbin].sizes[j].x == imageInfo.maxWidth &&
              cameraInfo->frameSizes[xbin].sizes[j].y == imageInfo.maxHeight ) {
            found = 1;
          }
        }
      }
      if ( !found ) {
        if (!(  cameraInfo->frameSizes[ xbin ].sizes = realloc (
            cameraInfo->frameSizes[ xbin ].sizes, ( numResolutions + 1 ) *
            sizeof ( FRAMESIZE )))) {
          fprintf ( stderr, "malloc for frame sizes failed\n" );
          return 0;
        }
        if (!( cameraInfo->frameModes[ xbin ] = ( struct modeInfo* )
            realloc ( cameraInfo->frameModes[ xbin ],
            sizeof ( struct modeInfo ) * ( numResolutions + 1 )))) {
          fprintf ( stderr, "malloc for frame modes failed\n" );
          return 0;
        }

        cameraInfo->frameSizes[xbin].sizes[ numResolutions ].x =
            imageInfo.maxWidth;
        cameraInfo->frameSizes[xbin].sizes[ numResolutions ].y =
            imageInfo.maxHeight;
        cameraInfo->frameModes[xbin][ numResolutions ].mode = mode;
        if ( imageInfo.maxWidth > cameraInfo->xSize || imageInfo.maxHeight >
            cameraInfo->ySize ) {
          cameraInfo->xSize = imageInfo.maxWidth;
          cameraInfo->ySize = imageInfo.maxHeight;
        }
        cameraInfo->frameSizes[xbin].numSizes++;
      }
    }
  }
  cameraInfo->maxResolutionX = cameraInfo->xSize;
  cameraInfo->maxResolutionY = cameraInfo->ySize;
  cameraInfo->binMode = firstBinMode;
  cameraInfo->pixelFormats = imageInfo.pixelFormatBitField;

  // Put the camera into a known state
  // FIX ME -- probably should just handle whatever is already set?
  if (( *p_fc2SetGigEImagingMode )( pgeContext, firstMode ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d for PGE GUID\n", i );
    // FIX ME -- free frame data
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  if (( *p_fc2GetGigEImageSettings )( pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get settings %d for PGE GUID\n", i );
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  if ( cameraInfo->pixelFormats & FC2_PIXEL_FORMAT_MONO8 ) {
    settings.pixelFormat = FC2_PIXEL_FORMAT_MONO8;
    cameraInfo->currentBytesPerPixel = 1;
  } else {
    // FIX ME
    fprintf ( stderr, "Don't know what to set default camera format to\n" );
  }
  if (( *p_fc2SetGigEImageSettings )( pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set settings %d for PGE GUID\n", i );
    ( *p_fc2DestroyContext )( pgeContext );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  cameraInfo->currentMode = firstMode;
  cameraInfo->currentVideoFormat = settings.pixelFormat;

  // Endianness for 12- and 16-bit images can apparently be
  // forced to little-endian as follows:
  //
  //  const unsigned int k_imageDataFmtReg = 0x1048;
  //  unsigned int value = 0;
  //  error = m_pCamera->ReadRegister( k_imageDataFmtReg, &value );
  //  value &= ~(0x1 << 0);
  //  error = m_pCamera->WriteRegister( k_imageDataFmtReg, value );
  //
  // However, that's not much use if the camera doesn't support it, so
  // perhaps we just need to cope with whatever mode the camera claims to
  // be in

  int depthMask = 0;
  if ( imageInfo.pixelFormatBitField & FC2_PIXEL_FORMAT_MONO8 ) {
    cameraInfo->videoGrey = 1;
    cameraInfo->bytesPerPixel = 1;
    depthMask |= 1;
  }
  if ( imageInfo.pixelFormatBitField & FC2_PIXEL_FORMAT_MONO16 ) {
    cameraInfo->videoGrey16 = 1;
    cameraInfo->bytesPerPixel = 2;
    checkDataFormat = 1;
    depthMask |= 2;
  }
  if ( imageInfo.pixelFormatBitField & FC2_PIXEL_FORMAT_MONO12 ) {
    cameraInfo->videoGrey12 = 1;
    cameraInfo->bytesPerPixel = 2;
    checkDataFormat = 1;
    depthMask |= 2;
  }
  if ( imageInfo.pixelFormatBitField & FC2_PIXEL_FORMAT_BGR ) {
    cameraInfo->videoRGB24 = 1;
    cameraInfo->bytesPerPixel = 3;
    depthMask |= 1;
  }
  if ( imageInfo.pixelFormatBitField & FC2_PIXEL_FORMAT_RGB ) {
    cameraInfo->videoRGB24 = 1;
    cameraInfo->bytesPerPixel = 3;
    depthMask |= 1;
  }
  if ( imageInfo.pixelFormatBitField & FC2_PIXEL_FORMAT_RAW8 ) {
    cameraInfo->videoRaw = 1;
    camera->features.rawMode = 1;
    cameraInfo->bytesPerPixel = 1;
    depthMask |= 1;
  }
  if ( imageInfo.pixelFormatBitField & FC2_PIXEL_FORMAT_RAW16 ) {
    cameraInfo->videoRaw = 1;
    camera->features.rawMode = 1;
    cameraInfo->bytesPerPixel = 2;
    checkDataFormat = 1;
    depthMask |= 2;
  }

  if ( imageInfo.pixelFormatBitField & ~( FC2_PIXEL_FORMAT_MONO8 |
      FC2_PIXEL_FORMAT_MONO16 | FC2_PIXEL_FORMAT_MONO12 |
      FC2_PIXEL_FORMAT_BGR | FC2_PIXEL_FORMAT_RGB |
      FC2_PIXEL_FORMAT_RAW8 | FC2_PIXEL_FORMAT_RAW16 )) {
    fprintf ( stderr, "Unsupported pixel formats exist: 0x%04x\n",
        imageInfo.pixelFormatBitField );
  }

  if ( checkDataFormat ) {
    if ( camInfo.iidcVer >= 132 ) {
      if (( *p_fc2ReadRegister )( pgeContext, FC2_REG_DATA_DEPTH,
          &dataFormat ) != FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't read PGE register 0x%04x\n",
            FC2_REG_DATA_DEPTH );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
      // FIX ME
      // This is allegedly the other way around, but only this way works
      // for me
      cameraInfo->bigEndian = (( dataFormat >> 16 ) & 0x80 ) ? 0 : 1;
    } else {
      if (( *p_fc2ReadRegister )( pgeContext, FC2_REG_IMAGE_DATA_FORMAT,
          &dataFormat ) != FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't read PGE register 0x%04x\n",
            FC2_REG_IMAGE_DATA_FORMAT );
        ( *p_fc2DestroyContext )( pgeContext );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
      if (( dataFormat & 0x80000000 ) == 0 ) {
        fprintf ( stderr, "Image Data Format register unsupported\n" );
      }
      cameraInfo->bigEndian = ( dataFormat & 0xff ) ? 1 : 0;
    }

    // depthMask is only relevant here, where we're checking the data format
    // because we have multiple options for bytes per pixel
    if ( depthMask == 3 ) {
      camera->controls[ OA_CAM_CTRL_BIT_DEPTH ] = OA_CTRL_TYPE_DISCRETE;
    }
  }

  // The largest buffer size we should need

  cameraInfo->buffers = 0;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * cameraInfo->bytesPerPixel;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( struct PGEbuffer ));
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
          cameraInfo->buffers[j].start = 0;
        }
      }
      // FIX ME -- free frame data
      ( *p_fc2DestroyContext )( pgeContext );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
  }

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamPGEcontroller, ( void* ) camera )) {
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamPGEcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }

  cameraInfo->pgeContext = pgeContext;
  cameraInfo->initialised = 1;
  return camera;
}


static void
_PGEInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaPGEInitCamera;
  camera->funcs.closeCamera = oaPGECloseCamera;

  camera->funcs.setControl = oaPGECameraSetControl;
  camera->funcs.readControl = oaPGECameraReadControl;
  camera->funcs.testControl = oaPGECameraTestControl;
  camera->funcs.getControlRange = oaPGECameraGetControlRange;
  camera->funcs.getControlDiscreteSet = oaPGECameraGetControlDiscreteSet;

  camera->funcs.startStreaming = oaPGECameraStartStreaming;
  camera->funcs.stopStreaming = oaPGECameraStopStreaming;
  camera->funcs.isStreaming = oaPGECameraIsStreaming;

  camera->funcs.setResolution = oaPGECameraSetResolution;
  camera->funcs.setROI = oaPGECameraSetROI;
  camera->funcs.testROISize = oaPGECameraTestROISize;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaPGECameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaPGECameraGetFramePixelFormat;

  camera->funcs.enumerateFrameRates = oaPGECameraGetFrameRates;
  camera->funcs.setFrameInterval = oaPGECameraSetFrameInterval;

  camera->funcs.getMenuString = oaPGECameraGetMenuString;
}


int
oaPGECloseCamera ( oaCamera* camera )
{
  void*		dummy;
  PGE_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    ( *p_fc2DestroyContext )( cameraInfo->pgeContext );

    if ( cameraInfo->frameRates.numRates ) {
     free (( void* ) cameraInfo->frameRates.rates );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    free (( void* ) camera->_common );
    free (( void* ) cameraInfo );
    free (( void* ) camera );

  } else {
    return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
