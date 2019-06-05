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

#ifdef HAVE_QT5
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
    void		configure ( void );
    void		setCapturedFramesDisplayInterval ( int );
    void		setEnabled ( int );
    void		enableDroppedDisplay ( int );
    void		enableScreenUpdates ( int );
    void		setDisplayFPS ( int );
    void		setFirstFrameTime ( void );
    void		beginRecording ( void );
    void		forceRecordingStop ( void );

  public slots:
    void		recentreReticle ( void );
    void		derotateReticle ( void );
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
    void		stopRecording ( void );

  private:
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
    int			hasTemp;
    int			hasDroppedFrames;
    int			reticleCentreX;
    int			reticleCentreY;
    int			flipX;
    int			flipY;
    int			demosaic;
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
    int			recordingInProgress;
    int			manualStop;
    int			focusScore;
    void*		stackBuffer[2];
    int			stackBufferLength;
    int			currentStackBuffer;
    void*		stackBufferInUse;
    unsigned int*	averageBuffer;
    unsigned int	averageBufferLength;
    unsigned int	totalFrames;

    unsigned int	reduceTo8Bit ( void*, void*, int, int, int );
    void		mousePressEvent ( QMouseEvent* );
    void		mouseMoveEvent ( QMouseEvent* );
    void		mouseReleaseEvent ( QMouseEvent* );
    void		wheelEvent ( QWheelEvent* );
    void		recalculateDimensions ( int );
    int			checkBuffers ( ViewWidget* );

    QVector<QRgb>	greyscaleColourTable;
    QVector<QRgb>	falseColourTable;

};
