/* config.h.in.cmake */

//#define libACEStream_VERSION_MAJOR @VERSION_MAJOR@
//#define libACEStream_VERSION_MINOR @VERSION_MINOR@
//#define libACEStream_VERSION_MICRO @VERSION_MICRO@
//#define libACEStream_VERSION_DEVEL @VERSION_DEVEL@

/* Name of package */
//#cmakedefine @PACKAGE@_PACKAGE "@PACKAGE@"
#define LIBACESTREAM_PACKAGE "@PACKAGE@"
//#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
//#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#define LIBACESTREAM_PACKAGE_NAME "@PACKAGE@"
//#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#define LIBACESTREAM_PACKAGE_STRING "@PACKAGE@ @VERSION@"
//#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
//#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
//#undef PACKAGE_URL

/* Define to the version of this package. */
#define LIBACESTREAM_PACKAGE_VERSION "@VERSION@"
//#undef PACKAGE_VERSION

/* Version number of package */
//#undef VERSION

#cmakedefine VALGRIND_SUPPORT
