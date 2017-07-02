#!/bin/bash

export DEBEMAIL=james@openastro.org
export DEBFULLNAME="James Fidell"

version=`cat version`

srcdir=oacapture-$version
debdir=debian
debsrc=$debdir/source

tar jxf oacapture-$version.tar.bz2
cd $srcdir
dh_make -y -s -f ../oacapture-$version.tar.bz2

cp ../debfiles/control $debdir
cp ../debfiles/copyright $debdir
cp ../debfiles/changelog $debdir
echo 9 >> $debdir/compat
sed -e 's/needs="[^"]*"/needs="X11"/' \
    -e 's/see-menu-manual/Science/' < $debdir/menu.ex > $debdir/menu
chmod +x $debdir/menu
sed -e '/purge\|remove/a\
      service udev restart' < $debdir/postinst.ex > $debdir/postinst
chmod +x $debdir/postinst
sed -e '/purge\|remove/a\
      service udev restart' < $debdir/postrm.ex > $debdir/postrm
chmod +x $debdir/postrm
echo "3.0 (quilt)" > $debsrc/format
cat ../debfiles/rules.overrides >> $debdir/rules

dpkg-query -l cfitsio-dev 2>&1 >/dev/null
if [ $? -eq 0 ]; then
  cp $debdir/control $debdir/control.orig
  sed -e 's/libcfitsio-dev/cfitsio-dev/g' < $debdir/control.orig > $debdir/control
  rm $debdir/control.orig
fi

dpkg-query -l libhidapi-dev 2>&1 >/dev/null
if [ $? -ne 0 ]; then
  cp $debdir/control $debdir/control.orig
  sed -e 's/, libhidapi-dev//g' < $debdir/control.orig > $debdir/control
  rm $debdir/control.orig
fi

export QUILT_PATCHES="debian/patches"
export QUILT_PATCH_OPTS="--reject-format=unified"
export QUILT_DIFF_ARGS="-p ab --no-timestamps --no-index --color=auto"
export QUILT_REFRESH_ARGS="-p ab --no-timestamps --no-index"
mkdir -p $QUILT_PATCHES

test -f $debdir/README.Debian && rm $debdir/README.Debian
test -f $debdir/README.Debian && rm $debdir/README.Debian
test -f $debdir/README.source && rm $debdir/README.source
test -f $debdir/oacapture-docs.docs && rm $debdir/oacapture-docs.docs
rm $debdir/*.[Ee][Xx]

dpkg-buildpackage -us -uc
