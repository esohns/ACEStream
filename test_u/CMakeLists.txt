include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/scripts/Threads.cmake)

include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/scripts/ACE.cmake)

option (BUILD_TEST_U "build test_u" OFF)

# user interface
option (GUI_SUPPORT "enable graphical user interface support" ON)
if (GUI_SUPPORT)
 add_definitions (-DGUI_SUPPORT)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts/PreUIFramework.cmake)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts/SetUIFramework.cmake)

 if (CURSES_USE)
 elseif (GTK_USE)
  include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts/SetGTKVersion.cmake)

  if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions (-DG_ERRORCHECK_MUTEXES)
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 elseif (WXWIDGETS_USE)
  message (STATUS "wxWidgets_DEFINITIONS: ${wxWidgets_DEFINITIONS}")
  message (STATUS "wxWidgets_DEFINITIONS_DEBUG: ${wxWidgets_DEFINITIONS_DEBUG}")
  message (STATUS "wxWidgets_CXX_FLAGS: ${wxWidgets_CXX_FLAGS}")
  message (STATUS "wxWidgets_USE_FILE: ${wxWidgets_USE_FILE}")
# # *NOTE*: re-enable ANSI mode, iff available
# add_definitions (-DwxUSE_UNICODE=0)
  foreach (DEFINITION ${wxWidgets_DEFINITIONS})
   add_definitions (-D${DEFINITION})
  endforeach ()
  if (WIN32)
#  add_definitions (-D__WXMSW__)
#  add_definitions (-DWXUSINGDLL)
   if (MSVC)
    add_definitions (-DwxMSVC_VERSION_AUTO)
   endif (MSVC)
  endif (WIN32)
  add_definitions (-DwxUSE_GUI=1)
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
#   add_definitions (-D__WXDEBUG__)
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 endif ()
endif (GUI_SUPPORT)

add_library (${PROJECT_NAME}_Test_U STATIC "")

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  add_definitions (-DCOMMON_UI_HAS_DLL)
  add_definitions (-DCOMMON_UI_GTK_HAS_DLL)
  if (GTKGL_SUPPORT)
   add_definitions (-DCOMMON_GL_HAS_DLL)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

if (UNIX)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavutil)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)

 if (GUI_SUPPORT)
  if (CURSES_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CURSES_INCLUDE_DIRS}>)
  elseif (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")

   if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
    target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                                $<BUILD_INTERFACE:$ENV{LIB_ROOT}/gtkglarea>)
   endif (GTKGLAREA_SUPPORT)

   if (LIBGLADE_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                                $<BUILD_INTERFACE:${PKG_GLADE_INCLUDE_DIRS}>)
   endif (LIBGLADE_SUPPORT)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
  endif ()
 endif (GUI_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:/usr/local/src/ACE_wrappers>)

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/log>)
 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
  if (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
  elseif (QT_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/qt>)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
  endif ()
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>)

   if (OPENGL_SUPPORT AND GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
    target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtkglarea>)
   endif (OPENGL_SUPPORT AND GTKGLAREA_SUPPORT)

   if (LIBGLADE_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libglade-2.6.4/include/libglade-2.0>)
   endif (LIBGLADE_SUPPORT)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
  endif ()
 endif (GUI_SUPPORT)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
  if (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
  endif ()
 endif (GUI_SUPPORT)
endif ()

target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/modules/dev>)
endif (UNIX)
target_include_directories (${PROJECT_NAME}_Test_U PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/modules/lib>)

if (UNIX)

 if (GUI_SUPPORT)
  if (GTK_USE)
   target_link_libraries (${PROJECT_NAME}_Test_U ${GTK_LIBRARIES})
   if (GTKGLAREA_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_U X11)
    target_link_libraries (${PROJECT_NAME}_Test_U GL)
    target_link_libraries (${PROJECT_NAME}_Test_U $ENV{LIB_ROOT}/gtkglarea/gtkgl/.libs/libgtkgl-2.0.a)
   endif (GTKGLAREA_SUPPORT)

   if (GTKGL_SUPPORT)
