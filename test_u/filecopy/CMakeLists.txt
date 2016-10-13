option (GTK3_SUPPORT "compile Gtk3 support"
        ON)

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
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
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

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
elseif (WIN32)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
 include_directories ($ENV{ACE_ROOT})
# *TODO*: repair pkgconfig support
# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gtk-3.0)
# libglade
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (filecopy_EXE_SRCS
     stdafx.cpp
     stdafx.h
     ../test_u_common.h
     ../test_u_defines.h
     targetver.h
     test_u_filecopy.cpp
     test_u_filecopy_callbacks.cpp
     test_u_filecopy_callbacks.h
     test_u_filecopy_common.h
     test_u_filecopy_common_modules.h
     test_u_filecopy_defines.h
     test_u_filecopy_eventhandler.cpp
     test_u_filecopy_eventhandler.h
     test_u_filecopy_message.cpp
     test_u_filecopy_message.h
     test_u_filecopy_module_eventhandler.cpp
     test_u_filecopy_module_eventhandler.h
     test_u_filecopy_session_message.cpp
     test_u_filecopy_session_message.h
     test_u_filecopy_signalhandler.cpp
     test_u_filecopy_signalhandler.h
     test_u_filecopy_stream.cpp
     test_u_filecopy_stream.h)

set (EXECUTABLE filecopy)
add_executable (${EXECUTABLE} ${filecopy_EXE_SRCS} etc/filecopy.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
# *TODO*: repair WIN32 pkgconfig support
# gtk
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/pango-1.0.lib)
# libglade
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/glade/.libs/libglade-2.0.dll.a)
endif ()
target_link_libraries (${EXECUTABLE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/${EXECUTABLE}.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 install (FILES etc/${EXECUTABLE}.glade
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()
