/*****************************************************************************
 *
 * private.h -- header for Basler Pylon camera API
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

#ifndef OA_PYLON_PRIVATE_H
#define OA_PYLON_PRIVATE_H

extern int						_pylonInitLibraryFunctionPointers ( void );

extern GENAPIC_RESULT	( *p_PylonInitialize )( void );
extern GENAPIC_RESULT	( *p_PylonTerminate )( void );

extern GENAPIC_RESULT	( *p_PylonEnumerateDevices )( size_t* );
extern GENAPIC_RESULT	( *p_PylonCreateDeviceByIndex )( size_t,
													PYLON_DEVICE_HANDLE* );
extern GENAPIC_RESULT	( *p_PylonDeviceOpen )( PYLON_DEVICE_HANDLE, int );
extern GENAPIC_RESULT	( *p_PylonDeviceClose )( PYLON_DEVICE_HANDLE );
extern GENAPIC_RESULT	( *p_PylonDestroyDevice )( PYLON_DEVICE_HANDLE );

extern GENAPIC_RESULT	( *p_PylonDeviceFeatureIsReadable )( PYLON_DEVICE_HANDLE,
													const char* );
extern GENAPIC_RESULT	( *p_PylonDeviceFeatureIsAvailable )( PYLON_DEVICE_HANDLE,
													const char* );
extern GENAPIC_RESULT	( *p_PylonDeviceFeatureToString )( PYLON_DEVICE_HANDLE,
													const char*, char*, size_t* );
extern GENAPIC_RESULT	( *p_PylonDeviceFeatureFromString )( PYLON_DEVICE_HANDLE,
													const char*, const char* );

extern GENAPIC_RESULT	( *p_PylonGetDeviceInfoHandle )( size_t,
													PYLON_DEVICE_INFO_HANDLE* );
extern GENAPIC_RESULT	( *p_PylonDeviceInfoGetNumProperties )(
													PYLON_DEVICE_INFO_HANDLE, size_t* );
extern GENAPIC_RESULT	( *p_PylonDeviceInfoGetPropertyName )(
													PYLON_DEVICE_INFO_HANDLE, size_t, char*, size_t* );

extern GENAPIC_RESULT	( *p_PylonDeviceGetNodeMap )( PYLON_DEVICE_HANDLE,
												NODEMAP_HANDLE* );

extern GENAPIC_RESULT	( *p_PylonDeviceSetBooleanFeature )( PYLON_DEVICE_HANDLE,
												const char*, _Bool );
extern GENAPIC_RESULT	( *p_PylonDeviceSetIntegerFeature )( PYLON_DEVICE_HANDLE,
												const char*, int64_t );
extern GENAPIC_RESULT	( *p_PylonDeviceSetFloatFeature )( PYLON_DEVICE_HANDLE,
												const char*, double );
extern GENAPIC_RESULT	( *p_PylonDeviceGetBooleanFeature )( PYLON_DEVICE_HANDLE,
												const char*, _Bool* );
extern GENAPIC_RESULT	( *p_PylonDeviceGetIntegerFeature )( PYLON_DEVICE_HANDLE,
												const char*, int64_t* );
extern GENAPIC_RESULT	( *p_PylonDeviceGetFloatFeature )( PYLON_DEVICE_HANDLE,
												const char*, double* );

extern GENAPIC_RESULT	( *p_GenApiNodeMapGetNumNodes )( NODEMAP_HANDLE,
												size_t* );
extern GENAPIC_RESULT	( *p_GenApiNodeMapGetNodeByIndex )( NODEMAP_HANDLE,
												size_t, NODE_HANDLE* );
extern GENAPIC_RESULT	( *p_GenApiNodeMapGetNode )( NODEMAP_HANDLE,
												const char*, NODE_HANDLE* );
extern GENAPIC_RESULT	( *p_GenApiNodeGetName )( NODE_HANDLE, char*, size_t* );
extern GENAPIC_RESULT	( *p_GenApiNodeGetDisplayName )( NODE_HANDLE, char*,
												size_t* );
extern GENAPIC_RESULT	( *p_GenApiNodeGetDescription )( NODE_HANDLE, char*,
												size_t* );
extern GENAPIC_RESULT	( *p_GenApiNodeGetType )( NODE_HANDLE, EGenApiNodeType* );
extern GENAPIC_RESULT	( *p_GenApiNodeIsReadable )( NODE_HANDLE, _Bool* );
extern GENAPIC_RESULT	( *p_GenApiNodeIsAvailable )( NODE_HANDLE, _Bool* );
extern GENAPIC_RESULT	( *p_GenApiNodeIsWritable )( NODE_HANDLE, _Bool* );

extern GENAPIC_RESULT	( *p_GenApiEnumerationGetEntryByName  )( NODE_HANDLE,
													const char*, NODE_HANDLE* );
extern GENAPIC_RESULT	( *p_GenApiEnumerationGetNumEntries  )( NODE_HANDLE,
													size_t* );
extern GENAPIC_RESULT	( *p_GenApiEnumerationGetEntryByIndex  )( NODE_HANDLE,
													size_t, NODE_HANDLE* );

extern GENAPIC_RESULT	( *p_GenApiCategoryGetNumFeatures )( NODE_HANDLE,
												size_t* );
extern GENAPIC_RESULT	( *p_GenApiCategoryGetFeatureByIndex )( NODE_HANDLE,
												size_t, NODE_HANDLE* );

extern GENAPIC_RESULT	( *p_GenApiIntegerGetMin )( NODE_HANDLE, int64_t* );
extern GENAPIC_RESULT	( *p_GenApiIntegerGetMax )( NODE_HANDLE, int64_t* );
extern GENAPIC_RESULT	( *p_GenApiIntegerGetInc )( NODE_HANDLE, int64_t* );
extern GENAPIC_RESULT	( *p_GenApiIntegerGetValue )( NODE_HANDLE, int64_t* );

extern GENAPIC_RESULT	( *p_GenApiFloatGetMin )( NODE_HANDLE, double* );
extern GENAPIC_RESULT	( *p_GenApiFloatGetMax )( NODE_HANDLE, double* );
extern GENAPIC_RESULT	( *p_GenApiFloatGetValue )( NODE_HANDLE, double* );

extern GENAPIC_RESULT	( *p_GenApiNodeFromString )( NODE_HANDLE, const char* );
extern GENAPIC_RESULT	( *p_GenApiNodeToString )( NODE_HANDLE, char*, size_t* );

extern GENAPIC_RESULT	( *p_PylonDeviceGetNumStreamGrabberChannels )(
													PYLON_DEVICE_HANDLE, size_t* );
extern GENAPIC_RESULT	( *p_PylonDeviceGetStreamGrabber )( PYLON_DEVICE_HANDLE,
													size_t, PYLON_STREAMGRABBER_HANDLE* );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberOpen )(
													PYLON_STREAMGRABBER_HANDLE );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberGetWaitObject )(
													PYLON_STREAMGRABBER_HANDLE,
													PYLON_WAITOBJECT_HANDLE* );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberSetMaxNumBuffer )(
													PYLON_STREAMGRABBER_HANDLE, size_t );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberGetPayloadSize )(
													PYLON_DEVICE_HANDLE, PYLON_STREAMGRABBER_HANDLE,
													size_t* );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberSetMaxBufferSize )(
													PYLON_STREAMGRABBER_HANDLE, size_t );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberPrepareGrab )(
													PYLON_STREAMGRABBER_HANDLE );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberRegisterBuffer )(
													PYLON_STREAMGRABBER_HANDLE, void*, size_t,
													PYLON_STREAMBUFFER_HANDLE* );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberQueueBuffer )(
													PYLON_STREAMGRABBER_HANDLE,
													PYLON_STREAMBUFFER_HANDLE, const void* );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberStartStreamingIfMandatory )(
													PYLON_STREAMGRABBER_HANDLE );
extern GENAPIC_RESULT	( *p_PylonDeviceExecuteCommandFeature )(
													PYLON_DEVICE_HANDLE, const char* );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberStopStreamingIfMandatory )(
													PYLON_STREAMGRABBER_HANDLE );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberFlushBuffersToOutput )(
													PYLON_STREAMGRABBER_HANDLE );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberRetrieveResult )(
													PYLON_STREAMGRABBER_HANDLE, PylonGrabResult_t*,
													_Bool* );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberDeregisterBuffer )(
													PYLON_STREAMGRABBER_HANDLE,
													PYLON_STREAMBUFFER_HANDLE );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberFinishGrab )(
													PYLON_STREAMGRABBER_HANDLE );
extern GENAPIC_RESULT	( *p_PylonStreamGrabberClose )(
													PYLON_STREAMGRABBER_HANDLE );

extern GENAPIC_RESULT	( *p_PylonWaitObjectWait )( PYLON_WAITOBJECT_HANDLE,
													uint32_t, _Bool* );


typedef struct {
	const char*		pylonName;
	int						pixFormat;
} pylonFrameInfo;

typedef struct {
	const char*		pylonName;
	int						filter;
} pylonFilterInfo;

extern pylonFrameInfo	 _frameFormats[17];

#endif	/* OA_PYLON_PRIVATE_H */
