set (OGG_VORBIS_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_VORBIS libvorbis)
 if (PKG_VORBIS_FOUND)
  set (VORBIS_FOUND TRUE)
  set (VORBIS_INCLUDE_DIRS "${PKG_VORBIS_INCLUDE_DIRS}")
  set (VORBIS_LIBRARIES "${PKG_VORBIS_LIBRARIES}")
 endif (PKG_VORBIS_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
  find_package (libvorbis CONFIG)
  if (libvorbis_FOUND)
   set (VORBIS_FOUND TRUE)
   set (VORBIS_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR})
   set (VORBIS_LIBRARIES "${VCPKG_LIB_DIR}/vorbis.lib")
   set (VORBIS_LIB_DIR ${VCPKG_BIN_DIR})
  endif (libvorbis_FOUND)
 endif (VCPKG_USE)
 if (NOT VORBIS_FOUND)
  set (OGG_LIB_FILE "ogg.lib")
  find_library (OGG_LIBRARY
                NAMES ${OGG_LIB_FILE}
                PATHS ${VCPKG_LIB_DIR}
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OGG_LIB_FILE}"
                NO_DEFAULT_PATH)
  set (VORBIS_LIB_FILE "vorbis.lib")
  find_library (VORBIS_LIBRARY
                NAMES ${VORBIS_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/vorbis/build/msvc/lib
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${VORBIS_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT (OGG_LIBRARY AND VORBIS_LIBRARY))
   message (WARNING "could not find ${VORBIS_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found OGG library \"${OGG_LIBRARY}\"")
   message (STATUS "Found Vorbis library \"${VORBIS_LIBRARY}\"")
   set (VORBIS_FOUND TRUE)
   # *NOTE*: use vcpkg include dir for OGG headers
   set (VORBIS_INCLUDE_DIRS "${VCPKG_INCLUDE_DIR};$ENV{LIB_ROOT}/vorbis/include")
   set (VORBIS_LIBRARIES "${OGG_LIBRARY};${VORBIS_LIBRARY}")
   set (VORBIS_LIB_DIR "${VCPKG_LIB_DIR};$ENV{LIB_ROOT}/vorbis/build/msvc/lib/${CMAKE_BUILD_TYPE}")
  endif (NOT (OGG_LIBRARY AND VORBIS_LIBRARY))
 endif (NOT VORBIS_FOUND)
endif ()
if (VORBIS_FOUND)
 option (OGG_VORBIS_SUPPORT "enable OGG/Vorbis support" ${OGG_VORBIS_SUPPORT_DEFAULT})
 if (OGG_VORBIS_SUPPORT)
  add_definitions (-DVORBIS_SUPPORT)
 endif (OGG_VORBIS_SUPPORT)
endif (VORBIS_FOUND)
