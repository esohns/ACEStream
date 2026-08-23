set (THEORA_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_OGG ogg)
 pkg_check_modules (PKG_VORBIS theora)
 if (PKG_OGG_FOUND AND PKG_THEORA_FOUND)
  set (THEORA_FOUND TRUE)
  set (THEORA_INCLUDE_DIRS "${PKG_OGG_INCLUDE_DIRS};${PKG_THEORA_INCLUDE_DIRS}")
  set (THEORA_LIBRARIES "${PKG_OGG_LIBRARIES};${PKG_THEORA_LIBRARIES}")
endif (PKG_OGG_FOUND AND PKG_THEORA_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
  find_package (libtheora CONFIG)
  if (libtheora_FOUND)
   set (THEORA_FOUND TRUE)
   set (THEORA_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR})
   set (THEORA_LIBRARIES "${VCPKG_LIB_DIR}/theora.lib")
   set (THEORA_LIB_DIR ${VCPKG_BIN_DIR})
  endif (libtheora_FOUND)
 endif (VCPKG_USE)
 if (NOT THEORA_FOUND)
  set (OGG_LIB_FILE "ogg.lib")
  find_library (OGG_LIBRARY
                NAMES ${OGG_LIB_FILE}
                PATHS ${VCPKG_LIB_DIR}
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OGG_LIB_FILE}"
                NO_DEFAULT_PATH)
  set (THEORA_LIB_FILE "libtheora.lib")
  find_library (THEORA_LIBRARY
                NAMES ${THEORA_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/libtheora/win32/VS2010/x64
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${THEORA_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT (OGG_LIBRARY AND THEORA_LIBRARY))
   message (WARNING "could not find ${THEORA_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found OGG library \"${OGG_LIBRARY}\"")
   message (STATUS "Found Theora library \"${THEORA_LIBRARY}\"")
   set (THEORA_FOUND TRUE)
   # *NOTE*: use vcpkg include dir for OGG headers
   set (THEORA_INCLUDE_DIRS "${VCPKG_INCLUDE_DIR};$ENV{LIB_ROOT}/libtheora/include")
   set (THEORA_LIBRARIES "${OGG_LIBRARY};${THEORA_LIBRARY}")
   set (THEORA_LIB_DIR "${VCPKG_LIB_DIR};$ENV{LIB_ROOT}/libtheora/win32/VS2010/x64/${CMAKE_BUILD_TYPE}")
  endif (NOT (OGG_LIBRARY AND THEORA_LIBRARY))
 endif (NOT THEORA_FOUND)
endif ()
if (THEORA_FOUND)
 option (THEORA_SUPPORT "enable OGG/Theora support" ${THEORA_SUPPORT_DEFAULT})
 if (THEORA_SUPPORT)
  add_definitions (-DTHEORA_SUPPORT)
 endif (THEORA_SUPPORT)
endif (THEORA_FOUND)
