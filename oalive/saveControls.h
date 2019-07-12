/*****************************************************************************
 *
 * saveControls.h -- class declaration
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

#define	CAPTURE_TIFF	1
#define	CAPTURE_PNG		2
#define	CAPTURE_FITS	3

class SaveControls : public QWidget
{
  Q_OBJECT

  public:
    			SaveControls ( QWidget* );
    			~SaveControls();

  private:
    QVBoxLayout*	layout;
    QGridLayout*	grid;
    QLabel*		titleLabel;
    QLabel*		commentLabel;
    QLabel*		instrumentLabel;
    QLabel*		objectLabel;
    QLabel*		observerLabel;
    QLabel*		telescopeLabel;
    QLineEdit*		titleInput;
    QLineEdit*		commentInput;
    QLineEdit*		instrumentInput;
    QLineEdit*		objectInput;
    QLineEdit*		observerInput;
    QLineEdit*		telescopeInput;
    QCheckBox*		saveEachFrame;
    QCheckBox*		saveProcessedImage;
    QLabel*		frameLabel;
    QLabel*		processedLabel;
    QLineEdit*		frameFilename;
    QLineEdit*		processedFilename;
    QLineEdit*		captureDirectory;
    QPushButton*	newFolderButton;
    QPushButton*	openFolderButton;
    QHBoxLayout*	dirButtonBox;
    QLabel*		typeLabel;
    QComboBox*		typeMenu;
    QHBoxLayout*	typeBox;

  public slots:
    void		updateObserver ( void );
    void		updateTelescope ( void );
    void		updateInstrument ( void );
    void		updateObject ( void );
    void		updateComment ( void );
    void		updateFrameFilename ( void );
    void		updateProcessedFilename ( void );
    void		updateSaveEachFrame ( int );
    void		updateSaveProcessedImage ( int );
    void		setNewCaptureDirectory ( void );
    void		openCaptureDirectory ( void );
    void		fileTypeChanged ( int );
};
