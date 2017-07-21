/*****************************************************************************
 *
 * fits.c -- FITS data
 *
 * Copyright 2017 James Fidell (james@openastroproject.org)
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
#include <openastro/fits.h>

fitsKeyword	fitsKeywords[86] = {
  {
    .keyword = "SIMPLE",
    .shortDesc = "Always 'T'",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "BITPIX",
    .shortDesc = "Bit depth",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "NAXIS",
    .shortDesc = "Number of axes",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "NAXIS1",
    .shortDesc = "X axis",
    .keywordType = OA_FITS_BOOLEAN,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "NAXIS2",
    .shortDesc = "Y axis",
    .keywordType = OA_FITS_BOOLEAN,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "OBJECT",
    .shortDesc = "Name of object",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = 0
  },
  { 
    .keyword = "TELESCOP",
    .shortDesc = "Telescope",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = 0
  },
  { 
    .keyword = "INSTRUME",
    .shortDesc = "Camera",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = 0
  },
  { 
    .keyword = "OBSERVER",
    .shortDesc = "Observer's name",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = 0
  },
  { 
    .keyword = "DATE_OBS",
    .shortDesc = "Date observed",
    .keywordType = OA_FITS_TIMESTAMP,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = 0
  },
  { 
    .keyword = "BSCALE",
    .shortDesc = "Y axis",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = 0
  },
  { 
    .keyword = "BZERO",
    .shortDesc = "Y axis",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = 0
  },
  { 
    .keyword = "HISTORY",
    .shortDesc = "Image history",
    .keywordType = OA_FITS_BOOLEAN,
    .keywordOrigin = OA_FITS_ORG_STANDARD,
    .flags = OA_FITS_FLAG_REPEATABLE
  },
  { 
    .keyword = "EXPTIME",
    .shortDesc = "Total exposure time",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "CCD-TEMP",
    .shortDesc = "CCD temperature",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "XPIXSZ",
    .shortDesc = "Pixel width (um)",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "YPIXSZ",
    .shortDesc = "Pixel height (um)",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "XBINNING",
    .shortDesc = "X binning factor",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "YBINNING",
    .shortDesc = "Y binning factor",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "XORGSUBF",
    .shortDesc = "Subframe top left X",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "YORGSUBF",
    .shortDesc = "Subframe top left Y",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "EGAIN",
    .shortDesc = "Electronic gain (e-/ADU)",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "FOCALLEN",
    .shortDesc = "Telescope focal length",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "APTDIA",
    .shortDesc = "Telescope aperture diameter",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "APTAREA",
    .shortDesc = "Telescope aperture area",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "CBLACK",
    .shortDesc = "Black level (ADU)",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "CWHITE",
    .shortDesc = "White level (ADU)",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "PEDESTAL",
    .shortDesc = "Pedestal value",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "DATAMAX",
    .shortDesc = "Saturation value",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "SWCREATE",
    .shortDesc = "Software creating image",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "SWMODIFY",
    .shortDesc = "Software modifying image",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY | OA_FITS_FLAG_REPEATABLE
  },
  { 
    .keyword = "SBSTDVER",
    .shortDesc = "SBIG standard version",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = OA_FITS_FLAG_MANDATORY
  },
  { 
    .keyword = "FILTER",
    .shortDesc = "Filter used",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "TRAKTIME",
    .shortDesc = "Guider exposure time",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "SNAPSHOT",
    .shortDesc = "Number of combined images",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "SET-TEMP",
    .shortDesc = "CCD temperature set-point",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "IMAGETYP",
    .shortDesc = "Image type",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "OBJCTRA",
    .shortDesc = "RA of target",
    .keywordType = OA_FITS_HOUR_ANGLE,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "OBJCTDEC",
    .shortDesc = "DEC of target",
    .keywordType = OA_FITS_DEGREE_ANGLE,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "CENTAZ",
    .shortDesc = "Azimuth of target",
    .keywordType = OA_FITS_DEGREE_ANGLE,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "CENTALT",
    .shortDesc = "Altitude of target",
    .keywordType = OA_FITS_DEGREE_ANGLE,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "SITELAT",
    .shortDesc = "Imaging location latitude",
    .keywordType = OA_FITS_HOUR_ANGLE,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  { 
    .keyword = "SITELONG",
    .shortDesc = "Imaging location longitude",
    .keywordType = OA_FITS_HOUR_ANGLE,
    .keywordOrigin = OA_FITS_ORG_SBIG,
    .flags = 0
  },
  {
    .keyword = "AIRMASS",
    .shortDesc = "Relative atmospheric path length",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BAYERPAT",
    .shortDesc = "Bayer colour image",
    .keywordType = OA_FITS_BOOLEAN,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTAMBT",
    .shortDesc = "Boltwood ambient temperature", // deg C
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTCLOU",
    .shortDesc = "Boltwood cloud condition",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTDAY",
    .shortDesc = "Boltwood daylight level",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTDEW",
    .shortDesc = "Boltwood dewpoint", // deg C
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTHUM",
    .shortDesc = "Boltwood RH", // %age
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTRAIN",
    .shortDesc = "Boltwood rain condition",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTSKYT",
    .shortDesc = "Boltwood sky temp", // deg C
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "BOLTWIND",
    .shortDesc = "Boltwood wind speed", // km/h
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "CALSTAT",
    .shortDesc = "Calibration state of image", // B(ias)/D(ark)/F(lat)
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "CSTRETCH",
    .shortDesc = "Stretch mode",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "COLORTYP",
    .shortDesc = "Color sensor Bayer array type",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVRAD",
    .shortDesc = "DI solar radiation", // W/m^2
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVRAIN",
    .shortDesc = "DI accumulated rainfall", // mm/day
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVAMBT",
    .shortDesc = "DI ambient temperature", // deg C
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVDEW",
    .shortDesc = "DI dewpoint", // deg C
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVHUM",
    .shortDesc = "DI humidity", // %age
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVWIND",
    .shortDesc = "DI wind speed", // km/h
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVWINDD",
    .shortDesc = "DI wind direction", // deg
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DAVBAROM",
    .shortDesc = "DI barometric pressure", // hPa
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "DARKTIME",
    .shortDesc = "dark current integration time",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "FLIPSTAT",
    .shortDesc = "GEM pier flip status",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "FOCUSPOS",
    .shortDesc = "Focuser position in steps",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "FOCUSSZ",
    .shortDesc = "Focuser step size in microns",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "FOCUSTEM",
    .shortDesc = "Focuser temperature", // deg C
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "INPUTFMT",
    .shortDesc = "input file format",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "ISOSPEED",
    .shortDesc = "ISO camera setting",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "JD",
    .shortDesc = "Julian Day of exposure start",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "JD_GEO",
    .shortDesc = "Julian Day of exposure start",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "JD-HELIO",
    .shortDesc = "Julian Day of exposure midpoint",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "JD_HELIO",
    .shortDesc = "Julian Day of exposure midpoint",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "MIDPOINT",
    .shortDesc = "UT of midpoint of exposure",
    .keywordType = OA_FITS_TIMESTAMP,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "NOTES",
    .shortDesc = "user-entered information",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = OA_FITS_FLAG_REPEATABLE
  },
  {
    .keyword = "OBJCTALT",
    .shortDesc = "Altitude of center of image",
    .keywordType = OA_FITS_DEGREE_ANGLE,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "OBJCTAZ",
    .shortDesc = "Azimuth of center of image",
    .keywordType = OA_FITS_DEGREE_ANGLE,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "OBJCTHA",
    .shortDesc = "Hour angle of center of image",
    .keywordType = OA_FITS_HOUR_ANGLE,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "PIERSIDE",
    .shortDesc = "GEM Side-of-pier status",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "READOUTM",
    .shortDesc = "camera Readout Mode",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "ROTATANG",
    .shortDesc = "Rotator angle",
    .keywordType = OA_FITS_FLOAT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "TILEXY",
    .shortDesc = "tile position within a mosaic",
    .keywordType = OA_FITS_STRING,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "XBAYROFF",
    .shortDesc = "X offset of Bayer array",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  },
  {
    .keyword = "YBAYROFF",
    .shortDesc = "Y offset of Bayer array",
    .keywordType = OA_FITS_INT,
    .keywordOrigin = OA_FITS_ORG_MAXIMDL,
    .flags = 0
  }
};

