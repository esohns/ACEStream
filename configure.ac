#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# define version information
m4_define([M4_LIBACESTREAM_VERSION_MAJOR], [0])
m4_define([M4_LIBACESTREAM_VERSION_MINOR], [0])
m4_define([M4_LIBACESTREAM_VERSION_MICRO], [1])
m4_define([M4_LIBACESTREAM_VERSION_DEVEL], [devel])
m4_define([M4_LIBACESTREAM_VERSION],
          [M4_LIBACESTREAM_VERSION_MAJOR.M4_LIBACESTREAM_VERSION_MINOR.M4_LIBACESTREAM_VERSION_MICRO[]m4_ifset([M4_LIBACESTREAM_VERSION_DEVEL],-[M4_LIBACESTREAM_VERSION_DEVEL])])
m4_define([M4_LIBACESTREAM_VERSION_API]  , [1])
dnl # Libtool versioning.
dnl m4_define([dc_version_lt_current],[0])
dnl m4_define([dc_version_lt_revision],[0])
dnl m4_define([dc_version_lt_age],[0])
AC_SUBST([LIBACESTREAM_VERSION],[M4_LIBCOMMON_VERSION])
AC_SUBST([LIBACESTREAM_VERSION_MAJOR],[M4_LIBACESTREAM_VERSION_MAJOR])
AC_SUBST([LIBACESTREAM_VERSION_MINOR],[M4_LIBACESTREAM_VERSION_MINOR])
AC_SUBST([LIBACESTREAM_VERSION_MICRO],[M4_LIBACESTREAM_VERSION_MICRO])
dnl AC_SUBST([DC_VERSION_LIBTOOL],[dc_version_lt_current:dc_version_lt_revision:dc_version_lt_age])
# Version suffix.
m4_ifset([M4_LIBACESTREAM_VERSION_DEVEL],
         [AC_DEFINE(HAVE_VERSION_DEVEL, [1], [Define if a version suffix is present.])])
AC_SUBST([LIBACESTREAM_VERSION_API],[M4_LIBACESTREAM_VERSION_API])

AC_PREREQ([2.69])
AC_INIT([libACEStream],
        [M4_LIBACESTREAM_VERSION],
        [eriksohns@123mail.org],
        [libACEStream],
        [http://www.github.com/esohns/libACEStream])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([aux_config])
AC_CONFIG_MACRO_DIR([m4])

#AC_USE_SYSTEM_EXTENSIONS

# checks for programs
#AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_AR

# init other tools
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
# build time sanity check...
AM_SANITY_CHECK
LT_INIT

# prefix config.h
m4_define([LIBACESTREAM_CONFIG_H], [libacestream_config.h])
AX_PREFIX_CONFIG_H([LIBACESTREAM_CONFIG_H], [libacestream], [config.h])

# define meta-package
META_PACKAGE_NAME="libACEStream"
AC_DEFINE([META_PACKAGE_NAME], ["libACEStream"], [meta-package-name])

#AC_LANG([C++])

## (re)set the cpp compiler flags
#CXXFLAGS="-fno-implicit-templates"
CXXFLAGS=
# --> enable debug support ?
AC_MSG_CHECKING([enable debugging])
debug_default="no"
AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug], [turn on debugging [default="${debug_default}"]])],
              [case "${enableval}" in
                 yes) debug=yes ;;
                 no)  debug=no  ;;
                 *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
               esac],
              [debug=$debug_default])
AM_CONDITIONAL([DEBUG], [test x$debug = xyes])
if test x$debug = xyes; then
  CXXFLAGS="-g -O0 -Wall -fno-inline -D_DEBUG"
  AC_MSG_RESULT([yes])
else
  CXXFLAGS="-O3 -Wall -Werror -DACE_NDEBUG"
  AC_MSG_RESULT([no])
fi

# --> enable tracing support ?
AC_MSG_CHECKING([enable tracing])
tracing_default="no"
AC_ARG_ENABLE([tracing],
              [AS_HELP_STRING([--enable-tracing], [turn on tracing [default="${tracing_default}"]])],
              [case "${enableval}" in
                 yes) tracing=yes ;;
                 no)  tracing=no  ;;
                 *) AC_MSG_ERROR([bad value ${enableval} for --enable-tracing]) ;;
               esac],
              [tracing=$tracing_default])
AM_CONDITIONAL([TRACING], [test x$tracing = xyes])
if test x$tracing = xyes; then
  CXXFLAGS="${CXXFLAGS} -DSTREAM_NTRACE=0"
  AC_MSG_RESULT([yes])
else
  CXXFLAGS="${CXXFLAGS} -DSTREAM_NTRACE=1"
  AC_MSG_RESULT([no])
fi

# --> enable valgrind support ?
AC_MSG_CHECKING([enable valgrind support])
valgrind_support_default="no"
AC_ARG_ENABLE([valgrind-support],
              [AS_HELP_STRING([--enable-valgrind_support], [turn on valgrind support [default="$valgrind_support_default"]])],
              [case "${enableval}" in
                 yes) valgrind_support=yes ;;
                 no)  valgrind_support=no  ;;
                 *) AC_MSG_ERROR([bad value ${enableval} for --enable-valgrind-support]) ;;
               esac],
              [valgrind_support=$valgrind_support_default])
AM_CONDITIONAL([VALGRIND_SUPPORT], [test x$valgrind_support = xyes])
if test x$valgrind_support = xyes; then
  CXXFLAGS="${CXXFLAGS} -DLIBACESTREAM_ENABLE_VALGRIND_SUPPORT"
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

# export variables (so Makefile substitutions can be made)
AC_SUBST(META_PACKAGE_NAME)
AC_SUBST(CXXFLAGS)

# checks for typedefs, structures, and compiler characteristics
AC_C_INLINE
AC_STRUCT_TIMEZONE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# checks for header files
AM_COND_IF([VALGRIND_SUPPORT],
           [AC_CHECK_HEADER([valgrind/valgrind.h])])
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([stddef.h])

# checks for library functions
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([localtime_r])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([tzset])
AC_FUNC_MALLOC

# checks for libraries
AM_COND_IF([VALGRIND_SUPPORT],
           [PKG_CHECK_MODULES([PKGVALGRIND], [valgrind])])

# checks for other libraries (non-pkg-config)
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-lACE':
AC_CHECK_LIB([ACE], [main])

AC_CONFIG_FILES([Makefile
                 scripts/Makefile
                 src/Makefile])

AC_OUTPUT
