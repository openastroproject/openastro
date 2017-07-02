#!/bin/bash
#
# install.sh -- install script for the binary distribution
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
mkdir -p /usr/local/openastro/lib

rsync -a lib/ /usr/local/openastro/lib
ln -s /usr/local/openastro/lib/*.so /usr/local/lib
cp bin/oacapture /usr/local/openastro/bin
chmod 755 /usr/local/openastro/bin/oacapture

mkdir -p /usr/local/bin
ln -sf /usr/local/openastro/bin/oacapture /usr/local/bin

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

echo "install complete"
echo "For QHY cameras, remember to add yourself to the 'users' group and log in again"
