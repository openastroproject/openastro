#
# Makefile.am -- icons Makefile template
#
# Copyright 2019 James Fidell (james@openastroproject.org)
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

ICONS = icon_128x128@2x.png icon_128x128.png icon_16x16@2x.png icon_16x16.png \
				icon_256x256@2x.png icon_256x256.png icon_32x32@2x.png icon_32x32.png \
				icon_512x512@2x.png icon_512x512.png

BUILT_SOURCES = $(ICONS)

$(ICONS):
	cp ../../icons/oalive`echo $@ | sed 's/^.*[0-9][0-9]x//'` ./$@
