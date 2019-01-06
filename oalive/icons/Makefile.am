#
# Makefile.am -- icons Makefile template
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

ICONS = oalive16.png oalive32.png oalive.svg oalive24.png oalive48.png \
				oalive.xpm oalive256.png oalive640.png

BUILT_SOURCES = $(ICONS)

$(ICONS):
	cp ../../icons/$@ .

install-icons:
	mkdir -p $(DESTDIR)${datarootdir}/pixmaps
	$(INSTALL_DATA) oalive.xpm $(DESTDIR)${datarootdir}/pixmaps
	mkdir -p $(DESTDIR)${datarootdir}/icons/hicolor/scalable/apps
	$(INSTALL_DATA) oalive.svg \
		$(DESTDIR)${datarootdir}/icons/hicolor/scalable/apps
	for s in 16 24 32 48 256; do \
		d=$(DESTDIR)${datarootdir}/icons/hicolor/$$s"x"$$s/apps; \
		$(MKDIR_P) $$d || exit 1; \
		$(INSTALL_DATA) oalive$$s.png $$d/oalive.png || exit $$?; \
	done

install-data-local: install-icons
