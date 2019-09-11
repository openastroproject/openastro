/*****************************************************************************
 *
 * Spinoacam.h -- header for Point Grey Spinnaker camera API
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_SPIN_OACAM_H
#define OA_SPIN_OACAM_H

#include <spinnaker/spinc/SpinnakerC.h>

extern int		oaSpinGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaSpinInitCamera ( oaCameraDevice* );
extern int		oaSpinCloseCamera ( oaCamera* );

extern int		oaSpinCameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaSpinCameraSetControl ( oaCamera*, int,
				oaControlValue*, int );
extern int		oaSpinCameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaSpinCameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );
extern int		oaSpinCameraGetControlDiscreteSet ( oaCamera*, int,
				int32_t*, int64_t** );

extern int		oaSpinCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaSpinCameraStopStreaming ( oaCamera* );
extern int		oaSpinCameraIsStreaming ( oaCamera* );

extern int		oaSpinCameraSetResolution ( oaCamera*, int, int );
extern int		oaSpinCameraSetROI ( oaCamera*, int, int );
extern int		oaSpinCameraTestROISize ( oaCamera*, unsigned int,
			    unsigned int, unsigned int*, unsigned int* );

extern void*		oacamSpincontroller ( void* );
extern void*		oacamSpincallbackHandler ( void* );

extern const FRAMESIZES* oaSpinCameraGetFrameSizes ( oaCamera* );
extern const FRAMERATES* oaSpinCameraGetFrameRates ( oaCamera*, int, int );
extern int		oaSpinCameraSetFrameInterval ( oaCamera*, int, int );
extern int		oaSpinCameraGetFramePixelFormat ( oaCamera* );

extern const char*	oaSpinCameraGetMenuString ( oaCamera*, int, int );

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
extern SPINNAKERC_API	( *p_spinNodeIsReadable )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinNodeIsWritable )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinStringGetValue )( spinNodeHandle, char*,
				size_t* );
extern SPINNAKERC_API	( *p_spinEnumerationEntryGetEnumValue )(
				spinNodeHandle, size_t* );
extern SPINNAKERC_API	( *p_spinEnumerationGetCurrentEntry )(
				spinNodeHandle, spinNodeHandle* );
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
extern SPINNAKERC_API	( *p_spinBooleanGetValue )( spinNodeHandle, bool8_t* );
extern SPINNAKERC_API	( *p_spinFloatGetMin )( spinNodeHandle, double* );
extern SPINNAKERC_API	( *p_spinFloatGetMax )( spinNodeHandle, double* );
extern SPINNAKERC_API	( *p_spinFloatGetValue )( spinNodeHandle, double* );

#define SPINNAKER_MAX_BUFF_LEN	256

#endif	/* OA_SPIN_OACAM_H */
