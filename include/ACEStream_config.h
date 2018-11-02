/* config.h.cmake.in */

/* Name of package */
/* #undef  */
#define ACEStream_PACKAGE "ACEStream"
/* Define to the full name of this package. */
#define ACEStream_PACKAGE_NAME "ACEStream"
/* Define to the full name and version of this package. */
#define ACEStream_PACKAGE_STRING "ACEStream 0.0.1-devel"
#define ACEStream_PACKAGE_DESCRIPTION "user-space wrapper library for the pipes-and-filters pattern, based on the ACE framework (see: http://www.cs.wustl.edu/~schmidt/ACE.html). In particular, the library lightly encapsulates the ACE_Stream and ACE_Module classes, introducing a new set of (control) interfaces to support asynchronous operation and additional concepts, such as 'session' data and messages. In conjunction with additional, modular data processing functionality (see e.g.: https://github.com/esohns/libACENetwork), this approach facilitates the separation of data processing from application-specific logic and therefore enables portable approaches to (distributed) application design"
#define ACEStream_PACKAGE_DESCRIPTION_SUMMARY "(wrapper) library for streams functionality, based on the ACE framework"

/* Define to the home page for this package. */
#define ACEStream_PACKAGE_URL "https://github.com/esohns/libACEStream"
/* Define the name of the current package maintainer. */
#define ACEStream_PACKAGE_MAINTAINER "Erik Sohns"
/* Define to the address where bug reports for this package should be sent. */
#define ACEStream_PACKAGE_BUGREPORT "eriksohns@posteo.de"

/* Define to the version of this package. */
#define ACEStream_PACKAGE_VERSION "0.0.1"
#define ACEStream_PACKAGE_VERSION_FULL "0.0.1-devel"
/* Version numbers of package */
/* #undef ACEStream_VERSION_MAJOR */
/* #undef ACEStream_VERSION_MINOR */
//#define ACEStream_VERSION_MICRO 1
#define ACEStream_VERSION_MAJOR 0
#define ACEStream_VERSION_MINOR 0
#define ACEStream_VERSION_MICRO 1
#define ACEStream_VERSION_DEVEL "devel"

/* Define to the one symbol short name of this package. */
#define ACEStream_PACKAGE_TARNAME "ACEStream-0.0.1-devel.tar.gz"

/****************************************/
/* #undef TRACING */

/* #undef VALGRIND_SUPPORT */

//#define DEBUG_DEBUGGER
// sub-options - src
/* #undef SSL_SUPPORT */
/* #undef NETLINK_SUPPORT */
// sub-options - wlan
/* #undef WEXT_SUPPORT */
/* #undef NL80211_SUPPORT */
/* #undef DBUS_SUPPORT */
/* #undef DHCLIENT_SUPPORT */
/* #undef WLANAPI_SUPPORT */
/* #undef WINXP_SUPPORT */
