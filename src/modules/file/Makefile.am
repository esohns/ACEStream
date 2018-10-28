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
  stream_file_defines.h \
  stream_file_exports.h \
  stream_file_source.h \
  stream_file_source.inl \
  stream_file_sink.h \
  stream_file_sink.inl

lib_LTLIBRARIES = libACEStream_File.la

libACEStream_File_la_SOURCES = \
  stream_file_defines.h \
  stream_file_exports.h \
  stream_file_source.cpp \
  stream_file_source.h \
  stream_file_source.inl \
  stream_file_sink.cpp \
  stream_file_sink.h \
  stream_file_sink.inl
#  stdafx.cpp \
#  stdafx.h

libACEStream_File_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/Common/src \
  -I$(srcdir)/../..
libACEStream_File_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

