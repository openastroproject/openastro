#
# Makefile.am -- udev rules Makefile template
#
# Copyright 2017,2018,2019 James Fidell (james@openastroproject.org)
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


# The rules for ASI cameras should be installed with the ASI libraries
# so we don't need them here
RULESFILES = 70-atik.rules 70-celestron-camera.rules \
	70-iidc-cameras.rules 70-opticstar-cameras.rules \
	70-starlight-xpress.rules 70-tis-cameras-euvc.rules \
	70-tis-cameras.rules 70-tis-cameras-uvc.rules \
	70-xagyl-ftdi.rules

udevrulesdir = ${sysconfdir}/../lib/udev/rules.d
udevrules_DATA = $(RULESFILES)
