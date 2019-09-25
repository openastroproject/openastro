#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([openastro], [1.7.0], [james@openastroproject.org])
AC_CONFIG_SRCDIR([liboademosaic/oademosaic.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign subdir-objects])
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
checkSystemLibefwfilter=0
checkSystemLibtoupcam=0
checkSystemLibaltaircam=0
checkSystemLibaltaircamlegacy=0
checkSystemLibstarshootg=0
checkSystemLibnncam=0
checkSystemLibqhyccd=0
useisystemInclude=0
build_libusb=no
zwoSupportedArch=yes
case $host in
  *-linux-*)
    checkQtForPIC=1
    checkSystemFFMPEG=1
    checkSystemDC1394=1
    checkSystemLibuvc=1
    checkSystemLibhidapi=1
    checkSystemLibasicamera=1
    checkSystemLibefwfilter=1
    checkSystemLibtoupcam=1
    checkSystemLibqhyccd=1
    checkSystemLibaltaircam=1
    checkSystemLibaltaircamlegacy=1
    checkSystemLibstarshootg=1
    checkSystemLibnncam=1
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
have_spinnaker=no
have_dl=no
have_toupcam=yes
have_mallincam=yes
have_altaircam=yes
have_altaircamlegacy=yes
have_starshootg=yes
have_nncam=yes
have_qhyccd=yes
have_libavformat=no
have_libavutil=no
have_libavcodec=no
internal_uvc=no
have_libhidapi=no
have_libgphoto2=no

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
  AC_DEFINE([HAVE_LIBPNG],[1],[Have libpng])
  AC_MSG_NOTICE([Using system libpng library])
else
  AC_CHECK_LIB([png],[png_sig_cmp],[],[
    AC_MSG_ERROR([libpng is required])])
fi

if `$PKG_CONFIG --exists libjpeg`; then
  flags=`$PKG_CONFIG --cflags-only-I libjpeg`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L libjpeg` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l libjpeg` $LIBS"
  AC_DEFINE([HAVE_LIBJPG],[1],[Have libjpeg])
  AC_MSG_NOTICE([Using system libjpeg library])
else
  AC_CHECK_LIB([jpeg],[jpeg_create_decompress],[],[
    AC_MSG_ERROR([libjpeg is required])])
fi

if `$PKG_CONFIG --exists libraw`; then
  flags=`$PKG_CONFIG --cflags-only-I libraw`
  MOC_CFLAGS="$flags $MOC_CFLAGS"
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  CPPFLAGS="$flags $CPPFLAGS"
  LDFLAGS="`$PKG_CONFIG --libs-only-L libraw` $LDFLAGS"
  LIBS="`$PKG_CONFIG --libs-only-l libraw` $LIBS"
  AC_DEFINE([HAVE_LIBRAW],[1],[Have libraw])
  AC_MSG_NOTICE([Using system libraw library])
else
  AC_CHECK_LIB([raw],[libraw_open_buffer],[],[
    AC_MSG_ERROR([libraw is required])])
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

if `$PKG_CONFIG --exists 'libavcodec >= 57.0.0'`; then
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

if `$PKG_CONFIG --exists 'libavformat >= 57.0.0'`; then
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
  if `$PKG_CONFIG --exists 'libavcodec-ffmpeg >= 57.0.0'`; then
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

  if `$PKG_CONFIG --exists 'libavformat-ffmpeg >= 57.0.0'`; then
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

if test $use_system_ffmpeg = "no"
then
  AC_DEFINE([INTERNAL_FFMPEG],[1],[Using internal FFmpeg libraries])
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
    LIBUSB_CFLAGS_SUBDIR="$flags"
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
else
  # we'll always have this, because we use a local copy
  AC_DEFINE([HAVE_LIBUSB],[1],[Have libusb-1.0 library])
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
			# LIBDC1394_LDFLAGS="`$PKG_CONFIG --libs-only-L libdc1394-2`"
			# LIBDC1394_LIBS="`$PKG_CONFIG --libs-only-l libdc1394-2`"
			:
      use_system_dc1394=yes
    else
      AC_MSG_NOTICE([Using static libdc1394 to avoid system libusb])
      LIBDC1394_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libdc1394-2`"
			AC_DEFINE([HAVE_STATIC_LIBDC1394],[1],[Using static libdc1394])
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
    AC_MSG_NOTICE([Using system libuvc library for build])
    flags=`$PKG_CONFIG --cflags-only-I libuvc`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBUVC_CFLAGS="$flags"
    # can't use shared libuvc if we're not using the system libusb because
    # it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
		# skipping this because libuvc is now dynamically loaded
		#  LIBUVC_LDFLAGS="`$PKG_CONFIG --libs-only-L libuvc`"
		#  LIBUVC_LIBS="`$PKG_CONFIG --libs-only-l libuvc`"
			:
    else
      AC_MSG_NOTICE([Using static libuvc to avoid system libusb])
      LIBUVC_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libuvc`"
			AC_DEFINE([HAVE_STATIC_LIBUVC],[1],[Using static libuvc])
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
			AC_DEFINE([HAVE_STATIC_LIBUVC],[1],[Using static libuvc])
  # ])
  fi
