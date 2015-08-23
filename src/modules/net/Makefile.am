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
  stream_module_input_stream.h \
  stream_module_input_stream.inl \
  stream_module_net_common.h \
  stream_module_output_stream.h \
  stream_module_output_stream.inl \
  stream_module_tcpsource.h \
  stream_module_tcpsource.inl \
  stream_module_tcptarget.h \
  stream_module_tcptarget.inl

lib_LTLIBRARIES = libACEStream_Net.la

libACEStream_Net_la_SOURCES = \
  stream_module_input_stream.cpp \
  stream_module_input_stream.h \
  stream_module_input_stream.inl \
  stream_module_net_common.h \
  stream_module_output_stream.cpp \
  stream_module_output_stream.h \
  stream_module_output_stream.inl \
  stream_module_tcpsource.cpp \
  stream_module_tcpsource.h \
  stream_module_tcpsource.inl \
  stream_module_tcptarget.cpp \
  stream_module_tcptarget.h \
  stream_module_tcptarget.inl
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

