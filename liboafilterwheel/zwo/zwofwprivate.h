/*****************************************************************************
 *
 * zwofwprivate.h -- private headers for ZWO filter wheel API
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

#ifndef OA_ZWO_FW_PRIVATE_H
#define OA_ZWO_FW_PRIVATE_H

extern	int             ( *p_EFWGetNum )( void );
extern	EFW_ERROR_CODE  ( *p_EFWGetID )( int, int* );
extern	EFW_ERROR_CODE  ( *p_EFWOpen )( int );
extern	EFW_ERROR_CODE  ( *p_EFWGetProperty )( int, EFW_INFO* );
extern	EFW_ERROR_CODE	( *p_EFWSetPosition )( int, int );
extern	EFW_ERROR_CODE  ( *p_EFWClose )( int );

extern	int							_zwofwInitLibraryFunctionPointers ( void );

#endif	/* OA_ZWO_FW_PRIVATE_H */
