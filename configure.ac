#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([openastro], [1.1.0], [james@openastro.org])
AC_CONFIG_SRCDIR([liboademosaic/oademosaic.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE([disable])
AM_SILENT_RULES([yes])

AC_DISABLE_SHARED

# Checks for programs.

PKG_PROG_PKG_CONFIG

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AC_CHECK_PROGS(MOC, [moc moc-qt5 moc-qt4])
AC_CHECK_PROGS(UIC, [uic uic-qt5 uic-qt4])
AC_CHECK_PROGS(RCC, [rcc])
AC_CHECK_PROGS(LCONVERT, [lconvert])
if test -z "$MOC" || test -z "$UIC" || test -z "$RCC" || test -z "$LCONVERT"
then
AC_MSG_ERROR([Qt utility programs moc, uic, rcc and lconvert are required.])
fi

checkQtForPIC=0
checkSystemFFMPEG=0
checkSystemLibusb=1
checkSystemDC1394=1
checkSystemLibuvc=0
checkSystemLibhidapi=0
checkSystemLibasicamera=0
useisystemInclude=0
build_libusb=no
case $host in
  *-linux-*)
    checkQtForPIC=1
    checkSystemFFMPEG=1
    checkSystemDC1394=1
    checkSystemLibuvc=1
    checkSystemLibhidapi=1
    checkSystemLibasicamera=1
    ;;
  *-apple-*)
    useisystemInclude=1
    build_libusb=yes
    ;;
esac

# Retrieve Qt compilation and linker flags
have_qt5=no
qtver=`$MOC -v 2>&1 | sed 's/^.* \(.*\..*\)\..*$/\1/'`
qt_maj=`echo "$qtver" | cut -f1 -d.`
qt_min=`echo "$qtver" | cut -f2 -d.`

if test "$qt_maj" -eq 5
then
  have_qt5=yes
fi

if test "$have_qt5" == "yes"
then
  PKG_CHECK_MODULES(QT5, [Qt5Core, Qt5Gui, Qt5Network, Qt5Widgets], [], [
    AC_MSG_ERROR([Qt5 libraries are required.])])
  AC_MSG_NOTICE([Building using Qt5])
  if test $checkQtForPIC -ne 0; then
    $PKG_CONFIG --variable=qt_config Qt5Core | grep -q reduce_relocations
    red_rel=$?
    if test $red_rel -eq 0; then
      CXXFLAGS="-fPIC $CXXFLAGS"
      AC_MSG_NOTICE([Qt5 appears to require compilation with -fPIC])
    fi
  fi
  flags=`$PKG_CONFIG --cflags-only-I Qt5Core Qt5Gui Qt5Network Qt5Widgets`
  MOC_CFLAGS="$flags"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/-I/-isystem/g'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L Qt5Core Qt5Gui Qt5Network Qt5Widgets` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l Qt5Core Qt5Gui Qt5Network Qt5Widgets` $LIBS"
  AC_DEFINE([HAVE_QT5],[1],[Have Qt5])
else
  PKG_CHECK_MODULES(QT, [QtCore, QtGui, QtNetwork], [], [
    AC_MSG_ERROR([Qt libraries are required.])])
  if ! `$PKG_CONFIG --exists 'QtCore >= 4.8.0'`; then
    AC_MSG_ERROR([Qt >= 4.8.0 is required.])
  fi
  AC_MSG_NOTICE([Building using Qt4])
  flags=`$PKG_CONFIG --cflags-only-I QtCore QtGui QtNetwork`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/-I/-isystem/g'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L QtCore QtGui QtNetwork` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l QtCore QtGui QtNetwork` $LIBS"
  AC_DEFINE([HAVE_QT4],[1],[Have Qt4])
fi

# Checks for libraries.

have_v4l2=no
have_udev=no
have_ftdi1=no
have_flycapture2=no
have_dl=no
have_toupcam=yes
have_mallincam=yes
have_altaircam=yes
have_libavformat=no
have_libavutil=no
have_libavcodec=no
internal_uvc=no
have_libhidapi=no

case $host in
  x86_64-apple-*)
    CPPFLAGS="-isystem/opt/local/include $CPPFLAGS"
    LDFLAGS="-L/opt/local/lib $LDFLAGS"
    ;;
  i?86-apple-*)
    CPPFLAGS="-isystem/opt/local/include $CPPFLAGS"
    LDFLAGS="-L/opt/local/lib $LDFLAGS"
    ;;
esac

if `$PKG_CONFIG --exists libv4l2`; then
  flags=`$PKG_CONFIG --cflags-only-I libv4l2`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L libv4l2` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l libv4l2` $LIBS"
  have_v4l2=yes
  AC_DEFINE([HAVE_LIBV4L2],[1],[Have V4L2 library])
  AC_MSG_NOTICE([Using system libv4l2 library])
else
  AC_CHECK_LIB([v4l2],[v4l2_open],[
    LIBS="-lv4l2 $LIBS"
    AC_DEFINE([HAVE_LIBV4L2],[1],[Have V4L2 library])
    have_v4l2=yes
  ],[
    AC_MSG_WARN([No V4L2 library found.  V4L2 cameras will not be supported])])
fi

if `$PKG_CONFIG --exists libtiff-4`; then
  flags=`$PKG_CONFIG --cflags-only-I libtiff-4`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L libtiff-4` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l libtiff-4` $LIBS"
  have_tiff=yes
  AC_DEFINE([HAVE_LIBTIFF],[1],[Have libtiff])
  AC_MSG_NOTICE([Using system libtiff library])
else
  AC_CHECK_LIB([tiff],[TIFFOpen],[],[
    AC_MSG_ERROR([libtiff is required])])
fi

if `$PKG_CONFIG --exists libpng`; then
  flags=`$PKG_CONFIG --cflags-only-I libpng`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L libpng` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l libpng` $LIBS"
  have_png=yes
  AC_DEFINE([HAVE_LIBPNG],[1],[Have libpng])
  AC_MSG_NOTICE([Using system libpng library])
else
  AC_CHECK_LIB([png],[png_sig_cmp],[],[
    AC_MSG_ERROR([libpng is required])])
fi

