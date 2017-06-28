/*****************************************************************************
 *
 * EUVCusb.h -- header for EUVC camera USB interface
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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

#ifndef OA_EUVC_USB_H
#define OA_EUVC_USB_H

extern int    euvcUsbReadRegister ( EUVC_STATE*, uint8_t, uint8_t* );
extern int    euvcUsbWriteRegister ( EUVC_STATE*, uint8_t, uint8_t );
extern int    euvcUsbControlMsg ( EUVC_STATE*, uint8_t, uint8_t, uint16_t,
                  uint16_t, unsigned char*, uint16_t, unsigned int );
extern int    euvcUsbBulkTransfer ( EUVC_STATE*, unsigned char,
                  unsigned char*, int, unsigned int*, unsigned int );

extern void	euvcStatusCallback ( struct libusb_transfer* );

#define	USB_CTRL_TIMEOUT	5000
#define	USB_BULK_TIMEOUT	10000

#define USB_INTR_EP_OUT		0x01
#define USB_INTR_EP_IN		0x81
#define USB_BULK_EP_OUT		0x02
#define USB_BULK_EP_IN		0x82

#define USB_DIR_OUT		0x00
#define USB_DIR_IN		0x80

#define USB_CTRL_TYPE_STD	0x00
#define USB_CTRL_TYPE_CLASS	0x20
#define USB_CTRL_TYPE_VENDOR	0x40

#define USB_RECIP_DEVICE	0x00
#define USB_RECIP_INTERFACE	0x01
#define USB_RECIP_ENDPOINT	0x02
#define USB_RECIP_OTHER		0x03

#define REQ_SET_CUR		0x01
#define REQ_GET_CUR		0x81
#define REQ_GET_DEF		0x87

#endif	/* OA_EUVC_USB_H */
