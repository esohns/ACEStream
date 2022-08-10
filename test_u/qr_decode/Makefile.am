#  -I/usr/include/c++/3.4.6/backward
#  -I$(top_srcdir)/modules/ATCD/ACE
AM_CPPFLAGS = \
  -I$(top_srcdir)/../ATCD/ACE
#  $(PKGSDL_CFLAGS) \
#  $(PKGGTK_CFLAGS)
if VALGRIND_SUPPORT
AM_CPPFLAGS += \
  $(PKGVALGRIND_CFLAGS)
endif

DEFS = \
  -DHAVE_CONFIG_H
 # -DBASEDIR=\"$(datadir)/@PACKAGE_NAME@\"
if DEBUG
DEFS += \
  -DDEBUG_DEBUGGER
endif

bin_PROGRAMS = \
  capability_wrapper

noinst_HEADERS = stdafx.h
capability_wrapper_SOURCES = \
  test_i.cpp \
  stdafx.cpp \
  stdafx.h

#  -I$(top_srcdir)/modules/libCommon/src \
#  -I$(top_srcdir)/modules/libCommon/src/ui
capability_wrapper_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(srcdir)/../../src \
  -I$(srcdir)/..
capability_wrapper_LDFLAGS = \
  -L$(abs_builddir)/../../src/.libs
capability_wrapper_LDADD = \
  -lcap
#  -lACE \
#  $(PKGGTK_LIBS) \
#  -lCommon
#  -lCommon_UI \
#  -lACEStream \
#  -lACEStream_File \
#  -lACEStream_Misc

