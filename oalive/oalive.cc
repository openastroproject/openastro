/*****************************************************************************
 *
 * oalive.cc -- main application entrypoint
 *
 * Copyright 2015,2017 James Fidell (james@openastroproject.org)
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

#include <QApplication>

#include "version.h"
#include "state.h"
#include "mainWindow.h"

int
main ( int argc, char* argv[] )
{
  QApplication app ( argc, argv );
  QString translateDir;

  app.setOrganizationName( ORGANISATION_NAME );
  app.setApplicationName( APPLICATION_NAME );

  state.appPath = QCoreApplication::applicationDirPath();
  if ( state.appPath.endsWith ( "/MacOS" )) {
    state.appPath.chop ( 6 );
  }
#if USE_APP_PATH
  oaSetRootPath ( state.appPath.toStdString().c_str());
#endif

  QString locale = QLocale::system().name();

  QTranslator qtTranslator;
  if ( qtTranslator.load ( "qt_" + locale, QLibraryInfo::location (
      QLibraryInfo::TranslationsPath ))) {
    app.installTranslator ( &qtTranslator );
  }

  QTranslator appTranslator;
#if USE_APP_PATH
  translateDir = state.appPath + "/Resources/translations/";
#else
  translateDir = TRANSLATE_DIR;
#endif

  if ( appTranslator.load ( translateDir + "oacapture_" + locale )) {
    app.installTranslator ( &appTranslator );
  }

  MainWindow mainWindow;
  mainWindow.show();

  return app.exec();
}