if `$PKG_CONFIG --exists cfitsio`; then
  flags=`$PKG_CONFIG --cflags-only-I cfitsio`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L cfitsio` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l cfitsio` $LIBS"
  AC_DEFINE([HAVE_LIBCFITSIO],[1],[Have CFITSIO library])
else
  AC_CHECK_LIB([cfitsio],[ffopen],[
    LIBS="-lcfitsio $LIBS"
    AC_DEFINE([HAVE_LIBCFITSIO],[1],[Have CFITSIO library])
  ],[
    AC_MSG_ERROR([libcfitsio3 is required])])
fi

if `$PKG_CONFIG --exists libbz2`; then
  flags=`$PKG_CONFIG --cflags-only-I libbz2`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L libbz2` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l libbz2` $LIBS"
  have_bz2=yes
  AC_DEFINE([HAVE_LIBBZ2],[1],[Have libbz2])
  AC_MSG_NOTICE([Using system libbz2 library])
else
  AC_CHECK_LIB([bz2],[BZ2_bzDecompress],[],[
    AC_MSG_WARN([No bz2 library found])])
fi

if `$PKG_CONFIG --exists zlib`; then
  flags=`$PKG_CONFIG --cflags-only-I zlib`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L zlib` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l zlib` $LIBS"
  have_zlib=yes
  AC_DEFINE([HAVE_LIBZ],[1],[Have libz])
  AC_MSG_NOTICE([Using system libz library])
else
  AC_CHECK_LIB([z],[uncompress],[],[
    AC_MSG_ERROR([libz is required])])
fi

if `$PKG_CONFIG --exists liblzma`; then
  flags=`$PKG_CONFIG --cflags-only-I liblzma`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L liblzma` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l liblzma` $LIBS"
  have_lzma=yes
  AC_DEFINE([HAVE_LIBLZMA],[1],[Have liblzma])
  AC_MSG_NOTICE([Using system liblzma library])
else
  AC_CHECK_LIB([lzma],[lzma_code],[],[
    AC_MSG_ERROR([liblzma is required])])
fi

# sort out all the ffmpeg stuff
FFM_CPPFLAGS=''
FFM_LDFLAGS=''
FFM_LIBS=''

use_system_ffmpeg=no
if test $checkSystemFFMPEG -eq 1
then

# Some things can make you quite sweary, like people changing the names
# of packages between releases.  Grrr!

# First we check for packages without the "-ffmpeg" extension

if `$PKG_CONFIG --exists 'libavcodec >= 56.26.100'`; then
  flags=`$PKG_CONFIG --cflags-only-I libavcodec`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  FFM_CPPFLAGS="$flags $FFM_CPPFLAGS"
  FFM_LDFLAGS="`$PKG_CONFIG --libs-only-L libavcodec` $FFM_LDFLAGS"
  FFM_LIBS="`$PKG_CONFIG --libs-only-l libavcodec` $FFM_LIBS"
  have_libavcodec=yes
fi

if `$PKG_CONFIG --exists 'libavformat >= 56.25.101'`; then
  flags=`$PKG_CONFIG --cflags-only-I libavformat`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  FFM_CPPFLAGS="$flags $FFM_CPPFLAGS"
  FFM_LDFLAGS="`$PKG_CONFIG --libs-only-L libavformat` $FFM_LDFLAGS"
  FFM_LIBS="`$PKG_CONFIG --libs-only-l libavformat` $FFM_LIBS"
  have_libavformat=yes
fi

if `$PKG_CONFIG --exists 'libavutil >= 54.20.100'`; then
  flags=`$PKG_CONFIG --cflags-only-I libavutil`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  FFM_CPPFLAGS="$flags $FFM_CPPFLAGS"
  FFM_LDFLAGS="`$PKG_CONFIG --libs-only-L libavutil` $FFM_LDFLAGS"
  FFM_LIBS="`$PKG_CONFIG --libs-only-l libavutil` $FFM_LIBS"
  have_libavutil=yes
fi

if test "$have_libavcodec$have_libavformat$have_libavutil" != "yesyesyes"
then
  if `$PKG_CONFIG --exists 'libavcodec-ffmpeg >= 56.26.100'`; then
    flags=`$PKG_CONFIG --cflags-only-I libavcodec-ffmpeg`
    MOC_CFLAGS="$flags $MOC_CFLAGS"
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    FFM_CPPFLAGS="$flags $FFM_CPPFLAGS"
    FFM_LDFLAGS="`$PKG_CONFIG --libs-only-L libavcodec-ffmpeg` $FFM_LDFLAGS"
    FFM_LIBS="`$PKG_CONFIG --libs-only-l libavcodec-ffmpeg` $FFM_LIBS"
    have_libavcodec=yes
  else
    AC_MSG_WARN([No suitable version of system libavcodec found.])
  fi

  if `$PKG_CONFIG --exists 'libavformat-ffmpeg >= 56.25.101'`; then
    flags=`$PKG_CONFIG --cflags-only-I libavformat-ffmpeg`
    MOC_CFLAGS="$flags $MOC_CFLAGS"
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    FFM_CPPFLAGS="$flags $FFM_CPPFLAGS"
    FFM_LDFLAGS="`$PKG_CONFIG --libs-only-L libavformat-ffmpeg` $FFM_LDFLAGS"
    FFM_LIBS="`$PKG_CONFIG --libs-only-l libavformat-ffmpeg` $FFM_LIBS"
    have_libavformat=yes
  else
    AC_MSG_WARN([No suitable version of system libavcodec found.])
  fi

  if `$PKG_CONFIG --exists 'libavutil-ffmpeg >= 54.20.100'`; then
    flags=`$PKG_CONFIG --cflags-only-I libavutil-ffmpeg`
    MOC_CFLAGS="$flags $MOC_CFLAGS"
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    FFM_CPPFLAGS="$flags $FFM_CPPFLAGS"
    FFM_LDFLAGS="`$PKG_CONFIG --libs-only-L libavutil-ffmpeg` $FFM_LDFLAGS"
    FFM_LIBS="`$PKG_CONFIG --libs-only-l libavutil-ffmpeg` $FFM_LIBS"
    have_libavutil=yes
  else
    AC_MSG_WARN([No suitable version of system libavutil found.])
  fi
