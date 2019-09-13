/*****************************************************************************
 *
 * camera.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2018,2019
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

#pragma once

#include <oa_common.h>

#ifdef HAVE_QT5
#include <QtWidgets>
#endif
#include <QtCore>
#include "QtGui"

extern "C" {
#include <openastro/camera.h>
}


class Camera : public QObject
{
  Q_OBJECT

  public:
    			Camera();
    			~Camera();
    int			listConnected ( oaCameraDevice***, unsigned long );
    void    releaseInfo ( oaCameraDevice** );
    int			initialise ( oaCameraDevice*, const char*, QWidget* );
    void		disconnect ( void );

    int			startStreaming ( void* (*)(void*, void*, int, void* ), void* );
    void		stop ( void );
    void		releaseImageData ( void );
    int			startExposure ( time_t, void* (*)(void*, void*, int, void* ),
								void* );

    int			hasFrameFormat ( int );
    int			hasRawMode ( void );
    int			hasDemosaicMode ( void );
    int			hasBinning ( int64_t );
    int			hasROI ( void );
    int     hasFixedFrameSizes ( void );
    int     hasUnknownFrameSize ( void );
		int			hasReadableControls ( void );
    int			hasControl ( int );
    int			hasAuto ( int );
    int			isAuto ( int );
    int			hasFixedFrameRates ( int, int );
    int			hasFrameRateSupport ( void );
    int			isColour ( void );
    int			isInitialised ( void );
    int			isSingleShot ( void );
    int			pixelSizeX ( void );
    int			pixelSizeY ( void );
    int			frameSizeUnknown ( void );

    void		controlRange ( int, int64_t*, int64_t*, int64_t*,
                            int64_t* );
    void		controlDiscreteSet ( int, int32_t*, int64_t** );
    const char*		name ( void );
    const FRAMESIZES*	frameSizes ( void );
    const FRAMERATES*	frameRates ( int, int );
    void		delayFrameRateChanges ( void );
    int			videoFramePixelFormat ( void );

    void		populateControlValue ( oaControlValue*, uint32_t,
				int64_t );
    int64_t		unpackControlValue ( oaControlValue* );
    int			setControl ( int, int64_t );
    int64_t		readControl ( int );
    int			getAWBManualSetting ( void );
    int			setResolution ( int, int );
    int			setROI ( int, int );
    int			setFrameInterval ( int, int );
    float		getTemperature ( void );
    int			setFrameFormat ( int );
    int			testROISize ( unsigned int, unsigned int,
				unsigned int*, unsigned int* );

    const char*		getMenuString ( int, int );

  private:

    oaCamera*		cameraContext;
    int			initialised;
};
