#!/bin/bash
#
# INSTALL.sh -- install script for the binary distribution
#
# Copyright 2013,2014 James Fidell (james@openastroproject.org)
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

uid=`id -u`
if [ $uid -ne 0 ]; then
  echo "This script must be run as root"
  exit 1
fi

mkdir -p /usr/local/openastro/bin
mkdir -p /usr/local/openastro/lib/firmware/qhy

cp oacapture/oacapture /usr/local/openastro/bin
cp lib/firmware/qhy/* /usr/local/openastro/lib/firmware/qhy

chmod 755 /usr/local/openastro/bin/oacapture

mkdir -p /usr/local/bin
ln -sf /usr/local/openastro/bin/oacapture /usr/local/bin

UDEVDIR=/etc/udev/rules.d
if [ -d $UDEVDIR ]
then
  for rules in udev/*.rules
  do
    r=`basename $rules`
    target=/etc/udev/rules.d/$r
    replace=1
    if [ -f $target ]
    then
      cmp -s $rules $target
      if [ $? -eq 0 ]
      then
        # file exists and is the same
        replace=0
      else
        # file exists and is different
        n=1
        while [ -f $target.$n ]
        do
          n=$(( $n + 1 ))
        done
        cp $target $target.$n
      fi
    fi
    if [ $replace -ne 0 ]
    then
      cp $rules $target
    fi
  done
else
  echo "$UDEVDIR not found."
  echo
  echo "If you use an ASI camera you will need to install"
  echo "udev/70-asi-cameras.rules in the appropriate place"
  echo
  echo "If you use an Imaging Source camera you will need to install"
  echo "udev/70-tis-cameras.rules in the appropriate place"
  echo
  echo "If you use a QHY5 camera you will need to install"
  echo "udev/85-qhy-cameras.rules in the appropriate place"
  echo
  exit 1
fi

echo "install complete"