#    target_link_libraries (${PROJECT_NAME}_Test_U CommonOpenGL)
   endif (GTKGL_SUPPORT)

   target_link_libraries (${PROJECT_NAME}_Test_U CommonUI_GTK)
  elseif (WXWIDGETS_USE)
   target_link_libraries (${PROJECT_NAME}_Test_U ${wxWidgets_LIBRARIES})

   target_link_libraries (${PROJECT_NAME}_Test_U CommonUI_wxWidgets)
  endif ()

  target_link_libraries (${PROJECT_NAME}_Test_U CommonUI)
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (GTK_USE)
   target_link_libraries (${PROJECT_NAME}_Test_U ${GTK_LIBRARIES})

   if (GTKGL_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_U OpenGL32.lib)

    if (GTKGLAREA_SUPPORT)
     target_link_libraries (${PROJECT_NAME}_Test_U $ENV{LIB_ROOT}/libepoxy/lib/epoxy.lib)
     target_link_libraries (${PROJECT_NAME}_Test_U $ENV{LIB_ROOT}/gtkglarea/cmake/${CMAKE_BUILD_TYPE}/gtkglarea.lib)
    endif (GTKGLAREA_SUPPORT)
#    if (GTK2_USE)
#     target_link_libraries (${PROJECT_NAME}_Test_U "C:/Documents and Settings/Erik/My Documents/Visual Studio 2010/Projects/gtkglarea/gtk2/${CMAKE_BUILD_TYPE}/gtkglarea.lib")
#    elseif (GTK3_USE)
#     target_link_libraries (${PROJECT_NAME}_Test_U "C:/Documents and Settings/Erik/My Documents/Visual Studio 2010/Projects/gtkglarea/gtk3/${CMAKE_BUILD_TYPE}/gtkglarea.lib")
#    endif ()
#    target_link_libraries (${PROJECT_NAME}_Test_U CommonOpenGL)
   endif (GTKGL_SUPPORT)

   target_link_libraries (${PROJECT_NAME}_Test_U CommonUI_GTK)
  elseif (WXWIDGETS_USE)
   target_link_libraries (${PROJECT_NAME}_Test_U ${wxWidgets_LIBRARIES})

   target_link_libraries (${PROJECT_NAME}_Test_U CommonUI_wxWidgets)
  endif ()

  target_link_libraries (${PROJECT_NAME}_Test_U CommonUI)
 endif (GUI_SUPPORT)
endif ()

target_link_libraries (${PROJECT_NAME}_Test_U Common)

##########################################

add_dependencies (${PROJECT_NAME}_Test_U Common)
if (GUI_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_U CommonUI)
 if (GTK_SUPPORT)
  add_dependencies (${PROJECT_NAME}_Test_U CommonUI_GTK)
  if (GTKGL_SUPPORT)
   add_dependencies (${PROJECT_NAME}_Test_U CommonOpenGL)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

##########################################

set (source_files
     test_u_common.h
     test_u_common_modules.h
     test_u_defines.h
     test_u_tools.cpp
     test_u_tools.h
     test_u_tools.inl)
if (GUI_SUPPORT)
 if (GTK_USE)
  set (source_files
       ${source_files}
       test_u_gtk_common.h
       test_u_gtk_eventhandler.cpp
       test_u_gtk_eventhandler.h
       test_u_gtk_eventhandler.inl)
 elseif (QT_USE)
  set (source_files
       ${source_files}
       test_u_qt_common.h)
 elseif (WXWIDGETS_USE)
  set (source_files
       ${source_files}
       test_u_wxwidgets_common.h)
 endif ()
endif (GUI_SUPPORT)
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

##########################################

# add PCH
set_target_properties (${PROJECT_NAME}_Test_U PROPERTIES
                       COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Test_U)

# add all targets to the build-tree export set
#export (TARGETS ${PROJECT_NAME}_Test_U
#        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# ----------------------------------------

add_subdirectory (audioeffect)
add_subdirectory (camerascreen)
add_subdirectory (filecopy)
# *IMPORTANT NOTE*: comment temporarily to disambiguate ACEStream <-> ACENetwork
#                   interdependencies
add_subdirectory (http_get)
add_subdirectory (imagescreen)
add_subdirectory (riff_decoder)

# phonies
add_custom_target (test_u ALL
                   COMMENT "building test_u...")
add_dependencies (test_u audioeffect camerascreen filecopy HTTP_GET imagescreen RIFF_decoder)

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
