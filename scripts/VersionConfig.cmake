# project
set (PROJECT_NAME ACEStream)
if (UNIX)
 set (PROJECT_LIBNAME lib${PROJECT_NAME})
elseif (WIN32)
 set (PROJECT_LIBNAME ${PROJECT_NAME})
endif ()

##########################################

# version
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_MICRO 1)
set (VERSION_DEVEL devel)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
if (VERSION_DEVEL)
 set (VERSION_FULL ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}-${VERSION_DEVEL})
else ()
 set (VERSION_FULL ${VERSION})
endif ()

##########################################

# config.h
set (PACKAGE ${PROJECT_LIBNAME})
set (${PROJECT_NAME}_PACKAGE ${PACKAGE})
set (PACKAGE_NAME ${PROJECT_NAME})
set (${PROJECT_NAME}_PACKAGE_NAME ${PACKAGE_NAME})
set (PACKAGE_STRING "${PROJECT_NAME} ${VERSION_FULL}")
set (${PROJECT_NAME}_PACKAGE_STRING ${PACKAGE_STRING})
set (PACKAGE_DESCRIPTION "\
user-space wrapper library for the pipes-and-filters pattern, based on the ACE \
framework (see: http://www.cs.wustl.edu/~schmidt/ACE.html). In particular, the \
library lightly encapsulates the ACE_Stream and ACE_Module classes, introducing\
 a new set of (control) interfaces to support asynchronous operation and \
additional concepts, such as 'session' data and messages. In conjunction with \
additional, modular data processing functionality (see e.g.: \
https://github.com/esohns/libACENetwork), this approach facilitates the \
separation of data processing from application-specific logic and therefore \
enables portable approaches to (distributed) application design")
set (${PROJECT_NAME}_PACKAGE_DESCRIPTION ${PACKAGE_DESCRIPTION})
set (PACKAGE_DESCRIPTION_SUMMARY "(wrapper) library for streams functionality, based on the ACE framework")
set (${PROJECT_NAME}_PACKAGE_DESCRIPTION_SUMMARY ${PACKAGE_DESCRIPTION_SUMMARY})

set (PACKAGE_URL "https://github.com/esohns/lib${PROJECT_NAME}")
set (${PROJECT_NAME}_PACKAGE_URL ${PACKAGE_URL})
set (PACKAGE_MAINTAINER "Erik Sohns")
set (${PROJECT_NAME}_PACKAGE_MAINTAINER ${PACKAGE_MAINTAINER})
set (PACKAGE_BUGREPORT "eriksohns@posteo.de")
set (${PROJECT_NAME}_PACKAGE_BUGREPORT ${PACKAGE_BUGREPORT})

set (PACKAGE_VERSION ${VERSION})
set (${PROJECT_NAME}_PACKAGE_VERSION ${PACKAGE_VERSION})
set (PACKAGE_VERSION_FULL ${VERSION_FULL})
set (${PROJECT_NAME}_PACKAGE_VERSION_FULL ${PACKAGE_VERSION_FULL})
set (${PROJECT_NAME}_VERSION_MAJOR ${VERSION_MAJOR})
set (${PROJECT_NAME}_VERSION_MINOR ${VERSION_MINOR})
set (${PROJECT_NAME}_VERSION_MICRO ${VERSION_MICRO})
set (${PROJECT_NAME}_VERSION_DEVEL ${VERSION_DEVEL})

set (PACKAGE_TARNAME "${PROJECT_LIBNAME}-${VERSION_FULL}.tar.gz")
set (${PROJECT_NAME}_PACKAGE_TARNAME ${PACKAGE_TARNAME})

##########################################

# cpack
# Package section (see http://packages.debian.org/stable/)
set (PACKAGE_SECTION "devel") # Debian
#set (PACKAGE_SECTION "Development/Libraries")