fi

if test "$have_libavcodec$have_libavformat$have_libavutil" == "yesyesyes"
then
  FFMPEG_CFLAGS="$FFM_CPPFLAGS"
  FFMPEG_LIBS="$FFM_LIBS"
  FFMPEG_DIRS=""
  use_system_ffmpeg=yes
  AC_MSG_NOTICE([Using system FFmpeg libraries])
fi

fi

use_system_libusb=no
if test $checkSystemLibusb -eq 1
then
  if `$PKG_CONFIG --exists 'libusb-1.0 >= 1.0.21'`; then
    flags=`$PKG_CONFIG --cflags-only-I libusb-1.0`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBUSB_CFLAGS="$flags"
    LIBUSB_LDFLAGS="`$PKG_CONFIG --libs-only-L libusb-1.0`"
    LIBUSB_LIBS="`$PKG_CONFIG --libs-only-l libusb-1.0`"
    use_system_libusb=yes
    AC_DEFINE([HAVE_LIBUSB],[1],[Have libusb-1.0 library])
    AC_MSG_NOTICE([Using system libusb-1.0 library])
  else
  # commented out because we can't be sure of the version
  #  AC_CHECK_LIB([usb],[libusb_init],[
  #    LIBS="-lusb-1.0 $LIBS"
  #    AC_DEFINE([HAVE_LIBUSB],[1],[Have libusb-1.0 library])
  #  ],[
      AC_MSG_WARN([No suitable version of system libusb library found.  Using internal version.])
  # ])
    build_libusb=yes
  fi
fi

use_system_dc1394=no
if test $checkSystemDC1394 -eq 1
then
  if `$PKG_CONFIG --exists 'libdc1394-2 >= 2.2.4'`; then
    AC_MSG_NOTICE([Using system libdc1394 library])
    flags=`$PKG_CONFIG --cflags-only-I libdc1394-2`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBDC1394_CFLAGS="$flags"
    # can't use shared dc1394 if we're not using the system libusb because
    # it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBDC1394_LDFLAGS="`$PKG_CONFIG --libs-only-L libdc1394-2`"
      LIBDC1394_LIBS="`$PKG_CONFIG --libs-only-l libdc1394-2`"
      use_system_dc1394=yes
    else
      AC_MSG_NOTICE([Using static libdc1394 to avoid system libusb])
      LIBDC1394_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libdc1394-2`"
      LIBDC1394_LIBS="$mylibdir/libdc1394.a $mylibdir/libraw1394.a"
    fi
    AC_DEFINE([HAVE_LIBDC1394],[1],[Have dc1394 library])
  else
  # commented out because we can't be sure of the version
  #  AC_CHECK_LIB([dc1394],[dc1394_new],[
  #    LIBS="-ldc1394 $LIBS"
  #    AC_DEFINE([HAVE_LIBDC1394],[1],[Have dc1394 library])
  #  ],[
      AC_MSG_WARN([No suitable version of system dc1394 library found.  Using internal version.])
  # ])
  fi
fi

use_system_libuvc=no
if test $checkSystemLibuvc -eq 1
then
  if `$PKG_CONFIG --exists 'libuvc >= 0.0.5'`; then
    AC_MSG_NOTICE([Using system libuvc library])
    flags=`$PKG_CONFIG --cflags-only-I libuvc`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBUVC_CFLAGS="$flags"
    # can't use shared libuvc if we're not using the system libusb because
    # it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBUVC_LDFLAGS="`$PKG_CONFIG --libs-only-L libuvc`"
      LIBUVC_LIBS="`$PKG_CONFIG --libs-only-l libuvc`"
    else
      AC_MSG_NOTICE([Using static libuvc to avoid system libusb])
      LIBUVC_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libuvc`"
      LIBUVC_LIBS="$mylibdir/libuvc.a"
    fi
    use_system_libuvc=yes
    AC_DEFINE([HAVE_LIBUVC],[1],[Have libuvc library])
  else
  # commented out because we can't be sure of the version
  #  AC_CHECK_LIB([uvc],[uvc_init],[
  #    LIBS="-luvc $LIBS"
  #    AC_DEFINE([HAVE_LIBUVC],[1],[Have libuvc library])
  #  ],[
      AC_MSG_WARN([No suitable version of system libuvc library found.  Using internal version.])
      AC_DEFINE([HAVE_LIBUVC],[1],[Have libuvc library])
  # ])
  fi
fi

