ACLOCAL_AMFLAGS = \
  -I m4

DEFS += \
  -DSTREAM_BUILD_DLL=1 \
  -DPACKAGE=\"@PACKAGE_NAME@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ATCD/ACE \
  $(PKGXML2_CFLAGS)
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

noinst_HEADERS = \
  stdafx.h
include_HEADERS = \
  stream_module_htmlparser.h \
  stream_module_htmlparser.inl \
  stream_module_htmlwriter.h \
  stream_module_htmlwriter.inl \
  stream_module_xpathquery.h \
  stream_module_xpathquery.inl

lib_LTLIBRARIES = libACEStream_HTML.la

libACEStream_HTML_la_SOURCES = \
  stream_module_htmlparser.cpp \
  stream_module_htmlparser.h \
  stream_module_htmlparser.inl \
  stream_module_htmlwriter.cpp \
  stream_module_htmlwriter.h \
  stream_module_htmlwriter.inl \
  stream_module_xpathquery.cpp \
  stream_module_xpathquery.h \
  stream_module_xpathquery.inl
#  stdafx.cpp \
#  stdafx.h

libACEStream_HTML_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../..
libACEStream_HTML_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@
