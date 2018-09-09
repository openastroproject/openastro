To build the binaries the Qt4.8, cfitsio, tiff, pkgconfig, libdc1394,
libftdi1 and yasm packages are required.  I installed these from macports.
XCode must also be installed together with the command line tools (required
for macports to install anyhow).

If you're a homebrew user, you can simply copy-paste this to install all required packages:

  $ brew tap cartr/qt4
  $ brew tap-pin cartr/qt4
  $ brew install qt@4 cfitsio libtiff pkgconfig libdc1394 libftdi yasm automake libtool subversion libpng libhid autoconf-archive

To build all the binaries from the top level you should just be able
to do:

  $ ./configure && make

The usual flags for configure are of course supported.  If make tries to
recreate all the Makefiles and you don't want that, instead try:

  $ ./configure --disable-maintainer-mode && make
