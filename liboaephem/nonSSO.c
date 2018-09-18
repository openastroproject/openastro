/*****************************************************************************
 *
 * nonSSO.c -- calculations not relating to a specific solar system object
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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
#include <openastro/ephemeris.h>

int
oaDayNumber ( struct tm* date )
{
	int		dayNo, day, month, year;

	if ( date == 0 ) {
		date = gmtime ( 0 );
	}

	year = 1900 + date->tm_year;
	month = date->tm_mon + 1;
	day = date->tm_mday;

	dayNo = 367 * year - 7 * ( year + ( month + 9 ) / 12 ) / 4 +
		  275 * month / 9 + day - 730530;

	// add fractions of a day
	dayNo += date->tm_hour / 24.0 + date->tm_min / 1440.0 +
		  date->tm_sec / 86400.0;

	return dayNo;
}


int
oaJulianDayNumber ( struct tm* date )
{ 
  int   dayNo, day, month, year;

  if ( date == 0 ) {
    date = gmtime ( 0 );
  } 
  
  year = 1900 + date->tm_year;
  month = date->tm_mon + 1;
  day = date->tm_mday;
  
  dayNo = 367 * year - 7 * ( year + 5001 + ( month - 9 ) / 7 ) / 4 +
      275 * month / 9 + day - 1729777;

  // add fractions of a day
	if ( date->tm_hour >= 12 ) {
    dayNo += ( date->tm_hour - 12 ) / 24.0 + date->tm_min / 1440.0 +
        date->tm_sec / 86400.0;
	} else {
    dayNo += date->tm_hour / 24.0 + date->tm_min / 1440.0 +
        date->tm_sec / 86400.0 - 1;
	}

  return dayNo;
}


double
oaEclipticObliquity ( int day )
{
	double		ecl;

	ecl = 23.4393 - 3.563E-7 * day;
	return ecl;
}
