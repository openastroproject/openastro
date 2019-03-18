#
# Makefile.am -- QHY firmware Makefile template
#
# Copyright 2017,2019 James Fidell (james@openastroproject.org)
#
# License:
#
# This file is part of the Open Astro Project.
#
# The Open Astro Project is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Open Astro Project is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Open Astro Project.  If not, see
# <http://www.gnu.org/licenses/>.
#


FIRMWAREFILES = DSI2PRO.HEX IC16200A.HEX IC16803.HEX IC8300.HEX IC90A.HEX \
	IMG0H.HEX IMG2P.HEX IMG2S.HEX IMG50.HEX miniCam5.HEX POLEMASTER.HEX \
	QHY09000A.HEX QHY10.HEX QHY11.HEX QHY128.img QHY12.HEX QHY15.HEX \
  QHY160002AD.HEX QHY16000.HEX QHY16200A.HEX QHY163.img QHY165.img \
	QHY16803A.HEX QHY168.img QHY16.HEX QHY174.img QHY178.img QHY183.img \
	QHY2020.img QHY20.HEX QHY21.HEX QHY224.img QHY22.HEX QHY23.HEX QHY247.img \
	QHY27.HEX QHY28.HEX QHY290.img QHY294.img QHY29.HEX QHY2E.HEX QHY2.HEX \
	QHY2PRO.HEX QHY367.img QHY4040.img QHY42.img QHY42PRO.img QHY550.img \
	QHY5.HEX QHY5II.HEX QHY5III174.img QHY5III178.img QHY5III185.img \
	QHY5III224.img QHY5III290.img QHY5LOADER.HEX QHY5V.HEX QHY695A.HEX QHY6.HEX \
	QHY6OLD.HEX QHY6PRO.HEX QHY7.HEX QHY814A.HEX QHY8.HEX QHY8L.HEX QHY8M.HEX \
	QHY8PRO.HEX QHY90A.HEX QHY9.HEX QHY9L.HEX QHY9S.HEX QHYXXX.HEX SLAVEFIFO.HEX

# Nasty, this, but there doesn't appear to be a suitable other way to
# handle it

firmwaredir = ${prefix}/../lib/firmware/qhy
firmware_DATA = $(FIRMWAREFILES)
