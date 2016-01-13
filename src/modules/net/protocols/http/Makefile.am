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
  stream_module_source_http_get.h \
  stream_module_source_http_get.inl

lib_LTLIBRARIES = libACEStream_Net_Protocol_HTTP_Get.la

libACEStream_Net_Protocol_HTTP_Get_la_SOURCES = \
  stream_module_source_http_get.cpp \
  stream_module_source_http_get.h \
  stream_module_source_http_get.inl
#  stdafx.cpp \
#  stdafx.h
libACEStream_Net_Protocol_HTTP_Get_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../../..
libACEStream_Net_Protocol_HTTP_Get_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

