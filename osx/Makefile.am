#
# Makefile.am -- lib Makefile template
#
# Copyright 2019,2023
#		James Fidell (james@openastroproject.org)
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

SUBDIRS = oaCapture.iconset oalive.iconset

BUILT_SOURCES = oacapture.plist oalive.plist

%.plist: %.plist.in $(top_srcdir)/version
	-rm -f -f $@.tmp
	sed -e "s/@FULL_VERSION@/`cat $(top_srcdir)/version`/g" >$@.tmp $<
	cmp -s $@.tmp $@ || ( rm -f $@ && mv $@.tmp $@ )

install:

clean:

mostlyclean-local:
	-rm -rf $(BUILT_SOURCES)
