include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/scripts/Threads.cmake)

include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/scripts/ACE.cmake)

if (WIN32 AND MSVC)
# set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
   # needed for some MSVC installations
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
endif (WIN32 AND MSVC)

option (BUILD_TEST_I "build test_i" OFF)

# user interface
option (GUI_SUPPORT "enable graphical user interface support" ON)
if (GUI_SUPPORT)
 add_definitions (-DGUI_SUPPORT)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts/Macros.cmake)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts/PreUIFramework.cmake)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts/SetUIFramework.cmake)

 if (CURSES_USE)
 elseif (GTK_USE)
  include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts/SetGTKVersion.cmake)
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
   #add_definitions (-D__WXDEBUG__)
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 endif ()
endif (GUI_SUPPORT)

add_library (${PROJECT_NAME}_Test_I STATIC "")

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
 if (GUI_SUPPORT)
  if (CURSES_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CURSES_INCLUDE_DIRS}>)
  elseif (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")

   if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:$ENV{LIB_ROOT}/gtkglarea>)
   endif (GTKGLAREA_SUPPORT)

   if (LIBGLADE_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${PKG_GLADE_INCLUDE_DIRS}>)
   endif (LIBGLADE_SUPPORT)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
  endif ()
 endif (GUI_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:/usr/local/src/ACE_wrappers>)

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
  if (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
  endif ()
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (CURSES_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../pdcurses>)
  elseif (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>)

   if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
    target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../gtkglarea>)
   endif (GTKGLAREA_SUPPORT)

# libglade
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>)
  endif ()
 endif (GUI_SUPPORT)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
 if (GUI_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
  if (GTK_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
  elseif (QT_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/qt>)
  elseif (WXWIDGETS_USE)
   target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
  endif ()
 endif (GUI_SUPPORT)
endif ()
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/tree>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/modules/dev>)
endif (UNIX)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/modules/lib>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)

if (UNIX)
 if (GUI_SUPPORT)
  if (GTK_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${GTK_LIBRARIES})

   if (GTKGL_SUPPORT)
    if (GTKGLAREA_SUPPORT)
     target_link_libraries (${PROJECT_NAME}_Test_I X11)
     target_link_libraries (${PROJECT_NAME}_Test_I GL)
     target_link_libraries (${PROJECT_NAME}_Test_I $ENV{LIB_ROOT}/gtkglarea/gtkgl/.libs/libgtkgl-2.0.a)
     target_link_libraries (${PROJECT_NAME}_Test_I GL)
    endif (GTKGLAREA_SUPPORT)

#    target_link_libraries (${PROJECT_NAME}_Test_I CommonOpenGL)
   endif (GTKGL_SUPPORT)

   target_link_libraries (${PROJECT_NAME}_Test_I CommonUI_GTK)
  elseif (WXWIDGETS_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${wxWidgets_LIBRARIES})

   target_link_libraries (${PROJECT_NAME}_Test_I CommonUI_wxWidgets)
  endif ()

  target_link_libraries (${PROJECT_NAME}_Test_I CommonUI)
 endif (GUI_SUPPORT)
elseif (WIN32)
 if (GUI_SUPPORT)
  if (GTK_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${GTK_LIBRARIES})

   if (GTKGL_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_I OpenGL32.lib)

    if (GTKGLAREA_SUPPORT)
     target_link_libraries (${PROJECT_NAME}_Test_I $ENV{LIB_ROOT}/libepoxy/lib/epoxy.lib)
     target_link_libraries (${PROJECT_NAME}_Test_I $ENV{LIB_ROOT}/gtkglarea/cmake/${CMAKE_BUILD_TYPE}/gtkglarea.lib)
    endif (GTKGLAREA_SUPPORT)

#    target_link_libraries (${PROJECT_NAME}_Test_I CommonOpenGL)
   endif (GTKGL_SUPPORT)

   target_link_libraries (${PROJECT_NAME}_Test_I CommonUI_GTK)
  elseif (WXWIDGETS_USE)
   target_link_libraries (${PROJECT_NAME}_Test_I ${wxWidgets_LIBRARIES})
  endif ()

  target_link_libraries (${PROJECT_NAME}_Test_I CommonUI)
 endif (GUI_SUPPORT)
endif ()
target_link_libraries (${PROJECT_NAME}_Test_I Common)

##########################################

add_dependencies (${PROJECT_NAME}_Test_I Common)
if (GUI_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_I CommonUI)
 if (GTK_USE)
  add_dependencies (${PROJECT_NAME}_Test_I CommonUI_GTK)
  if (GTKGL_SUPPORT)
   add_dependencies (${PROJECT_NAME}_Test_I CommonOpenGL)
  endif (GTKGL_SUPPORT)
 endif (GTK_USE)
endif (GUI_SUPPORT)

##########################################

set (source_files
     test_i_configuration.h
     test_i_connection_common.h
     test_i_common.h
     test_i_defines.h)
if (GTK_USE)
 set (source_files
      ${source_files}
      test_i_gtk_common.h
      test_i_gtk_eventhandler.cpp
      test_i_gtk_eventhandler.h
      test_i_gtk_eventhandler.inl)
elseif (WXWIDGETS_USE)
 set (source_files
      ${source_files}
      test_i_wxwidgets_common.h)
endif ()
#install (FILES
#         ${PROJECT_NAME}_Test_I_SRCS
#         DESTINATION include)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
       )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Test_I
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

# ----------------------------------------

add_subdirectory (camsave)
# *IMPORTANT NOTE*: comment temporarily to disambiguate ACEStream <-> ACENetwork
#                   interdependencies
add_subdirectory (camstream)
add_subdirectory (filestream)
add_subdirectory (http_get)
add_subdirectory (http_get_2)
add_subdirectory (mp3player)

add_custom_target (test_i ALL
                   COMMENT "building test_i...")
if (UNIX)
 add_dependencies (test_i camsave camsource camtarget filesource filetarget HTTP_get_1 HTTP_get_2)
elseif (WIN32)
 add_dependencies (test_i camsave camsource camtarget camtarget_source camtarget_source_mf filesource filetarget HTTP_get_1 HTTP_get_2)
endif ()

cmake_policy (SET CMP0012 NEW)
if (NOT BUILD_TEST_I)
 set_target_properties (test_i PROPERTIES EXCLUDE_FROM_ALL 1)
 if (WIN32)
  set_target_properties (test_i PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif (WIN32)
endif (NOT BUILD_TEST_I)

# ----------------------------------------

# enable (unit/integration) tests
#include (CTest)
enable_testing ()
