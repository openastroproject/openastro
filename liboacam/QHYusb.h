/*****************************************************************************
 *
 * QHYusb.h -- header for QHY camera USB interface
 *
 * Copyright 2014,2015 James Fidell (james@openastroproject.org)
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

#ifndef OA_QHY_USB_H
#define OA_QHY_USB_H

extern int            _usbControlMsg ( QHY_STATE*, uint8_t, uint8_t, uint16_t,
                          uint16_t, unsigned char*, uint16_t, unsigned int );
extern int            _usbBulkTransfer ( QHY_STATE*, unsigned char,
                          unsigned char*, int, unsigned int*, unsigned int );
extern int            _i2cWrite16 ( QHY_STATE*, unsigned short,
			  unsigned short );
extern unsigned short _i2cRead16 ( QHY_STATE*, unsigned short );

#define QHY_CMD_CLEAR_FEATURE	0x01
#define QHY_CMD_DEFAULT_OUT	0x40
#define QHY_CMD_ENDP_OUT	0x42
#define QHY_CMD_DEFAULT_IN	0xc0
#define QHY_CMD_ENDP_IN		0xc2

#define QHY_BULK_ENDP_OUT	0x01
#define QHY_BULK_ENDP_IN	0x82
#define QHY_SDRAM_BULK_ENDP_IN	0x86

#define QHY_REQ_BEGIN_VIDEO	0xb3
#define QHY_REQ_SET_REGISTERS	0xb5
#define QHY_REQ_CAMERA_INFO	0xc2

#define	USB1_CTRL_TIMEOUT	5000
#define	USB1_BULK_TIMEOUT	10000
#define	USB2_TIMEOUT		5000

#endif	/* OA_QHY_USB_H */
