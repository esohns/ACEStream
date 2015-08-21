ACLOCAL_AMFLAGS = \
  -I m4

DEFS += \
  -DSTREAM_BUILD_DLL=1 \
  -DMETA_PACKAGE=\"@PACKAGE_NAME@\"

AM_CPPFLAGS = \
  -I$(top_srcdir)/modules/ATCD/ACE
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

noinst_HEADERS = \
  stdafx.h
include_HEADERS = \
  stream_module_messagehandler.h \
  stream_module_messagehandler.inl \
  stream_module_queuereader.h \
  stream_module_queuereader.inl \
  stream_module_runtimestatistic.h \
  stream_module_runtimestatistic.inl

lib_LTLIBRARIES = libACEStream_Misc.la

libACEStream_Misc_la_SOURCES = \
  stream_module_messagehandler.cpp \
  stream_module_messagehandler.h \
  stream_module_messagehandler.inl \
  stream_module_queuereader.cpp \
  stream_module_queuereader.h \
  stream_module_queuereader.inl \
  stream_module_runtimestatistic.cpp \
  stream_module_runtimestatistic.h \
  stream_module_runtimestatistic.inl
#  stdafx.cpp \
#  stdafx.h

libACEStream_Misc_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/modules/libCommon/src \
  -I$(srcdir)/../..
libACEStream_Misc_la_LDFLAGS = \
  -version-info @LIBACESTREAM_LT_VERSION_INFO@

