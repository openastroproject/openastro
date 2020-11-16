/*****************************************************************************
 *
 * to8Bit.h -- conversion to 8-bit header
 *
 * Copyright 2017,2018,2020
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

#ifndef OPENASTRO_VIDEO_ENDIAN_H
#define OPENASTRO_VIDEO_ENDIAN_H

extern void	oaBigEndian16BitTo8Bit ( void*, void*, unsigned int );
extern void	oaBigEndianShifted16BitTo8Bit ( void*, void*, unsigned int,
								unsigned int );
extern void	oaLittleEndian16BitTo8Bit ( void*, void*, unsigned int );
extern void	oaLittleEndianShifted16BitTo8Bit ( void*, void*, unsigned int,
								unsigned int );
extern void	oaPackedGrey12ToGrey8 ( void*, void*, unsigned int );


#endif	/* OPENASTRO_VIDEO_ENDIAN_H */
