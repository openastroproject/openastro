/*****************************************************************************
 *
 * Spindynloader.c -- dynamic loader for Spinnaker
 *
 * Copyright 2021,2023
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#include <spinc/SpinnakerC.h>

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Spinoacam.h"
#include "Spin.h"

// Pointers to Spinnaker functions so we can use them via libdl.

SPINNAKERC_API	( *p_spinSystemGetInstance )( spinSystem* );
SPINNAKERC_API	( *p_spinCameraListClear )( spinCameraList );
SPINNAKERC_API	( *p_spinCameraListCreateEmpty )( spinCameraList* );
SPINNAKERC_API	( *p_spinCameraListDestroy )( spinCameraList );
SPINNAKERC_API	( *p_spinCameraListGetSize )( spinCameraList, size_t* );
SPINNAKERC_API	( *p_spinInterfaceListClear )( spinInterfaceList );
SPINNAKERC_API	( *p_spinInterfaceListCreateEmpty )( spinInterfaceList* );
SPINNAKERC_API	( *p_spinInterfaceListDestroy )( spinInterfaceList );
SPINNAKERC_API	( *p_spinInterfaceListGetSize )( spinInterfaceList, size_t* );
SPINNAKERC_API	( *p_spinSystemGetCameras )( spinSystem, spinCameraList );
SPINNAKERC_API	( *p_spinSystemGetInterfaces )( spinSystem, spinInterfaceList );
SPINNAKERC_API	( *p_spinSystemReleaseInstance )( spinSystem );
SPINNAKERC_API	( *p_spinInterfaceListGet )( spinInterfaceList, size_t,
			spinInterface );
SPINNAKERC_API	( *p_spinInterfaceRelease )( spinInterface );
SPINNAKERC_API	( *p_spinInterfaceGetTLNodeMap )( spinInterface,
			spinNodeMapHandle* );
SPINNAKERC_API	( *p_spinNodeMapGetNumNodes )( spinNodeMapHandle, size_t* );
SPINNAKERC_API	( *p_spinNodeMapGetNode )( spinNodeMapHandle, const char*,
			spinNodeHandle* );
SPINNAKERC_API	( *p_spinNodeMapGetNodeByIndex )( spinNodeMapHandle, size_t,
			spinNodeHandle* );
SPINNAKERC_API	( *p_spinNodeIsAvailable )( spinNodeHandle, bool8_t* );
SPINNAKERC_API	( *p_spinNodeIsReadable )( spinNodeHandle, bool8_t* );
SPINNAKERC_API	( *p_spinNodeIsWritable )( spinNodeHandle, bool8_t* );
SPINNAKERC_API	( *p_spinNodeIsImplemented )( spinNodeHandle, bool8_t* );
SPINNAKERC_API	( *p_spinStringGetValue )( spinNodeHandle, char*, size_t* );
SPINNAKERC_API	( *p_spinEnumerationEntryGetEnumValue )( spinNodeHandle,
			size_t* );
SPINNAKERC_API	( *p_spinEnumerationEntryGetIntValue )( spinNodeHandle,
			int64_t* );
SPINNAKERC_API	( *p_spinEnumerationSetEnumValue )( spinNodeHandle, size_t );
SPINNAKERC_API	( *p_spinEnumerationSetIntValue )( spinNodeHandle, int64_t );
SPINNAKERC_API	( *p_spinEnumerationEntryGetSymbolic )( spinNodeHandle,
			char*, size_t* );
SPINNAKERC_API	( *p_spinInterfaceGetCameras )( spinInterface, spinCameraList );
SPINNAKERC_API	( *p_spinCameraListGet )( spinCameraList, size_t, spinCamera* );
SPINNAKERC_API	( *p_spinCameraGetTLDeviceNodeMap )( spinCamera,
			spinNodeMapHandle* );
SPINNAKERC_API	( *p_spinCameraRelease )( spinCamera );
SPINNAKERC_API	( *p_spinCameraGetNodeMap )( spinCamera, spinNodeMapHandle* );
SPINNAKERC_API	( *p_spinCategoryGetNumFeatures )( spinNodeMapHandle, size_t* );
SPINNAKERC_API	( *p_spinCategoryGetFeatureByIndex )( spinNodeMapHandle,
			size_t, spinNodeMapHandle* );
SPINNAKERC_API	( *p_spinNodeGetType )( spinNodeHandle, spinNodeType* );
SPINNAKERC_API	( *p_spinNodeGetName )( spinNodeHandle, char*, size_t* );
SPINNAKERC_API	( *p_spinNodeGetDisplayName )( spinNodeHandle, char*, size_t* );
SPINNAKERC_API	( *p_spinCameraInit )( spinCamera );
SPINNAKERC_API	( *p_spinCameraDeInit )( spinCamera );
SPINNAKERC_API	( *p_spinCameraGetGuiXml )( spinCamera, char*, size_t* );
SPINNAKERC_API	( *p_spinEnumerationGetNumEntries )( spinNodeHandle, size_t* );
SPINNAKERC_API	( *p_spinEnumerationGetEntryByIndex )( spinNodeHandle, size_t,
			spinNodeHandle* );
SPINNAKERC_API	( *p_spinEnumerationGetEntryByName )( spinNodeHandle,
			const char*, spinNodeHandle* );
SPINNAKERC_API	( *p_spinEnumerationGetCurrentEntry )( spinNodeHandle,
			spinNodeHandle* );
SPINNAKERC_API	( *p_spinNodeToString )( spinNodeHandle, char*, size_t* );
SPINNAKERC_API	( *p_spinIntegerGetMin )( spinNodeHandle, int64_t* );
SPINNAKERC_API	( *p_spinIntegerGetMax )( spinNodeHandle, int64_t* );
SPINNAKERC_API	( *p_spinIntegerGetInc )( spinNodeHandle, int64_t* );
SPINNAKERC_API	( *p_spinIntegerGetValue )( spinNodeHandle, int64_t* );
SPINNAKERC_API	( *p_spinIntegerSetValue )( spinNodeHandle, int64_t );
SPINNAKERC_API	( *p_spinBooleanGetValue )( spinNodeHandle, bool8_t* );
SPINNAKERC_API	( *p_spinBooleanSetValue )( spinNodeHandle, bool8_t );
SPINNAKERC_API	( *p_spinFloatGetMin )( spinNodeHandle, double* );
SPINNAKERC_API	( *p_spinFloatGetMax )( spinNodeHandle, double* );
SPINNAKERC_API	( *p_spinFloatGetValue )( spinNodeHandle, double* );
SPINNAKERC_API	( *p_spinFloatSetValue )( spinNodeHandle, double );
SPINNAKERC_API	( *p_spinCameraBeginAcquisition )( spinCamera );
SPINNAKERC_API	( *p_spinCameraEndAcquisition )( spinCamera );
#if HAVE_LIBSPINNAKER_V1
SPINNAKERC_API	( *p_spinImageEventCreate )( spinImageEvent*,
				spinImageEventFunction, void* );
SPINNAKERC_API	( *p_spinCameraRegisterImageEvent )( spinCamera,
				spinImageEvent );
SPINNAKERC_API	( *p_spinCameraUnregisterImageEvent )( spinCamera,
				spinImageEvent );
SPINNAKERC_API	( *p_spinImageEventDestroy )( spinImageEvent );
#else
SPINNAKERC_API	( *p_spinImageEventHandlerCreate )( spinImageEventHandler*,
				spinImageEventFunction, void* );
SPINNAKERC_API	( *p_spinCameraRegisterImageEventHandler )( spinCamera,
				spinImageEventHandler );
SPINNAKERC_API	( *p_spinCameraUnregisterImageEventHandler )( spinCamera,
				spinImageEventHandler );
SPINNAKERC_API	( *p_spinImageEventHandlerDestroy )( spinImageEventHandler );
#endif
SPINNAKERC_API	( *p_spinImageIsIncomplete )( spinImage, bool8_t* );
SPINNAKERC_API	( *p_spinImageGetStatus )( spinImage, spinImageStatus* );
SPINNAKERC_API	( *p_spinImageGetData )( spinImage, void** );
SPINNAKERC_API	( *p_spinImageGetValidPayloadSize )( spinImage, size_t* );

static void*		_getDLSym ( void*, const char* );

/**
 * Cycle through the list of cameras returned by the Spinnaker library
 */

