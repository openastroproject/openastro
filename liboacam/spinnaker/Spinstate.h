/*****************************************************************************
 *
 * Spinstate.h -- Point Grey Gig-E Spinnaker camera state header
 *
 * Copyright 2018,2019,2021
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

#ifndef OA_SPINNAKER_STATE_H
#define OA_SPINNAKER_STATE_H

#include <spinnaker/spinc/SpinnakerC.h>
#include <openastro/util.h>

#include "sharedState.h"


typedef struct SPINNAKER_STATE {

#include "sharedDecs.h"

  uint64_t		deviceId;
  uint64_t		ipAddress;

	double			minFloatBlacklevel;
	double			maxFloatBlacklevel;
	double			minFloatGamma;
	double			maxFloatGamma;

  // pointers to shared library functions so we can use them if they are
  // present

  void*			p_spinSystemGetInstance;
  void*			p_spinCameraListClear;
  void*			p_spinCameraListCreateEmpty;
  void*			p_spinCameraListDestroy;
  void*			p_spinCameraListGetSize;
  void*			p_spinInterfaceListClear;
  void*			p_spinInterfaceListCreateEmpty;
  void*			p_spinInterfaceListDestroy;
  void*			p_spinInterfaceListGetSize;
  void*			p_spinSystemGetCameras;
  void*			p_spinSystemGetInterfaces;
  void*			p_spinSystemReleaseInstance;
  void*			p_spinInterfaceListGet;
  void*			p_spinInterfaceRelease;
  void*			p_spinInterfaceGetTLNodeMap;
  void*			p_spinNodeMapGetNode;
  void*			p_spinNodeIsAvailable;
  void*			p_spinNodeIsReadable;
  void*			p_spinStringGetValue;
  void*			p_spinIntegerGetValue;
  void*			p_spinEnumerationEntryGetEnumValue;
  void*			p_spinEnumerationGetCurrentEntry;
  void*			p_spinInterfaceGetCameras;
  void*			p_spinCameraListGet;
  void*			p_spinCameraGetTLDeviceNodeMap;
  void*			p_spinCameraRelease;

} SPINNAKER_STATE;

#endif	/* OA_SPINNAKER_STATE_H */
