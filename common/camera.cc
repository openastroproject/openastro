/*****************************************************************************
 *
 * camera.cc -- camera interface class
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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

#include <openastro/demosaic.h>

#include "captureSettings.h"
#include "fitsSettings.h"
#include "demosaicSettings.h"
#include "camera.h"

#define	DEFAULT_FRAME_TIME	100

#define cameraFuncs		cameraContext->funcs
#define cameraControls(c)	cameraContext->OA_CAM_CTRL_TYPE(c)
#define cameraFeatures		cameraContext->features

Camera::Camera()
{
  initialised = 0;
}


Camera::~Camera()
{
  if ( initialised ) {
    stop();
    disconnect();
  }
}


void
Camera::populateControlValue ( oaControlValue* cp, uint32_t c, int64_t v )
{
  cp->valueType = cameraControls( c );
  switch ( cameraControls( c ) ) {
    case OA_CTRL_TYPE_INT32:
      cp->int32 = v & 0xffffffff;
      break;
    case OA_CTRL_TYPE_BOOLEAN:
      cp->boolean = v ? 1 : 0;
      break;
    case OA_CTRL_TYPE_MENU:
    case OA_CTRL_TYPE_DISC_MENU:
      cp->menu = v & 0xffffffff;
      break;
    case OA_CTRL_TYPE_BUTTON:
      break;
    case OA_CTRL_TYPE_INT64:
      cp->int64 = v;
      break;
    case OA_CTRL_TYPE_DISCRETE:
      cp->discrete = v & 0xffffffff;
      break;
    default:
      qWarning() << __FUNCTION__ << " called with invalid control type " <<
        cameraControls( c ) << " for control " << c;
  }
}


int64_t
Camera::unpackControlValue ( oaControlValue *cp )
{
  int64_t res;

  switch ( cp->valueType ) {
    case OA_CTRL_TYPE_INT32:
      res = cp->int32;
      break;
    case OA_CTRL_TYPE_BOOLEAN:
      res = cp->boolean;
      break;
    case OA_CTRL_TYPE_MENU:
    case OA_CTRL_TYPE_DISC_MENU:
      res = cp->menu;
      break;
    case OA_CTRL_TYPE_READONLY:
      res = cp->readonly;
      break;
    case OA_CTRL_TYPE_INT64:
      res = cp->int64;
      // FIX ME -- because at the moment Qt controls can only handle 32-bit
      // values
      res &= 0xffffffff;
      break;
    case OA_CTRL_TYPE_DISCRETE:
      res = cp->discrete;
      break;
    default:
      qWarning() << "Camera" << __FUNCTION__ <<
        " called with invalid control type " <<
        cameraControls( cp->valueType );
      res = -1;
  }

  return res;
}


int
Camera::hasFrameFormat ( int format )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }

  return cameraContext->frameFormats[ format ];
}


int
Camera::hasBinning ( int64_t factor )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  if ( !cameraControls( OA_CAM_CTRL_BINNING )) {
    return 0;
  }
  populateControlValue ( &v, OA_CAM_CTRL_BINNING, factor );
  return ( OA_ERR_NONE == cameraFuncs.testControl ( cameraContext,
      OA_CAM_CTRL_BINNING, &v )) ? 1 : 0;
}


int
Camera::testROISize ( unsigned int xRes, unsigned int yRes,
    unsigned int* suggX, unsigned int* suggY )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }

  return cameraFuncs.testROISize ( cameraContext, xRes, yRes, suggX, suggY );
}


int
Camera::hasROI ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.flags & OA_CAM_FEATURE_ROI;
}


int Camera::hasFixedFrameSizes ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.flags & OA_CAM_FEATURE_FIXED_FRAME_SIZES;
}


int Camera::hasUnknownFrameSize ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.flags & OA_CAM_FEATURE_FRAME_SIZE_UNKNOWN;
}


int
Camera::hasFrameRateSupport ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.flags & OA_CAM_FEATURE_FRAME_RATES;
}


int
Camera::hasFixedFrameRates ( int xRes __attribute__((unused)),
		int yRes __attribute__((unused)))
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }

  return cameraFeatures.flags & OA_CAM_FEATURE_FRAME_RATES;
}


int
Camera::hasReadableControls ( void )
{
	if ( !initialised ) {
		qWarning() << __FUNCTION__ << " called with camera uninitialised";
		return 0;
	}
	return cameraFeatures.flags & OA_CAM_FEATURE_READABLE_CONTROLS;
}


int
Camera::hasControl ( int control )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }

  return cameraControls( control );
}


int
Camera::hasAuto ( int control )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }

  return ( cameraFuncs.hasAuto ( cameraContext, control ) > 0 ) ? 1 : 0;
}


int
Camera::isAuto ( int control )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  } 

  return cameraFuncs.isAuto ( cameraContext, control );
}


int
Camera::isColour ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return ( cameraFeatures.flags & ( OA_CAM_FEATURE_RAW_MODE |
			OA_CAM_FEATURE_DEMOSAIC_MODE ));
}


float
Camera::getTemperature ( void )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  if ( !cameraControls( OA_CAM_CTRL_TEMPERATURE )) {
    return -273.15;
  }
  cameraFuncs.readControl ( cameraContext, OA_CAM_CTRL_TEMPERATURE, &v );
  float tempReading = v.int32;
  tempReading /= 10;
  return tempReading;
}


const char*
Camera::name()
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return "";
  }
  return cameraContext->deviceName;
}


void
Camera::controlRange ( int control, int64_t* min, int64_t* max,
    int64_t* step, int64_t* def )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  } else {
    cameraFuncs.getControlRange ( cameraContext, control, min, max, step, def );
  }
  // FIX ME -- this is because the sliders can't handle more than 32-bit
  // values
  *min &= 0xffffffff;
  *max &= 0xffffffff;
  *step &= 0xffffffff;
  *def &= 0xffffffff;
  return;
}


void
Camera::controlDiscreteSet ( int control, int32_t *count, int64_t** values )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  } else {
    cameraFuncs.getControlDiscreteSet ( cameraContext, control, count, values );
  }
  return;
}


const FRAMESIZES*
Camera::frameSizes ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFuncs.enumerateFrameSizes ( cameraContext );
}


// FIX ME -- might be nice to make this a tidier type at some point.  vector?
const FRAMERATES*
Camera::frameRates ( int xRes, int yRes)
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFuncs.enumerateFrameRates ( cameraContext, xRes, yRes );
}


// FIX ME -- might be nice to make this a tidier type at some point.  vector?
int
Camera::listConnected ( oaCameraDevice*** devs, unsigned long flags )
{
  return ( oaGetCameras ( devs, flags ));
}


void
Camera::releaseInfo ( oaCameraDevice** devs )
{
  return ( oaReleaseCameras ( devs ));
}


// FIX ME -- handling of the return code here is really scabby.  Do it
// properly at some point

int
Camera::initialise ( oaCameraDevice* device, const char* appName,
		QWidget* topWidget )
{
  int ret;

  disconnect();

  if ( !device ) {
    qWarning() << "device is null!";
    return -1;
  }

  // At the moment this is really just for OSX
  if ( device->hasLoadableFirmware && !device->firmwareLoaded ) {

    QMessageBox* loading = new QMessageBox ( QMessageBox::NoIcon,
        appName, tr ( "Attempting to load camera firmware" ),
        QMessageBox::NoButton, topWidget );
    QAbstractButton* b = loading->button ( QMessageBox::Ok );
    if ( b ) {
      loading->removeButton ( b );
    }
    QApplication::setOverrideCursor ( Qt::WaitCursor );
    loading->show();
    ret = device->loadFirmware ( device );
    QApplication::restoreOverrideCursor();
    loading->hide();
    delete loading;

    if ( ret != OA_ERR_NONE ) {
      switch ( -ret ) {

        case OA_ERR_RESCAN_BUS:
          return 1;
          break;

        case OA_ERR_MANUAL_FIRMWARE:
          QMessageBox::warning ( topWidget, appName,
              tr ( "The firmware could not be loaded.  It must be loaded "
              "manually." ));
          break;

        case OA_ERR_FXLOAD_NOT_FOUND:
          QMessageBox::warning ( topWidget, appName,
              tr ( "The firmware could not be loaded.  The fxload utility "
              "was not found." ));
          break;

        case OA_ERR_FXLOAD_ERROR:
          QMessageBox::warning ( topWidget, appName,
              tr ( "The firmware could not be loaded.  The fxload utility "
              "returned an error." ));
          break;

        case OA_ERR_FIRMWARE_UNKNOWN:
          QMessageBox::warning ( topWidget, appName,
              tr ( "The firmware could not be found." ));
          break;

        default:
          QMessageBox::warning ( topWidget, appName,
              tr ( "Unexpected error loading firmware." ));
          break;
      }

      return -1;
    }
  }

  if (( cameraContext = device->initCamera ( device ))) {
    initialised = 1;
    return 0;
  }
  return -1;
}


void
Camera::disconnect ( void )
{
  if ( initialised ) {
    cameraFuncs.closeCamera ( cameraContext );
    initialised = 0;
  }
}


int
Camera::setControl ( int control, int64_t value )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  populateControlValue ( &v, control, value );
  return cameraFuncs.setControl ( cameraContext, control, &v, 0 );
}


int64_t
Camera::readControl ( int control )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }

  if ( cameraFuncs.readControl ( cameraContext, control, &v ) ==
			OA_ERR_NONE ) {
		return unpackControlValue ( &v );
	}
	qWarning() << "error trying to read control" << control;
	return 0;
}


int
Camera::setResolution ( int x, int y )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  return cameraFuncs.setResolution ( cameraContext, x, y );
}


int
Camera::setROI ( int x, int y )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  return cameraFuncs.setROI ( cameraContext, x, y );
}


int
Camera::setFrameInterval ( int numerator, int denominator )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  return cameraFuncs.setFrameInterval ( cameraContext, numerator,
      denominator );
}


int
Camera::startStreaming ( void* ( *callback )( void*, void*, int, void* ),
		void* state )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  return cameraFuncs.startStreaming ( cameraContext, callback, state );
}


void
Camera::stop ( void )
{
  if ( !initialised ) {
    return;
  }

  if (( cameraFeatures.flags & OA_CAM_FEATURE_STREAMING ) &&
			cameraFuncs.isStreaming ( cameraContext )) {
    cameraFuncs.stopStreaming ( cameraContext );
		return;
  }

	if ( cameraFeatures.flags & OA_CAM_FEATURE_SINGLE_SHOT ) {
    cameraFuncs.abortExposure ( cameraContext );
	}
}


int
Camera::videoFramePixelFormat ( void )
{
  int format;

  format = cameraFuncs.getFramePixelFormat ( cameraContext );
  if ( demosaicConf.monoIsRawColour ) {
    if ( format == OA_PIX_FMT_GREY16LE ) {
      switch ( demosaicConf.cfaPattern ) {
        case OA_DEMOSAIC_RGGB:
          format = OA_PIX_FMT_RGGB16LE;
          break;
        case OA_DEMOSAIC_BGGR:
          format = OA_PIX_FMT_BGGR16LE;
          break;
        case OA_DEMOSAIC_GRBG:
          format = OA_PIX_FMT_GRBG16LE;
          break;
        case OA_DEMOSAIC_GBRG:
          format = OA_PIX_FMT_GBRG16LE;
          break;
        case OA_DEMOSAIC_CMYG:
          format = OA_PIX_FMT_CMYG16LE;
          break;
        case OA_DEMOSAIC_MCGY:
          format = OA_PIX_FMT_MCGY16LE;
          break;
        case OA_DEMOSAIC_YGCM:
          format = OA_PIX_FMT_YGCM16LE;
          break;
        case OA_DEMOSAIC_GYMC:
          format = OA_PIX_FMT_GYMC16LE;
          break;
      }
    } else {
      if ( format == OA_PIX_FMT_GREY16BE ) {
        switch ( demosaicConf.cfaPattern ) {
          case OA_DEMOSAIC_RGGB:
            format = OA_PIX_FMT_RGGB16BE;
            break;
          case OA_DEMOSAIC_BGGR:
            format = OA_PIX_FMT_BGGR16BE;
            break;
          case OA_DEMOSAIC_GRBG:
            format = OA_PIX_FMT_GRBG16BE;
            break;
          case OA_DEMOSAIC_GBRG:
            format = OA_PIX_FMT_GBRG16BE;
            break;
          case OA_DEMOSAIC_CMYG:
            format = OA_PIX_FMT_CMYG16BE;
            break;
          case OA_DEMOSAIC_MCGY:
            format = OA_PIX_FMT_MCGY16BE;
            break;
          case OA_DEMOSAIC_YGCM:
            format = OA_PIX_FMT_YGCM16BE;
            break;
          case OA_DEMOSAIC_GYMC:
            format = OA_PIX_FMT_GYMC16BE;
            break;
        }
      }
    }
  }
  return format;
}


int
Camera::setFrameFormat ( int format )
{
  int ret;
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  populateControlValue ( &v, OA_CAM_CTRL_FRAME_FORMAT, format );
  ret = cameraFuncs.setControl ( cameraContext, OA_CAM_CTRL_FRAME_FORMAT,
      &v, 0 );
  return ret;
}


int
Camera::isInitialised ( void )
{
  return initialised;
}


int
Camera::hasRawMode ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? ( cameraFeatures.flags & OA_CAM_FEATURE_RAW_MODE ) : 0;
}


int
Camera::hasDemosaicMode ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
  }
  return initialised ? ( cameraFeatures.flags &
			OA_CAM_FEATURE_DEMOSAIC_MODE ) : 0;
}


const char*
Camera::getMenuString ( int control, int index )
{
 if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return "";
  }
  return cameraFuncs.getMenuString ( cameraContext, control, index );
}


int
Camera::getAWBManualSetting ( void )
{
 if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFuncs.getAutoWBManualSetting ( cameraContext );
}


int
Camera::pixelSizeX ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.pixelSizeX;
}


int
Camera::pixelSizeY ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.pixelSizeY;
}


int
Camera::frameSizeUnknown ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.flags & OA_CAM_FEATURE_FRAME_SIZE_UNKNOWN;
}


int
Camera::startExposure ( time_t when,
		void* ( *callback )( void*, void*, int, void* ), void* state )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return -1;
  }

  return cameraFuncs.startExposure ( cameraContext, when, callback, state );
}


int
Camera::isSingleShot ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with camera uninitialised";
    return 0;
  }
  return cameraFeatures.flags & OA_CAM_FEATURE_SINGLE_SHOT;
}
