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

extern "C" {
#include <openastro/util.h>
}

#include "version.h"
#include "mainWindow.h"
#include "state.h"


void usage();

int
main ( int argc, char* argv[] )
{
  QApplication app ( argc, argv );
  QString translateDir;
  QString configFile = "";

	unsigned int logLevel = OA_LOG_NONE;
	unsigned int logType = OA_LOG_NONE;

#if HAVE_QT5
	QStringList debugTypeValues = { "none", "app", "camera", "fw", "timer" };
	QStringList debugLevelValues = { "none", "error", "warning", "info",
			"debug" };
	int	i, j, found;
#endif

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

#if HAVE_QT5
	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption configFileOption ( "c",
		QCoreApplication::translate ( "main", "name of config file to use" ),
		QCoreApplication::translate ( "main", "filename" ), "" );
	parser.addOption ( configFileOption );

	QCommandLineOption debugLevelOption ( "debug-level",
		QCoreApplication::translate ( "main",
			"Debug level (error|warning|info|debug)" ),
		QCoreApplication::translate ( "main", "level" ), "" );
	parser.addOption ( debugLevelOption );

	QCommandLineOption debugTypeOption ( "debug-type",
		QCoreApplication::translate ( "main",
			"Debug type ( app, camera, fw, timer, all )" ),
		QCoreApplication::translate ( "main", "type" ), "" );
	parser.addOption ( debugTypeOption );

	// Process the actual command line arguments given by the user
	parser.process ( app );

  QString debugLevel = parser.value ( debugLevelOption );
	if ( debugLevel != "" ) {
		found = 0;
		for ( i = 1; !found && i <= OA_LOG_DEBUG; i++ ) {
			if ( debugLevelValues[i] == debugLevel ) {
				found = 1;
				logLevel = i;
			}
		}
		if ( !found ) {
			qWarning() << "Unknown debug level: " << debugLevel;
		}
	}

  QString debugTypes = parser.value ( debugTypeOption );
	if ( debugTypes != "" ) {
		QStringList types = debugTypes.split ( "," );
		for ( j = 0; j < types.size(); j++ ) {
			found = 0;
			for ( i = 1; !found && i <= OA_LOG_NUM_TYPES; i++ ) {
				if ( types[j] == "all" ) {
					found = 1;
					logType = OA_LOG_TYPE_ALL;
				} else {
					if ( debugTypeValues[i] == types[j] ) {
						found = 1;
						logType |= ( 1 << ( i - 1 ));
					}
				}
			}
			if ( !found ) {
				qWarning() << "Unknown debug type: " << types[j];
			}
		}
	}

	configFile = parser.value ( configFileOption );

#else

  // FIX ME -- This all a bit cack-handed.  Find a better way to do it when
  // more command line parameters are required
  QStringList args = QCoreApplication::arguments();

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
#endif

	oaSetLogLevel ( logLevel );
	oaSetLogType ( logType );

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
