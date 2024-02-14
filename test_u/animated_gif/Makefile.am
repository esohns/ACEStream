#  -I/usr/include/c++/3.4.6/backward
#  -I$(top_srcdir)/modules/ATCD/ACE
AM_CPPFLAGS = \
  -I$(top_srcdir)/../ATCD/ACE \
  $(PKGSDL_CFLAGS) \
  $(PKGGTK_CFLAGS)
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

DEFS = \
  -DHAVE_CONFIG_H \
  -DBASEDIR=\"$(datadir)/@PACKAGE_NAME@\"
if DEBUG
DEFS += \
  -DDEBUG_DEBUGGER
endif

check_PROGRAMS = \
  stream_filecopy

noinst_HEADERS = stdafx.h
stream_filecopy_SOURCES = \
  ../test_u_common.h \
  ../test_u_defines.h \
  test_u_filecopy.cpp \
  test_u_filecopy_callbacks.cpp \
  test_u_filecopy_callbacks.h \
  test_u_filecopy_common.h \
  test_u_filecopy_common_modules.h \
  test_u_filecopy_defines.h \
  test_u_filecopy_eventhandler.cpp \
  test_u_filecopy_eventhandler.h \
  test_u_filecopy_message.cpp \
  test_u_filecopy_message.h \
  test_u_filecopy_module_eventhandler.cpp \
  test_u_filecopy_module_eventhandler.h \
  test_u_filecopy_session_message.cpp \
  test_u_filecopy_session_message.h \
  test_u_filecopy_signalhandler.cpp \
  test_u_filecopy_signalhandler.h \
  test_u_filecopy_stream.cpp \
  test_u_filecopy_stream.h
#  stdafx.cpp \
#  stdafx.h

#  -I$(top_srcdir)/modules/Common/src \
#  -I$(top_srcdir)/modules/Common/src/ui
stream_filecopy_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(top_srcdir)/../Common/src \
  -I$(top_srcdir)/../Common/src/ui \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/modules/file \
  -I$(top_srcdir)/src/modules/misc \
  -I$(srcdir)/..
stream_filecopy_LDFLAGS = \
  -L$(abs_top_srcdir)/modules/ATCD/ACE/build/linux/lib \
  $(PKGGTK_LDFLAGS) \
  -L$(abs_builddir)/../../modules/Common/src/.libs \
  -L$(abs_builddir)/../../modules/Common/src/ui/.libs \
  -L$(abs_builddir)/../../src/.libs \
  -L$(abs_builddir)/../../src/modules/file/.libs \
  -L$(abs_builddir)/../../src/modules/misc/.libs
stream_filecopy_LDADD = \
  -lACE \
  $(PKGGTK_LIBS) \
  -lCommon \
  -lCommon_UI \
  -lACEStream \
  -lACEStream_File \
  -lACEStream_Misc

SUBDIRS = \
  . \
  etc

EXTRA_DIST = \
  ReadMe.txt \
  targetver.h

