/*****************************************************************************
 *
 * oacapture.cc -- main application entrypoint
 *
 * Copyright 2013,2014,2017,2018 James Fidell (james@openastroproject.org)
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
#include "mainWindow.h"
#include "state.h"


void usage();

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

  // FIX ME -- This all a bit cack-handed.  Find a better way to do it when
  // more command line parameters are required
  QStringList args = QCoreApplication::arguments();
  QString configFile = "";
  if ( !args.isEmpty()) {
    int p = 1;
    while  ( p < args.size()) {
      if ( configFile == "" ) {
        if ( args[p] == "-c" ) {
          p++;
          if ( p < args.size()) {
            configFile = args[p];
            p++;
          } else {
            usage();
          }
        } else {
          if ( args[p].startsWith ( "-c" )) {
            configFile = args[p].right ( args[p].size() - 2 );
            p++;
          } else {
            usage();
          }
        }
      } else {
        usage();
      }
    }
  }

  MainWindow mainWindow ( configFile );
  mainWindow.show();

  return app.exec();
}


void
usage()
{
  qCritical( "usage: oacapture [-c <config filename>]" );
  exit(1);
}
