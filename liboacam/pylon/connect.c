/*****************************************************************************
 *
 * connect.c -- Initialise Basler Pylon cameras
 *
 * Copyright 2020
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

#include <pylonc/PylonC.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/demosaic.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "private.h"
#include "oacam.h"
#include "state.h"
#include "nodes.h"


static void _pylonInitFunctionPointers ( oaCamera* );

pylonFrameInfo	_frameFormats[17] = {
	{ "Mono8", OA_PIX_FMT_GREY8 },
	{ "Mono10", OA_PIX_FMT_GREY10 },
	{ "Mono12", OA_PIX_FMT_GREY12_16LE },
	{ "RGB8", OA_PIX_FMT_RGB24 },
	{ "BGR8", OA_PIX_FMT_BGR24 },
	{ "BayerBG8", OA_PIX_FMT_BGGR8 },
	{ "BayerBG10", OA_PIX_FMT_BGGR10 },
	{ "BayerBG12", OA_PIX_FMT_BGGR12 },
	{ "BayerGB8", OA_PIX_FMT_GBRG8 },
	{ "BayerGB10", OA_PIX_FMT_GBRG10 },
	{ "BayerGB12", OA_PIX_FMT_GBRG12 },
	{ "BayerGR8", OA_PIX_FMT_GRBG8 },
	{ "BayerGR10", OA_PIX_FMT_GRBG10 },
	{ "BayerGR12", OA_PIX_FMT_GRBG12},
	{ "BayerRG8", OA_PIX_FMT_RGGB8 },
	{ "BayerRG10", OA_PIX_FMT_RGGB10 },
	{ "BayerRG12", OA_PIX_FMT_RGGB12 },
};

pylonFilterInfo	_filterTypes[4] = {
	{ "BayerRG", OA_DEMOSAIC_RGGB },
	{ "BayerGB", OA_DEMOSAIC_GBRG },
	{ "BayerGR", OA_DEMOSAIC_GRBG },
	{ "BayerBG", OA_DEMOSAIC_BGGR }
};

#define CLOSE_PYLON	\
		( p_PylonDeviceClose )( cameraInfo->deviceHandle ); \
		( p_PylonDestroyDevice )( cameraInfo->deviceHandle ); \
		( p_PylonTerminate )();


oaCamera*
oaPylonInitCamera ( oaCameraDevice* device )
{
  oaCamera*									camera;
	PYLON_STATE*							cameraInfo;
  COMMON_INFO*							commonInfo;
  DEVICE_INFO*							devInfo;
	size_t										deviceIndex;
	//size_t										numNodes, numFeatures, numProps;
	PYLON_DEVICE_HANDLE				deviceHandle;
	// PYLON_DEVICE_INFO_HANDLE	infoHandle;
	NODEMAP_HANDLE						nodeMap;
	NODE_HANDLE								node, format, setting;
	//NODE_HANDLE								controls, format, aoi, colour, image, timer;
	GENAPIC_RESULT						res;
	int												i, j, ret;
	int												binMax, numFormats, gainFound, expTimeFound;
	unsigned int							expTimeCtrl;
	char											strBuff[256];
	_Bool											available;
	// char											descBuff[2048];
	int64_t										imin, imax, istep, icurr;
	int64_t										hmin, hmax, hstep, hcurr;
	int64_t										vmin, vmax, vstep, vcurr;
	double										fmin, fmax, fcurr;
	size_t										len;

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( PYLON_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  _pylonInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  camera->interface = device->interface;
  cameraInfo->initialised = 0;
  devInfo = device->_private;

	( p_PylonInitialize )();

	deviceIndex = devInfo->devIndex;
	if (( p_PylonCreateDeviceByIndex )( deviceIndex, &deviceHandle ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "Error creating Basler device by index\n" );
		FREE_DATA_STRUCTS;
		( p_PylonTerminate )();
		return 0;
	}
  cameraInfo->deviceHandle = deviceHandle;

#ifdef DEBUG
	if (( res = ( p_PylonGetDeviceInfoHandle )( deviceIndex, &infoHandle )) !=
			GENAPI_E_OK ) {
		unsigned int r = res;
		fprintf ( stderr, "PylonGetDeviceInfoHandle() failed: %08x\n", r );
		FREE_DATA_STRUCTS;
		( p_PylonTerminate )();
		return 0;
	}

	if (( p_PylonDeviceInfoGetNumProperties )( infoHandle, &numProps ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "PylonDeviceInfoGetNumProperties() failed\n" );
		FREE_DATA_STRUCTS;
		( p_PylonTerminate )();
		return 0;
	}

	for ( i = 0; i < numProps; i++ ) {
		len = sizeof ( strBuff );
		if (( p_PylonDeviceInfoGetPropertyName )( infoHandle, i, strBuff,
				&len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "PylonDeviceInfoGetPropertyName () failed\n" );
			( p_PylonTerminate )();
			FREE_DATA_STRUCTS;
			return 0;
		}
		printf ( "property %d = '%s'\n", i, strBuff );
	}
#endif	/* DEBUG */

	if (( p_PylonDeviceOpen )( deviceHandle, PYLONC_ACCESS_MODE_CONTROL |
			PYLONC_ACCESS_MODE_STREAM ) != GENAPI_E_OK ) {
		fprintf ( stderr, "Error opening Basler device %d\n", i );
		FREE_DATA_STRUCTS;
		( p_PylonTerminate )();
		return 0;
	}

	if (( res = ( p_PylonDeviceGetNodeMap )( deviceHandle, &nodeMap ))
			!= GENAPI_E_OK ) {
		unsigned int r = res;
		fprintf ( stderr, "PylonDeviceGetNodeMap() failed: %08x\n", r );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

#ifdef DEBUG
	if (( p_GenApiNodeMapGetNumNodes )( nodeMap, &numNodes ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeMapGetNumNodes() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	for ( i = 0; i < numNodes; i++ ) {
		len = sizeof ( strBuff );
		if (( p_GenApiNodeMapGetNodeByIndex )( nodeMap, i, &node )
				!= GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeMapGetNodeByIndex () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeGetName )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		len = sizeof ( descBuff );
		if (( p_GenApiNodeGetDescription )( node, descBuff, &len ) !=
				GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetDescription() failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		printf ( "node %d name = '%s'\n%s\n\n", i, strBuff, descBuff );
	}

	if (( p_GenApiNodeMapGetNode )( nodeMap, "AnalogControls", &controls ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeMapGetNode() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	if (( p_GenApiCategoryGetNumFeatures )( controls, &numFeatures ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiCategoryGetNumFeatures() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	for ( i = 0; i < numFeatures; i++ ) {
		len = sizeof ( strBuff );
		if (( p_GenApiCategoryGetFeatureByIndex )( controls, i, &node )
				!= GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiCategoryGetFeatureByIndex () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeGetName )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		printf ( "controls, feature %d = '%s'\n", i, strBuff );
	}

	if (( p_GenApiNodeMapGetNode )( nodeMap, "ImageFormat", &format ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeMapGetNode() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	if (( p_GenApiCategoryGetNumFeatures )( format, &numFeatures ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiCategoryGetNumFeatures() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	for ( i = 0; i < numFeatures; i++ ) {
		len = sizeof ( strBuff );
		if (( p_GenApiCategoryGetFeatureByIndex )( format, i, &node )
				!= GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiCategoryGetFeatureByIndex () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeGetName )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		printf ( "format, feature %d = '%s'\n", i, strBuff );
	}

	if (( p_GenApiNodeMapGetNode )( nodeMap, "AOI", &aoi ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeMapGetNode() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	if (( p_GenApiCategoryGetNumFeatures )( aoi, &numFeatures ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiCategoryGetNumFeatures() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	for ( i = 0; i < numFeatures; i++ ) {
		len = sizeof ( strBuff );
		if (( p_GenApiCategoryGetFeatureByIndex )( aoi, i, &node )
				!= GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiCategoryGetFeatureByIndex () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeGetName )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		printf ( "AOI, feature %d = '%s'\n", i, strBuff );
	}

	if (( p_GenApiNodeMapGetNode )( nodeMap, "ColorImprovementsControl",
			&colour ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeMapGetNode() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	if (( p_GenApiCategoryGetNumFeatures )( colour, &numFeatures ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiCategoryGetNumFeatures() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	for ( i = 0; i < numFeatures; i++ ) {
		len = sizeof ( strBuff );
		if (( p_GenApiCategoryGetFeatureByIndex )( colour, i, &node )
				!= GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiCategoryGetFeatureByIndex () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeGetName )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		printf ( "colour, feature %d = '%s'\n", i, strBuff );
	}

	if (( p_GenApiNodeMapGetNode )( nodeMap, "AcquisitionTrigger",
			&image ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeMapGetNode() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	if (( p_GenApiCategoryGetNumFeatures )( image, &numFeatures ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiCategoryGetNumFeatures() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	for ( i = 0; i < numFeatures; i++ ) {
		len = sizeof ( strBuff );
		if (( p_GenApiCategoryGetFeatureByIndex )( image, i, &node )
				!= GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiCategoryGetFeatureByIndex () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeGetName )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		printf ( "image, feature %d = '%s'\n", i, strBuff );
	}

	if (( p_GenApiNodeMapGetNode )( nodeMap, "TimerControls",
			&timer ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeMapGetNode() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	if (( p_GenApiCategoryGetNumFeatures )( timer, &numFeatures ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiCategoryGetNumFeatures() failed\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	for ( i = 0; i < numFeatures; i++ ) {
		len = sizeof ( strBuff );
		if (( p_GenApiCategoryGetFeatureByIndex )( timer, i, &node )
				!= GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiCategoryGetFeatureByIndex () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeGetName )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName () failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		printf ( "timer, feature %d = '%s'\n", i, strBuff );
	}
#endif
	
  cameraInfo->runMode = CAM_RUN_MODE_STOPPED;

  camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;

	// There are a number of options here depending on the version of the
	// naming conventions used by the camera.

	gainFound = 0;
	cameraInfo->gainIsFloat = 0;
	if (( ret = _pylonGetFloatNode ( nodeMap, "Gain", 0x03, &node )) ==
			OA_ERR_NONE ) {
		if ( _pylonGetFloatSettings ( node, &fmin, &fmax, &fcurr ) !=
				OA_ERR_NONE ) {
			fprintf ( stderr, "Can't get settings for Gain\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		cameraInfo->gainIsFloat = 1;
		fprintf ( stderr, "can't handle float gain: %f, %f, %f\n", fmin,
				fmax, fcurr );
	}

	if ( !gainFound ) {
		if (( ret = _pylonGetIntNode ( nodeMap, "GainRaw", 0x03, &node )) ==
				OA_ERR_NONE ) {
			if ( _pylonGetIntSettings ( node, &imin, &imax, &istep, &icurr ) !=
					OA_ERR_NONE ) {
				fprintf ( stderr, "Can't get settings for GainRaw\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			fprintf ( stderr, "gain: %ld, %ld, %ld, %ld\n", imin, imax, istep,
					icurr );
			camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT64;
			commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = imin;
			commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = imax;
			commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = istep;
			// a guess here
			commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = icurr;
			gainFound = 1;
		}
	}

	if ( gainFound ) {
		if (( ret = _pylonGetEnumerationNode ( nodeMap, "GainAuto", 0x3,
				&node )) == OA_ERR_NONE ) {
			// _pylonShowEnumValues ( node, "GainAuto" );
			if (( p_GenApiEnumerationGetEntryByName )( node, "Off", &setting ) !=
					GENAPI_E_OK ) {
				fprintf ( stderr, "Can't get GainAuto Off setting by name\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			if (( p_GenApiNodeIsAvailable )( setting, &available ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Can't get GainAuto Off availability\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			if ( available ) {
				if (( p_GenApiEnumerationGetEntryByName )( node, "Continuous",
						&setting ) != GENAPI_E_OK ) {
					fprintf ( stderr, "Can't get GainAuto Continuous setting by name\n" );
					FREE_DATA_STRUCTS;
					CLOSE_PYLON;
					return 0;
				}
				if (( p_GenApiNodeIsAvailable )( setting, &available ) !=
						GENAPI_E_OK ) {
					fprintf ( stderr, "Can't get GainAuto Continuous availability\n" );
					FREE_DATA_STRUCTS;
					CLOSE_PYLON;
					return 0;
				}
				if ( available ) {
					int autogain = OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN );
					camera->OA_CAM_CTRL_TYPE( autogain ) = OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_MIN( autogain ) = 0;
					commonInfo->OA_CAM_CTRL_MAX( autogain ) = 1;
					commonInfo->OA_CAM_CTRL_STEP( autogain ) = 1;
					commonInfo->OA_CAM_CTRL_DEF( autogain ) = 1;
					fprintf ( stderr, "GainAuto is available\n" );
				}
			}
		}
	}

	// Ideally need to pick up gamma here, but across different versions
	// of the Genicam spec the implementation seems to change quite a bit :(

	// White balance looks a bit messy, too.  Perhaps should just turn it
	// to manual and reset it?

	// And now the exposure controls.  First we want to set "ExposureMode" to
	// "Timed"

	if (( ret = _pylonGetEnumerationNode ( nodeMap, "ExposureMode", 0x03,
			&node )) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get Pylon ExposureMode, err = %d\n", ret );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	if (( p_GenApiEnumerationGetEntryByName )( node, "Timed", &setting ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "Can't get ExposureMode Timed setting by name\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}
	if (( p_GenApiNodeIsAvailable )( setting, &available ) != GENAPI_E_OK ) {
		fprintf ( stderr, "Can't get ExposureMode Timed availability\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}
	if ( available ) {
		if (( p_GenApiNodeFromString )( node, "Timed" ) != GENAPI_E_OK ) { 
			fprintf ( stderr, "Can't set ExposureMode = Timed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
	} else {
		fprintf ( stderr, "Don't know if ExposureMode has a suitable setting\n" );
	}

  // And then find the valid settings for ExposureTime (or ExposureTimeAbs,
	// or ExposureTimeRaw)

	expTimeFound = 0;
	if (( ret = _pylonGetFloatNode ( nodeMap, "ExposureTime", 0x03, &node )) ==
			OA_ERR_NONE ) {
		if ( _pylonGetFloatSettings ( node, &fmin, &fmax, &fcurr ) !=
				OA_ERR_NONE ) {
			fprintf ( stderr, "Can't get settings for ExposureTime\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		// the float values are already in microseconds
		imin = fmin;
		imax = fmax;
		istep = 1;
		icurr = fcurr;
		expTimeFound = 1;
		expTimeCtrl = OA_CAM_CTRL_EXPOSURE_ABSOLUTE;
		( void ) strcpy ( cameraInfo->exposureTimeName, "ExposureTime" );
	}

	if ( !expTimeFound ) {
		if (( ret = _pylonGetFloatNode ( nodeMap, "ExposureTimeAbs", 0x03,
				&node )) == OA_ERR_NONE ) {
			if ( _pylonGetFloatSettings ( node, &fmin, &fmax, &fcurr ) !=
					OA_ERR_NONE ) {
				fprintf ( stderr, "Can't get settings for ExposureTimeAbs\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			// the float values are in microseconds
			imin = fmin;
			imax = fmax;
			istep = 1;
			icurr = fcurr;
			expTimeFound = 1;
			expTimeCtrl = OA_CAM_CTRL_EXPOSURE_ABSOLUTE;
			( void ) strcpy ( cameraInfo->exposureTimeName, "ExposureTimeAbs" );
		}
	}

	if ( !expTimeFound ) {
		if (( ret = _pylonGetIntNode ( nodeMap, "ExposureTimeRaw", 0x03,
				&node )) == OA_ERR_NONE ) {
			if ( _pylonGetIntSettings ( node, &imin, &imax, &istep, &icurr ) !=
					OA_ERR_NONE ) {
				fprintf ( stderr, "Can't get settings for ExposureTimeRaw\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			expTimeFound = 1;
			expTimeCtrl = OA_CAM_CTRL_EXPOSURE_UNSCALED;
		}
	}

	if ( expTimeFound ) {
		camera->OA_CAM_CTRL_TYPE( expTimeCtrl ) = OA_CTRL_TYPE_INT64;
		commonInfo->OA_CAM_CTRL_MIN( expTimeCtrl ) = imin;
		commonInfo->OA_CAM_CTRL_MAX( expTimeCtrl ) = imax;
		commonInfo->OA_CAM_CTRL_STEP( expTimeCtrl ) = istep;
		// a guess here
		commonInfo->OA_CAM_CTRL_DEF( expTimeCtrl ) = icurr;
		fprintf ( stderr, "exposure time: %ld, %ld, %ld, %ld\n", imin, imax,
				istep, icurr );
	}

	// And do we have an auto mode for exposure?

	if ( expTimeFound ) {
		if (( ret = _pylonGetEnumerationNode ( nodeMap, "ExposureAuto", 0x3,
				&node )) == OA_ERR_NONE ) {
			// _pylonShowEnumValues ( node, "ExposureAuto" );
			if (( p_GenApiEnumerationGetEntryByName )( node, "Off", &setting ) !=
					GENAPI_E_OK ) {
				fprintf ( stderr, "Can't get ExposureAuto Off setting by name\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			if (( p_GenApiNodeIsAvailable )( setting, &available ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Can't get ExposureAuto Off availability\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			if ( available ) {
				if (( p_GenApiEnumerationGetEntryByName )( node, "Continuous",
						&setting ) != GENAPI_E_OK ) {
					fprintf ( stderr,
							"Can't get ExposureAuto Continuous setting by name\n" );
					FREE_DATA_STRUCTS;
					CLOSE_PYLON;
					return 0;
				}
				if (( p_GenApiNodeIsAvailable )( setting, &available ) !=
						GENAPI_E_OK ) {
					fprintf ( stderr,
							"Can't get ExposureAuto Continuous availability\n" );
					FREE_DATA_STRUCTS;
					CLOSE_PYLON;
					return 0;
				}
				if ( available ) {
					int autoexp = OA_CAM_CTRL_MODE_AUTO( expTimeCtrl );
					camera->OA_CAM_CTRL_TYPE( autoexp ) = OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_MIN( autoexp ) = 0;
					commonInfo->OA_CAM_CTRL_MAX( autoexp ) = 1;
					commonInfo->OA_CAM_CTRL_STEP( autoexp ) = 1;
					commonInfo->OA_CAM_CTRL_DEF( autoexp ) = 1;
					fprintf ( stderr, "ExposureAuto is available\n" );
				}
			}
		}
	}

	// horizontal flip

	if (( ret = _pylonGetBooleanNode ( nodeMap, "ReverseX", 0x03, &node )) ==
			OA_ERR_NONE ) {
		int flip = OA_CAM_CTRL_HFLIP;
		camera->OA_CAM_CTRL_TYPE( flip ) = OA_CTRL_TYPE_BOOLEAN;
		commonInfo->OA_CAM_CTRL_MIN( flip ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( flip ) = 1;
		commonInfo->OA_CAM_CTRL_STEP( flip ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( flip ) = 0;
	}

	// and vertical flip

	if (( ret = _pylonGetBooleanNode ( nodeMap, "ReverseY", 0x03, &node )) ==
			OA_ERR_NONE ) {
		int flip = OA_CAM_CTRL_VFLIP;
		camera->OA_CAM_CTRL_TYPE( flip ) = OA_CTRL_TYPE_BOOLEAN;
		commonInfo->OA_CAM_CTRL_MIN( flip ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( flip ) = 1;
		commonInfo->OA_CAM_CTRL_STEP( flip ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( flip ) = 0;
	}

	binMax = 1;
	if (( ret = _pylonGetIntNode ( nodeMap, "BinningHorizontal", 0x03,
			&node )) == OA_ERR_NONE ) {
		if ( _pylonGetIntSettings ( node, &hmin, &hmax, &hstep, &hcurr ) !=
				OA_ERR_NONE ) {
			fprintf ( stderr, "Can't get settings for BinningHorizontal\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		cameraInfo->node_BinningHorizontal = node;
		if ( hmax > 1 ) {
			if (( ret = _pylonGetIntNode ( nodeMap, "BinningVertical", 0x03,
					&node )) == OA_ERR_NONE ) {
				if ( _pylonGetIntSettings ( node, &vmin, &vmax, &vstep, &vcurr ) !=
						OA_ERR_NONE ) {
					fprintf ( stderr, "Can't get settings for BinningVertical\n" );
					FREE_DATA_STRUCTS;
					CLOSE_PYLON;
					return 0;
				}
			}
			cameraInfo->node_BinningVertical = node;
			if ( vmax > 1 ) {
				if ( hstep != 1 || vstep != 1 ) {
					fprintf ( stderr, "Can't handle mismatched binning steps\n" );
				} else {
					binMax = ( hmax > vmax ) ? vmax : hmax;
					if ( binMax > 1 ) {
						camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) =
								OA_CTRL_TYPE_INT32;
						commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BINNING ) = 1;
						commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BINNING ) = binMax;
						commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BINNING ) = 1;
						commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BINNING ) = 1;
					}
				}
			}
		}
	}
	cameraInfo->maxBinning = binMax;
			
	// get the frame size
	// FIX ME -- handle non-writeable frame sizes.  Need to know the size even
	// if it can't be changed

	camera->features.flags |= OA_CAM_FEATURE_ROI;
	if (( ret = _pylonGetIntNode ( nodeMap, "Width", 0x03, &node )) ==
			OA_ERR_NONE || ret == -OA_ERR_NOT_WRITEABLE ) {
		if ( _pylonGetIntSettings ( node, &imin, &imax, &istep, &icurr ) !=
				OA_ERR_NONE ) {
			fprintf ( stderr, "Can't get settings for Width\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		cameraInfo->minResolutionX = imin;
		cameraInfo->maxResolutionX = imax;
		cameraInfo->xSize = icurr;
		cameraInfo->xSizeStep = istep;
		if ( ret == -OA_ERR_NOT_WRITEABLE ) {
			camera->features.flags &= ~OA_CAM_FEATURE_ROI;
		}
	} else {
		fprintf ( stderr, "Can't determine frame width\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}
	if (( ret = _pylonGetIntNode ( nodeMap, "Height", 0x03, &node )) ==
			OA_ERR_NONE || ret == -OA_ERR_NOT_WRITEABLE ) {
		if ( _pylonGetIntSettings ( node, &imin, &imax, &istep, &icurr ) !=
				OA_ERR_NONE ) {
			fprintf ( stderr, "Can't get settings for Height\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		cameraInfo->minResolutionY = imin;
		cameraInfo->maxResolutionY = imax;
		cameraInfo->ySize = icurr;
		cameraInfo->ySizeStep = istep;
		if ( ret == -OA_ERR_NOT_WRITEABLE ) {
			camera->features.flags &= ~OA_CAM_FEATURE_ROI;
		}
	} else {
		if ( ret == -OA_ERR_NOT_WRITEABLE ) {
		} else {
			fprintf ( stderr, "Can't determine frame width\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
	}

	// Find which frame formats are available

	if (( ret = _pylonGetEnumerationNode ( nodeMap, "PixelFormat", 1, &node )) !=
			OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get Pylon PixelFormat, err = %d\n", ret );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	len = sizeof ( strBuff );
	if (( p_GenApiNodeToString )( node, strBuff, &len ) != GENAPI_E_OK ) {
		fprintf ( stderr, "Can't get current Pylon PixelFormat\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	cameraInfo->currentFrameFormatIdx = -1;
  numFormats = sizeof ( _frameFormats ) / sizeof ( pylonFrameInfo );
	for ( i = 0; i < numFormats; i++ ) {
		if (( p_GenApiEnumerationGetEntryByName )( node,
				_frameFormats[i].pylonName, &format ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiEnumerationGetEntryByName() failed\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if ( format != GENAPIC_INVALID_HANDLE ) {
			if (( p_GenApiNodeIsAvailable )( format, &available ) != GENAPI_E_OK ) {
				fprintf ( stderr, "GenApiEnumerationGetEntryByName() failed\n" );
				FREE_DATA_STRUCTS;
				CLOSE_PYLON;
				return 0;
			}
			if ( available ) {
				fprintf ( stderr, "format '%s' is supported\n",
						_frameFormats[i].pylonName );
				int oaFormat = _frameFormats[i].pixFormat;
				if ( !oaFrameFormats[ oaFormat ].monochrome ) {
					cameraInfo->colour = 1;
				}
				camera->frameFormats[ oaFormat ] = 1;
				if ( !strcmp ( strBuff, _frameFormats[i].pylonName )) {
					cameraInfo->currentFrameFormatIdx = i;
					cameraInfo->currentFrameFormat = oaFormat;
				}
				if ( cameraInfo->maxBytesPerPixel <
						oaFrameFormats[ oaFormat ].bytesPerPixel ) {
					cameraInfo->maxBytesPerPixel =
							oaFrameFormats[ oaFormat ].bytesPerPixel;
				}
				if ( oaFrameFormats[ oaFormat ].rawColour ) {
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
				}
				if ( oaFrameFormats[ oaFormat ].fullColour ) {
					camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
				}
			}
		}
	}

	if ( cameraInfo->currentFrameFormatIdx < 0 ) {
		// FIX ME -- set first available format?
		fprintf ( stderr, "Can't determine current frame format\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	cameraInfo->cfaPattern = 0;
	if ( cameraInfo->colour ) {
		if (( ret = _pylonGetEnumerationNode ( nodeMap, "PixelColorFilter", 1,
				&node )) != OA_ERR_NONE ) {
			fprintf ( stderr, "Can't get Pylon PixelColorFilter, err = %d\n", ret );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
	
		len = sizeof ( strBuff );
		if (( p_GenApiNodeToString )( node, strBuff, &len ) != GENAPI_E_OK ) {
			fprintf ( stderr, "Can't get current Pylon PixelFormat\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}

		for ( i = 0; i < 4 && !cameraInfo->cfaPattern; i++ ) {
			if ( !strcmp ( strBuff, _filterTypes[i].pylonName )) {
				cameraInfo->cfaPattern = _filterTypes[i].filter;
			}
		}
	}

	for ( i = 1; i <= binMax; i++ ) {
		if (!( cameraInfo->frameSizes[i].sizes = ( FRAMESIZE* ) malloc (
					sizeof ( FRAMESIZE )))) {
			fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
			if ( i ) {
			}
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		cameraInfo->frameSizes[i].sizes[0].x = cameraInfo->maxResolutionX / i;
		cameraInfo->frameSizes[i].sizes[0].y = cameraInfo->maxResolutionY / i;
		cameraInfo->frameSizes[i].numSizes = 1;
	}
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;
  camera->features.flags |= OA_CAM_FEATURE_FIXED_FRAME_SIZES;

	if (( ret = _pylonGetEnumerationNode ( nodeMap, "AcquisitionMode", 0x1,
			&node )) == OA_ERR_NONE ) {
		if (( p_GenApiEnumerationGetEntryByName )( node, "SingleFrame",
				&setting ) != GENAPI_E_OK ) {
			fprintf ( stderr,
					"Can't get AcquisitionMode SingleFrame setting by name\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeIsAvailable )( setting, &available ) != GENAPI_E_OK ) {
			fprintf ( stderr,
					"Can't get AcquisitionMode SingleFrame availability\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if ( available ) {
			camera->features.flags |= OA_CAM_FEATURE_SINGLE_SHOT;
		}
		if (( p_GenApiEnumerationGetEntryByName )( node, "Continuous",
				&setting ) != GENAPI_E_OK ) {
			fprintf ( stderr,
					"Can't get AcquisitionMode SingleFrame setting by name\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if (( p_GenApiNodeIsAvailable )( setting, &available ) != GENAPI_E_OK ) {
			fprintf ( stderr,
					"Can't get AcquisitionMode SingleFrame availability\n" );
			FREE_DATA_STRUCTS;
			CLOSE_PYLON;
			return 0;
		}
		if ( available ) {
			camera->features.flags |= OA_CAM_FEATURE_STREAMING;
		}
	}
	if (!( camera->features.flags & ( OA_CAM_FEATURE_SINGLE_SHOT |
			OA_CAM_FEATURE_STREAMING ))) {
		fprintf ( stderr, "Can't find any suitable acquisition mode\n" );
		FREE_DATA_STRUCTS;
		CLOSE_PYLON;
		return 0;
	}

	cameraInfo->binMode = OA_BIN_MODE_NONE;
	cameraInfo->currentBytesPerPixel = 1;
	cameraInfo->imageBufferLength = 0;
	cameraInfo->currentAbsoluteExposure = 100; // dummy value

	// FIX ME -- Handle frame rates?

	// FIX ME -- put the camera into a known state?
	//   full size frame
	//   no binning
	//   RGB8/Mono8
	//   unflipped

	// Don't know if these will need turning off, but it's in the demo
	// code.

  if (( p_PylonDeviceFeatureIsAvailable )( cameraInfo->deviceHandle,
			"EnumEntry_TriggerSelector_AcquisitionStart" )) {
    if (( p_PylonDeviceFeatureFromString )( cameraInfo->deviceHandle,
					"TriggerSelector", "AcquisitionStart" ) == GENAPI_E_OK ) {
      if (( p_PylonDeviceFeatureFromString )( cameraInfo->deviceHandle,
						"TriggerMode", "Off" ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Can't set TriggerMode\n" );
			}
		} else {
			fprintf ( stderr, "Can't set TriggerSelector\n" );
		}
	}

  if (( p_PylonDeviceFeatureIsAvailable )( cameraInfo->deviceHandle,
			"EnumEntry_TriggerSelector_FrameBurstStart" )) {
    if (( p_PylonDeviceFeatureFromString )( cameraInfo->deviceHandle,
					"TriggerSelector", "FrameBurstStart" ) == GENAPI_E_OK ) {
      if (( p_PylonDeviceFeatureFromString )( cameraInfo->deviceHandle,
						"TriggerMode", "Off" ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Can't set TriggerMode\n" );
			}
		} else {
			fprintf ( stderr, "Can't set TriggerSelector\n" );
		}
	}

  if (( p_PylonDeviceFeatureIsAvailable )( cameraInfo->deviceHandle,
			"EnumEntry_TriggerSelector_FrameStart" )) {
    if (( p_PylonDeviceFeatureFromString )( cameraInfo->deviceHandle,
					"TriggerSelector", "FrameStart" ) == GENAPI_E_OK ) {
      if (( p_PylonDeviceFeatureFromString )( cameraInfo->deviceHandle,
						"TriggerMode", "Off" ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Can't set TriggerMode\n" );
			}
		} else {
			fprintf ( stderr, "Can't set TriggerSelector\n" );
		}
	}

	cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( frameBuffer ));
	for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
		cameraInfo->buffers[i].start = 0;
	}
	cameraInfo->configuredBuffers = 0;

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
	cameraInfo->nextBuffer = 0;
	cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
	cameraInfo->buffersFree = OA_CAM_BUFFERS;

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamPylonController, ( void* ) camera )) {
		for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
			if ( cameraInfo->frameSizes[ j ].numSizes ) {
				free (( void* ) cameraInfo->frameSizes[ j ].sizes );
			}
		}
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
		free (( void* ) cameraInfo->buffers );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
		CLOSE_PYLON;
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamPylonCallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
		for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
			if ( cameraInfo->frameSizes[ j ].numSizes ) {
				free (( void* ) cameraInfo->frameSizes[ j ].sizes );
			}
		}
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
		free (( void* ) cameraInfo->buffers );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
		CLOSE_PYLON;
    return 0;
  }

  cameraInfo->initialised = 1;
  return camera;
}


static void
_pylonInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaPylonInitCamera;
  camera->funcs.closeCamera = oaPylonCloseCamera;

  camera->funcs.testControl = oaPylonCameraTestControl;
  camera->funcs.getControlRange = oaPylonCameraGetControlRange;
  // camera->funcs.getControlDiscreteSet = oaPylonCameraGetControlDiscreteSet;

  camera->funcs.testROISize = oaPylonCameraTestROISize;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaPylonCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaPylonCameraGetFramePixelFormat;

  // camera->funcs.enumerateFrameRates = oaPylonCameraGetFrameRates;

  // camera->funcs.getMenuString = oaPylonCameraGetMenuString;
}


int
oaPylonCloseCamera ( oaCamera* camera )
{
  void*		dummy;
  PYLON_STATE*	cameraInfo;
	int			j;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    CLOSE_PYLON;

    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }

		for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
			if ( cameraInfo->frameSizes[ j ].numSizes ) {
				free (( void* ) cameraInfo->frameSizes[ j ].sizes );
			}
		}

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );

		free (( void* ) cameraInfo->buffers );
    free (( void* ) camera->_common );
    free (( void* ) cameraInfo );
    free (( void* ) camera );

  } else {
    return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
