/*****************************************************************************
 *
 * ZWASI2oacam.h -- header for ZW ASI camera API v2
 *
 * Copyright 2015,2016,2018 James Fidell (james@openastroproject.org)
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

#ifndef ZWASI2_OACAM_H
#define ZWASI2_OACAM_H

extern int		oaZWASI2GetCameras ( CAMERA_LIST*, unsigned long, int );
extern oaCamera*	oaZWASI2InitCamera ( oaCameraDevice* );
extern int              oaZWASI2CloseCamera ( oaCamera* );

extern int		oaZWASI2CameraTestControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaZWASI2CameraReadControl ( oaCamera*, int,
				oaControlValue* );
extern int		oaZWASI2CameraGetControlRange ( oaCamera*, int,
				int64_t*, int64_t*, int64_t*, int64_t* );

extern void*		oacamZWASI2controller ( void* );

extern int		oaZWASI2CameraGetFramePixelFormat ( oaCamera* );

#endif	/* ZWASI2_OACAM_H */
