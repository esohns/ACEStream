#ifndef _LIBACESTREAM_CONFIG_H
#define _LIBACESTREAM_CONFIG_H 1
 
/* libacestream_config.h. Generated automatically at end of configure. */
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
/* #undef HAVE_DECL_TZNAME */

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef LIBACESTREAM_HAVE_DLFCN_H
#define LIBACESTREAM_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you have the `gettimeofday' function. */
#ifndef LIBACESTREAM_HAVE_GETTIMEOFDAY
#define LIBACESTREAM_HAVE_GETTIMEOFDAY 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef LIBACESTREAM_HAVE_INTTYPES_H
#define LIBACESTREAM_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `ACE' library (-lACE). */
/* #undef HAVE_LIBACE */

/* Define to 1 if you have the `pthread' library (-lpthread). */
#ifndef LIBACESTREAM_HAVE_LIBPTHREAD
#define LIBACESTREAM_HAVE_LIBPTHREAD 1
#endif

/* Define to 1 if you have the `localtime_r' function. */
#ifndef LIBACESTREAM_HAVE_LOCALTIME_R
#define LIBACESTREAM_HAVE_LOCALTIME_R 1
#endif

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#ifndef LIBACESTREAM_HAVE_MALLOC
#define LIBACESTREAM_HAVE_MALLOC 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifndef LIBACESTREAM_HAVE_MEMORY_H
#define LIBACESTREAM_HAVE_MEMORY_H 1
#endif

/* Define to 1 if stdbool.h conforms to C99. */
#ifndef LIBACESTREAM_HAVE_STDBOOL_H
#define LIBACESTREAM_HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stddef.h> header file. */
#ifndef LIBACESTREAM_HAVE_STDDEF_H
#define LIBACESTREAM_HAVE_STDDEF_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef LIBACESTREAM_HAVE_STDINT_H
#define LIBACESTREAM_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef LIBACESTREAM_HAVE_STDLIB_H
#define LIBACESTREAM_HAVE_STDLIB_H 1
#endif

/* Define to 1 if you have the `strerror' function. */
#ifndef LIBACESTREAM_HAVE_STRERROR
#define LIBACESTREAM_HAVE_STRERROR 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifndef LIBACESTREAM_HAVE_STRINGS_H
#define LIBACESTREAM_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef LIBACESTREAM_HAVE_STRING_H
#define LIBACESTREAM_HAVE_STRING_H 1
#endif

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
#ifndef LIBACESTREAM_HAVE_STRUCT_TM_TM_ZONE
#define LIBACESTREAM_HAVE_STRUCT_TM_TM_ZONE 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef LIBACESTREAM_HAVE_SYS_STAT_H
#define LIBACESTREAM_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef LIBACESTREAM_HAVE_SYS_TYPES_H
#define LIBACESTREAM_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#ifndef LIBACESTREAM_HAVE_TM_ZONE
#define LIBACESTREAM_HAVE_TM_ZONE 1
#endif

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
/* #undef HAVE_TZNAME */

/* Define to 1 if you have the `tzset' function. */
#ifndef LIBACESTREAM_HAVE_TZSET
#define LIBACESTREAM_HAVE_TZSET 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef LIBACESTREAM_HAVE_UNISTD_H
#define LIBACESTREAM_HAVE_UNISTD_H 1
#endif

/* Define if a version suffix is present. */
/* #undef HAVE_VERSION_DEVEL */

/* Define to 1 if the system has the type `_Bool'. */
#ifndef LIBACESTREAM_HAVE__BOOL
#define LIBACESTREAM_HAVE__BOOL 1
#endif

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#ifndef LIBACESTREAM_LT_OBJDIR
#define LIBACESTREAM_LT_OBJDIR ".libs/"
#endif

/* meta-package-name */
#ifndef LIBACESTREAM_META_PACKAGE_NAME
#define LIBACESTREAM_META_PACKAGE_NAME "libACEStream"
#endif

/* Name of package */
#ifndef LIBACESTREAM_PACKAGE
#define LIBACESTREAM_PACKAGE "libACEStream"
#endif

/* Define to the address where bug reports for this package should be sent. */
#ifndef LIBACESTREAM_PACKAGE_BUGREPORT
#define LIBACESTREAM_PACKAGE_BUGREPORT "eriksohns@123mail.org"
#endif

/* Define to the full name of this package. */
#ifndef LIBACESTREAM_PACKAGE_NAME
#define LIBACESTREAM_PACKAGE_NAME "libACEStream"
#endif

/* Define to the full name and version of this package. */
#ifndef LIBACESTREAM_PACKAGE_STRING
#define LIBACESTREAM_PACKAGE_STRING "libACEStream 0.0.1-devel"
#endif

/* Define to the one symbol short name of this package. */
#ifndef LIBACESTREAM_PACKAGE_TARNAME
#define LIBACESTREAM_PACKAGE_TARNAME "libACEStream"
#endif

/* Define to the home page for this package. */
#ifndef LIBACESTREAM_PACKAGE_URL
#define LIBACESTREAM_PACKAGE_URL "http://www.github.com/esohns/libACEStream"
#endif

/* Define to the version of this package. */
#ifndef LIBACESTREAM_PACKAGE_VERSION
#define LIBACESTREAM_PACKAGE_VERSION "0.0.1-devel"
#endif

/* Define to 1 if you have the ANSI C header files. */
#ifndef LIBACESTREAM_STDC_HEADERS
#define LIBACESTREAM_STDC_HEADERS 1
#endif

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Version number of package */
#ifndef LIBACESTREAM_VERSION
#define LIBACESTREAM_VERSION "0.0.1-devel"
#endif

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef ssize_t */
 
/* once: _LIBACESTREAM_CONFIG_H */
#endif
