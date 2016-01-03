ACLOCAL_AMFLAGS = \
  -I m4

DEFS += \
  -DSTREAM_BUILD_DLL=1 \
  -DPACKAGE=\"@PACKAGE_NAME@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ATCD/ACE
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

noinst_HEADERS = \
  stdafx.h
include_HEADERS = \
  stream_vis_common.h \
  stream_vis_defines.h \
  stream_vis_exports.h \
  stream_vis_gtk_drawingarea.h \
  stream_vis_gtk_drawingarea.inl \
  stream_vis_tools.h

lib_LTLIBRARIES = libACEStream_Visualization.la

libACEStream_Visualization_la_SOURCES = \
  stream_vis_common.h \
  stream_vis_defines.h \
  stream_vis_exports.h \
  stream_vis_gtk_drawingarea.cpp \
  stream_vis_gtk_drawingarea.h \
  stream_vis_gtk_drawingarea.inl
  stream_vis_tools.cpp \
  stream_vis_tools.h \
#  stdafx.cpp \
#  stdafx.h

libACEStream_Visualization_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../..
libACEStream_Visualization_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

