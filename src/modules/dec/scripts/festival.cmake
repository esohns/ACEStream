set (FESTIVAL_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_FESTIVAL festival)
 if (PKG_FESTIVAL_FOUND)
  set (FESTIVAL_FOUND TRUE)
  set (FESTIVAL_INCLUDE_DIRS "${PKG_FESTIVAL_INCLUDE_DIRS}")
  set (FESTIVAL_LIBRARIES "${PKG_FESTIVAL_LIBRARIES}")
 else ()
  set (SPEECHTOOLS_BASE_LIB_FILE "libestbase.so")
  find_library (SPEECHTOOLS_BASE_LIBRARY ${SPEECHTOOLS_BASE_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_BASE_LIB_FILE}")
  if (NOT SPEECHTOOLS_BASE_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_BASE_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_BASE_LIBRARY)
  set (SPEECHTOOLS_LIB_FILE "libestools.so")
  find_library (SPEECHTOOLS_LIBRARY ${SPEECHTOOLS_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_LIB_FILE}")
  if (NOT SPEECHTOOLS_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_LIBRARY)
  set (SPEECHTOOLS_STRING_LIB_FILE "libeststring.so")
  find_library (SPEECHTOOLS_STRING_LIBRARY ${SPEECHTOOLS_STRING_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_STRING_LIB_FILE}")
  if (NOT SPEECHTOOLS_STRING_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_STRING_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_STRING_LIBRARY)
  set (FESTIVAL_LIB_FILE "libfestival.so")
  find_library (FESTIVAL_LIBRARY ${FESTIVAL_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/festival/src/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${FESTIVAL_LIB_FILE}")
  if (NOT FESTIVAL_LIBRARY)
   message (WARNING "could not find ${FESTIVAL_LIB_FILE}, continuing")
  endif (NOT FESTIVAL_LIBRARY)
  if (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY)
   message (STATUS "Found ${FESTIVAL_LIB_FILE} library \"${FESTIVAL_LIBRARY}\"")
   set (FESTIVAL_FOUND TRUE)
   set (FESTIVAL_INCLUDE_DIRS "$ENV{LIB_ROOT}/speech_tools/include,$ENV{LIB_ROOT}/festival/src/include")
   set (FESTIVAL_LIBRARIES "${SPEECHTOOLS_BASE_LIBRARY};${SPEECHTOOLS_LIBRARY};${SPEECHTOOLS_STRING_LIBRARY};${FESTIVAL_LIBRARY}")
#   set (FESTIVAL_LIB_DIR "$ENV{LIB_ROOT}/festival/lib")
  endif (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY) 
 endif (PKG_FESTIVAL_FOUND)
elseif (WIN32)
 if (VCPKG_SUPPORT)
#  cmake_policy (SET CMP0074 OLD)
  find_package (festival CONFIG)
  if (festival_FOUND)
   set (FESTIVAL_FOUND TRUE)
   if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
       CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set (FESTIVAL_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/bin")
   else ()
    set (FESTIVAL_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin")
   endif ()
  endif (festival_FOUND)
 endif (VCPKG_SUPPORT)
 if (NOT festival_FOUND)
  set (SPEECHTOOLS_BASE_LIB_FILE "libestbase.lib")
  find_library (SPEECHTOOLS_BASE_LIBRARY ${SPEECHTOOLS_BASE_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_BASE_LIB_FILE}")
  if (NOT SPEECHTOOLS_BASE_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_BASE_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_BASE_LIBRARY)
  set (SPEECHTOOLS_LIB_FILE "libestools.lib")
  find_library (SPEECHTOOLS_LIBRARY ${SPEECHTOOLS_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_LIB_FILE}")
  if (NOT SPEECHTOOLS_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_LIBRARY)
  set (SPEECHTOOLS_STRING_LIB_FILE "libeststring.lib")
  find_library (SPEECHTOOLS_STRING_LIBRARY ${SPEECHTOOLS_STRING_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_STRING_LIB_FILE}")
  if (NOT SPEECHTOOLS_STRING_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_STRING_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_STRING_LIBRARY)
  set (FESTIVAL_LIB_FILE "libFestival.lib")
  find_library (FESTIVAL_LIBRARY ${FESTIVAL_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/festival/src/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${FESTIVAL_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT FESTIVAL_LIBRARY)
   message (WARNING "could not find ${FESTIVAL_LIB_FILE}, continuing")
  endif (NOT FESTIVAL_LIBRARY)
  if (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY)
   message (STATUS "Found ${FESTIVAL_LIB_FILE} library \"${FESTIVAL_LIBRARY}\"")
   set (FESTIVAL_FOUND TRUE)
   set (FESTIVAL_INCLUDE_DIRS "$ENV{LIB_ROOT}/speech_tools/include;$ENV{LIB_ROOT}/festival/src/include")
   set (FESTIVAL_LIBRARIES "${SPEECHTOOLS_BASE_LIBRARY};${SPEECHTOOLS_LIBRARY};${SPEECHTOOLS_STRING_LIBRARY};${FESTIVAL_LIBRARY}")
#   set (FESTIVAL_LIB_DIR "$ENV{LIB_ROOT}/festival/src/lib")
  endif (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY)
 endif (NOT festival_FOUND)
endif ()
if (FESTIVAL_FOUND)
 option (FESTIVAL_SUPPORT "enable festival support" ${FESTIVAL_SUPPORT_DEFAULT})
 if (FESTIVAL_SUPPORT)
  add_definitions (-DFESTIVAL_SUPPORT)
  if (WIN32)
   add_definitions (-DSYSTEM_IS_WIN32)
  endif (WIN32)
#  include_directories (${FESTIVAL_INCLUDE_DIRS})
 endif (FESTIVAL_SUPPORT)
endif (FESTIVAL_FOUND)
