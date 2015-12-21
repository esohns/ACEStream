cmake_minimum_required (VERSION 2.8)

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
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
 # *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/lib/gtk-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)

############## next target ################
set (${PROJECT_NAME}_Device_LIB_SRCS
     stream_dev_exports.h
     stream_module_dev_common.h
     stream_module_dev_defines.h
     stream_module_dev_tools.cpp
     stream_module_dev_tools.h
     stream_module_camsource_directshow.cpp
     stream_module_camsource_directshow.h
     stream_module_camsource_directshow.inl
     stream_module_camsource.cpp
     stream_module_camsource.h
     stream_module_camsource.inl
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_Device SHARED ${ACEStream_Device_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Device Common)
add_dependencies (${PROJECT_NAME}_Device ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Device ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Device ${MYSQL_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Device PROPERTIES DEFINE_SYMBOL STREAM_DEV_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Device PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Device
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_dev_exports.h
         stream_module_dev_common.h
         stream_module_dev_defines.h
         stream_module_dev_tools.h
         stream_module_camsource_directshow.h
         stream_module_camsource_directshow.inl
         stream_module_camsource.h
         stream_module_camsource.inl
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Device
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