else
  # we'll always have this, because we use a local copy
  AC_DEFINE([HAVE_LIBUVC],[1],[Have libuvc library])
	AC_DEFINE([HAVE_STATIC_LIBUVC],[1],[Using static libuvc])
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
else
  # we'll always have this, because we use a local copy
  AC_DEFINE([HAVE_LIBHIDAPI],[1],[Have libhidapi-libusb library])
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

use_system_libasicamera2=no
if test $checkSystemLibasicamera -eq 1
then
  if `$PKG_CONFIG --exists libasicamera2`; then
    AC_MSG_NOTICE([Using system libASICamera2 library])
    flags=`$PKG_CONFIG --cflags-only-I libasicamera2`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBASI2_CFLAGS="$flags"
    # can't use shared libASICamera2 if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
			# LIBASI2_LDFLAGS="`$PKG_CONFIG --libs-only-L libasicamera2`"
			# LIBASI2_LIBS="`$PKG_CONFIG --libs-only-l libasicamera2`"
			use_system_libasicamera2=yes
    else
      AC_MSG_NOTICE([Using static libASICamera2 to avoid system libusb])
      LIBASI2_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libasicamera2`"
			AC_DEFINE([HAVE_STATIC_LIBASICAMERA2],[1],[Using static libASICamera2])
      LIBASI2_LIBS="$mylibdir/libASICamera2.a"
    fi
    AC_DEFINE([HAVE_LIBASI2],[1],[Have libASICamera2])
  else
    AC_CHECK_LIB([ASICamera2],[ASIGetNumOfConnectedCameras],[
      LIBS="-lASICamera2 $LIBS"
      AC_DEFINE([HAVE_LIBASI2],[1],[Have libASICamera2])
    ],[
      AC_DEFINE([HAVE_LIBASI2],[1],[Have libASICamera2])
			AC_DEFINE([HAVE_STATIC_LIBASICAMERA2],[1],[Using static libASICamera2])
      AC_MSG_WARN([No suitable version of system libASICamera2 found.  Using internal version.])
    ])
  fi
fi

use_system_libefwfilter=no
if test $checkSystemLibefwfilter -eq 1
then
  if `$PKG_CONFIG --exists libefwfilter`; then
    AC_MSG_NOTICE([Using system libEFWFilter library])
    flags=`$PKG_CONFIG --cflags-only-I libefwfilter`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBZWOFW_CFLAGS="$flags"
    # can't use shared libEFWFilter if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
			# LIBZWOFW_LDFLAGS="`$PKG_CONFIG --libs-only-L libefwfilter`"
			# LIBZWOFW_LIBS="`$PKG_CONFIG --libs-only-l libefwfilter`"
			use_system_libefwfilter=yes
    else
      AC_MSG_NOTICE([Using static libEFWFilter to avoid system libusb])
      LIBZWOFW_LDFLAGS=""
      mylibdir="`$PKG_CONFIG --variable=libdir libefwfilter`"
			AC_DEFINE([HAVE_STATIC_LIBEFWFILTER],[1],[Using static libEFWFilter])
      LIBZWOFW_LIBS="$mylibdir/libEFWFilter.a"
    fi
    AC_DEFINE([HAVE_LIBZWOFW],[1],[Have libEFWFilter])
  else
    AC_CHECK_LIB([EFWFilter],[EFWGetNum],[
      LIBS="-lEFWFilter $LIBS"
      AC_DEFINE([HAVE_LIBZWOFW],[1],[Have libEFWFilter])
    ],[
			AC_DEFINE([HAVE_LIBZWOFW],[1],[Have libEFWFilter])
			AC_DEFINE([HAVE_STATIC_LIBEFWFILTER],[1],[Using static libEFWFilter])
      AC_MSG_WARN([No suitable version of system libEFWFilter found. Using internal version.])
    ])
  fi
fi

use_system_libtoupcam=no
use_system_libmallincam=no
if test $checkSystemLibtoupcam -eq 1
then
  if `$PKG_CONFIG --exists libtoupcam`; then
    AC_MSG_NOTICE([Using system libtoupcam library])
    flags=`$PKG_CONFIG --cflags-only-I libtoupcam`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBTOUPCAM_CFLAGS="$flags"
    # can't use shared libtoupcam if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBTOUPCAM_LDFLAGS="`$PKG_CONFIG --libs-only-L libtoupcam`"
      LIBTOUPCAM_LIBS="`$PKG_CONFIG --libs-only-l libtoupcam`"
    else
      AC_MSG_WARN([libtoupcam not compatible with internal libusb.  No support for Touptek cameras.])
    fi
    use_system_libtoupcam=yes
    AC_DEFINE([HAVE_LIBTOUPCAM],[1],[Have libtoupcam])
  else
    AC_CHECK_LIB([toupcam],[Toupcam_Open],[
      LIBS="-ltoupcam $LIBS"
      AC_DEFINE([HAVE_LIBTOUPCAM],[1],[Have libtoupcam])
    ],[
      AC_MSG_WARN([libtoupcam not found.  Dynamic support for Touptek cameras.])
    ])
  fi
fi

use_system_libaltaircam=no
if test $checkSystemLibaltaircam -eq 1
then
  if `$PKG_CONFIG --exists libaltaircam`; then
    AC_MSG_NOTICE([Using system libaltaircam library])
    flags=`$PKG_CONFIG --cflags-only-I libaltaircam`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBALTAIRCAM_CFLAGS="$flags"
    # can't use shared libaltaircam if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBALTAIRCAM_LDFLAGS="`$PKG_CONFIG --libs-only-L libaltaircam`"
      LIBALTAIRCAM_LIBS="`$PKG_CONFIG --libs-only-l libaltaircam`"
    else
      AC_MSG_WARN([libaltaircam not compatible with internal libusb.  No support for Altair cameras.])
    fi
    use_system_libaltaircam=yes
    AC_DEFINE([HAVE_LIBALTAIRCAM],[1],[Have libaltaircam])
  else
    AC_CHECK_LIB([altaircam],[Altaircam_Open],[
      LIBS="-laltaircam $LIBS"
      AC_DEFINE([HAVE_LIBALTAIRCAM],[1],[Have libaltaircam])
    ],[
      AC_MSG_WARN([libaltaircam not found.  Dynamic support for Altair cameras.])
    ])
  fi
fi

use_system_libaltaircamlegacy=no
if test $checkSystemLibaltaircamlegacy -eq 1
then
  if `$PKG_CONFIG --exists libaltaircamlegacy`; then
    AC_MSG_NOTICE([Using system libaltaircamlegacy library])
    flags=`$PKG_CONFIG --cflags-only-I libaltaircamlegacy`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBALTAIRCAM_LEGACY_CFLAGS="$flags"
    # can't use shared libaltaircamlegacy if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBALTAIRCAM_LEGACY_LDFLAGS="`$PKG_CONFIG --libs-only-L libaltaircamlegacy`"
      LIBALTAIRCAM_LEGACY_LIBS="`$PKG_CONFIG --libs-only-l libaltaircamlegacy`"
    else
      AC_MSG_WARN([libaltaircamlegacy not compatible with internal libusb.  No support for legacy Altair cameras.])
    fi
    use_system_libaltaircamlegacy=yes
    AC_DEFINE([HAVE_LIBALTAIRCAM_LEGACY],[1],[Have libaltaircamlegacy])
  else
    AC_CHECK_LIB([altaircamlegacy],[Altaircam_Open],[
      LIBS="-laltaircamlegacy $LIBS"
      AC_DEFINE([HAVE_LIBALTAIRCAM_LEGACY],[1],[Have libaltaircamlegacy])
    ],[
      AC_MSG_WARN([libaltaircamlegacy not found.  Dynamic support for legacy Altair cameras.])
    ])
  fi
fi

use_system_libstarshootg=no
if test $checkSystemLibstarshootg -eq 1
then
  if `$PKG_CONFIG --exists libstarshootg`; then
    AC_MSG_NOTICE([Using system libstarshootg library])
    flags=`$PKG_CONFIG --cflags-only-I libstarshootg`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBSTARSHOOTG_CFLAGS="$flags"
    # can't use shared libstarshootg if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBSTARSHOOTG_LDFLAGS="`$PKG_CONFIG --libs-only-L libstarshootg`"
      LIBSTARSHOOTG_LIBS="`$PKG_CONFIG --libs-only-l libstarshootg`"
    else
      AC_MSG_WARN([libstarshootg not compatible with internal libusb.  No support for Starshoot G cameras.])
    fi
    use_system_libstarshootg=yes
    AC_DEFINE([HAVE_LIBSTARSHOOTG],[1],[Have libstarshootg])
  else
    AC_CHECK_LIB([starshootg],[Starshootg_Open],[
      LIBS="-lstarshootg $LIBS"
      AC_DEFINE([HAVE_LIBSTARSHOOTG],[1],[Have libstarshootg])
    ],[
      AC_MSG_WARN([libstarshootg not found.  Dynamic support for Starshoot G cameras.])
    ])
  fi
fi

use_system_libnncam=no
if test $checkSystemLibnncam -eq 1
then
  if `$PKG_CONFIG --exists libnncam`; then
    AC_MSG_NOTICE([Using system libnncam library])
    flags=`$PKG_CONFIG --cflags-only-I libnncam`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBNNCAM_CFLAGS="$flags"
    # can't use shared libnncam if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBNNCAM_LDFLAGS="`$PKG_CONFIG --libs-only-L libnncam`"
      LIBNNCAM_LIBS="`$PKG_CONFIG --libs-only-l libnncam`"
    else
      AC_MSG_WARN([libnncam not compatible with internal libusb.  No support for Risingcam/Levenhuk cameras.])
    fi
    use_system_libnncam=yes
    AC_DEFINE([HAVE_LIBNNCAM],[1],[Have libnncam])
  else
    AC_CHECK_LIB([nncam],[Nncam_Open],[
      LIBS="-lnncam $LIBS"
      AC_DEFINE([HAVE_LIBNNCAM],[1],[Have libnncam])
    ],[
      AC_MSG_WARN([libnncam not found.  Dynamic support for Risingcam/Levenhuk cameras.])
    ])
  fi
fi

use_system_libqhyccd=no
if test $checkSystemLibqhyccd -eq 1
then
  if `$PKG_CONFIG --exists libqhyccd`; then
    AC_MSG_NOTICE([Using system libqhyccd library])
    flags=`$PKG_CONFIG --cflags-only-I libqhyccd`
    if test $useisystemInclude -eq 1; then
      flags=`echo "$flags" | sed 's/^-I/-isystem/'`
    fi
    LIBQHYCCD_CFLAGS="$flags"
    # can't use shared libqhyccd if we're not using the system libusb
    # because it will try to pull it in anyhow
    if test "x$use_system_libusb" == "xyes"; then
      LIBQHYCCD_LDFLAGS="`$PKG_CONFIG --libs-only-L libqhyccd`"
      LIBQHYCCD_LIBS="`$PKG_CONFIG --libs-only-l libqhyccd`"
    else
      AC_MSG_WARN([libqhyccd not compatible with internal libusb.  No support for QHY cameras using libqhyccd.])
    fi
    use_system_libqhyccd=yes
    AC_DEFINE([HAVE_LIBQHYCCD],[1],[Have libqhyccd])
  else
    AC_CHECK_LIB([qhyccd],[InitQHYCCDResource],[
      LIBS="-lqhyccd $LIBS"
      AC_DEFINE([HAVE_LIBQHYCCD],[1],[Have libqhyccd])
    ],[
      AC_MSG_WARN([libqhyccd not found.  Dynamic support for QHY cameras using libqhyccd.])
    ])
  fi
fi

AC_CHECK_LIB([m],[exp10],[
  AC_DEFINE([HAVE_EXP10],[1],[Have exp10 function in libm])
],[
  AC_MSG_WARN([No exp10() function found.  Using internal version])])

AC_CHECK_LIB([dl],[dlopen],[
  LIBS="-ldl $LIBS"
  AC_DEFINE([HAVE_LIBDL],[1],[Have dl library])
  have_dl=yes
],[
  AC_MSG_WARN([No dl library found.  No big deal])])

AC_CHECK_HEADERS([flycapture/C/FlyCapture2_C.h],[
  AC_DEFINE([HAVE_LIBFLYCAPTURE2],[1],[Have flycapture2-c library])
  have_flycapture2=yes
],[
  AC_MSG_WARN([flycapture2 headers not found.  Point Grey Gig-E cameras will not be supported via this library])])

if test "x$have_dl" == "xyes" -a "x$have_flycapture2" == "xyes"
then
  LDFLAGS="-rdynamic $LDFLAGS"
fi

spinnakerlib="-lSpinnaker_C "
AC_CHECK_LIB([Spinnaker_C],[spinSystemGetInstance],[
  LIBS="$spinnakerlib$LIBS"
  AC_DEFINE([HAVE_LIBSPINNAKER],[1],[Have Spinnaker_C library])
  have_spinnaker=yes
],[
  AC_MSG_WARN([No Spinnaker library found.  Point Grey cameras will not be supported via this library])])

if test "x$have_dl" == "xyes" -a "x$have_flycapture2" == "xno" -a "x$have_spinnaker" == "xyes"
then
  LDFLAGS="-rdynamic $LDFLAGS"
fi

if `$PKG_CONFIG --exists libgphoto2`; then
  AC_MSG_NOTICE([Using system libqpgoto2 library])
  flags=`$PKG_CONFIG --cflags-only-I libgphoto2`
  if test $useisystemInclude -eq 1; then
    flags=`echo "$flags" | sed 's/^-I/-isystem/'`
  fi
  LIBGPHOTO2_CFLAGS="$flags"
  # can't use shared libgphoto2 if we're not using the system libusb
  # because it will try to pull it in anyhow
  if test "x$use_system_libusb" == "xno"; then
    AC_MSG_NOTICE([Using static libgphoto2 to avoid system libusb])
    LIBGPHOTO2_LDFLAGS=""
    AC_DEFINE([HAVE_STATIC_LIBGPHOTO2],[1],[Using static libgphoto2])
    LIBGPHOTO2_LIBS="-static -lgphoto2 -dynamic"
  fi
  AC_DEFINE([HAVE_LIBGPHOTO2],[1],[Have libgphoto2])
	have_libgphoto2=yes
else
  AC_CHECK_LIB([gphoto2],[gp_camera_new],[
    LIBS="-lgphoto2 $LIBS"
		have_libgphoto2=yes
    AC_DEFINE([HAVE_LIBGPHOTO2],[1],[Have libgphoto2])
  ],[
    AC_MSG_WARN([libgphoto2 not found.  DSLR support disabled.])
  ])
fi

# Checks for header files.
AC_CHECK_HEADERS([errno.h fcntl.h float.h limits.h math.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h fitsio.h cfitsio/fitsio.h ftdi.h libftdi/ftdi.h libftdi1/ftdi.h sys/sendfile.h sys/time.h syslog.h stdbool.h])
# C++ versions
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([cerrno cfloat climits cmath cstdint cstdlib cstring cstrings])
AC_LANG_POP

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
AC_CHECK_FUNCS([creat64 open64 lseek64])
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

embffmpeg_configure_args="$embffmpeg_configure_args --enable-memalign-hack --enable-static --disable-debug --disable-programs --disable-network --disable-encoders --enable-encoder=utvideo --enable-encoder=rawvideo --enable-encoder=qtrle --disable-doc --disable-muxers --enable-muxer=avi --enable-muxer=mov --disable-bsfs --disable-indevs --disable-protocols --enable-protocol=file --disable-iconv --disable-vaapi --disable-vda --disable-decoders --disable-demuxers --disable-filters --disable-devices --disable-doc"

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

  LIBUSB_CFLAGS="-I \$(top_srcdir)/ext/libusb"
  LIBUSB_CFLAGS_SUBDIR="-I \$(top_srcdir)/ext/libusb/libusb"
  LIBUSB_LDFLAGS=""
  LIBUSB_LIBS="\$(top_builddir)/ext/libusb/libusb/libusb-1.0.la"

  # Should be done with AS_LN_S in all probability, but I can't find
  # any documentation
  if ! test -e "$ac_top_build_prefix"ext/libusb/libusb-1.0
  then
    ln -s ./libusb "$ac_top_build_prefix"ext/libusb/libusb-1.0
  fi

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
AC_SUBST(LIBUSB_CFLAGS_SUBDIR)
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

# save_LIBUSB_CFLAGS="$LIBUSB_CFLAGS"
# save_LIBUSB_LDFLAGS="$LIBUSB_LDFLAGS"
# save_LIBUSB_LIBS="$LIBUSB_LIBS"

# if test "$use_system_libusb" == "no"; then
#   export LIBUSB_CFLAGS="-I "`pwd`"/ext/libusb/libusb"
#   export LIBUSB_LDFLAGS=""
#   export LIBUSB_LIBS=`pwd`"/ext/libusb/libusb/libusb-1.0.la"
# fi

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
     if test "$use_system_libusb" == "no"; then
       export LIBUSB_CFLAGS="-I $origdir/ext/libusb/libusb"
       export LIBUSB_LDFLAGS=""
       export LIBUSB_LIBS="$origdir/ext/libusb/libusb/libusb-1.0.la"
		 fi
     cd "$ac_top_build_prefix"ext/libdc1394 &&
         $confcmd $emblibdc1394_configure_args ||
         AC_MSG_ERROR([Failed to configure embedded libdc1394 tree])
     cd "$origdir"
    ],
    [emblibdc1394_configure_args="$emblibdc1394_configure_args"])
  AC_DEFINE([HAVE_LIBDC1394],[1],[Using included libdc1394 library])

# LIBUSB_CFLAGS="$save_LIBUSB_CFLAGS"
# LIBUSB_LDFLAGS="$save_LIBUSB_LDFLAGS"
# LIBUSB_LIBS="$save_LIBUSB_LIBS"
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
SHLIBS_CFLAGS=""

APP_PATH_CFLAGS=""
DEFAULT_PATH_CFLAGS=""
DYNLIB_EXTN_CFLAGS="-DDYNLIB_EXT_SO"

libudev_default=yes
# work around broken autoconf
ext1=ext; ext2=ext; ext3=ext; ext4=ext; ext5=ext; ext6=ext
ext7=ext; ext8=ext; ext9=ext; extA=ext; extB=ext; extC=ext
extD=ext; extE=ext; extF=ext; extG=ext; extH=ext; extI=ext
extJ=ext; extK=ext; extL=ext; extM=ext
case $host in
  *-apple-*)
    AC_CONFIG_LINKS([$ext1/libASICamera/libASICamera2.a:ext/libASICamera/mac/libASICamera2.a])
    AC_CONFIG_LINKS([$ext2/libtoupcam/libtoupcam.dylib:ext/libtoupcam/mac/libtoupcam.dylib])
    AC_CONFIG_LINKS([$ext3/libaltaircam/libaltaircam.dylib:ext/libaltaircam/mac64/libaltaircam.dylib])
    AC_CONFIG_LINKS([$ext4/libEFWFilter/libEFWFilter.a:ext/libEFWFilter/mac/libEFWFilter.a])
    AC_CONFIG_LINKS([$ext5/libaltaircamlegacy/libaltaircamlegacy.dylib:ext/libaltaircamlegacy/mac64/libaltaircamlegacy.dylib])
    AC_CONFIG_LINKS([$ext6/libqhyccd/libqhyccd.dylib:ext/libqhyccd/mac64/libqhyccd.dylib])
    AC_CONFIG_LINKS([$ext7/libstarshootg/libstarshootg.dylib:ext/libstarshootg/mac/libstarshootg.dylib])
    AC_CONFIG_LINKS([$ext7/libnncam/libnncam.dylib:ext/libnncam/mac/libnncam.dylib])
    OSX_FRAMEWORKS="-framework Carbon -framework VideoDecodeAcceleration -framework CoreVideo -framework IOKit $FW_CORE_MEDIA $FW_VID_TBOX"
    FXLOAD_CFLAGS="-DFXLOAD_PATH=\\\"/MacOS/fxload\\\" -DFIRMWARE_QHY_PATH=\\\"/Resources/firmware/qhy\\\""
		SHLIBS_CFLAGS="-DSHLIB_PATH=\\\"/MacOS/\\\""
    LIBHIDAPI_LIBS="\$(top_builddir)/ext/libhidapi/mac/libhidapi.la"
    APP_PATH_CFLAGS="-DUSE_APP_PATH=1"
    DEFAULT_PATH_CFLAGS="-DUSE_HOME_DEFAULT=1"
    DYNLIB_EXTN_CFLAGS="-DDYNLIB_EXT_DYLIB"
    OSX_WARNINGS="-Qunused-arguments"
    AC_DEFINE([USB_OVERFLOW_HANGS],[1],[USB overflow causes hang])
    ;;
  i?86-*-*)
    AC_CONFIG_LINKS([$ext7/libASICamera/libASICamera2.a:ext/libASICamera/x86/libASICamera2.a])
    AC_CONFIG_LINKS([$ext8/libEFWFilter/libEFWFilter.a:ext/libEFWFilter/x86/libEFWFilter.a])
    ;;
  x86_64-*-*)
    AC_CONFIG_LINKS([$ext9/libASICamera/libASICamera2.a:ext/libASICamera/x64/libASICamera2.a])
    AC_CONFIG_LINKS([$extA/libEFWFilter/libEFWFilter.a:ext/libEFWFilter/x64/libEFWFilter.a])
    ;;
  armv8l-*-*|aarch64-*-*)
    AC_CONFIG_LINKS([$extB/libASICamera/libASICamera2.a:ext/libASICamera/armv8/libASICamera2.a])
    AC_CONFIG_LINKS([$extC/libEFWFilter/libEFWFilter.a:ext/libEFWFilter/armv8/libEFWFilter.a])
    ;;
  armv7l-*-*)
    AC_CONFIG_LINKS([$extD/libASICamera/libASICamera2.a:ext/libASICamera/armv7/libASICamera2.a])
    AC_CONFIG_LINKS([$extE/libEFWFilter/libEFWFilter.a:ext/libEFWFilter/armv7/libEFWFilter.a])
    ;;
  armv6l-*-*)
    AC_CONFIG_LINKS([$extF/libASICamera/libASICamera2.a:ext/libASICamera/armv6/libASICamera2.a])
    AC_CONFIG_LINKS([$extG/libEFWFilter/libEFWFilter.a:ext/libEFWFilter/armv6/libEFWFilter.a])
    ;;
  armv5l-*-*)
    AC_CONFIG_LINKS([$extH/libASICamera/libASICamera2.a:ext/libASICamera/armv5/libASICamera2.a])
    AC_CONFIG_LINKS([$extI/libEFWFilter/libEFWFilter.a:ext/libEFWFilter/armv5/libEFWFilter.a])
    ;;
  *)
    if test "x$use_system_libasicamera2" != "xyes" -o "x$use_system_libefwfilter" != "xyes"
    then
      zwoSupportedArch=no
      AC_MSG_NOTICE([No compatible ASI library for architecture $host.])
      LIBASI2_CFLAGS="-DHAVE_LIBASI2=0"
      AC_MSG_NOTICE([No compatible ASI2 library for architecture $host.  Support for ASI cameras not included])
      LIBZWOFW_LIBS=""
      LIBZWOFW_CFLAGS="-DHAVE_LIBZWOFW=0"
      AC_MSG_NOTICE([No compatible EFWFilter library for architecture $host.  Support for ZWO filter wheels not included])
    fi
    ;;
esac

if test "x$zwoSupportedArch" == "xyes"
then
  if test "x$use_system_libasicamera2" == "xno"; then
    AC_DEFINE([HAVE_LIBASI2],[1],[Have libASICamera2])
		AC_DEFINE([HAVE_STATIC_LIBASICAMERA2],[1],[Using static libASICamera2])
    LIBASI2_CFLAGS="-DHAVE_LIBASI2=1"
    LIBASI2_LIBS="\$(top_builddir)/ext/libASICamera/libASICamera2.a"
  fi
  if test "x$use_system_libefwfilter" == "xno"; then
    AC_DEFINE([HAVE_LIBZWOFW],[1],[Have libEFWFilter])
		AC_DEFINE([HAVE_STATIC_LIBEFWFILTER],[1],[Using static libEFWFilter])
    LIBZWOFW_CFLAGS="-DHAVE_LIBZWOFW=1"
    LIBZWOFW_LIBS="\$(top_builddir)/ext/libEFWFilter/libEFWFilter.a"
  fi
fi

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

if test "x$use_system_libtoupcam" == "xno"
then
	if test "x$have_toupcam" != "xno"
	then
		LIBTOUPCAM_CFLAGS="-I \$(top_srcdir)/ext/libtoupcam/include"
		LIBTOUPCAM_LIBS=""
		AC_DEFINE([HAVE_LIBTOUPCAM],[1],[Have Toupcam library support])
	fi
fi

if test "x$use_system_libmallincam" == "xno"
then
	if test "x$have_mallincam" != "xno"
	then
		LIBMALLINCAM_CFLAGS="-I \$(top_srcdir)/ext/libmallincam/include"
		LIBMALLINCAM_LIBS=""
		AC_DEFINE([HAVE_LIBMALLINCAM],[1],[Have Mallincam library support])
	fi
fi

if test "x$use_system_libaltaircam" == "xno"
then
	if test "x$have_altaircam" != "xno"
	then
		LIBALTAIRCAM_CFLAGS="-I \$(top_srcdir)/ext/libaltaircam/include"
		LIBALTAIRCAM_LIBS=""
		AC_DEFINE([HAVE_LIBALTAIRCAM],[1],[Have Altair camera library support])
	fi
fi

if test "x$use_system_libaltaircamlegacy" == "xno"
then
	if test "x$have_altaircamlegacy" != "xno"
	then
		LIBALTAIRCAM_LEGACY_CFLAGS="-I \$(top_srcdir)/ext/libaltaircamlegacy/include"
		LIBALTAIRCAM_LEGACY_LIBS=""
		AC_DEFINE([HAVE_LIBALTAIRCAM_LEGACY],[1],[Have legacy Altair camera library support])
	fi
fi

if test "x$use_system_libstarshootg" == "xno"
then
	if test "x$have_starshootg" != "xno"
	then
		LIBSTARSHOOTG_CFLAGS="-I \$(top_srcdir)/ext/libstarshootg/include"
		LIBSTARSHOOTG_LIBS=""
		AC_DEFINE([HAVE_LIBSTARSHOOTG],[1],[Have Starshoot G camera library support])
	fi
fi

if test "x$use_system_libnncam" == "xno"
then
	if test "x$have_nncam" != "xno"
	then
		LIBNNCAM_CFLAGS="-I \$(top_srcdir)/ext/libnncam/include"
		LIBNNCAM_LIBS=""
		AC_DEFINE([HAVE_LIBNNCAM],[1],[Have Risingcam/Levenhuk camera library support])
	fi
fi

if test "x$use_system_libqhyccd" == "xno"
then
	if test "x$have_qhyccd" != "xno"
	then
		LIBQHYCCD_CFLAGS="-I \$(top_srcdir)/ext/libqhyccd/include"
		LIBQHYCCD_LIBS=""
		AC_DEFINE([HAVE_LIBQHYCCD],[1],[Have qhyccd library support])
	fi
fi

AC_DEFINE([HAVE_LIBMALLINCAM],[1],[Have Mallincam library support])

AC_SUBST(OSX_FRAMEWORKS)
AC_SUBST(OSX_WARNINGS)
AC_SUBST(FXLOAD_CFLAGS)
AC_SUBST(FXLOAD_CFLAGS)
AC_SUBST(LIBUVC_CFLAGS)
AC_SUBST(LIBUVC_LIBS)
AC_SUBST(LIBASI2_CFLAGS)
AC_SUBST(LIBASI2_LIBS)
AC_SUBST(LIBTOUPCAM_CFLAGS)
AC_SUBST(LIBTOUPCAM_LIBS)
AC_SUBST(LIBMALLINCAM_CFLAGS)
AC_SUBST(LIBMALLINCAM_LIBS)
AC_SUBST(LIBALTAIRCAM_CFLAGS)
AC_SUBST(LIBALTAIRCAM_LIBS)
AC_SUBST(LIBALTAIRCAM_LEGACY_CFLAGS)
AC_SUBST(LIBALTAIRCAM_LEGACY_LIBS)
AC_SUBST(LIBSTARSHOOTG_CFLAGS)
AC_SUBST(LIBSTARSHOOTG_LIBS)
AC_SUBST(LIBNNCAM_CFLAGS)
AC_SUBST(LIBNNCAM_LIBS)
AC_SUBST(LIBQHYCCD_CFLAGS)
AC_SUBST(LIBQHYCCD_LIBS)
AC_SUBST(LIBGPHOTO2_CFLAGS)
AC_SUBST(LIBGPHOTO2_LIBS)
AC_SUBST(LIBZWOFW_LIBS)
AC_SUBST(LIBFTDI_CFLAGS)
AC_SUBST(LIBFTDI_LIBS)
AC_SUBST(APP_PATH_CFLAGS)
AC_SUBST(DEFAULT_PATH_CFLAGS)
AC_SUBST(DYNLIB_EXTN_CFLAGS)
AC_SUBST(MOC_CFLAGS)
AC_SUBST(SHLIBS_CFLAGS)

AM_CONDITIONAL([LIBASI2_COND], [test "x$zwoSupportedArch" == "xyes"])
AM_CONDITIONAL([LIBZWOFW_COND], [test "x$zwoSupportedArch" == "xyes"])
AM_CONDITIONAL([LIBV4L2_COND], [test "x$have_v4l2" == "xyes"])
AM_CONDITIONAL([LIBDC1394_COND], [test "x$use_system_dc1394" == "xno"])
AM_CONDITIONAL([LIBUDEV_COND], [test "x$have_udev" == "xyes"])
AM_CONDITIONAL([LIBFTDI_COND], [test "x$have_ftdi1" == "xyes"])
AM_CONDITIONAL([ATIK_COND], [test "x$have_ftdi1" == "xyes" -o "x$have_udev" == "xyes"])
AM_CONDITIONAL([LIBFLYCAPTURE2_COND], [test "x$have_flycapture2" == "xyes"])
AM_CONDITIONAL([LIBSPINNAKER_COND], [test "x$have_spinnaker" == "xyes"])
AM_CONDITIONAL([LIBDL_COND], [test "x$have_dl" == "xyes"])
AM_CONDITIONAL([LIBTOUPCAM_COND], [test "x$have_toupcam" == "xyes"])
AM_CONDITIONAL([LIBMALLINCAM_COND], [test "x$have_mallincam" == "xyes"])
AM_CONDITIONAL([LIBALTAIRCAM_COND], [test "x$have_altaircam" == "xyes"])
AM_CONDITIONAL([LIBALTAIRCAM_LEGACY_COND], [test "x$have_altaircamlegacy" == "xyes"])
AM_CONDITIONAL([LIBSTARSHOOTG_COND], [test "x$have_starshootg" == "xyes"])
AM_CONDITIONAL([LIBNNCAM_COND], [test "x$have_nncam" == "xyes"])
AM_CONDITIONAL([LIBQHYCCD_COND], [test "x$have_qhyccd" == "xyes"])
AM_CONDITIONAL([LIBGPHOTO2_COND], [test "x$have_libgphoto2" == "xyes"])
AM_CONDITIONAL([LIBFFMPEG_COND], [test "x$use_system_ffmpeg" == "xno"])
AM_CONDITIONAL([LIBUSB_COND], [test "x$use_system_libusb" == "xno"])
AM_CONDITIONAL([BUILD_LIBUSB_COND], [test "x$build_libusb" == "xyes"])
AM_CONDITIONAL([LIBHIDAPI_COND], [test "x$use_system_libhidapi" == "xno"])
AM_CONDITIONAL([INT_LIBUVC_COND], [test "x$internal_uvc" == "xyes"])

AC_CONFIG_FILES([Makefile common/Makefile liboautil/Makefile liboacam/Makefile liboacam/altair/Makefile liboacam/altair-legacy/Makefile liboacam/atik/Makefile liboacam/euvc/Makefile liboacam/iidc/Makefile liboacam/mallincam/Makefile liboacam/flycap2/Makefile liboacam/spinnaker/Makefile liboacam/pwc/Makefile liboacam/qhy/Makefile liboacam/qhyccd/Makefile liboacam/starshootg/Makefile liboacam/risingcam/Makefile liboacam/sx/Makefile liboacam/toupcam/Makefile liboacam/uvc/Makefile liboacam/v4l2/Makefile liboacam/zwo/Makefile liboacam/dummy/Makefile liboacam/gphoto2/Makefile liboademosaic/Makefile liboaSER/Makefile liboavideo/Makefile liboafilterwheel/Makefile liboafilterwheel/sx/Makefile liboafilterwheel/xagyl/Makefile liboafilterwheel/zwo/Makefile liboafilterwheel/brightstar/Makefile liboaimgproc/Makefile liboaPTR/Makefile liboaephem/Makefile oacapture/Makefile oacapture/icons/Makefile oacapture/desktop/Makefile oacapture/translations/Makefile ext/Makefile ext/libuvc/Makefile ext/libuvc/src/Makefile ext/libwindib/Makefile oalive/Makefile oalive/icons/Makefile oalive/desktop/Makefile udev/Makefile lib/Makefile lib/firmware/Makefile lib/firmware/qhy/Makefile bin/Makefile packagers/Makefile packagers/deb/Makefile packagers/deb/debfiles/Makefile packagers/rpm/Makefile osx/Makefile osx/oaCapture.iconset/Makefile osx/oalive.iconset/Makefile])
AC_OUTPUT
