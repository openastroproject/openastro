/*****************************************************************************
 *
 * outputHandler.cc -- output hander (mostly) virtual class
 *
 * Copyright 2013,2014,2018 James Fidell (james@openastroproject.org)
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

#include <oa_common.h>

#include <time.h>

#include "commonConfig.h"
#include "commonState.h"
#include "trampoline.h"
#include "outputHandler.h"
#include "captureSettings.h"


OutputHandler::OutputHandler ( int x, int y, int n, int d,
		QString nameTemplate, trampolineFuncs* tramps ) :
		trampolines ( tramps ), filenameTemplate ( nameTemplate )
{
  Q_UNUSED( x );
  Q_UNUSED( y );
  Q_UNUSED( n );
  Q_UNUSED( d );

	if ( nameTemplate == "" ) {
		filenameTemplate = commonConfig.fileNameTemplate.toStdString().c_str();
    generateFilename();
	}
}


void
OutputHandler::generateFilename ( void )
{
  struct tm*	tm;
  time_t	epochSecs;

  epochSecs = time(0);
  tm = localtime ( &epochSecs );

  QString seconds;
  seconds.setNum ( tm->tm_sec );
  if ( tm->tm_sec < 10 ) {
    seconds = "0" + seconds;
  }
  QString minutes;
  minutes.setNum ( tm->tm_min );
  if ( tm->tm_min < 10 ) {
    minutes = "0" + minutes;
  }
  QString hours;
  hours.setNum ( tm->tm_hour );
  if ( tm->tm_hour < 10 ) {
    hours = "0" + hours;
  }
  QString day;
  day.setNum ( tm->tm_mday );
  if ( tm->tm_mday < 10 ) {
    day = "0" + day;
  }
  QString month;
  month.setNum ( tm->tm_mon + 1 );
  if ( tm->tm_mon < 9 ) {
    month = "0" + month;
  }
  QString year;
  year.setNum ( tm->tm_year + 1900 );
  QString epoch;
  epoch.setNum ( epochSecs );
  QString date = year + month + day;
  QString time = hours + minutes + seconds;

  QString index, gain, exposureMs, exposureS;
  unsigned int exposure;
  index = QString("%1").arg ( commonState.captureIndex,
			captureConf.indexDigits, 10, QChar('0'));
  gain = QString("%1").arg ( trampolines->getCurrentGain());
  exposure = trampolines->getCurrentExposure();
  exposureMs = QString("%1").arg ( exposure / 1000 );
  exposureS = QString("%1").arg (( int ) ( exposure / 1000000 ));

  filename = filenameTemplate;
  filename.replace ( "%DATE", date );
  filename.replace ( "%TIME", time );
  filename.replace ( "%YEAR", year );
  filename.replace ( "%MONTH", month );
  filename.replace ( "%DAY", day );
  filename.replace ( "%HOUR", hours );
  filename.replace ( "%MINUTE", minutes );
  filename.replace ( "%SECOND", seconds );
  filename.replace ( "%EPOCH", epoch );
  filename.replace ( "%FILTER", trampolines->getCurrentFilterName());
  filename.replace ( "%PROFILE", trampolines->getCurrentProfileName());
  filename.replace ( "%INDEX", index );
  filename.replace ( "%GAIN", gain );
  filename.replace ( "%EXPMS", exposureMs );
  filename.replace ( "%EXPS", exposureS );

  filename.replace ( "%U", date + "-" + time );
  filename.replace ( "%D", date );
  filename.replace ( "%T", time );
  filename.replace ( "%Y", year );
  filename.replace ( "%M", month );
  filename.replace ( "%d", day );
  filename.replace ( "%H", hours );
  filename.replace ( "%M", minutes );
  filename.replace ( "%S", seconds );
  filename.replace ( "%I", index );

  filename.replace ( "%E", epoch );
  filename.replace ( "%G", gain );
  filename.replace ( "%x", exposureMs );
  filename.replace ( "%X", exposureS );

  if ( filename[0] != '/' ) {
		QString d = commonConfig.captureDirectory;
    if ( d == "" ) {
			d = commonState.currentDirectory;
    }
    filename = d + "/" + filename;
  }

  fullSaveFilePath = "";
}


unsigned int
OutputHandler::getFrameCount ( void )
{
  return frameCount;
}


QString
OutputHandler::getFilename ( void )
{
  return filename;
}


QString
OutputHandler::getNewFilename ( void )
{
  generateFilename();
  return filename;
}


QString
OutputHandler::getRecordingFilename ( void )
{
  return fullSaveFilePath;
}


QString
OutputHandler::getRecordingBasename ( void )
{
  return filenameRoot;
}
