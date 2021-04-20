/*****************************************************************************
 *
 * Spin.h -- internal stuff
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

#ifndef	OA_SPINNAKER_SPIN_H
#define	OA_SPINNAKER_SPIN_H

#include <spinc/SpinnakerC.h>

#define	AUTO_SHARPNESS_OFF					0
#define	AUTO_SHARPNESS_ONCE					1
#define	AUTO_SHARPNESS_CONTINUOUS		2

#if HAVE_LIBSPINNAKER_V1
// This appears to have changed between V1 and V2
#define DeviceType_GigEVision DeviceType_GEV
#endif

extern int						_spinFormatMap[];

extern int						_spinInitLibraryFunctionPointers ( void );

extern SPINNAKERC_API	( *p_spinSystemGetInstance )( spinSystem* );
extern SPINNAKERC_API	( *p_spinCameraListClear )( spinCameraList );
extern SPINNAKERC_API	( *p_spinCameraListCreateEmpty )( spinCameraList* );
extern SPINNAKERC_API	( *p_spinCameraListDestroy )( spinCameraList );
extern SPINNAKERC_API	( *p_spinCameraListGetSize )( spinCameraList, size_t* );
extern SPINNAKERC_API	( *p_spinInterfaceListClear )( spinInterfaceList );
extern SPINNAKERC_API	( *p_spinInterfaceListCreateEmpty )
				( spinInterfaceList* );
extern SPINNAKERC_API	( *p_spinInterfaceListDestroy )( spinInterfaceList );
extern SPINNAKERC_API	( *p_spinInterfaceListGetSize )( spinInterfaceList,
				size_t* );
extern SPINNAKERC_API	( *p_spinSystemGetCameras )( spinSystem,
				spinCameraList );
extern SPINNAKERC_API	( *p_spinSystemGetInterfaces )( spinSystem,
				spinInterfaceList );
extern SPINNAKERC_API	( *p_spinSystemReleaseInstance )( spinSystem );
extern SPINNAKERC_API	( *p_spinInterfaceListGet )( spinInterfaceList, size_t,
				spinInterface );
extern SPINNAKERC_API	( *p_spinInterfaceRelease )( spinInterface );
extern SPINNAKERC_API	( *p_spinInterfaceGetTLNodeMap )( spinInterface,
				spinNodeMapHandle* );
extern SPINNAKERC_API	( *p_spinNodeMapGetNode )( spinNodeMapHandle,
				const char*, spinNodeHandle* );
extern SPINNAKERC_API	( *p_spinNodeIsAvailable )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinNodeIsImplemented )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinNodeIsReadable )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinNodeIsWritable )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinStringGetValue )( spinNodeHandle, char*,
				size_t* );
extern SPINNAKERC_API	( *p_spinEnumerationEntryGetEnumValue )(
				spinNodeHandle, size_t* );
extern SPINNAKERC_API	( *p_spinEnumerationEntryGetIntValue )(
				spinNodeHandle, int64_t* );
extern SPINNAKERC_API	( *p_spinEnumerationSetEnumValue )( spinNodeHandle,
				size_t );
extern SPINNAKERC_API	( *p_spinEnumerationSetIntValue )( spinNodeHandle,
				int64_t );
extern SPINNAKERC_API	( *p_spinEnumerationEntryGetSymbolic )(
				spinNodeHandle, char*, size_t* );
extern SPINNAKERC_API	( *p_spinInterfaceGetCameras )( spinInterface,
				spinCameraList );
extern SPINNAKERC_API	( *p_spinCameraListGet )( spinCameraList, size_t,
				spinCamera* );
extern SPINNAKERC_API	( *p_spinCameraGetTLDeviceNodeMap )( spinCamera,
				spinNodeMapHandle* );
extern SPINNAKERC_API	( *p_spinCameraRelease )( spinCamera );
extern SPINNAKERC_API	( *p_spinCameraGetNodeMap )( spinCamera,
				spinNodeMapHandle* );
extern SPINNAKERC_API	( *p_spinCategoryGetNumFeatures )( spinNodeMapHandle,
				size_t* );
extern SPINNAKERC_API	( *p_spinCategoryGetFeatureByIndex )( spinNodeMapHandle,
				size_t, spinNodeMapHandle* );
extern SPINNAKERC_API	( *p_spinNodeGetType )( spinNodeHandle, spinNodeType* );
extern SPINNAKERC_API	( *p_spinNodeGetName )( spinNodeHandle, char*, size_t* );
extern SPINNAKERC_API	( *p_spinNodeGetDisplayName )( spinNodeHandle, char*,
				size_t* );
extern SPINNAKERC_API	( *p_spinCameraInit )( spinCamera );
extern SPINNAKERC_API	( *p_spinCameraDeInit )( spinCamera );
extern SPINNAKERC_API	( *p_spinCameraGetGuiXml )( spinCamera, char*,
				size_t* );
extern SPINNAKERC_API	( *p_spinEnumerationGetNumEntries )( spinNodeHandle,
				size_t* );
extern SPINNAKERC_API	( *p_spinEnumerationGetEntryByIndex )( spinNodeHandle,
				size_t, spinNodeHandle* );
extern SPINNAKERC_API	( *p_spinEnumerationGetCurrentEntry )( spinNodeHandle,
				spinNodeHandle* );
extern SPINNAKERC_API	( *p_spinNodeToString )( spinNodeHandle, char*,
				size_t* );
extern SPINNAKERC_API	( *p_spinIntegerGetMin )( spinNodeHandle, int64_t* );
extern SPINNAKERC_API	( *p_spinIntegerGetMax )( spinNodeHandle, int64_t* );
extern SPINNAKERC_API	( *p_spinIntegerGetInc )( spinNodeHandle, int64_t* );
extern SPINNAKERC_API	( *p_spinIntegerGetValue )( spinNodeHandle, int64_t* );
extern SPINNAKERC_API	( *p_spinIntegerSetValue )( spinNodeHandle, int64_t );
extern SPINNAKERC_API	( *p_spinBooleanGetValue )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinBooleanSetValue )( spinNodeHandle, bool8_t );
extern SPINNAKERC_API	( *p_spinFloatGetMin )( spinNodeHandle, double* );
extern SPINNAKERC_API	( *p_spinFloatGetMax )( spinNodeHandle, double* );
extern SPINNAKERC_API	( *p_spinFloatGetValue )( spinNodeHandle, double* );
extern SPINNAKERC_API	( *p_spinFloatSetValue )( spinNodeHandle, double );
extern SPINNAKERC_API	( *p_spinCameraBeginAcquisition )( spinCamera );
extern SPINNAKERC_API	( *p_spinCameraEndAcquisition )( spinCamera );
extern SPINNAKERC_API	( *p_spinImageEventHandlerCreate )(
				spinImageEventHandler*, spinImageEventFunction, void* );
extern SPINNAKERC_API	( *p_spinCameraRegisterImageEventHandler )(
				spinCamera, spinImageEventHandler );
extern SPINNAKERC_API ( *p_spinCameraUnregisterImageEventHandler )( spinCamera,
				spinImageEventHandler );
extern SPINNAKERC_API ( *p_spinImageEventHandlerDestroy )(
				spinImageEventHandler );
extern SPINNAKERC_API ( *p_spinImageIsIncomplete )( spinImage, bool8_t* );
extern SPINNAKERC_API ( *p_spinImageGetStatus )( spinImage, spinImageStatus* );
extern SPINNAKERC_API	( *p_spinImageGetData )( spinImage, void** );
extern SPINNAKERC_API	( *p_spinImageGetValidPayloadSize )( spinImage, size_t* );

#define SPINNAKER_MAX_BUFF_LEN	256

#endif /* OA_SPINNAKER_SPIN_H */
