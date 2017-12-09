/*****************************************************************************
 *
 * IMG132E.h -- header for IMG132E-specific control
 *
 * Copyright 2017 James Fidell (james@openastroproject.org)
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

#ifndef OA_IMG132E_H
#define OA_IMG132E_H

extern int		_IMG132EInitCamera ( oaCamera* );
extern void*		oacamIMG132Econtroller ( void* );
extern int		oaIMG132EInitialiseRegisters ( QHY_STATE* );


#define IMG132E_IMAGE_WIDTH		1280
#define IMG132E_IMAGE_HEIGHT		1024

#define IMG132E_DEFAULT_SPEED		0
#define IMG132E_DEFAULT_EXPOSURE	20000
#define IMG132E_DEFAULT_GAIN		0x400

#define IMX035_REG_STBY			0x00
#define IMX035_REG_TESTEN		0x01
#define IMX035_REG_VREVERSE		0x02
#define IMX035_REG_ADRES		0x03
#define IMX035_REG_FRSEL		0x04
#define IMX035_REG_SSBRK		0x05
#define IMX035_REG_SVS_LO		0x06
#define IMX035_REG_SVS_HI		0x07
#define IMX035_REG_SHS1_LO		0x08
#define IMX035_REG_SHS1_HI		0x09
#define IMX035_REG_SPL1_LO		0x12
#define IMX035_REG_SPL1_HI		0x13
#define IMX035_REG_WIN_1		0x16
#define IMX035_REG_WIN_2		0x17
#define IMX035_REG_WIN_3		0x18
#define IMX035_REG_WIN_4		0x19
#define IMX035_REG_WIN_5		0x1a
#define IMX035_REG_WIN_6		0x1b
#define IMX035_REG_AGAIN_LO		0x1c
#define IMX035_REG_AGAIN_HI		0x1d
#define IMX035_REG_DGAIN		0x1e
#define IMX035_REG_BLACKLEVEL		0x1f
#define IMX035_REG_XMSTA		0x26
#define IMX035_REG_VMAX_LO		0x7c
#define IMX035_REG_VMAX_HI		0x7d
#define IMX035_REG_HMAX_LO		0x7e
#define IMX035_REG_HMAX_HI		0x7f

#endif	/* OA_IMG132E_H */
