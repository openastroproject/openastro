/*****************************************************************************
 *
 * outputHandler.h -- class declaration
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
 *   James Fidell (james@openastroproject.org)
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
#include <QtGui>

extern "C" {
#include <openastro/camera.h>
}

#include "trampoline.h"


class OutputHandler
{
  public:
    			OutputHandler ( int, int, int, int, QString, trampolineFuncs* );
    virtual		~OutputHandler() {};
    unsigned int	getFrameCount ( void );

    virtual int		openOutput() = 0;
    virtual int		addFrame ( void*, const char*, int64_t,
                            const char*, FRAME_METADATA* ) = 0;
    virtual void	closeOutput() = 0;
    virtual int		outputExists ( void ) = 0;
    virtual int		outputWritable ( void ) = 0;
    QString		getFilename ( void );
    QString		getNewFilename ( void );
    QString		getRecordingFilename ( void );
    QString		getRecordingBasename ( void );
    int			writesDiscreteFiles;

  protected:
    int			frameCount;
    QString		fullSaveFilePath;
    QString		filenameRoot;
    void		generateFilename ( void );
		trampolineFuncs*	trampolines;

  private:
    QString		filename;
		QString		filenameTemplate;
};
