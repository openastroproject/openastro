/*****************************************************************************
 *
 * touptek-conf.h -- Touptek build configuration for Teleskop Service cameras
 *
 * Copyright 2024 James Fidell (james@openastroproject.org)
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

#ifndef OA_TSCAM_CONF_H
#define OA_TSCAM_CONF_H

#include <tscam.h>

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
#define TT_FUNC( prefix, suffix)	prefix ## Tscam ## suffix

// library handle type
#define	TT_HANDLE									HTscam

// config flag prefix
#define	TT_FLAG( flag )						TSCAM_FLAG_ ## flag

// option flag prefix
#define	TT_OPTION( option )				TSCAM_OPTION_ ## option

// library function name prefix
#define	TT_LIB_PREFIX							"Tscam"

// pointer to library function
#define	TT_LIB_PTR( func )				p_ ## Tscam_ ## func

// driver name
#define	TT_DRIVER									"Tscam"

// define prefix
#define	TT_DEFINE( var )					TSCAM_ ## var

// variable type prefix
#define TT_VAR_TYPE( type )				Tscam ## type

// function type
#define TT_FUNC_TYPE( prefix, suffix )	prefix ## TSCAM_ ## suffix

// shared object name
#define	TT_SOLIB									"Tscam"

// Patch library code?
//#define	TT_PATCH_BINARY

// interface name
#define	TT_INTERFACE							OA_CAM_IF_TSCAM

#endif	/* OA_TSCAM_CONF_H */
