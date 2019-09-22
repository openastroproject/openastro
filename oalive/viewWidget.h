/*****************************************************************************
 *
 * viewWidget.h -- class declaration
 *
 * Copyright 2015,2016,2018,2019 James Fidell (james@openastroproject.org)
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

#if HAVE_QT5
#include <QtWidgets>
#endif
#include <QtGui>

extern "C" {
#include <openastro/camera.h>
}

#include "configuration.h"


class ViewWidget : public QFrame
{
  Q_OBJECT

  public:
    			ViewWidget ( QWidget* parent = 0 );
    			~ViewWidget();

    void		setVideoFramePixelFormat ( int );
    void		enableTempDisplay ( int );
    void		enableFlipX ( int );
    void		enableFlipY ( int );
    static void*	addImage ( void*, void*, int, void* );
    void		restart ( void );

    void		updateFrameSize ( void );
    void		zoomUpdated ( int );
    void		configure ( void );
    void		setCapturedFramesDisplayInterval ( int );
    void		setEnabled ( int );
    void		enableDroppedDisplay ( int );
    void		enableScreenUpdates ( int );
    void		setDisplayFPS ( int );
    void		setFirstFrameTime ( void );
    void		beginRecording ( void );
    void		forceRecordingStop ( void );
		void		setBlackLevel ( int );
		void		setWhiteLevel ( int );
		void		setBrightness ( int );
		void		setContrast ( int );
		void		setSaturation ( int );
		void		setGamma ( int );
		int			getStackedFrames ( void );

  public slots:
    void		recentreReticle ( void );
    void		derotateReticle ( void );
		void		redrawImage ( void );
    void		setMonoPalette ( QColor );

  protected:
    void		paintEvent ( QPaintEvent* );

  signals:
    void		updateFrameCount ( unsigned int );
    void		updateActualFrameRate ( unsigned int );
    void		updateTemperature ( void );
    void		updateDroppedFrames ( void );
    void		updateProgress ( unsigned int );
    void		updateHistogram ( void );
    void		updateDisplay ( void );
    void		updateBatteryLevel ( void );
    void		stopRecording ( void );
    void		startNextExposure ( void );
		void		updateStackedFrameCount ( void );

  private:
		void			_recalcCoeffs ( void );
		void			_displayCoeffs ( void );
		int				_unpackImageFrame ( ViewWidget*, void*, int*, int*, unsigned int*,
									unsigned int* );
		int				_unpackJPEG8 ( ViewWidget*, void*, int*, int*, unsigned int*,
									unsigned int* );
		int				_unpackLibraw ( ViewWidget*, void*, int*, int*, unsigned int*,
									unsigned int* );
		void			_processAndDisplay ( void*, ViewWidget*, unsigned long, int );

    QImage		image;
    int			currentZoom;
    int			currentZoomX;
    int			currentZoomY;
    int			capturedFramesDisplayInterval;
    unsigned long	lastCapturedFramesUpdateTime;
    int			frameDisplayInterval;
    unsigned long	lastDisplayUpdateTime;
    int			videoFramePixelFormat;
    unsigned int	framesInLastSecond;
    long		secondForFrameCount;
    long		secondForTemperature;
    long		secondForDropped;
    long		secondForAutoControls;
    long		minuteForBatteryLevel;
    int			hasTemp;
    int			hasDroppedFrames;
    int			reticleCentreX;
    int			reticleCentreY;
    int			flipX;
    int			flipY;
    int			demosaic;
		void*		rgbBuffer;
		int			rgbBufferSize;
    void*		viewImageBuffer[2];
    int			viewBufferLength;
    void*		writeImageBuffer[2];
    int			writeBufferLength;
    int                 expectedSize;
    int                 screenUpdatesEnabled;
    int                 savedXSize;
    int                 savedYSize;
    int                 lastPointerX;
    int                 lastPointerY;
    int                 movingReticle;
    int                 rotatingReticle;
    int                 diagonalLength;
    qreal		rotationAngle;
    QTransform		rotationTransform;
    int			setNewFirstFrameTime;
    pthread_mutex_t	imageMutex;
    int			focusScore;
		void**	previousFrames;
		unsigned int	previousFrameArraySize;
    unsigned int	nextFrame;
		unsigned int	maxFrames;
		unsigned int	frameLimit;

    unsigned int	reduceTo8Bit ( void*, void*, int, int, int );
    void		mousePressEvent ( QMouseEvent* );
    void		mouseMoveEvent ( QMouseEvent* );
    void		mouseReleaseEvent ( QMouseEvent* );
    void		wheelEvent ( QWheelEvent* );
    int			checkBuffers ( ViewWidget* );
    void		recalculateDimensions ( int );

    QVector<QRgb>	greyscaleColourTable;
    QVector<QRgb>	falseColourTable;

		int						blackPoint;
		int						whitePoint;
		int						brightness;
		int						contrast;
		int						saturation;
		double				gammaExponent;

		double				coeff_b;
		double				coeff_c;
		double				coeff_t;
		double				coeff_s;
		double				coeff_r;
		double				coeff_sr;
		double				coeff_sg;
		double				coeff_sb;
		double				coeff_r1;
		double				coeff_r2;
		double				coeff_r3;
		double				coeff_g1;
		double				coeff_g2;
		double				coeff_g3;
		double				coeff_b1;
		double				coeff_b2;
		double				coeff_b3;
		double				coeff_tbr;

		int						viewPixelFormat;
		int						currentViewBuffer;
		void*					viewBuffer;
		void*					originalBuffer;
		int						abortProcessing;
};
