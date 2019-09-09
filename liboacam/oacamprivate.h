/*****************************************************************************
 *
 * oacamprivate.h -- shared declarations not exposed to the cruel world
 *
 * Copyright 2014,2015,2017,2019 James Fidell (james@openastroproject.org)
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
#include <openastro/camera.h>
#include <openastro/controller.h>

#define OA_CLEAR(x)	memset ( &(x), 0, sizeof ( x ))

typedef struct {
  oaCameraDevice**      cameraList;
  unsigned int          numCameras;
  unsigned int          maxCameras;
} CAMERA_LIST;

typedef struct {
  int64_t               minVal[3][ OA_CAM_CTRL_LAST_P1 ];
  int64_t               maxVal[3][ OA_CAM_CTRL_LAST_P1 ];
  int64_t               stepVal[3][ OA_CAM_CTRL_LAST_P1 ];
  int64_t               defVal[3][ OA_CAM_CTRL_LAST_P1 ];
} COMMON_INFO;

extern int		oacamHasAuto ( oaCamera*, int );

extern void		oacamSetDebugLevel ( int );
extern void		oacamClearDebugLevel ( int );
extern void		oacamAddDebugLevel ( int );
extern void		oacamDebugMsg ( int, const char*, ... );
extern int64_t		oacamGetControlValue ( oaControlValue* );
extern int		_oaCheckCameraArraySize ( CAMERA_LIST* );
extern void		_oaFreeCameraDeviceList ( CAMERA_LIST* );
extern int		_oaInitCameraStructs ( oaCamera**, void**, size_t, COMMON_INFO**);


extern char*		installPathRoot;

#define OA_CAM_CTRL_MIN(x)      minVal[OA_CAM_CTRL_MODIFIER(x)][OA_CAM_CTRL_MODE_BASE(x)]
#define OA_CAM_CTRL_MAX(x)      maxVal[OA_CAM_CTRL_MODIFIER(x)][OA_CAM_CTRL_MODE_BASE(x)]
#define OA_CAM_CTRL_DEF(x)      defVal[OA_CAM_CTRL_MODIFIER(x)][OA_CAM_CTRL_MODE_BASE(x)]
#define OA_CAM_CTRL_STEP(x)     stepVal[OA_CAM_CTRL_MODIFIER(x)][OA_CAM_CTRL_MODE_BASE(x)]

#define OA_CAM_CTRL_AUTO_MIN(x)         minVal[OA_CAM_CTRL_MODIFIER_AUTO][OA_CAM_CTRL_MODE_BASE(x)]
#define OA_CAM_CTRL_AUTO_MAX(x)         maxVal[OA_CAM_CTRL_MODIFIER_AUTO][OA_CAM_CTRL_MODE_BASE(x)]
#define OA_CAM_CTRL_AUTO_DEF(x)         defVal[OA_CAM_CTRL_MODIFIER_AUTO][OA_CAM_CTRL_MODE_BASE(x)]
#define OA_CAM_CTRL_AUTO_STEP(x)        stepVal[OA_CAM_CTRL_MODIFIER_AUTO][OA_CAM_CTRL_MODE_BASE(x)]

#define FREE_DATA_STRUCTS		free (( void* ) commonInfo ); free (( void* ) cameraInfo ); free (( void* ) camera )

#endif /* OA_CAM_PRIVATE_H */
