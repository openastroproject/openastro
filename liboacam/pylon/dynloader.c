/*****************************************************************************
 *
 * dynloader.c -- dynamic loader for Pylon SDK
 *
 * Copyright 2020 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#endif
#include <openastro/errno.h>

#include "private.h"

// Pointers to SDK functions so we can use them via libdl.

GENAPIC_RESULT	( *p_PylonInitialize )( void );
GENAPIC_RESULT	( *p_PylonTerminate )( void );

GENAPIC_RESULT	( *p_PylonEnumerateDevices )( size_t* );
GENAPIC_RESULT	( *p_PylonCreateDeviceByIndex )( size_t, PYLON_DEVICE_HANDLE* );
GENAPIC_RESULT	( *p_PylonDeviceOpen )( PYLON_DEVICE_HANDLE, int );
GENAPIC_RESULT	( *p_PylonDeviceClose )( PYLON_DEVICE_HANDLE );
GENAPIC_RESULT	( *p_PylonDestroyDevice )( PYLON_DEVICE_HANDLE );

GENAPIC_RESULT	( *p_PylonDeviceFeatureIsReadable )( PYLON_DEVICE_HANDLE,
										const char* );
GENAPIC_RESULT	( *p_PylonDeviceFeatureIsAvailable )( PYLON_DEVICE_HANDLE,
										const char* );
GENAPIC_RESULT	( *p_PylonDeviceFeatureToString )( PYLON_DEVICE_HANDLE,
										const char*, char*, size_t* );
GENAPIC_RESULT	( *p_PylonDeviceFeatureFromString )( PYLON_DEVICE_HANDLE,
										const char*, const char* );

GENAPIC_RESULT	( *p_PylonGetDeviceInfoHandle )( size_t,
										PYLON_DEVICE_INFO_HANDLE* );
GENAPIC_RESULT	( *p_PylonDeviceInfoGetNumProperties )(
										PYLON_DEVICE_INFO_HANDLE, size_t* );
GENAPIC_RESULT	( *p_PylonDeviceInfoGetPropertyName )(
										PYLON_DEVICE_INFO_HANDLE, size_t, char*, size_t* );

GENAPIC_RESULT	( *p_PylonDeviceGetNodeMap )( PYLON_DEVICE_HANDLE,
										NODEMAP_HANDLE* );

GENAPIC_RESULT	( *p_PylonDeviceSetBooleanFeature )( PYLON_DEVICE_HANDLE,
										const char*, _Bool );
GENAPIC_RESULT	( *p_PylonDeviceSetIntegerFeature )( PYLON_DEVICE_HANDLE,
										const char*, int64_t );
GENAPIC_RESULT	( *p_PylonDeviceSetFloatFeature )( PYLON_DEVICE_HANDLE,
										const char*, double );
GENAPIC_RESULT	( *p_PylonDeviceGetBooleanFeature )( PYLON_DEVICE_HANDLE,
										const char*, _Bool* );
GENAPIC_RESULT	( *p_PylonDeviceGetIntegerFeature )( PYLON_DEVICE_HANDLE,
										const char*, int64_t* );
GENAPIC_RESULT	( *p_PylonDeviceGetFloatFeature )( PYLON_DEVICE_HANDLE,
										const char*, double* );

GENAPIC_RESULT	( *p_GenApiNodeMapGetNumNodes )( NODEMAP_HANDLE, size_t* );
GENAPIC_RESULT	( *p_GenApiNodeMapGetNodeByIndex )( NODEMAP_HANDLE, size_t,
										NODE_HANDLE* );
GENAPIC_RESULT	( *p_GenApiNodeMapGetNode )( NODEMAP_HANDLE, const char*,
										NODE_HANDLE* );
GENAPIC_RESULT	( *p_GenApiNodeGetName )( NODE_HANDLE, char*, size_t* );
GENAPIC_RESULT	( *p_GenApiNodeGetDisplayName )( NODE_HANDLE, char*, size_t* );
GENAPIC_RESULT	( *p_GenApiNodeGetDescription )( NODE_HANDLE, char*, size_t* );
GENAPIC_RESULT	( *p_GenApiNodeGetType )( NODE_HANDLE, EGenApiNodeType* );
GENAPIC_RESULT	( *p_GenApiNodeIsReadable )( NODE_HANDLE, _Bool* );
GENAPIC_RESULT	( *p_GenApiNodeIsAvailable )( NODE_HANDLE, _Bool* );
GENAPIC_RESULT	( *p_GenApiNodeIsWritable )( NODE_HANDLE, _Bool* );

GENAPIC_RESULT	( *p_GenApiEnumerationGetEntryByName  )( NODE_HANDLE,
										const char*, NODE_HANDLE* );
GENAPIC_RESULT	( *p_GenApiEnumerationGetNumEntries  )( NODE_HANDLE, size_t* );
GENAPIC_RESULT	( *p_GenApiEnumerationGetEntryByIndex  )( NODE_HANDLE, size_t,
										NODE_HANDLE* );

GENAPIC_RESULT	( *p_GenApiCategoryGetNumFeatures )( NODE_HANDLE, size_t* );
GENAPIC_RESULT	( *p_GenApiCategoryGetFeatureByIndex )( NODE_HANDLE, size_t,
										NODE_HANDLE* );

GENAPIC_RESULT	( *p_GenApiIntegerGetMin )( NODE_HANDLE, int64_t* );
GENAPIC_RESULT	( *p_GenApiIntegerGetMax )( NODE_HANDLE, int64_t* );
GENAPIC_RESULT	( *p_GenApiIntegerGetInc )( NODE_HANDLE, int64_t* );
GENAPIC_RESULT	( *p_GenApiIntegerGetValue )( NODE_HANDLE, int64_t* );

GENAPIC_RESULT	( *p_GenApiFloatGetMin )( NODE_HANDLE, double* );
GENAPIC_RESULT	( *p_GenApiFloatGetMax )( NODE_HANDLE, double* );
GENAPIC_RESULT	( *p_GenApiFloatGetValue )( NODE_HANDLE, double* );

GENAPIC_RESULT	( *p_GenApiNodeFromString )( NODE_HANDLE, const char* );
GENAPIC_RESULT	( *p_GenApiNodeToString )( NODE_HANDLE, char*, size_t* );

GENAPIC_RESULT	( *p_PylonDeviceGetNumStreamGrabberChannels )(
										PYLON_DEVICE_HANDLE, size_t* );
GENAPIC_RESULT	( *p_PylonDeviceGetStreamGrabber )( PYLON_DEVICE_HANDLE, size_t,
										PYLON_STREAMGRABBER_HANDLE* );
GENAPIC_RESULT	( *p_PylonStreamGrabberOpen )(
										PYLON_STREAMGRABBER_HANDLE );
GENAPIC_RESULT	( *p_PylonStreamGrabberGetWaitObject )(
										PYLON_STREAMGRABBER_HANDLE, PYLON_WAITOBJECT_HANDLE* );
GENAPIC_RESULT	( *p_PylonStreamGrabberSetMaxNumBuffer )(
										PYLON_STREAMGRABBER_HANDLE, size_t );
GENAPIC_RESULT	( *p_PylonStreamGrabberGetPayloadSize )(
										PYLON_DEVICE_HANDLE, PYLON_STREAMGRABBER_HANDLE, size_t* );
GENAPIC_RESULT	( *p_PylonStreamGrabberSetMaxBufferSize )(
										PYLON_STREAMGRABBER_HANDLE, size_t );
GENAPIC_RESULT	( *p_PylonStreamGrabberPrepareGrab )(
										PYLON_STREAMGRABBER_HANDLE );
GENAPIC_RESULT	( *p_PylonStreamGrabberRegisterBuffer )(
										PYLON_STREAMGRABBER_HANDLE, void*, size_t,
										PYLON_STREAMBUFFER_HANDLE* );
GENAPIC_RESULT	( *p_PylonStreamGrabberQueueBuffer )(
										PYLON_STREAMGRABBER_HANDLE,
										PYLON_STREAMBUFFER_HANDLE, const void* );
GENAPIC_RESULT	( *p_PylonStreamGrabberStartStreamingIfMandatory )(
										PYLON_STREAMGRABBER_HANDLE );
GENAPIC_RESULT	( *p_PylonDeviceExecuteCommandFeature )(
										PYLON_DEVICE_HANDLE, const char* );
GENAPIC_RESULT	( *p_PylonStreamGrabberStopStreamingIfMandatory )(
										PYLON_STREAMGRABBER_HANDLE );
GENAPIC_RESULT	( *p_PylonStreamGrabberFlushBuffersToOutput )(
										PYLON_STREAMGRABBER_HANDLE );
GENAPIC_RESULT	( *p_PylonStreamGrabberRetrieveResult )(
										PYLON_STREAMGRABBER_HANDLE, PylonGrabResult_t*, _Bool* );
GENAPIC_RESULT	( *p_PylonStreamGrabberDeregisterBuffer )(
										PYLON_STREAMGRABBER_HANDLE, PYLON_STREAMBUFFER_HANDLE );
GENAPIC_RESULT	( *p_PylonStreamGrabberFinishGrab )(
										PYLON_STREAMGRABBER_HANDLE );
GENAPIC_RESULT	( *p_PylonStreamGrabberClose )( PYLON_STREAMGRABBER_HANDLE );

GENAPIC_RESULT	( *p_PylonWaitObjectWait )( PYLON_WAITOBJECT_HANDLE,
										uint32_t, _Bool* );

#if HAVE_LIBDL
static void*		_getDLSym ( void*, const char* );
#endif

/**
 * Cycle through the list of cameras returned by the Flycapture library
 */

