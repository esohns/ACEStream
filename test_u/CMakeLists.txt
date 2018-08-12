option (BUILD_TEST_U "build test_u" OFF)
option (GTK_SUPPORT "enable GTK support" ON)
# option (GTK3_SUPPORT "enable Gtk3 support" ON)
include (CMakeDependentOption)
CMAKE_DEPENDENT_OPTION (GTK3_SUPPORT "enable GTK 3 support" ON
                        "GTK_SUPPORT" OFF)
CMAKE_DEPENDENT_OPTION (GTK2_SUPPORT "enable GTK 2 support" ON
                        "GTK_SUPPORT; NOT GTK3_SUPPORT" OFF)
CMAKE_DEPENDENT_OPTION (GTKGL_SUPPORT "enable GTK OpenGL support" ON
                        "GTK_SUPPORT" OFF)

add_library (${PROJECT_NAME}_Test_U STATIC "")

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)
if (GTK_SUPPORT)
 add_definitions (-DCOMMON_UI_HAS_DLL)
 add_definitions (-DCOMMON_UI_GTK_HAS_DLL)
 if (GTKGL_SUPPORT)
  add_definitions (-DCOMMON_GL_HAS_DLL)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)

include (FindPkgConfig)
if (UNIX)
 if (GTK_SUPPORT)
  if (GTK2_SUPPORT)
   pkg_check_modules (PKG_GTK REQUIRED gmodule-2.0 gthread-2.0 gtk+-2.0)
  elseif (GTK3_SUPPORT)
   pkg_check_modules (PKG_GTK REQUIRED gtk+-3.0)
  endif ()
 endif (GTK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src>)
 #include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/timer)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${PKG_GTK_INCLUDE_DIRS}>)

  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui>)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src/ui/gtk>)
 endif (GTK_SUPPORT)
elseif (WIN32)
# *TODO*: repair WIN32 pkgconfig support
## gtk
# if (GTK_SUPPORT)
#  if (GTK2_SUPPORT)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/atk-1.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/gdk-pixbuf-2.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/cairo>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/pango-1.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/glib-2.0/include>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/glib-2.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/lib/gtk-2.0/include>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk2/include/gtk-2.0>)
#  elseif (GTK3_SUPPORT)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/atk-1.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/gdk-pixbuf-2.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/cairo>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/pango-1.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/lib/glib-2.0/include>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/glib-2.0>)
#   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
#                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtk3/include/gtk-3.0>)
#  endif ()
## libglade
## include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)
# endif (GTK_SUPPORT)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src>)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/timer)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui>)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui/gtk>)
 endif (GTK_SUPPORT)
endif ()
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/modules/lib>)
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)

if (WIN32)
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_U OpenGL32.lib)
   target_link_libraries (${PROJECT_NAME}_Test_U "C:/Documents and Settings/Erik/My Documents/Visual Studio 2010/Projects/gtkglarea/${CMAKE_BUILD_TYPE}/gtkglarea.lib")
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif (WIN32)

add_dependencies (${PROJECT_NAME}_Test_U Common)
if (GTK_SUPPORT)
 if (GTKGL_SUPPORT)
  add_dependencies (${PROJECT_NAME}_Test_U CommonOpenGL)
 endif (GTKGL_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_U CommonUI)
 add_dependencies (${PROJECT_NAME}_Test_U CommonUI_GTK)
endif (GTK_SUPPORT)

target_link_libraries (${PROJECT_NAME}_Test_U Common)
if (GTK_SUPPORT)
 if (GTKGL_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Test_U CommonOpenGL)
 endif (GTKGL_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_U CommonUI)
 target_link_libraries (${PROJECT_NAME}_Test_U CommonUI_GTK)
endif (GTK_SUPPORT)

set (source_files
     test_u_common.h
     test_u_common_modules.h
     test_u_defines.h)
if (GTK_SUPPORT)
 set (source_files
      ${source_files}
      test_u_gtk_common.h
      test_u_gtk_eventhandler.cpp
      test_u_gtk_eventhandler.h
      test_u_gtk_eventhandler.inl)
endif (GTK_SUPPORT)
#install (FILES
#         ${PROJECT_NAME}_Test_U_GTK_SRCS
#         DESTINATION include)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
       )
 #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Test_U
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

# ----------------------------------------

add_subdirectory (audioeffect)
add_subdirectory (camsave)
add_subdirectory (filecopy)
add_subdirectory (http_get)
add_subdirectory (riff_decoder)

# phonies
add_custom_target (test_u ALL
                   COMMENT "building test_u...")
add_dependencies (test_u audioeffect camsave filecopy HTTP_GET RIFF_decoder)

cmake_policy (SET CMP0012 NEW)
if (NOT BUILD_TEST_U)
 set_target_properties (test_u PROPERTIES EXCLUDE_FROM_ALL 1)
 if (WIN32)
  set_target_properties (test_u PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif (WIN32)
endif (NOT BUILD_TEST_U)

# ----------------------------------------

# enable (unit/integration) tests
#include (CTest)
enable_testing ()
