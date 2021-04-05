/*****************************************************************************
 *
 * Spinstrings.c -- definitions of string values
 *
 * Copyright 2018,2021
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

#include <oa_common.h>

const char*	analogueFeatures[] = {
  "Gain Selector",
  "Gain Auto",
  "Gain",
  "Auto Gain Lower Limit",
  "Auto Gain Upper Limit",
  "Black Level",
  "Black Level Enabled",
  "Unknown analogue feature 7",
  "Unknown analogue feature 8",
  "Unknown analogue feature 9",
  "Unknown analogue feature 10",
  "Gamma",
  "Gamma Enabled",
  "Sharpness",
  "Sharpness Enabled",
  "Sharpness Auto"
};


const char* deviceFeatures[] = {
  "Device Indicator Mode",
  "Vendor Name",
  "Sensor Description",
  "Model Name",
  "Device Version",
  "Device SVN Version",
  "Device Firmware Version",
  "Device ID",
  "Device Serial Number",
  "Device User ID",
  "Device Scan Type",
  "Device Temperature",
  "Reset Device",
  "Device Uptime",
  "Auto Function AOIs Control",
  "Unknown device feature 15",
  "Unknown device feature 16",
  "Unknown device feature 17",
  "Unknown device feature 18",
  "Device Power Supply Selector",
  "Unknown device feature 20",
  "Unknown device feature 21",
  "Power Supply Voltage",
  "Power Supply Current",
  "Device Max Throughput",
  "Device Link Throughput Limit"
};


const char*	aquisitionFeatures[] = {
  "Trigger Selector",
  "Trigger Mode",
  "Unknown aquisition feature 2",
  "Trigger Source",
  "Trigger Activation",
  "Unknown aquisition feature 5",
  "Trigger Delay",
  "Trigger Delay Enabled",
  "Exposure Mode",
  "Exposure Auto",
  "Exposure Time",
  "Exposure Time Abs",
  "Auto Exposure Lower Limit",
  "Auto Exposure Upper Limit",
  "Exposure Compensation Auto",
  "Exposure Compensation",
  "Auto Exposure Compensation Lower Limit",
  "Auto Exposure Compensation Upper Limit",
  "Acquisition Mode",
  "Acquisition Start",
  "Acquisition Stop",
  "Frame Rate Auto",
  "Acquisition Frame Rate Control Enabled",
  "Acquisition Frame Rate"
};


const char*	formatFeatures[] = {
  "Pixel Format",
  "Unknown format feature 1",
  "Width",
  "Height",
  "Offset X",
  "Offset Y",
  "Sensor Width",
  "Sensor Height",
  "Max Width",
  "Max Height",
  "Video Mode",
  "Binning Horizontal",
  "Binning Vertical",
  "Unknown format feature 13",
  "Unknown format feature 14",
  "Reverse X",
  "Pixel BigEndian",
  "Pixel Coding",
  "Pixel Size",
  "Pixel Color Filter",
  "Pixel Dynamic Range Min",
  "Pixel Dynamic Range Max",
  "Test Image Selector",
  "Test Pattern"
};

const char* nodeTypes[] =
{
	"Value",
	"Base",
	"Integer",
	"Boolean",
	"Float",
	"Command",
	"String",
	"Register",
	"Enumeration",
	"Enumeration Entry",
	"Category",
	"Port"
};
