/*****************************************************************************
 *
 * errno.h -- header for API error codes
 *
 * Copyright 2014,2015,2016,2019 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_ERRNO_H
#define OPENASTRO_ERRNO_H

#define OA_ERR_NONE			0
#define OA_ERR_OUT_OF_RANGE		1
#define OA_ERR_INVALID_CONTROL		2
#define OA_ERR_INVALID_SIZE		3
#define OA_ERR_SYSTEM_ERROR		4
#define OA_ERR_INVALID_BIT_DEPTH	5
#define OA_ERR_RESCAN_BUS		6
#define OA_ERR_MANUAL_FIRMWARE		7
#define OA_ERR_FXLOAD_NOT_FOUND		8
#define OA_ERR_FXLOAD_ERROR		9
#define OA_ERR_FIRMWARE_UNKNOWN		10
#define OA_ERR_INVALID_CONTROL_TYPE	11
#define OA_ERR_UNIMPLEMENTED		12
#define OA_ERR_INVALID_COMMAND		13
#define OA_ERR_INVALID_CAMERA		14
#define OA_ERR_IGNORED			15
#define OA_ERR_CAMERA_IO		16
#define OA_ERR_MEM_ALLOC		17
#define OA_ERR_INVALID_TIMER		18
#define OA_ERR_TIMER_RUNNING		19
#define OA_ERR_INVALID_TIMER_MODE	20
#define	OA_ERR_UNSUPPORTED_FORMAT	21
#define	OA_ERR_LIBRARY_NOT_FOUND	22
#define	OA_ERR_SYMBOL_NOT_FOUND	23

#endif	/* OPENASTRO_ERRNO_H */