int
_pylonInitLibraryFunctionPointers ( void )
{
#if HAVE_LIBDL
  static void*		libHandle = 0;

  if ( !libHandle ) {
    if (!( libHandle = dlopen( "/opt/pylon/lib/libpylonc.so", RTLD_LAZY ))) {
      return OA_ERR_LIBRARY_NOT_FOUND;
    }
  }

  dlerror();

  if (!( *( void** )( &p_PylonInitialize ) = _getDLSym ( libHandle,
      "PylonInitialize" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonTerminate ) = _getDLSym ( libHandle,
      "PylonTerminate" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonEnumerateDevices ) = _getDLSym ( libHandle,
      "PylonEnumerateDevices" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonCreateDeviceByIndex ) = _getDLSym ( libHandle,
      "PylonCreateDeviceByIndex" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceOpen ) = _getDLSym ( libHandle,
      "PylonDeviceOpen" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceClose ) = _getDLSym ( libHandle,
      "PylonDeviceClose" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDestroyDevice ) = _getDLSym ( libHandle,
      "PylonDestroyDevice" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceFeatureIsReadable ) = _getDLSym ( libHandle,
      "PylonDeviceFeatureIsReadable" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceFeatureIsAvailable ) =
			_getDLSym ( libHandle, "PylonDeviceFeatureIsAvailable" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceFeatureToString ) =
      _getDLSym ( libHandle, "PylonDeviceFeatureToString" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceFeatureFromString ) =
      _getDLSym ( libHandle, "PylonDeviceFeatureFromString" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonGetDeviceInfoHandle ) =
      _getDLSym ( libHandle, "PylonGetDeviceInfoHandle" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceInfoGetNumProperties ) =
      _getDLSym ( libHandle, "PylonDeviceInfoGetNumProperties" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceInfoGetPropertyName ) =
      _getDLSym ( libHandle, "PylonDeviceInfoGetPropertyName" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_PylonDeviceGetNodeMap ) =
      _getDLSym ( libHandle, "PylonDeviceGetNodeMap" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_PylonDeviceSetBooleanFeature ) =
      _getDLSym ( libHandle, "PylonDeviceSetBooleanFeature" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceSetIntegerFeature ) =
      _getDLSym ( libHandle, "PylonDeviceSetIntegerFeature" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceSetFloatFeature ) =
      _getDLSym ( libHandle, "PylonDeviceSetFloatFeature" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceGetBooleanFeature ) =
      _getDLSym ( libHandle, "PylonDeviceGetBooleanFeature" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceGetIntegerFeature ) =
      _getDLSym ( libHandle, "PylonDeviceGetIntegerFeature" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceGetFloatFeature ) =
      _getDLSym ( libHandle, "PylonDeviceGetFloatFeature" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_GenApiNodeMapGetNumNodes ) =
      _getDLSym ( libHandle, "GenApiNodeMapGetNumNodes" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeMapGetNodeByIndex ) =
      _getDLSym ( libHandle, "GenApiNodeMapGetNodeByIndex" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeMapGetNode ) =
      _getDLSym ( libHandle, "GenApiNodeMapGetNode" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeGetName ) =
      _getDLSym ( libHandle, "GenApiNodeGetName" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeGetDisplayName ) =
      _getDLSym ( libHandle, "GenApiNodeGetDisplayName" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeGetDescription ) =
      _getDLSym ( libHandle, "GenApiNodeGetDescription" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeGetType ) =
      _getDLSym ( libHandle, "GenApiNodeGetType" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeIsReadable ) =
      _getDLSym ( libHandle, "GenApiNodeIsReadable" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeIsAvailable ) =
      _getDLSym ( libHandle, "GenApiNodeIsAvailable" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeIsWritable ) =
      _getDLSym ( libHandle, "GenApiNodeIsWritable" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_GenApiEnumerationGetEntryByName ) =
      _getDLSym ( libHandle, "GenApiEnumerationGetEntryByName" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiEnumerationGetNumEntries ) =
      _getDLSym ( libHandle, "GenApiEnumerationGetNumEntries" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiEnumerationGetEntryByIndex ) =
      _getDLSym ( libHandle, "GenApiEnumerationGetEntryByIndex" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_GenApiCategoryGetNumFeatures ) =
      _getDLSym ( libHandle, "GenApiCategoryGetNumFeatures" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiCategoryGetFeatureByIndex ) =
      _getDLSym ( libHandle, "GenApiCategoryGetFeatureByIndex" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_GenApiIntegerGetMin ) =
      _getDLSym ( libHandle, "GenApiIntegerGetMin" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiIntegerGetMax ) =
      _getDLSym ( libHandle, "GenApiIntegerGetMax" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiIntegerGetInc ) =
      _getDLSym ( libHandle, "GenApiIntegerGetInc" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiIntegerGetValue ) =
      _getDLSym ( libHandle, "GenApiIntegerGetValue" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_GenApiFloatGetMin ) =
      _getDLSym ( libHandle, "GenApiFloatGetMin" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiFloatGetMax ) =
      _getDLSym ( libHandle, "GenApiFloatGetMax" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiFloatGetValue ) =
      _getDLSym ( libHandle, "GenApiFloatGetValue" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_GenApiNodeFromString ) =
      _getDLSym ( libHandle, "GenApiNodeFromString" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_GenApiNodeToString ) =
      _getDLSym ( libHandle, "GenApiNodeToString" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_PylonDeviceGetNumStreamGrabberChannels ) =
      _getDLSym ( libHandle, "PylonDeviceGetNumStreamGrabberChannels" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceGetStreamGrabber ) =
      _getDLSym ( libHandle, "PylonDeviceGetStreamGrabber" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberOpen ) =
      _getDLSym ( libHandle, "PylonStreamGrabberOpen" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberGetWaitObject ) =
      _getDLSym ( libHandle, "PylonStreamGrabberGetWaitObject" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberSetMaxNumBuffer ) =
      _getDLSym ( libHandle, "PylonStreamGrabberSetMaxNumBuffer" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberGetPayloadSize ) =
      _getDLSym ( libHandle, "PylonStreamGrabberGetPayloadSize" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberSetMaxBufferSize ) =
      _getDLSym ( libHandle, "PylonStreamGrabberSetMaxBufferSize" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberPrepareGrab ) =
      _getDLSym ( libHandle, "PylonStreamGrabberPrepareGrab" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberRegisterBuffer ) =
      _getDLSym ( libHandle, "PylonStreamGrabberRegisterBuffer" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberQueueBuffer ) =
      _getDLSym ( libHandle, "PylonStreamGrabberQueueBuffer" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberStartStreamingIfMandatory ) =
      _getDLSym ( libHandle, "PylonStreamGrabberStartStreamingIfMandatory" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonDeviceExecuteCommandFeature ) =
      _getDLSym ( libHandle, "PylonDeviceExecuteCommandFeature" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberStopStreamingIfMandatory ) =
      _getDLSym ( libHandle, "PylonStreamGrabberStopStreamingIfMandatory" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberFlushBuffersToOutput ) =
      _getDLSym ( libHandle, "PylonStreamGrabberFlushBuffersToOutput" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberRetrieveResult ) =
      _getDLSym ( libHandle, "PylonStreamGrabberRetrieveResult" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberDeregisterBuffer ) =
      _getDLSym ( libHandle, "PylonStreamGrabberDeregisterBuffer" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberFinishGrab ) =
      _getDLSym ( libHandle, "PylonStreamGrabberFinishGrab" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
  if (!( *( void** )( &p_PylonStreamGrabberClose ) =
      _getDLSym ( libHandle, "PylonStreamGrabberClose" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }

  if (!( *( void** )( &p_PylonWaitObjectWait ) =
      _getDLSym ( libHandle, "PylonWaitObjectWait" ))) {
    dlclose ( libHandle );
		libHandle = 0;
		return OA_ERR_SYMBOL_NOT_FOUND;
  }
#else /* HAVE_LIBDL */

  p_PylonInitialize = PylonInitialize;
  p_PylonTerminate = PylonTerminate;

  p_PylonEnumerateDevices = PylonEnumerateDevices;
  p_PylonCreateDeviceByIndex = PylonCreateDeviceByIndex;
  p_PylonDeviceOpen = PylonDeviceOpen;
  p_PylonDeviceClose = PylonDeviceClose;
  p_PylonDestroyDevice = PylonDestroyDevice;
  p_PylonDeviceFeatureIsReadable = PylonDeviceFeatureIsReadable;
  p_PylonDeviceFeatureIsAvailable = PylonDeviceFeatureIsAvailable;
  p_PylonDeviceFeatureToString = PylonDeviceFeatureToString;
  p_PylonDeviceFeatureFromString = PylonDeviceFeatureFromString;
	p_PylonGetDeviceInfoHandle = PylonGetDeviceInfoHandle;
	p_PylonDeviceInfoGetNumProperties = PylonDeviceInfoGetNumProperties;
	p_PylonDeviceInfoGetPropertyName = PylonDeviceInfoGetPropertyName;
	p_PylonDeviceGetNodeMap = PylonDeviceGetNodeMap;
	p_PylonDeviceSetBooleanFeature = PylonDeviceSetBooleanFeature;
	p_PylonDeviceSetIntegerFeature = PylonDeviceSetIntegerFeature;
	p_PylonDeviceSetFloatFeature = PylonDeviceSetFloatFeature;
	p_PylonDeviceGetBooleanFeature = PylonDeviceGetBooleanFeature;
	p_PylonDeviceGetIntegerFeature = PylonDeviceGetIntegerFeature;
	p_PylonDeviceGetFloatFeature = PylonDeviceGetFloatFeature;

	p_GenApiNodeMapGetNumNodes = GenApiNodeMapGetNumNodes;
	p_GenApiNodeMapGetNodeByIndex = GenApiNodeMapGetNodeByIndex;

	p_GenApiNodeGetName = GenApiNodeGetName;
	p_GenApiNodeGetDisplayName = GenApiNodeGetDisplayName;
	p_GenApiNodeGetDescription = GenApiNodeGetDescription;
	p_GenApiNodeGetType = GenApiNodeGetType;
	p_GenApiNodeIsReadable = GenApiNodeIsReadable;
	p_GenApiNodeIsAvailable = GenApiNodeIsAvailable;
	p_GenApiNodeIsWritable = GenApiNodeIsWritable;

	p_GenApiCategoryGetNumFeatures = GenApiCategoryGetNumFeatures;
	p_GenApiCategoryGetFeatureByIndex = GenApiCategoryGetFeatureByIndex;

	p_GenApiEnumerationGetEntryByName = GenApiEnumerationGetEntryByName;
	p_GenApiEnumerationGetNumEntries = GenApiEnumerationGetNumEntries;
	p_GenApiEnumerationGetEntryByIndex = GenApiEnumerationGetEntryByIndex;

	p_GenApiIntegerGetMin = GenApiIntegerGetMin;
	p_GenApiIntegerGetMax = GenApiIntegerGetMax;
	p_GenApiIntegerGetInc = GenApiIntegerGetInc;
	p_GenApiIntegerGetValue = GenApiIntegerGetValue;

	p_GenApiFloatGetMin = GenApiFloatGetMin;
	p_GenApiFloatGetMax = GenApiFloatGetMax;
	p_GenApiFloatGetValue = GenApiFloatGetValue;

	p_GenApiNodeFromString = GenApiNodeFromString;
	p_GenApiNodeToString = GenApiNodeToString;
	p_PylonDeviceGetNumStreamGrabberChannels =
			PylonDeviceGetNumStreamGrabberChannels;
	p_PylonDeviceGetStreamGrabber = PylonDeviceGetStreamGrabber;
	p_PylonStreamGrabberOpen = PylonStreamGrabberOpen;
	p_PylonStreamGrabberGetWaitObject =
			PylonStreamGrabberGetWaitObject;
	p_PylonStreamGrabberSetMaxNumBuffer = PylonStreamGrabberSetMaxNumBuffer;
	p_PylonStreamGrabberGetPayloadSize = PylonStreamGrabberGetPayloadSize;
	p_PylonStreamGrabberSetMaxBufferSize = PylonStreamGrabberSetMaxBufferSize;
	p_PylonStreamGrabberPrepareGrab = PylonStreamGrabberPrepareGrab;
	p_PylonStreamGrabberRegisterBuffer = PylonStreamGrabberRegisterBuffer;
	p_PylonStreamGrabberQueueBuffer = PylonStreamGrabberQueueBuffer;
	p_PylonStreamGrabberStartStreamingIfMandatory =
			PylonStreamGrabberStartStreamingIfMandatory;
	p_PylonDeviceExecuteCommandFeature = PylonDeviceExecuteCommandFeature;
	p_PylonStreamGrabberStopStreamingIfMandatory =
			PylonStreamGrabberStopStreamingIfMandatory;
	p_PylonStreamGrabberFlushBuffersToOutput =
			PylonStreamGrabberFlushBuffersToOutput;
	p_PylonStreamGrabberRetrieveResult = PylonStreamGrabberRetrieveResult;
	p_PylonStreamGrabberDeregisterBuffer = PylonStreamGrabberDeregisterBuffer;
	p_PylonStreamGrabberFinishGrab = PylonStreamGrabberFinishGrab;
	p_PylonStreamGrabberClose = PylonStreamGrabberClose;

	p_PylonWaitObjectWait = PylonWaitObjectWait;

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
    fprintf ( stderr, "libpylonc DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif
