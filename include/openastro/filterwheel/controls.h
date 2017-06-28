/*****************************************************************************
 *
 * filterwheel/controls.h -- filter wheel API (sub)header for wheel controls
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_FILTERWHEEL_CONTROLS_H
#define OPENASTRO_FILTERWHEEL_CONTROLS_H

#define	OA_FW_CTRL_MOVE_ABSOLUTE_SYNC	0x01
#define	OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC	0x02
#define	OA_FW_CTRL_MOVE_RELATIVE_SYNC	0x03
#define	OA_FW_CTRL_MOVE_RELATIVE_ASYNC	0x04
#define	OA_FW_CTRL_SPEED		0x05
#define	OA_FW_CTRL_WARM_RESET		0x06
#define	OA_FW_CTRL_COLD_RESET		0x07
#define	OA_FW_CTRL_LAST_P1		OA_FW_CTRL_COLD_RESET+1

#endif	/* OPENASTRO_FILTERWHEEL_CONTROLS_H */
