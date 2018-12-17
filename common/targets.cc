/*****************************************************************************
 *
 * targets.cc -- names of targets
 *
 * Copyright 2013,2014 James Fidell (james@openastroproject.org)
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

#include <QtCore>

#include "targets.h"

const char*	targetList[NUM_TARGETS] = {
  QT_TRANSLATE_NOOP( "QObject", "Other" ),
  QT_TRANSLATE_NOOP( "QObject", "Mercury" ),
  QT_TRANSLATE_NOOP( "QObject", "Venus" ),
  QT_TRANSLATE_NOOP( "QObject", "Earth" ),
  QT_TRANSLATE_NOOP( "QObject", "Mars" ),
  QT_TRANSLATE_NOOP( "QObject", "Jupiter" ),
  QT_TRANSLATE_NOOP( "QObject", "Saturn" ),
  QT_TRANSLATE_NOOP( "QObject", "Uranus" ),
  QT_TRANSLATE_NOOP( "QObject", "Neptune" ),
  QT_TRANSLATE_NOOP( "QObject", "Pluto" ),
  QT_TRANSLATE_NOOP( "QObject", "Moon" ),
  QT_TRANSLATE_NOOP( "QObject", "Sun" )
};


const QString
targetName ( unsigned int targetIdx )
{
	if ( targetIdx >= NUM_TARGETS ) {
		return "";
	}

	return QObject::tr ( targetList[ targetIdx ]);
}
