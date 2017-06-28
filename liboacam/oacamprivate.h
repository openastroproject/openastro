/*****************************************************************************
 *
 * oacamprivate.h -- shared declarations not exposed to the cruel world
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

#ifndef OA_CAM_PRIVATE_H
#define OA_CAM_PRIVATE_H

#include <oa_common.h>
#include <openastro/controller.h>

#define OA_CLEAR(x)	memset ( &(x), 0, sizeof ( x ))

typedef struct {
  oaCameraDevice**      cameraList;
  unsigned int          numCameras;
  unsigned int          maxCameras;
} CAMERA_LIST;

typedef struct {
  int64_t               min[ OA_CAM_CTRL_LAST_P1 ];
  int64_t               max[ OA_CAM_CTRL_LAST_P1 ];
  int64_t               step[ OA_CAM_CTRL_LAST_P1 ];
  int64_t               def[ OA_CAM_CTRL_LAST_P1 ];
} COMMON_INFO;

extern int		oacamHasAuto ( oaCamera*, int );

extern void		oacamSetDebugLevel ( int );
extern void		oacamClearDebugLevel ( int );
extern void		oacamAddDebugLevel ( int );
extern void		oacamDebugMsg ( int, const char*, ... );
extern int64_t		oacamGetControlValue ( oaControlValue* );
extern int		_oaCheckCameraArraySize ( CAMERA_LIST* );
extern void		_oaFreeCameraDeviceList ( CAMERA_LIST* );

extern char*		installPathRoot;

#endif /* OA_CAM_PRIVATE_H */
