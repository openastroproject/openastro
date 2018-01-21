/*****************************************************************************
 *
 * V4L2camclass.h -- Find the highest configured command for camera class
 *
 * Copyright 2013,2014 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBV4L2

#include <libv4l2.h>

// This is very ugly, but there's no way to know what the "highest" camera
// class command is because it isn't defined anywhere.  The V4L2 spec
// says that the drivers are supposed to return EINVAL when they run out
// of controls, but that appears to be somewhat unreliable

#ifndef V4L2_CAMERA_CLASS_LASTP1
#ifdef V4L2_CID_AUTO_FOCUS_RANGE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_AUTO_FOCUS_RANGE+1)
#else
#ifdef V4L2_CID_AUTO_FOCUS_STATUS
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_AUTO_FOCUS_STATUS+1)
#else
#ifdef V4L2_CID_AUTO_FOCUS_STOP
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_AUTO_FOCUS_STOP+1)
#else
#ifdef V4L2_CID_AUTO_FOCUS_START
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_AUTO_FOCUS_START+1)
#else
#ifdef V4L2_CID_3A_LOCK
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_3A_LOCK+1)
#else
#ifdef V4L2_CID_SCENE_MODE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_SCENE_MODE+1)
#else
#ifdef V4L2_CID_EXPOSURE_METERING
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_EXPOSURE_METERING+1)
#else
#ifdef V4L2_CID_ISO_SENSITIVITY_AUTO
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_ISO_SENSITIVITY_AUTO+1)
#else
#ifdef V4L2_CID_ISO_SENSITIVITY
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_ISO_SENSITIVITY+1)
#else
#ifdef V4L2_CID_IMAGE_STABILIZATION
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_IMAGE_STABILIZATION+1)
#else
#ifdef V4L2_CID_AUTO_FOCUS_STATUS
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_AUTO_FOCUS_STATUS+1)
#else
#ifdef V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE+1)
#else
#ifdef V4L2_CID_AUTO_EXPOSURE_BIAS
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_AUTO_EXPOSURE_BIAS+1)
#else
#ifdef V4L2_CID_IRIS_RELATIVE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_IRIS_RELATIVE+1)
#else
#ifdef V4L2_CID_IRIS_ABSOLUTE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_IRIS_ABSOLUTE+1)
#else
#ifdef V4L2_CID_PRIVACY
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_PRIVACY+1)
#else
#ifdef V4L2_CID_ZOOM_CONTINUOUS
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_ZOOM_CONTINUOUS+1)
#else
#ifdef V4L2_CID_ZOOM_RELATIVE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_ZOOM_RELATIVE+1)
#else
#ifdef V4L2_CID_ZOOM_ABSOLUTE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_ZOOM_ABSOLUTE+1)
#else
#ifdef V4L2_CID_FOCUS_AUTO
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_FOCUS_AUTO+1)
#else
#ifdef V4L2_CID_FOCUS_RELATIVE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_FOCUS_RELATIVE+1)
#else
#ifdef V4L2_CID_FOCUS_ABSOLUTE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_FOCUS_ABSOLUTE+1)
#else
#ifdef V4L2_CID_TILT_ABSOLUTE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_TILT_ABSOLUTE+1)
#else
#ifdef V4L2_CID_PAN_ABSOLUTE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_PAN_ABSOLUTE+1)
#else
#ifdef V4L2_CID_TILT_RESET
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_TILT_RESET+1)
#else
#ifdef V4L2_CID_PAN_RESET
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_PAN_RESET+1)
#else
#ifdef V4L2_CID_TILT_RELATIVE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_TILT_RELATIVE+1)
#else
#ifdef V4L2_CID_PAN_RELATIVE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_PAN_RELATIVE+1)
#else
#ifdef V4L2_CID_EXPOSURE_AUTO_PRIORITY
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_EXPOSURE_AUTO_PRIORITY+1)
#else
#ifdef V4L2_CID_EXPOSURE_ABSOLUTE
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_EXPOSURE_ABSOLUTE+1)
#else
#ifdef V4L2_CID_EXPOSURE_AUTO
#define V4L2_CAMERA_CLASS_LASTP1 (V4L2_CID_EXPOSURE_AUTO+1)
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#endif /* HAVE_LIBV4L2 */
