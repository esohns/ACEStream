set (OPUS_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_OGG libogg)
 pkg_check_modules (PKG_OPUS libopus)
 if (PKG_OGG_FOUND AND PKG_OPUS_FOUND)
  set (OPUS_FOUND TRUE)
  set (OPUS_INCLUDE_DIRS "${PKG_OGG_INCLUDE_DIRS};${PKG_OPUS_INCLUDE_DIRS}")
  set (OPUS_LIBRARIES "${PKG_OGG_LIBRARIES};${PKG_OPUS_LIBRARIES}")
 endif (PKG_OGG_FOUND AND PKG_OPUS_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
  find_package (libogg CONFIG)
  find_package (libopus CONFIG)
  if (libogg_FOUND AND libopus_FOUND)
   set (OPUS_FOUND TRUE)
   set (OPUS_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR})
   set (OPUS_LIBRARIES "${VCPKG_LIB_DIR}/ogg.lib;${VCPKG_LIB_DIR}/opus.lib")
   set (OPUS_LIB_DIR ${VCPKG_BIN_DIR})
  endif (libogg_FOUND AND libopus_FOUND)
 endif (VCPKG_USE)
 if (NOT OPUS_FOUND)
  set (OGG_LIB_FILE "ogg.lib")
  find_library (OGG_LIBRARY
                NAMES ${OGG_LIB_FILE}
                PATHS ${VCPKG_LIB_DIR}
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OGG_LIB_FILE}"
                NO_DEFAULT_PATH)
  set (OPUS_LIB_FILE "opus.lib")
  find_library (OPUS_LIBRARY
                NAMES ${OPUS_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/opus/build/msvc
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OPUS_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT (OGG_LIBRARY AND OPUS_LIBRARY))
   message (WARNING "could not find ${OPUS_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found OGG library \"${OGG_LIBRARY}\"")
   message (STATUS "Found Opus library \"${OPUS_LIBRARY}\"")
   set (OPUS_FOUND TRUE)
   # *NOTE*: use vcpkg include dir for OGG headers
   set (OPUS_INCLUDE_DIRS "${VCPKG_INCLUDE_DIR};$ENV{LIB_ROOT}/opus/include")
   set (OPUS_LIBRARIES "${OGG_LIBRARY};${OPUS_LIBRARY}")
   set (OPUS_LIB_DIR "${VCPKG_LIB_DIR};$ENV{LIB_ROOT}/opus/build/msvc/${CMAKE_BUILD_TYPE}")
  endif (NOT (OGG_LIBRARY AND OPUS_LIBRARY))
 endif (NOT OPUS_FOUND)
endif ()
if (OPUS_FOUND)
 option (OPUS_SUPPORT "enable Opus support" ${OPUS_SUPPORT_DEFAULT})
 if (OPUS_SUPPORT)
  add_definitions (-DOPUS_SUPPORT)
 endif (OPUS_SUPPORT)
endif (OPUS_FOUND)
