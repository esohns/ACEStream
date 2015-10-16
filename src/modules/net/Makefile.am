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
  stream_module_net_common.h \
  stream_module_io.h \
  stream_module_io.inl \
  stream_module_io_stream.h \
  stream_module_io_stream.inl \
  stream_module_source.h \
  stream_module_source.inl \
  stream_module_target.h \
  stream_module_target.inl

lib_LTLIBRARIES = libACEStream_Net.la

libACEStream_Net_la_SOURCES = \
  stream_module_net_common.h \
  stream_module_io.cpp \
  stream_module_io.h \
  stream_module_io.inl \
  stream_module_io_stream.cpp \
  stream_module_io_stream.h \
  stream_module_io_stream.inl \
  stream_module_source.cpp \
  stream_module_source.h \
  stream_module_source.inl \
  stream_module_target.cpp \
  stream_module_target.h \
  stream_module_target.inl
#  stdafx.cpp \
#  stdafx.h
libACEStream_Net_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../.. \
  -I$(srcdir)/../misc \
  -I$(srcdir)/modules/libACENetwork/src \
  -I$(srcdir)/modules/libACENetwork/src/client_server
libACEStream_Net_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

SUBDIRS = \
  modules \
  .

DIST_SUBDIRS = \
  .
if BUILD_MODULES
  DIST_SUBDIRS += modules
endif

