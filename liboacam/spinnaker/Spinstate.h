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

#include <spinc/SpinnakerC.h>
#include <openastro/util.h>

#include "sharedState.h"


typedef struct SPINNAKER_STATE {

#include "sharedDecs.h"

  uint64_t				deviceId;
  uint64_t				ipAddress;

	double					minFloatBlacklevel;
	double					maxFloatBlacklevel;
	double					minFloatGamma;
	double					maxFloatGamma;
	double					minFloatHue;
	double					maxFloatHue;
	double					minFloatSaturation;
	double					maxFloatSaturation;

	spinNodeHandle	gain;
	spinNodeHandle	autoGain;

	spinNodeHandle	gamma;
	spinNodeHandle	gammaEnabled;

	spinNodeHandle	hue;
	spinNodeHandle	autoHue;
	spinNodeHandle	hueEnabled;

	spinNodeHandle	saturation;
	spinNodeHandle	autoSaturation;
	spinNodeHandle	saturationEnabled;

	spinNodeHandle	sharpness;
	spinNodeHandle	autoSharpness;
	spinNodeHandle	sharpnessEnabled;

} SPINNAKER_STATE;

#endif	/* OA_SPINNAKER_STATE_H */
