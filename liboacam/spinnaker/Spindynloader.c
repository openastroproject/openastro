/*****************************************************************************
 *
 * Spindyloader.c -- dynamic loader for Spinnaker
 *
 * Copyright 2021
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
#endif
#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <openastro/util.h>
#include <spinc/SpinnakerC.h>

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
SPINNAKERC_API	( *p_spinNodeMapGetNode )( spinNodeMapHandle, const char*,
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
SPINNAKERC_API	( *p_spinImageEventHandlerCreate )( spinImageEventHandler*,
				spinImageEventFunction, void* );
SPINNAKERC_API	( *p_spinCameraRegisterImageEventHandler )( spinCamera,
				spinImageEventHandler );
SPINNAKERC_API	( *p_spinCameraUnregisterImageEventHandler )( spinCamera,
				spinImageEventHandler );
SPINNAKERC_API	( *p_spinImageEventHandlerDestroy )( spinImageEventHandler );
SPINNAKERC_API	( *p_spinImageIsIncomplete )( spinImage, bool8_t* );
SPINNAKERC_API	( *p_spinImageGetStatus )( spinImage, spinImageStatus* );
SPINNAKERC_API	( *p_spinImageGetData )( spinImage, void** );
SPINNAKERC_API	( *p_spinImageGetValidPayloadSize )( spinImage, size_t* );

#if HAVE_LIBDL
static void*		_getDLSym ( void*, const char* );
#endif

/**
 * Cycle through the list of cameras returned by the Spinnaker library
 */

