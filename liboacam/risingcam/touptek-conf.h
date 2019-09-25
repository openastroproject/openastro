/*****************************************************************************
 *
 * touptek-conf.h -- Touptek build configuration for Risingcam cameras
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_RISINGCAM_CONF_H
#define OA_RISINGCAM_CONF_H

#include <nncam.h>

// Defines for:
//
// 1. function name (prefix?)
// 2. library handle type
// 3. config flag prefix
// 4. option flag prefix
// 5. pointer to library function
// 6.	driver name
// 7. shared object name
// 8. include library patching code
// 9. OA_CAM_IF_<name>

// Function name
#define TT_FUNC( prefix, suffix)	prefix ## Nncam ## suffix

// library handle type
#define	TT_HANDLE									HNnCam

// config flag prefix
#define	TT_FLAG( flag )						NNCAM_FLAG_ ## flag

// option flag prefix
#define	TT_OPTION( option )				NNCAM_OPTION_ ## option

// library function name
#define	TT_LIB_FUNC( func )				Nncam_ ## func

// pointer to library function
#define	TT_LIB_PTR( func )				p_ ## Nncam_ ## func

// driver name
#define	TT_DRIVER									"Nncam"

// define prefix
#define	TT_DEFINE( var )					NNCAM_ ## var

// variable type prefix
#define TT_VAR_TYPE( type )				Nncam ## type

// function type
#define TT_FUNC_TYPE( prefix, suffix )	prefix ## NNCAM_ ## suffix

// shared object name
#define	TT_SOLIB									"nncam"

// Patch library code?
#undef	TT_PATCH_BINARY

// interface name
#define	TT_INTERFACE							OA_CAM_IF_RISINGCAM

#endif	/* OA_RISINGCAM_CONF_H */
