include (libraries_init)

if (DEFINED ENV{LIB_ROOT})
 set (PROJECTM_DIR "$ENV{LIB_ROOT}/projectm")
endif (DEFINED ENV{LIB_ROOT})
if (UNIX)
 set (PROJECTM_BUILD_DIR "${PROJECTM_DIR}/build/gcc")
 set (PROJECTM_LIB "libprojectM-4${LIB_FILE_SUFFIX}.so")
 set (PROJECTM_PL_LIB "libprojectM-4-playlist${LIB_FILE_SUFFIX}.so")

 # include (FindPkgConfig)
 # pkg_check_modules (PKG_PROJECTM libprojectM)
 # if (PKG_PROJECTM_FOUND)
 #  set (PROJECTM_FOUND TRUE)
 #  set (PROJECTM_INCLUDE_DIRS "${PKG_PROJECTM_INCLUDE_DIRS}")
 #  set (PROJECTM_LIBRARIES "${PKG_PROJECTM_LIBRARIES}")
 # endif (PKG_PROJECTM_FOUND)
elseif (WIN32)
 set (PROJECTM_BUILD_DIR "${PROJECTM_DIR}/build/msvc")
 set (PROJECTM_LIB "projectM-4${LIB_FILE_SUFFIX}.lib")
 set (PROJECTM_PL_LIB "projectM-4-playlist${LIB_FILE_SUFFIX}.lib")
endif ()
find_library (PROJECTM_LIBRARY ${PROJECTM_LIB}
              PATHS ${PROJECTM_BUILD_DIR}/src/libprojectM
              PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
              DOC "searching for ${PROJECTM_LIB}"
              NO_DEFAULT_PATH)
find_library (PROJECTM_PL_LIBRARY ${PROJECTM_PL_LIB}
              PATHS ${PROJECTM_BUILD_DIR}/src/playlist
              PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
              DOC "searching for ${PROJECTM_PL_LIB}"
              NO_DEFAULT_PATH)
if (NOT (PROJECTM_LIBRARY AND PROJECTM_PL_LIBRARY))
 message (WARNING "could not find ${PROJECTM_LIB}, continuing")
else ()
 message (STATUS "Found projectM library \"${PROJECTM_LIBRARY}\"")
 message (STATUS "Found projectM playlist library \"${PROJECTM_PL_LIBRARY}\"")
 set (PROJECTM_FOUND TRUE)
 set (PROJECTM_LIBRARIES "${PROJECTM_LIBRARY};${PROJECTM_PL_LIBRARY}")
 set (PROJECTM_INCLUDE_DIRS "${PROJECTM_DIR}/src/api/include;${PROJECTM_BUILD_DIR}/src/api/include;${PROJECTM_DIR}/src/playlist/api;${PROJECTM_BUILD_DIR}/src/playlist/include")
 set (PROJECTM_LIB_DIRS "${PROJECTM_BUILD_DIR}/src/libprojectM/${CMAKE_BUILD_TYPE};${PROJECTM_BUILD_DIR}/src/playlist/${CMAKE_BUILD_TYPE}")
endif (NOT (PROJECTM_LIBRARY AND PROJECTM_PL_LIBRARY))
if (PROJECTM_FOUND)
 option (PROJECTM_SUPPORT "enable projectM support" ON)
 if (PROJECTM_SUPPORT)
  add_definitions (-DPROJECTM_SUPPORT)
 endif (PROJECTM_SUPPORT)
endif (PROJECTM_FOUND)
