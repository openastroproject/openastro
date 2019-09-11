/*****************************************************************************
 *
 * atikSerialoacam.h -- header for Atik serial (FTDI-based) camera API
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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

#ifndef OA_ATIK_SERIAL_OACAM_H
#define OA_ATIK_SERIAL_OACAM_H

#ifdef HAVE_FTDI_H
#include <ftdi.h>
#endif
#ifdef HAVE_LIBFTDI1_FTDI_H
#include <libftdi1/ftdi.h>
#endif
#ifdef HAVE_LIBFTDI_FTDI_H
#include <libftdi/ftdi.h>
#endif
#include <libusb-1.0/libusb.h>

#include "atikSerialstate.h"

extern int		oaAtikSerialGetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaAtikSerialInitCamera ( oaCameraDevice* );
extern int              oaAtikSerialCloseCamera ( oaCamera* );

extern int		oaAtikSerialCameraTestControl ( oaCamera*, int,
			    oaControlValue* );
extern int		oaAtikSerialCameraSetControl ( oaCamera*, int,
			    oaControlValue*, int );
extern int		oaAtikSerialCameraReadControl ( oaCamera*, int,
			    oaControlValue* );
extern int		oaAtikSerialCameraGetControlRange ( oaCamera*, int,
			    int64_t*, int64_t*, int64_t*, int64_t* );

extern int		oaAtikSerialCameraStartStreaming ( oaCamera*,
				void* (*)(void*, void*, int, void* ), void* );
extern int		oaAtikSerialCameraStopStreaming ( oaCamera* );
extern int		oaAtikSerialCameraIsStreaming ( oaCamera* );

extern int		oaAtikSerialCameraSetResolution ( oaCamera*, int, int );

extern void*		oacamAtikSerialcontroller ( void* );
extern void*		oacamAtikSerialcallbackHandler ( void* );

extern const FRAMESIZES* oaAtikSerialCameraGetFrameSizes ( oaCamera* );
extern int		oaAtikSerialCameraGetFramePixelFormat ( oaCamera* );

#if HAVE_LIBUDEV
extern int              _atikUdevSerialCamWrite ( AtikSerial_STATE*,
                            const unsigned char*, int );
extern int              _atikUdevSerialCamRead ( AtikSerial_STATE*,
                            unsigned char*, int );
extern int              _atikUdevSerialCamReadToZero ( AtikSerial_STATE*,
                            unsigned char*, unsigned int );
extern int		_atikUdevSerialCamReadBlock ( AtikSerial_STATE*,
                            unsigned char*, int );
#endif

#if HAVE_LIBFTDI
extern int              _atikFTDISerialCamWrite ( AtikSerial_STATE*,
                            const unsigned char*, int );
extern int              _atikFTDISerialCamRead ( AtikSerial_STATE*,
                            unsigned char*, int );
extern int              _atikFTDISerialCamReadToZero ( AtikSerial_STATE*,
                            unsigned char*, unsigned int );
extern int		_atikFTDISerialCamReadBlock ( AtikSerial_STATE*,
                            unsigned char*, int );
#endif


struct atikSerialCam {
  unsigned int	vendorId;
  unsigned int	productId;
  const char*	name;
  unsigned int	devType;
};

#define	NULL_READS		20
#define OPTIMAL_READ_SIZE	3968

#define DEFAULT_EXPOSURE        500

#endif	/* OA_ATIK_SERIAL_OACAM_H */
