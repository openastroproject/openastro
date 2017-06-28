/*****************************************************************************
 *
 * outputHandler.h -- class declaration
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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

class OutputHandler
{
  public:
    			OutputHandler ( int, int, int, int, QString );
    virtual		~OutputHandler() {};
    unsigned int	getFrameCount ( void );

    virtual int		openOutput() = 0;
    virtual int		addFrame ( void*, const char* ) = 0;
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

  private:
    QString		filename;
    QString		filenameTemplate;
};
