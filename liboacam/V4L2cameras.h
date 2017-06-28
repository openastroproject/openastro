/*****************************************************************************
 *
 * V4L2cameras.h -- header for camera recognition functions
 *
 * Copyright 2014 James Fidell (james@openastroproject.org)
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

#ifndef	OA_V4L2_CAMERAS_H
#define	OA_V4L2_CAMERAS_H

extern int	v4l2isSPC900 ( oaCameraDevice* );

// comes from the PWC driver, hope it won't change...
#define PWC_CID_CUSTOM(ctrl) ((V4L2_CID_USER_BASE | 0xf000) + custom_ ## ctrl)

enum { custom_autocontour, custom_contour, custom_noise_reduction,
        custom_awb_speed, custom_awb_delay,
        custom_save_user, custom_restore_user, custom_restore_factory };

#endif	/* OA_V4L2_CAMERAS_H */
