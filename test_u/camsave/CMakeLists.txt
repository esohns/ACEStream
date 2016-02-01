cmake_minimum_required (VERSION 2.8)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 find_package (Threads REQUIRED)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES ace build/linux/ace
                DOC "searching for ${ACE_LIB_FILE}")
 else ()
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                DOC "searching for ${ACE_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_LIB_FILE)
 set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()
add_definitions (-DACE_HAS_DLL)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L REQUIRED libv4l2)
 pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
endif ()
# *TODO*: repair WIN32 pkgconfig support

find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_HAS_DLL)

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)
if (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
endif ()

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_V4L_INCLUDE_DIRS})
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
 # *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gtk-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/include/libglade-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE camsave)
set (${EXECUTABLE}_EXE_SRCS
     ../test_u_common.h
     ../test_u_defines.h
     targetver.h
     test_u_camsave.cpp
     test_u_camsave_callbacks.cpp
     test_u_camsave_callbacks.h
     test_u_camsave_common.h
     test_u_camsave_common_modules.h
     test_u_camsave_defines.h
     test_u_camsave_eventhandler.cpp
     test_u_camsave_eventhandler.h
     test_u_camsave_message.cpp
     test_u_camsave_message.h
     test_u_camsave_module_eventhandler.cpp
     test_u_camsave_module_eventhandler.h
     test_u_camsave_session_message.cpp
     test_u_camsave_session_message.h
     test_u_camsave_signalhandler.cpp
     test_u_camsave_signalhandler.h
     test_u_camsave_stream.cpp
     test_u_camsave_stream.h
     stdafx.cpp
     stdafx.h)

add_executable (${EXECUTABLE} ${camsave_EXE_SRCS} etc/${EXECUTABLE}.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_V4L_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_GTK_GLADE_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} Strmiids.lib)
# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gobject-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/pango-1.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gdk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gtk-win32-2.0.lib)
endif ()
target_link_libraries (${EXECUTABLE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Device)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/${EXECUTABLE}.glade
         DESTINATION etc/lib${PROJECT_NAME}/camsave COMPONENT config)
