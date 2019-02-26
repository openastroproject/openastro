/*****************************************************************************
 *
 * timer/controls.h -- TIMER API (sub)header for controls
 *
 * Copyright 2015,2016,2019 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_TIMER_CONTROLS_H
#define OPENASTRO_TIMER_CONTROLS_H

#define	OA_TIMER_CTRL_SYNC		0x01
#define	OA_TIMER_CTRL_NMEA		0x02
#define	OA_TIMER_CTRL_STATUS		0x03
#define	OA_TIMER_CTRL_COUNT		0x04
#define	OA_TIMER_CTRL_INTERVAL		0x05
#define	OA_TIMER_CTRL_MODE		0x06
#define	OA_TIMER_CTRL_EXT_LED_ENABLE		0x07

#define	OA_TIMER_CTRL_LAST_P1		OA_TIMER_CTRL_EXT_LED_ENABLE+1

#endif	/* OPENASTRO_TIMER_CONTROLS_H */