use_system_libhidapi=no
if test $checkSystemLibhidapi -eq 1
then
  if `$PKG_CONFIG --exists 'hidapi-libusb >= 0.8.0'`; then
    AC_MSG_NOTICE([Using system libhidapi-libusb library])
    flags=`$PKG_CONFIG --cflags-only-I hidapi-libusb`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBHIDAPI_CFLAGS="$flags"
    # can't use shared hidapi-libusb if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBHIDAPI_LDFLAGS="`$PKG_CONFIG --libs-only-L hidapi-libusb`"
      LIBHIDAPI_LIBS="`$PKG_CONFIG --libs-only-l hidapi-libusb`"
    else
      AC_MSG_NOTICE([Using static hidapi-libusb to avoid system libusb])
      LIBHIDAPI_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir hidapi-libusb`"
      LIBHIDAPI_LIBS="$mylibdir/libhidapi-libusb.a"
    fi
    have_libhidapi=yes
    use_system_libhidapi=yes
    AC_DEFINE([HAVE_LIBHIDAPI],[1],[Have libhidapi-libusb library])
  else
  # commented out because we can't be sure of the version
  #  AC_CHECK_LIB([hidapi-libusb],[hid_open],[
  #    LIBS="-lhidapi-libusb $LIBS"
  #    AC_DEFINE([HAVE_LIBHIDAPI],[1],[Have libhidapi-libusb library])
  #    have_libhidapi=yes
  #  ],[
      AC_MSG_WARN([No suitable version of system libhidapi-libusb library found.  Using internal version.])
  # ])
  fi
fi

if `$PKG_CONFIG --exists libftdi1`; then
  flags=`$PKG_CONFIG --cflags-only-I libftdi1`
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  LIBFTDI_CFLAGS="$flags"
  # can't use shared libftdi1 if we're not using the system libusb
  # because it will try to pull it in anyhow
  if test "x$use_system_libusb" == "xyes"; then
    LIBFTDI_LDFLAGS="`$PKG_CONFIG --libs-only-L libftdi1`"
    LIBFTDI_LIBS="`$PKG_CONFIG --libs-only-l libftdi1`"
  else
    AC_MSG_NOTICE([Using static libftdi1 to avoid system libusb])
    LIBFTDI_LDFLAGS=""
    mylibdir="`$PKG_CONFIG --variable=libdir libftdi1`"
    LIBFTDI_LIBS="$mylibdir/libftdi1.a"
  fi
  have_ftdi1=yes
  AC_DEFINE([HAVE_LIBFTDI],[1],[Have ftdi1 library])
else
  if test "x$use_system_libusb" == "xyes"; then
    AC_CHECK_LIB([ftdi1],[ftdi_new],[
      LIBFTDI_LIBS="-lftdi1"
      AC_DEFINE([HAVE_LIBFTDI],[1],[Have ftdi1 library])
      have_ftdi1=yes
    ],[
      AC_CHECK_LIB([ftdi],[ftdi_new],[
        LIBFTDI_LIBS="-lftdi"
        AC_DEFINE([HAVE_LIBFTDI],[1],[Have ftdi1 library])
        have_ftdi1=yes
      ],[
        AC_MSG_WARN([No ftdi1 library found.  Xagyl filter wheels can not be supported using libftdi1])])])
  else
    AC_MSG_WARN([No ftdi1 library found.  Xagyl filter wheels can not be supported using libftdi1])
  fi
fi

use_system_libasicamera=no
use_system_libasicamera2=no
if test $checkSystemLibasicamera -eq 1
then
  if `$PKG_CONFIG --exists libasicamera`; then
    AC_MSG_NOTICE([Using system libasicamera library])
    flags=`$PKG_CONFIG --cflags-only-I libasicamera`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBASI_CFLAGS="$flags"
    # can't use shared libasicamera if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBASI_LDFLAGS="`$PKG_CONFIG --libs-only-L libasicamera`"
      LIBASI_LIBS="`$PKG_CONFIG --libs-only-l libasicamera`"
    else
      AC_MSG_NOTICE([Using static libasicamera to avoid system libusb])
      LIBASI_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libasicamera`"
      LIBASI_LIBS="$mylibdir/libASICamera.a"
    fi
    have_libasicamera=yes
    use_system_libasicamera=yes
    AC_DEFINE([HAVE_LIBASI],[1],[Have libasicamera])
  else
    AC_CHECK_LIB([ASICamera],[getNumberOfConnectedCameras],[
      LIBS="-lASICamera $LIBS"
      AC_DEFINE([HAVE_LIBASI],[1],[Have libasicamera])
      have_libasicamera=yes
    ],[
      AC_MSG_WARN([libasicamera not found.  Attempting to use libasicamera2.])
    ])
  fi
  if `$PKG_CONFIG --exists libasicamera2`; then
    AC_MSG_NOTICE([Using system libasicamera2 library])
    flags=`$PKG_CONFIG --cflags-only-I libasicamera2`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBASI_CFLAGS="$flags"
    # can't use shared libasicamera2 if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBASI_LDFLAGS="`$PKG_CONFIG --libs-only-L libasicamera2`"
      LIBASI2_LIBS="`$PKG_CONFIG --libs-only-l libasicamera2`"
    else
      AC_MSG_NOTICE([Using static libasicamera2 to avoid system libusb])
      LIBASI_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libasicamera2`"
      LIBASI2_LIBS="$mylibdir/libASICamera2.a"
    fi
    have_libasicamera=yes
    use_system_libasicamera2=yes
    AC_DEFINE([HAVE_LIBASI2],[1],[Have libasicamera2])
  else
    AC_CHECK_LIB([ASICamera2],[ASIGetNumOfConnectedCameras],[
      LIBS="-lASICamera2 $LIBS"
      AC_DEFINE([HAVE_LIBASI2],[1],[Have libasicamera2])
      have_libasicamera=yes
    ],[
      AC_MSG_WARN([No suitable version of system libasicamera2 found.  Using internal version.])
    ])
  fi
fi

AC_CHECK_LIB([m],[exp10],[
  AC_DEFINE([HAVE_EXP10],[1],[Have exp10 function in libm])
],[
  AC_MSG_WARN([No exp10() function found.  Using internal version])])

flycaplib="-lflycapture-c "
AC_CHECK_LIB([dl],[dlopen],[
  LIBS="-ldl $LIBS"
  AC_DEFINE([HAVE_LIBDL],[1],[Have dl library])
  have_dl=yes
  flycaplib=""
],[
  AC_MSG_WARN([No dl library found.  No big deal])])

AC_CHECK_LIB([flycapture-c],[fc2CreateGigEContext],[
  LIBS="$flycaplib$LIBS"
  AC_DEFINE([HAVE_LIBFLYCAPTURE2],[1],[Have flycapture2-c library])
  have_flycapture2=yes
],[
  AC_MSG_WARN([No flycapture2 library found.  Point Grey Gig-E cameras will not be supported])])

if test "x$have_dl" == "xyes" -a "x$have_flycapture2" == "xyes"
then
  LDFLAGS="-rdynamic $LDFLAGS"
fi

# Checks for header files.
AC_CHECK_HEADERS([errno.h fcntl.h float.h limits.h math.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h fitsio.h cfitsio/fitsio.h ftdi.h libftdi/ftdi.h libftdi1/ftdi.h sys/sendfile.h sys/time.h syslog.h])

FW_CORE_MEDIA=''
FW_VID_TBOX=''
AC_CHECK_HEADERS([CoreMedia/CoreMedia.h],[FW_CORE_MEDIA="-framework CoreMedia"])
AC_CHECK_HEADERS([VideoToolbox/VideoToolbox.h],[FW_VID_TBOX="-framework VideoToolbox"])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd gettimeofday select sqrt])
AC_CHECK_FUNCS([bzero memset memcpy])
AC_CHECK_FUNCS([fopen64 fseeko64 ftello64 stat64 freopen64])
AC_CHECK_FUNCS([fseeki64 ftelli64])
AC_CHECK_FUNCS([clock_gettime mkdir pow strcasecmp strchr strcspn strdup])
AC_CHECK_FUNCS([strerror strncasecmp strndup strrchr strstr strtoul])

AC_CHECK_DECLS([ASI_AUTO_MAX_EXP_MS],[],[],[#include <ASICamera2.h>])

if test "$use_system_ffmpeg" == "no"
then

# A whole pile of ugly stuff to make sure we get the FFmpeg we want
FFMPEG_CFLAGS="-I \$(top_srcdir)/ext/ffmpeg"

FFMPEG_LIBS="\$(top_builddir)/ext/ffmpeg/libavformat/libavformat.a \
               \$(top_builddir)/ext/ffmpeg/libavcodec/libavcodec.a \
               \$(top_builddir)/ext/ffmpeg/libavutil/libavutil.a"

AC_ARG_WITH(ffmpeg-extra-configure, 
  AC_HELP_STRING([--with-ffmpeg-extra-configure="xxx"],
  [extra configure options for internal ffmpeg ./configure script]),,
  with_ffmpeg_extra_configure=no)

embffmpeg_configure_args="--prefix=$prefix"

embffmpeg_configure_args="$embffmpeg_configure_args --enable-memalign-hack --enable-static --disable-debug --disable-programs --disable-network --disable-encoders --enable-encoder=utvideo --enable-encoder=rawvideo --disable-doc --disable-muxers --enable-muxer=avi --enable-muxer=mov --disable-bsfs --disable-indevs --disable-protocols --enable-protocol=file --disable-iconv --disable-vaapi --disable-vda --disable-decoders --disable-demuxers --disable-filters --disable-devices"

if test "$enable_shared" = "yes"; then
  embffmpeg_configure_args="$embffmpeg_configure_args --enable-shared"
else
  embffmpeg_configure_args="$embffmpeg_configure_args --disable-shared"
fi

if test "x$cross_compiling" = xyes; then
  embffmpeg_configure_args="$embffmpeg_configure_args --enable-cross-compile \
      --target-os=$host_os --arch=$host_cpu --cross-prefix=$host_alias-"
fi

FFMPEG_OPTS="`$srcdir/ext/ffmpeg/configure --help`"
can_disable=`echo "$FFMPEG_OPTS" | grep 'disable-ffmpeg'`
if test "$can_disable" != ""; then
  embffmpeg_configure_args="$embffmpeg_configure_args --disable-ffmpeg"
fi

if test "x$with_ffmpeg_extra_configure" != "xno"; then
  embffmpeg_configure_args="$embffmpeg_configure_args $with_ffmpeg_extra_configure"
fi

AC_CONFIG_COMMANDS([configure-embedded-ffmpeg],
  [echo "Configuring included FFmpeg instance with args $embffmpeg_configure_args"
   origdir=`pwd`
   dnl Don't put path on the configure call when not needed, as FFmpeg's configure relies on it
   dnl to detect out-of-tree builds
   if test -z "$srcdir" -o "$srcdir" = .; then
     confcmd=./configure
   else
     confcmd="$origdir"/"$ac_top_srcdir"/ext/ffmpeg/configure
   fi

   AS_MKDIR_P(["$ac_top_build_prefix"ext/ffmpeg])
   cd "$ac_top_build_prefix"ext/ffmpeg &&
       $confcmd $embffmpeg_configure_args ||
       AC_MSG_ERROR([Failed to configure embedded FFmpeg tree])
   cd "$origdir"
  ],
  [embffmpeg_configure_args="$embffmpeg_configure_args"])
AC_MSG_NOTICE([Using included FFmpeg code])

FFMPEG_SUBDIRS=ffmpeg

fi

AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LIBS)
AC_SUBST(FFMPEG_SUBDIRS)

# Same play again, but this time for libusb-1.0

if test "$use_system_libusb" == "no" -o "$build_libusb" == "yes"
then

  LIBUSB_CFLAGS="-I \$(top_srcdir)/ext/libusb/libusb"
  LIBUSB_LDFLAGS=""
  LIBUSB_LIBS="\$(top_builddir)/ext/libusb/libusb/libusb-1.0.la"

  AC_ARG_WITH(libusb-extra-configure, 
    AC_HELP_STRING([--with-libusb-extra-configure="xxx"],
    [extra configure options for internal libusb ./configure script]),,
    with_libusb_extra_configure=no)

  emblibusb_configure_args="--prefix=$prefix"

  if test "$enable_shared" = "yes"; then
    emblibusb_configure_args="$emblibusb_configure_args --enable-shared --enable-examples-build"
  else
    emblibusb_configure_args="$emblibusb_configure_args --disable-shared --enable-examples-build"
  fi

  if test "x$cross_compiling" = xyes; then
    emblibusb_configure_args="$emblibusb_configure_args --enable-cross-compile \
        --target-os=$host_os --arch=$host_cpu --cross-prefix=$host_alias-"
  fi

  if test "x$with_libusb_extra_configure" != "xno"; then
    emblibusb_configure_args="$emblibusb_configure_args $with_libusb_extra_configure"
  fi

  AC_CONFIG_COMMANDS([configure-embedded-libusb],
    [echo "Configuring included libusb-1.0 instance with args $emblibusb_configure_args"
     origdir=`pwd`
     dnl Don't put path on the configure call when not needed
     if test -z "$srcdir" -o "$srcdir" = .; then
       confcmd=./configure
     else
       confcmd="$origdir"/"$ac_top_srcdir"/ext/libusb/configure
     fi

     AS_MKDIR_P(["$ac_top_build_prefix"ext/libusb])
     cd "$ac_top_build_prefix"ext/libusb &&
         $confcmd $emblibusb_configure_args ||
         AC_MSG_ERROR([Failed to configure embedded libusb-1.0 tree])
     cd "$origdir"
    ],
    [emblibusb_configure_args="$emblibusb_configure_args"])
  AC_MSG_NOTICE([Using included libusb-1.0 code])
fi

AC_SUBST(LIBUSB_CFLAGS)
AC_SUBST(LIBUSB_LDFLAGS)
AC_SUBST(LIBUSB_LIBS)

# And again, for libhidapi this time

if test "$use_system_libhidapi" == "no"
then
  LIBHIDAPI_CFLAGS="-I \$(top_srcdir)/ext/libhidapi/hidapi"
  LIBHIDAPI_LIBS="\$(top_builddir)/ext/libhidapi/libusb/libhidapi-libusb.la"

  if test "$use_system_libusb" == "no"; then
    export libusb_CFLAGS="-I "`pwd`"/ext/libusb/libusb"
    export libusb_LDFLAGS=""
    export libusb_LIBS=`pwd`"/ext/libusb/libusb/libusb-1.0.la"
  fi

  AC_ARG_WITH(libhidapi-extra-configure,
    AC_HELP_STRING([--with-libhidapi-extra-configure="xxx"],
    [extra configure options for internal libhidapi ./configure script]),,
    with_libhidapi_extra_configure=no)

  emblibhidapi_configure_args="--prefix=$prefix"

  if test "$enable_shared" = "yes"; then
    emblibhidapi_configure_args="$emblibhidapi_configure_args --enable-shared"
  else
    emblibhidapi_configure_args="$emblibhidapi_configure_args --disable-shared"
  fi

  if test "x$cross_compiling" = xyes; then
    emblibhidapi_configure_args="$emblibhidapi_configure_args --enable-cross-compile \
        --target-os=$host_os --arch=$host_cpu --cross-prefix=$host_alias-"
  fi

  if test "x$with_libhidapi_extra_configure" != "xno"; then
    emblibhidapi_configure_args="$emblibhidapi_configure_args $with_libhidapi_extra_configure"
  fi


  AC_CONFIG_COMMANDS([configure-embedded-libhidapi],
    [echo "Configuring included libhidapi instance with args $emblibhidapi_configure_args"
     origdir=`pwd`
     dnl Don't put path on the configure call when not needed
     if test -z "$srcdir" -o "$srcdir" = .; then
       confcmd=./configure
     else
       confcmd="$origdir"/"$ac_top_srcdir"/ext/libhidapi/configure
     fi

     AS_MKDIR_P(["$ac_top_build_prefix"ext/libhidapi])
    cd "$ac_top_build_prefix"ext/libhidapi && ./bootstrap &&
         $confcmd $emblibhidapi_configure_args ||
         AC_MSG_ERROR([Failed to configure embedded libhidapi-1.0 tree])
     cd "$origdir"
    ],
    [emblibhidapi_configure_args="$emblibhidapi_configure_args"])
  AC_MSG_NOTICE([Using included libhidapi code])
fi

AC_SUBST(LIBHIDAPI_CFLAGS)
AC_SUBST(LIBHIDAPI_LIBS)
AC_SUBST(LIBHIDAPI_SUBDIRS)

# And once more, for libdc1394

if test "$use_system_dc1394" == "no"
then
  LIBDC1394_CFLAGS="-I \$(top_srcdir)/ext/libdc1394"
  LIBDC1394_LIBS="\$(top_builddir)/ext/libdc1394/dc1394/libdc1394.la"

  if test "$use_system_libusb" == "no"; then
    export LIBUSB_CFLAGS="-I "`pwd`"/ext/libusb/libusb"
    export LIBUSB_LDFLAGS=""
    export LIBUSB_LIBS=`pwd`"/ext/libusb/libusb/libusb-1.0.la"
  fi

  AC_ARG_WITH(libdc1394-extra-configure,
    AC_HELP_STRING([--with-libdc1394-extra-configure="xxx"],
    [extra configure options for internal libdc1394 ./configure script]),,
    with_libdc1394_extra_configure=no)

  emblibdc1394_configure_args="--prefix=$prefix --disable-shared --disable-sdltest --disable-examples"

  if test "$enable_shared" = "yes"; then
    emblibdc1394_configure_args="$emblibdc1394_configure_args --enable-shared"
  else
    emblibdc1394_configure_args="$emblibdc1394_configure_args --disable-shared"
  fi

  if test "x$cross_compiling" = xyes; then
    emblibdc1394_configure_args="$emblibdc1394_configure_args --enable-cross-compile \
        --target-os=$host_os --arch=$host_cpu --cross-prefix=$host_alias-"
  fi

  if test "x$with_libdc1394_extra_configure" != "xno"; then
    emblibdc1394_configure_args="$emblibdc1394_configure_args $with_libdc1394_extra_configure"
  fi


  AC_CONFIG_COMMANDS([configure-embedded-libdc1394],
    [echo "Configuring included libdc1394 instance with args $emblibdc1394_configure_args"
     origdir=`pwd`
     dnl Don't put path on the configure call when not needed
     if test -z "$srcdir" -o "$srcdir" = .; then
       confcmd=./configure
     else
       confcmd="$origdir"/"$ac_top_srcdir"/ext/libdc1394/configure
     fi

     AS_MKDIR_P(["$ac_top_build_prefix"ext/libdc1394])
    cd "$ac_top_build_prefix"ext/libdc1394 &&
         $confcmd $emblibdc1394_configure_args ||
         AC_MSG_ERROR([Failed to configure embedded libdc1394 tree])
     cd "$origdir"
    ],
    [emblibdc1394_configure_args="$emblibdc1394_configure_args"])
  AC_DEFINE([HAVE_LIBDC1394],[1],[Using included libdc1394 library])

fi

AC_SUBST(LIBDC1394_CFLAGS)
AC_SUBST(LIBDC1394_LIBS)
AC_SUBST(LIBDC1394_SUBDIRS)

LIBWINDIB_CFLAGS="-I \$(top_srcdir)/ext/libwindib"
LIBWINDIB_LIBS="\$(top_builddir)/ext/libwindib/libwindib.la"
AC_SUBST(LIBWINDIB_CFLAGS)
AC_SUBST(LIBWINDIB_LIBS)
MOC_CFLAGS="$LIBWINDIB_CFLAGS $MOC_CFLAGS"

AC_CANONICAL_HOST

OSX_FRAMEWORKS=""
OSX_WARNINGS=""
FXLOAD_CFLAGS="-DFXLOAD_PATH=\\\"\$(bindir)/fxload\\\" -DFIRMWARE_QHY_PATH=\\\"/lib/firmware/qhy\\\""

APP_PATH_CFLAGS=""
DEFAULT_PATH_CFLAGS=""
if test "x$use_system_libasicamera" == "xno"; then
  if test -f $(top_builddir)/ext/libASICamera/libASICamera.a
  then
    LIBASI_CFLAGS="-DHAVE_LIBASI=1"
    DYNLIB_EXTN_CFLAGS="-DDYNLIB_EXT_SO"
    LIBASI_LIBS="\$(top_builddir)/ext/libASICamera/libASICamera.a"
  fi
fi
if test "x$use_system_libasicamera2" == "xno"; then
  if test -f $(top_builddir)/ext/libASICamera/libASICamera2.a
  then
    LIBASI2_CFLAGS="-DHAVE_LIBASI2=1"
    DYNLIB_EXTN_CFLAGS="-DDYNLIB_EXT_SO"
    LIBASI2_LIBS="\$(top_builddir)/ext/libASICamera/libASICamera2.a"
  fi
fi

libudev_default=yes
# work around broken autoconf
ext1=ext; ext2=ext; ext3=ext; ext4=ext; ext5=ext; ext6=ext
ext7=ext; ext8=ext; ext9=ext; extA=ext; extB=ext; extC=ext
extD=ext; extE=ext; extF=ext; extG=ext; extH=ext; extI=ext
extJ=ext; extK=ext; extL=ext; extM=ext
case $host in
  x86_64-apple-*)
    AC_CONFIG_LINKS([$ext7/libASICamera/libASICamera2.a:ext/libASICamera/mac/libASICamera2.a])
    AC_CONFIG_LINKS([$extH/libtoupcam/libtoupcam.dylib:ext/libtoupcam/mac64/libtoupcam.dylib])
    AC_CONFIG_LINKS([$extI/libaltaircam/libaltaircam.dylib:ext/libaltaircam/mac64/libaltaircam.dylib])
    OSX_FRAMEWORKS="-framework Carbon -framework CoreVideo -framework IOKit $FW_CORE_MEDIA $FW_VID_TBOX"
    FXLOAD_CFLAGS="-DFXLOAD_PATH=\\\"/MacOS/fxload\\\" -DFIRMWARE_QHY_PATH=\\\"/Resources/firmware/qhy\\\""
    LIBHIDAPI_LIBS="\$(top_builddir)/ext/libhidapi/mac/libhidapi.la"
    have_toupcam=yes
    have_mallincam=yes
    have_altaircam=yes
    APP_PATH_CFLAGS="-DUSE_APP_PATH=1"
    DEFAULT_PATH_CFLAGS="-DUSE_HOME_DEFAULT=1"
    DYNLIB_EXTN_CFLAGS="-DDYNLIB_EXT_DYLIB"
    OSX_WARNINGS="-Qunused-arguments"
    AC_DEFINE([USB_OVERFLOW_HANGS],[1],[USB overflow causes hang])
    ;;
  i?86-apple-*)
    AC_CONFIG_LINKS([$ext8/libASICamera/libASICamera2.a:ext/libASICamera/mac/libASICamera2.a])
    OSX_FRAMEWORKS="-framework Carbon -framework CoreVideo -framework IOKit $FW_CORE_MEDIA $FW_VID_TBOX"
    FXLOAD_CFLAGS="-DFXLOAD_PATH=\\\"/MacOS/fxload\\\" -DFIRMWARE_QHY_PATH=\\\"/Resources/firmware/qhy\\\""
    LIBHIDAPI_LIBS="\$(top_builddir)/ext/libhidapi/mac/libhidapi.la"
    APP_PATH_CFLAGS="-DUSE_APP_PATH=1"
    DEFAULT_PATH_CFLAGS="-DUSE_HOME_DEFAULT=1"
    DYNLIB_EXTN_CFLAGS="-DDYNLIB_EXT_DYLIB"
    OSX_WARNINGS="-Qunused-arguments"
    AC_DEFINE([USB_OVERFLOW_HANGS],[1],[USB overflow causes hang])
    ;;
  i?86-*-*)
    AC_CONFIG_LINKS([$ext9/libASICamera/libASICamera2.a:ext/libASICamera/x86/libASICamera2.a])
    have_toupcam=yes
    have_mallincam=yes
    have_altaircam=yes
    ;;
  x86_64-*-*)
    AC_CONFIG_LINKS([$extA/libASICamera/libASICamera2.a:ext/libASICamera/x64/libASICamera2.a])
    ;;
  armv7l-*-*)
    AC_CONFIG_LINKS([$extE/libASICamera/libASICamera2.a:ext/libASICamera/armv7/libASICamera2.a])
    have_toupcam=yes
    have_mallincam=yes
    have_altaircam=yes
    ;;
  armv6l-*-*)
    AC_CONFIG_LINKS([$extB/libASICamera/libASICamera2.a:ext/libASICamera/armv6/libASICamera2.a])
    ;;
  armv5l-*-*)
    AC_CONFIG_LINKS([$extC/libASICamera/libASICamera2.a:ext/libASICamera/armv5/libASICamera2.a])
    ;;
  *)
    if test "x$use_system_libasicamera" == "xno"; then
      LIBASI_LIBS=""
      LIBASI_CFLAGS="-DHAVE_LIBASI=0"
      AC_MSG_NOTICE([No compatible ASI library for architecture $host.])
    fi
    if test "x$use_system_libasicamera2" == "xno"; then
      LIBASI2_LIBS=""
      LIBASI2_CFLAGS="-DHAVE_LIBASI2=0"
      AC_MSG_NOTICE([No compatible ASI2 library for architecture $host.  Support for ASI cameras not included])
    fi
    ;;
esac

if test "x$use_system_libuvc" == "xno"; then
  LIBUVC_CFLAGS="-I \$(top_srcdir)/ext/libuvc/include"
  LIBUVC_LIBS="\$(top_builddir)/ext/libuvc/src/libuvc.la"
  AC_DEFINE([HAVE_LIBUVC],[1],[Have libuvc library])
  AC_MSG_NOTICE([Using included libuvc])
  internal_uvc=yes
fi

AC_ARG_WITH(libudev,
  AC_HELP_STRING([--with-libudev],[build with libudev]),have_libudev=$with_libudev,with_libudev=$libudev_default)

if test "x$with_libudev" != "xno"; then
  with_libudev=no
  AC_CHECK_LIB([udev],[udev_new],[
    LIBS="-ludev $LIBS"
    AC_DEFINE([HAVE_LIBUDEV],[1],[Have udev library])
    have_udev=yes
  ],[
    AC_MSG_WARN([No udev library found.  Xagyl filter wheels can not be supported using libudev])])
fi

## Deal with Toupcam libtoupcam library
#
#AC_ARG_WITH(toupcam_cflags,
#  AC_HELP_STRING([--with-toupcam-cflags="xxx"],
#  [CFLAGS for toupcam library]),,
#  with_toupcam_cflags=no)
#AC_ARG_WITH(toupcam_ldflags,
#  AC_HELP_STRING([--with-toupcam-ldflags="xxx"],
#  [LDFLAGS for toupcam library]),,
#  with_toupcam_ldflags=no)

#LIBTOUPCAM_LDFLAGS="-L \$(top_srcdir)/ext/libtoupcam"
#if test "x$with_toupcam_cflags" != "xno"
#then
#  LIBTOUPCAM_CFLAGS="$with_toupcam_cflags"
#fi
#if test "x$with_toupcam_ldflags" != "xno"
#then
#  LIBTOUPCAM_LDFLAGS="$with_toupcam_ldflags"
#fi

#oldLIBS=$LIBS
#LIBS="$LIBTOUPCAM_LDFLAGS $LIBS"
#AC_CHECK_LIB([toupcam],[Toupcam_Enum],[
#  LIBS="-lv4l2 $LIBS"
if test "x$have_toupcam" != "xno"
then
  LIBTOUPCAM_CFLAGS="-I \$(top_srcdir)/ext/libtoupcam/include"
  AC_DEFINE([HAVE_LIBTOUPCAM],[1],[Have Toupcam library])
  AC_DEFINE([HAVE_LIBMALLINCAM],[1],[Have Mallincam library])
  AC_DEFINE([HAVE_LIBALTAIRCAM],[1],[Have Altair camera library])
#],[
#  AC_MSG_WARN([No toupcam library found.  Touptek cameras will not be supported])])
fi
#LIBS=$oldLIBS
AC_SUBST(LIBTOUPCAM_CFLAGS)
#AC_SUBST(LIBTOUPCAM_LDFLAGS)

AC_SUBST(OSX_FRAMEWORKS)
AC_SUBST(OSX_WARNINGS)
AC_SUBST(FXLOAD_CFLAGS)
AC_SUBST(FXLOAD_CFLAGS)
AC_SUBST(LIBUVC_CFLAGS)
AC_SUBST(LIBUVC_LIBS)
AC_SUBST(LIBASI_CFLAGS)
AC_SUBST(LIBASI_LIBS)
AC_SUBST(LIBASI2_CFLAGS)
AC_SUBST(LIBASI2_LIBS)
AC_SUBST(LIBFTDI_CFLAGS)
AC_SUBST(LIBFTDI_LIBS)
AC_SUBST(APP_PATH_CFLAGS)
AC_SUBST(DEFAULT_PATH_CFLAGS)
AC_SUBST(DYNLIB_EXTN_CFLAGS)
AC_SUBST(MOC_CFLAGS)

AM_CONDITIONAL([LIBASI_COND], [test "x$LIBASI_LIBS" != "x"])
AM_CONDITIONAL([LIBASI2_COND], [test "x$LIBASI2_LIBS" != "x"])
AM_CONDITIONAL([LIBASI_COMMON_COND], [test "x$LIBASI_LIBS$LIBASI2_LIBS" != "x"])
AM_CONDITIONAL([LIBV4L2_COND], [test "x$have_v4l2" == "xyes"])
AM_CONDITIONAL([LIBDC1394_COND], [test "x$use_system_dc1394" == "xno"])
AM_CONDITIONAL([LIBUDEV_COND], [test "x$have_udev" == "xyes"])
AM_CONDITIONAL([LIBFTDI_COND], [test "x$have_ftdi1" == "xyes"])
AM_CONDITIONAL([ATIK_COND], [test "x$have_ftdi1" == "xyes" -o "x$have_udev" == "xyes"])
AM_CONDITIONAL([LIBFLYCAPTURE2_COND], [test "x$have_flycapture2" == "xyes"])
AM_CONDITIONAL([LIBDL_COND], [test "x$have_dl" == "xyes"])
AM_CONDITIONAL([LIBTOUPCAM_COND], [test "x$have_toupcam" == "xyes"])
AM_CONDITIONAL([LIBMALLINCAM_COND], [test "x$have_mallincam" == "xyes"])
AM_CONDITIONAL([LIBALTAIRCAM_COND], [test "x$have_altaircam" == "xyes"])
AM_CONDITIONAL([LIBFFMPEG_COND], [test "x$use_system_ffmpeg" == "xno"])
AM_CONDITIONAL([LIBUSB_COND], [test "x$use_system_libusb" == "xno"])
AM_CONDITIONAL([BUILD_LIBUSB_COND], [test "x$build_libusb" == "xyes"])
AM_CONDITIONAL([LIBHIDAPI_COND], [test "x$use_system_libhidapi" == "xno"])
AM_CONDITIONAL([INT_LIBUVC_COND], [test "x$internal_uvc" == "xyes"])

AC_CONFIG_FILES([Makefile liboautil/Makefile liboacam/Makefile liboademosaic/Makefile liboaSER/Makefile liboavideo/Makefile liboafilterwheel/Makefile liboaimgproc/Makefile liboaPTR/Makefile oacapture/Makefile oacapture/icons/Makefile oacapture/desktop/Makefile oacapture/translations/Makefile ext/Makefile ext/libuvc/Makefile ext/libuvc/src/Makefile ext/libwindib/Makefile oalive/Makefile oalive/icons/Makefile oalive/desktop/Makefile udev/Makefile lib/Makefile lib/firmware/Makefile lib/firmware/qhy/Makefile])
AC_OUTPUT
