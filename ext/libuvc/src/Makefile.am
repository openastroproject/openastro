AM_CPPFLAGS = \
  -I$(top_srcdir)/include \
  $(LIBUSB_CFLAGS_SUBDIR) \
  $(LIBUVC_CFLAGS)

lib_LTLIBRARIES = libuvc.la

libuvc_la_SOURCES = \
  ctrl.c ctrl-gen.c device.c diag.c frame.c frame-mjpeg.c init.c misc.c \
  stream.c
