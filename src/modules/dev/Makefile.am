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
  stream_dev_exports.h \
  stream_dev_common.h \
  stream_dev_defines.h \
  stream_dev_tools.h \
  stream_dev_cam_source.h \
  stream_dev_cam_source.inl \
  stream_dev_cam_source_directshow.h \
  stream_dev_cam_source_directshow.inl

lib_LTLIBRARIES = libACEStream_Device.la

libACEStream_Device_la_SOURCES = \
  stream_dev_exports.h \
  stream_dev_common.h \
  stream_dev_defines.h \
  stream_dev_tools.cpp \
  stream_dev_tools.h \
  stream_dev_cam_source.cpp \
  stream_dev_cam_source.h \
  stream_dev_cam_source.inl \
  stream_dev_cam_source_directshow.cpp \
  stream_dev_cam_source_directshow.h \
  stream_dev_cam_source_directshow.inl
#  stdafx.cpp \
#  stdafx.h

libACEStream_Device_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../..
libACEStream_Device_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