int
_spinInitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL
  static void*		libHandle = 0;

  if ( !libHandle ) {
#if HAVE_LIBSPINNAKER_V1
    if (!( libHandle = dlopen( "libSpinnaker_C.so.1", RTLD_LAZY ))) {
#else
    if (!( libHandle = dlopen( "libSpinnaker_C.so.2", RTLD_LAZY ))) {
#endif
			oaLogWarning ( OA_LOG_CAMERA, "%s: libSpinnaker_C.so not found",
					__func__ );
      return 0;
    }
  }

  dlerror();

  if (!( *( void** )( &p_spinSystemGetInstance ) = _getDLSym ( libHandle,
      "spinSystemGetInstance" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListClear ) = _getDLSym ( libHandle,
      "spinCameraListClear" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListCreateEmpty ) = _getDLSym ( libHandle,
      "spinCameraListCreateEmpty" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListDestroy ) = _getDLSym ( libHandle,
      "spinCameraListDestroy" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListGetSize ) = _getDLSym ( libHandle,
      "spinCameraListGetSize" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListClear ) = _getDLSym ( libHandle,
      "spinInterfaceListClear" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListCreateEmpty ) = _getDLSym ( libHandle,
      "spinInterfaceListCreateEmpty" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListDestroy ) = _getDLSym ( libHandle,
      "spinInterfaceListDestroy" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListGetSize ) = _getDLSym ( libHandle,
      "spinInterfaceListGetSize" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinSystemGetCameras ) = _getDLSym ( libHandle,
      "spinSystemGetCameras" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinSystemGetInterfaces ) = _getDLSym ( libHandle,
      "spinSystemGetInterfaces" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinSystemReleaseInstance ) = _getDLSym ( libHandle,
      "spinSystemReleaseInstance" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListGet ) = _getDLSym ( libHandle,
      "spinInterfaceListGet" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceRelease ) = _getDLSym ( libHandle,
      "spinInterfaceRelease" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceGetTLNodeMap ) = _getDLSym ( libHandle,
      "spinInterfaceGetTLNodeMap" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeMapGetNode ) = _getDLSym ( libHandle,
      "spinNodeMapGetNode" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeIsAvailable ) = _getDLSym ( libHandle,
      "spinNodeIsAvailable" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeIsImplemented ) = _getDLSym ( libHandle,
      "spinNodeIsImplemented" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeIsReadable ) = _getDLSym ( libHandle,
      "spinNodeIsReadable" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeIsWritable ) = _getDLSym ( libHandle,
      "spinNodeIsWritable" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinStringGetValue ) = _getDLSym ( libHandle,
      "spinStringGetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationEntryGetEnumValue ) = _getDLSym (
      libHandle, "spinEnumerationEntryGetEnumValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationSetEnumValue ) = _getDLSym (
      libHandle, "spinEnumerationSetEnumValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationEntryGetIntValue ) = _getDLSym (
      libHandle, "spinEnumerationEntryGetIntValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationSetIntValue ) = _getDLSym (
      libHandle, "spinEnumerationSetIntValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationEntryGetSymbolic ) = _getDLSym (
      libHandle, "spinEnumerationEntryGetSymbolic" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceGetCameras ) = _getDLSym ( libHandle,
      "spinInterfaceGetCameras" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListGet ) = _getDLSym ( libHandle,
      "spinCameraListGet" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraGetTLDeviceNodeMap ) = _getDLSym ( libHandle,
      "spinCameraGetTLDeviceNodeMap" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraRelease ) = _getDLSym ( libHandle,
      "spinCameraRelease" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraGetNodeMap ) = _getDLSym ( libHandle,
      "spinCameraGetNodeMap" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraGetNodeMap ) = _getDLSym ( libHandle,
      "spinCameraGetNodeMap" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCategoryGetNumFeatures ) = _getDLSym ( libHandle,
      "spinCategoryGetNumFeatures" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCategoryGetFeatureByIndex ) = _getDLSym (
      libHandle, "spinCategoryGetFeatureByIndex" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeGetType ) = _getDLSym ( libHandle,
      "spinNodeGetType" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeGetName ) = _getDLSym ( libHandle,
      "spinNodeGetName" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeGetDisplayName ) = _getDLSym ( libHandle,
      "spinNodeGetDisplayName" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraInit ) = _getDLSym ( libHandle,
      "spinCameraInit" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraDeInit ) = _getDLSym ( libHandle,
      "spinCameraDeInit" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraGetGuiXml ) = _getDLSym ( libHandle,
      "spinCameraGetGuiXml" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationGetNumEntries ) = _getDLSym ( libHandle,
      "spinEnumerationGetNumEntries" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationGetEntryByIndex ) = _getDLSym (
      libHandle, "spinEnumerationGetEntryByIndex" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationGetEntryByName ) = _getDLSym (
      libHandle, "spinEnumerationGetEntryByName" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinEnumerationGetCurrentEntry ) = _getDLSym (
      libHandle, "spinEnumerationGetCurrentEntry" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeToString ) = _getDLSym ( libHandle,
      "spinNodeToString" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinIntegerGetMin ) = _getDLSym ( libHandle,
      "spinIntegerGetMin" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinIntegerGetMax ) = _getDLSym ( libHandle,
      "spinIntegerGetMax" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinIntegerGetInc ) = _getDLSym ( libHandle,
      "spinIntegerGetInc" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinIntegerGetValue ) = _getDLSym ( libHandle,
      "spinIntegerGetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinIntegerSetValue ) = _getDLSym ( libHandle,
      "spinIntegerSetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinBooleanGetValue ) = _getDLSym ( libHandle,
      "spinBooleanGetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinBooleanSetValue ) = _getDLSym ( libHandle,
      "spinBooleanSetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinFloatGetMin ) = _getDLSym ( libHandle,
      "spinFloatGetMin" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinFloatGetMax ) = _getDLSym ( libHandle,
      "spinFloatGetMax" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinFloatGetValue ) = _getDLSym ( libHandle,
      "spinFloatGetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinFloatSetValue ) = _getDLSym ( libHandle,
      "spinFloatSetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraBeginAcquisition ) = _getDLSym ( libHandle,
      "spinCameraBeginAcquisition" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraEndAcquisition ) = _getDLSym ( libHandle,
      "spinCameraEndAcquisition" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinImageEventHandlerCreate ) = _getDLSym ( libHandle,
      "spinImageEventHandlerCreate" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraRegisterImageEventHandler ) =
			_getDLSym ( libHandle, "spinCameraRegisterImageEventHandler" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraUnregisterImageEventHandler ) =
			_getDLSym ( libHandle, "spinCameraUnregisterImageEventHandler" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinImageEventHandlerDestroy ) =
			_getDLSym ( libHandle, "spinImageEventHandlerDestroy" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinImageIsIncomplete ) = _getDLSym ( libHandle,
      "spinImageIsIncomplete" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinImageGetStatus ) =
			_getDLSym ( libHandle, "spinImageGetStatus" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinImageGetData ) = _getDLSym ( libHandle,
			"spinImageGetData" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinImageGetValidPayloadSize ) = _getDLSym (
			libHandle, "spinImageGetValidPayloadSize" ))) {
    return 0;
  }

#else /* HAVE_LIBDL */

  p_spinSystemGetInstance = spinSystemGetInstance;
  p_spinCameraListClear = spinCameraListClear;
  p_spinCameraListCreateEmpty = spinCameraListCreateEmpty;
  p_spinCameraListDestroy = spinCameraListDestroy;
  p_spinCameraListGetSize = spinCameraListGetSize;
  p_spinInterfaceListClear = spinInterfaceListClear;
  p_spinInterfaceListCreateEmpty = spinInterfaceListCreateEmpty;
  p_spinInterfaceListDestroy = spinInterfaceListDestroy;
  p_spinInterfaceListGetSize = spinInterfaceListGetSize;
  p_spinSystemGetCameras = spinSystemGetCameras;
  p_spinSystemGetInterfaces = spinSystemGetInterfaces;
  p_spinSystemReleaseInstance = spinSystemReleaseInstance;
  p_spinInterfaceListGet = spinInterfaceListGet;
  p_spinInterfaceRelease = spinInterfaceRelease;
  p_spinInterfaceGetTLNodeMap = spinInterfaceGetTLNodeMap;
  p_spinNodeMapGetNode = spinNodeMapGetNode;
  p_spinNodeIsAvailable = spinNodeIsAvailable;
  p_spinNodeIsImplemented = spinNodeIsImplemented;
  p_spinNodeIsReadable = spinNodeIsReadable;
  p_spinNodeIsWritable = spinNodeIsWritable;
  p_spinStringGetValue = spinStringGetValue;
  p_spinEnumerationEntryGetEnumValue = spinEnumerationEntryGetEnumValue;
  p_spinEnumerationSetEnumValue = spinEnumerationSetEnumValue;
  p_spinEnumerationEntryGetIntValue = spinEnumerationEntryGetIntValue;
  p_spinEnumerationSetIntValue = spinEnumerationSetIntValue;
  p_spinEnumerationEntryGetSymbolic = spinEnumerationEntryGetSymbolic;
  p_spinInterfaceGetCameras = spinInterfaceGetCameras;
  p_spinCameraListGet = spinCameraListGet;
  p_spinCameraGetTLDeviceNodeMap = spinCameraGetTLDeviceNodeMap;
  p_spinCameraRelease = spinCameraRelease;
  p_spinCameraGetNodeMap = spinCameraGetNodeMap;
  p_spinCategoryGetNumFeatures = spinCategoryGetNumFeatures;
  p_spinCategoryGetFeatureByIndex = spinCategoryGetFeatureByIndex;
  p_spinNodeGetType = spinNodeGetType;
  p_spinNodeGetName = spinNodeGetName;
  p_spinNodeGetDisplayName = spinNodeGetDisplayName;
  p_spinCameraInit = spinCameraInit;
  p_spinCameraDeInit = spinCameraDeInit;
  p_spinCameraGetGuiXml = spinCameraGetGuiXml;
  p_spinEnumerationGetNumEntries = spinEnumerationGetNumEntries;
  p_spinEnumerationGetEntryByIndex = spinEnumerationGetEntryByIndex;
  p_spinEnumerationGetEntryByName = spinEnumerationGetEntryByName;
  p_spinEnumerationGetCurrentEntry = spinEnumerationGetCurrentEntry;
  p_spinNodeToString = spinNodeToString;
  p_spinIntegerGetMin = spinIntegerGetMin;
  p_spinIntegerGetMax = spinIntegerGetMax;
  p_spinIntegerGetInc = spinIntegerGetInc;
  p_spinIntegerGetValue = spinIntegerGetValue;
  p_spinIntegerSetValue = spinIntegerSetValue;
  p_spinBooleanGetValue = spinBooleanGetValue;
  p_spinBooleanSetValue = spinBooleanSetValue;
  p_spinFloatGetMin = spinFloatGetMin;
  p_spinFloatGetMax = spinFloatGetMax;
  p_spinFloatGetValue = spinFloatGetValue;
  p_spinFloatSetValue = spinFloatSetValue;
  p_spinCameraBeginAcquisition = spinCameraBeginAcquisition;
  p_spinCameraEndAcquisition = spinCameraEndAcquisition;
	p_spinImageEventHandlerCreate = spinImageEventHandlerCreate;
	p_spinCameraRegisterImageEventHandler = spinCameraRegisterImageEventHandler;
	p_spinCameraUnregisterImageEventHandler =
			spinCameraUnregisterImageEventHandler;
	p_spinImageEventHandlerDestroy = spinImageEventHandlerDestroy;
	p_spinImageIsIncomplete = spinImageIsIncomplete;
	p_spinImageGetStatus = spinImageGetStatus;
	p_spinImageGetData = spinImageGetData;
	p_spinImageGetValidPayloadSize = spinImageGetValidPayloadSize;

#endif /* HAVE_LIBDL */

  return OA_ERR_NONE;
}


#if HAVE_LIBDL
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
#endif
