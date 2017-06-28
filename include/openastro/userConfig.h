/*****************************************************************************
 *
 * userConfig.h -- header for user configuration of devices
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

#ifndef OPENASTRO_USER_CONFIG_H
#define OPENASTRO_USER_CONFIG_H

#define	OA_USB_MANUFACTURER_MAX_LEN	64
#define	OA_USB_PRODUCT_MAX_LEN		64
#define	OA_USB_SERIAL_MAX_LEN		64

#define	OA_UDC_FLAG_NONE		0x00
#define OA_UDC_FLAG_VID_PID		0x01
#define OA_UDC_FLAG_MANUFACTURER	0x02
#define OA_UDC_FLAG_PRODUCT		0x04
#define OA_UDC_FLAG_SERIAL		0x08
#define OA_UDC_FLAG_FS_PATH		0x10
#define	OA_UDC_FLAG_USB_ALL		0x1f

typedef struct {
  unsigned int	vendorId;
  unsigned int	productId;
  char		manufacturer [ OA_USB_MANUFACTURER_MAX_LEN ];
  char		product [ OA_USB_PRODUCT_MAX_LEN ];
  char		serialNo [ OA_USB_SERIAL_MAX_LEN ];
  char		filesystemPath [ PATH_MAX ];
} userDeviceConfig;

#endif	/* OPENASTRO_USER_CONFIG_H */
