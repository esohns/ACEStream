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
  stream_module_filereader.h \
  stream_module_filereader.inl \
  stream_module_filewriter.h \
  stream_module_filewriter.inl

lib_LTLIBRARIES = libACEStream_File.la

libACEStream_File_la_SOURCES = \
  stream_module_filereader.cpp \
  stream_module_filereader.h \
  stream_module_filereader.inl \
  stream_module_filewriter.cpp \
  stream_module_filewriter.h \
  stream_module_filewriter.inl
#  stdafx.cpp \
#  stdafx.h

libACEStream_File_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../..
libACEStream_File_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

