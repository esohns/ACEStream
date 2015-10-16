SUBDIRS = \
  src
if BUILD_MODULES
 SUBDIRS += modules
endif
if TEST_SUPPORT
  SUBDIRS += test_u test_i
endif

DIST_SUBDIRS = \
  3rd_party \
  m4 \
  scripts \
  src \
  test_u \
  test_i
if BUILD_MODULES
  DIST_SUBDIRS += modules
endif

EXTRA_DIST = \
  autogen.sh \
  configure.gnu \
  LICENSE \
  README.md