int
_spinInitLibraryFunctionPointers ( void )
{
  static void*		libHandle = 0;
	char						libPath[ PATH_MAX+1 ];
#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*		libName = "/usr/local/lib/libSpinnaker_C.dylib";
#else
#if HAVE_LIBSPINNAKER_V1
  const char*		libName = "libSpinnaker_C.so.1";
#else
#if HAVE_LIBSPINNAKER_V2
  const char*		libName = "libSpinnaker_C.so.2";
#else
#if HAVE_LIBSPINNAKER_V3
  const char*		libName = "libSpinnaker_C.so.3";
#endif
#endif
#endif
#endif
#ifdef RETRY_SO_WITHOUT_PATH
	int						tryWithoutPath = 1;
#endif

	*libPath = 0;
	dlerror();
  if ( !libHandle ) {
		if ( installPathRoot ) {
			( void ) strncpy ( libPath, installPathRoot, PATH_MAX );
		}
#ifdef SHLIB_PATH
		( void ) strncat ( libPath, SHLIB_PATH, PATH_MAX );
#endif
#ifdef RETRY_SO_WITHOUT_PATH
retry:
#endif
		( void ) strncat ( libPath, libName, PATH_MAX );

    if (!( libHandle = dlopen ( libPath, RTLD_LAZY ))) {
#ifdef RETRY_SO_WITHOUT_PATH
			if ( tryWithoutPath ) {
				tryWithoutPath = 0;
				*libPath = 0;
				goto retry;
			}
#endif
			oaLogWarning ( OA_LOG_CAMERA, "%s: %s not loaded, error '%s'", __func__,
					libPath, dlerror());
	    return OA_ERR_LIBRARY_NOT_FOUND;
	  }

		if (!( *( void** )( &p_spinSystemGetInstance ) = _getDLSym ( libHandle,
		    "spinSystemGetInstance" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraListClear ) = _getDLSym ( libHandle,
		    "spinCameraListClear" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraListCreateEmpty ) = _getDLSym ( libHandle,
		    "spinCameraListCreateEmpty" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraListDestroy ) = _getDLSym ( libHandle,
		    "spinCameraListDestroy" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraListGetSize ) = _getDLSym ( libHandle,
		    "spinCameraListGetSize" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceListClear ) = _getDLSym ( libHandle,
		    "spinInterfaceListClear" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceListCreateEmpty ) = _getDLSym ( libHandle,
		    "spinInterfaceListCreateEmpty" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceListDestroy ) = _getDLSym ( libHandle,
		    "spinInterfaceListDestroy" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceListGetSize ) = _getDLSym ( libHandle,
		    "spinInterfaceListGetSize" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinSystemGetCameras ) = _getDLSym ( libHandle,
		    "spinSystemGetCameras" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinSystemGetInterfaces ) = _getDLSym ( libHandle,
		    "spinSystemGetInterfaces" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinSystemReleaseInstance ) = _getDLSym ( libHandle,
		    "spinSystemReleaseInstance" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceListGet ) = _getDLSym ( libHandle,
		    "spinInterfaceListGet" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceRelease ) = _getDLSym ( libHandle,
		    "spinInterfaceRelease" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceGetTLNodeMap ) = _getDLSym ( libHandle,
		    "spinInterfaceGetTLNodeMap" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeMapGetNumNodes ) = _getDLSym ( libHandle,
		    "spinNodeMapGetNumNodes" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeMapGetNode ) = _getDLSym ( libHandle,
		    "spinNodeMapGetNode" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeMapGetNodeByIndex ) = _getDLSym ( libHandle,
		    "spinNodeMapGetNodeByIndex" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeIsAvailable ) = _getDLSym ( libHandle,
		    "spinNodeIsAvailable" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeIsImplemented ) = _getDLSym ( libHandle,
		    "spinNodeIsImplemented" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeIsReadable ) = _getDLSym ( libHandle,
		    "spinNodeIsReadable" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeIsWritable ) = _getDLSym ( libHandle,
		    "spinNodeIsWritable" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinStringGetValue ) = _getDLSym ( libHandle,
		    "spinStringGetValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationEntryGetEnumValue ) = _getDLSym (
		    libHandle, "spinEnumerationEntryGetEnumValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationSetEnumValue ) = _getDLSym (
		    libHandle, "spinEnumerationSetEnumValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationEntryGetIntValue ) = _getDLSym (
		    libHandle, "spinEnumerationEntryGetIntValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationSetIntValue ) = _getDLSym (
		    libHandle, "spinEnumerationSetIntValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationEntryGetSymbolic ) = _getDLSym (
		    libHandle, "spinEnumerationEntryGetSymbolic" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinInterfaceGetCameras ) = _getDLSym ( libHandle,
		    "spinInterfaceGetCameras" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraListGet ) = _getDLSym ( libHandle,
		    "spinCameraListGet" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraGetTLDeviceNodeMap ) = _getDLSym ( libHandle,
		    "spinCameraGetTLDeviceNodeMap" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraRelease ) = _getDLSym ( libHandle,
		    "spinCameraRelease" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraGetNodeMap ) = _getDLSym ( libHandle,
		    "spinCameraGetNodeMap" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraGetNodeMap ) = _getDLSym ( libHandle,
		    "spinCameraGetNodeMap" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCategoryGetNumFeatures ) = _getDLSym ( libHandle,
		    "spinCategoryGetNumFeatures" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCategoryGetFeatureByIndex ) = _getDLSym (
		    libHandle, "spinCategoryGetFeatureByIndex" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeGetType ) = _getDLSym ( libHandle,
		    "spinNodeGetType" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeGetName ) = _getDLSym ( libHandle,
		    "spinNodeGetName" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeGetDisplayName ) = _getDLSym ( libHandle,
		    "spinNodeGetDisplayName" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraInit ) = _getDLSym ( libHandle,
		    "spinCameraInit" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraDeInit ) = _getDLSym ( libHandle,
		    "spinCameraDeInit" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraGetGuiXml ) = _getDLSym ( libHandle,
		    "spinCameraGetGuiXml" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationGetNumEntries ) = _getDLSym ( libHandle,
		    "spinEnumerationGetNumEntries" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationGetEntryByIndex ) = _getDLSym (
		    libHandle, "spinEnumerationGetEntryByIndex" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationGetEntryByName ) = _getDLSym (
		    libHandle, "spinEnumerationGetEntryByName" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinEnumerationGetCurrentEntry ) = _getDLSym (
		    libHandle, "spinEnumerationGetCurrentEntry" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinNodeToString ) = _getDLSym ( libHandle,
		    "spinNodeToString" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinIntegerGetMin ) = _getDLSym ( libHandle,
		    "spinIntegerGetMin" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinIntegerGetMax ) = _getDLSym ( libHandle,
		    "spinIntegerGetMax" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinIntegerGetInc ) = _getDLSym ( libHandle,
		    "spinIntegerGetInc" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinIntegerGetValue ) = _getDLSym ( libHandle,
		    "spinIntegerGetValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinIntegerSetValue ) = _getDLSym ( libHandle,
		    "spinIntegerSetValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinBooleanGetValue ) = _getDLSym ( libHandle,
		    "spinBooleanGetValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinBooleanSetValue ) = _getDLSym ( libHandle,
		    "spinBooleanSetValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinFloatGetMin ) = _getDLSym ( libHandle,
		    "spinFloatGetMin" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinFloatGetMax ) = _getDLSym ( libHandle,
		    "spinFloatGetMax" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinFloatGetValue ) = _getDLSym ( libHandle,
		    "spinFloatGetValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinFloatSetValue ) = _getDLSym ( libHandle,
		    "spinFloatSetValue" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraBeginAcquisition ) = _getDLSym ( libHandle,
		    "spinCameraBeginAcquisition" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraEndAcquisition ) = _getDLSym ( libHandle,
		    "spinCameraEndAcquisition" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
#if HAVE_LIBSPINNAKER_V1
		if (!( *( void** )( &p_spinImageEventCreate ) = _getDLSym ( libHandle,
		    "spinImageEventCreate" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraRegisterImageEvent ) =
				_getDLSym ( libHandle, "spinCameraRegisterImageEvent" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraUnregisterImageEvent ) =
				_getDLSym ( libHandle, "spinCameraUnregisterImageEvent" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinImageEventDestroy ) =
				_getDLSym ( libHandle, "spinImageEventDestroy" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
#else
		if (!( *( void** )( &p_spinImageEventHandlerCreate ) = _getDLSym ( libHandle,
		    "spinImageEventHandlerCreate" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraRegisterImageEventHandler ) =
				_getDLSym ( libHandle, "spinCameraRegisterImageEventHandler" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinCameraUnregisterImageEventHandler ) =
				_getDLSym ( libHandle, "spinCameraUnregisterImageEventHandler" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinImageEventHandlerDestroy ) =
				_getDLSym ( libHandle, "spinImageEventHandlerDestroy" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
#endif
		if (!( *( void** )( &p_spinImageIsIncomplete ) = _getDLSym ( libHandle,
		    "spinImageIsIncomplete" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinImageGetStatus ) =
				_getDLSym ( libHandle, "spinImageGetStatus" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinImageGetData ) = _getDLSym ( libHandle,
				"spinImageGetData" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
		if (!( *( void** )( &p_spinImageGetValidPayloadSize ) = _getDLSym (
				libHandle, "spinImageGetValidPayloadSize" ))) {
		  return -OA_ERR_SYMBOL_NOT_FOUND;
		}
	}

  return OA_ERR_NONE;
}


static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    oaLogError ( OA_LOG_CAMERA, "%s: spinnaker DL error: %s", __func__, error );
    addr = 0;
  }

  return addr;
}

#endif	/* HAVE_LIBDL */
