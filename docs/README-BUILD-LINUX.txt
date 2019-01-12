To build the binaries a few prerequisites are required.

The binaries link against the ZWO ASI camera and ffmpeg libraries as
well as libusb1, the dc1394 library libv4l and Qt4.  The latter three
should be fairly easy to find the development packages for in most
distributions.  On Ubuntu, Mint (and perhaps Debian) you probably need:

  libv4l-dev
  libqt4-dev
  libdc1394-22-dev
  libcfitsio3-dev
  libudev-dev
  libtiff-dev
	libsdl1.2-dev
  qt4-dev-tools
  qt4-qmake
  gawk
  gcc
  g++
  yasm
  autoconf
  autoconf-archive
  libtool
	libusb-1.0-0-dev
	libhidapi-dev

On Fedora I found these to be sufficient:

  gcc
  gcc-c++
  systemd-devel
  libv4l-devel
  libtiff-devel
  libdc1394-devel
  libudev-devel
  cfitsio-devel
  qt
  qt-config
  qt-devel
  qt-x11
  yasm
  autoconf
  libtool
	SDL

As of release 0.0.6 the external libraries fmpeg and libusb are
included with the sources and build with them to make life a little
more predictable.

You'll also need to install the fxload packages for the QHY5 to work,
and your user must be in the "users" group for the device to be usable
by a user other than root.

yasm is required for the ffmpeg build.  Tweaking may be necessary to
get all the prerequisites installed, but configure should tell you
about most things.

To build all the binaries from the top level you should just be able
to do:

  $ ./configure && make

The usual flags for configure are of course supported.  If make tries to
rebuild all the makefiles etc. and you don't want that, try:

  $ ./configure --disable-maintainer-mode && make

instead.
