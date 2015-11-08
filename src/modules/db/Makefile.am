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
  stream_db_exports.h \
  stream_module_db_common.h \
  stream_module_db_defines.h \
  stream_module_db_tools.h \
  stream_module_mysqlreader.h \
  stream_module_mysqlreader.inl \
  stream_module_mysqlwriter.h \
  stream_module_mysqlwriter.inl

lib_LTLIBRARIES = libACEStream_Database.la

libACEStream_Database_la_SOURCES = \
  stream_db_exports.h \
  stream_module_db_common.h \
  stream_module_db_defines.h \
  stream_module_db_tools.cpp \
  stream_module_db_tools.h \
  stream_module_mysqlreader.cpp \
  stream_module_mysqlreader.h \
  stream_module_mysqlreader.inl \
  stream_module_mysqlwriter.cpp \
  stream_module_mysqlwriter.h \
  stream_module_mysqlwriter.inl
#  stdafx.cpp \
#  stdafx.h

libACEStream_Database_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../..
libACEStream_Database_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@
