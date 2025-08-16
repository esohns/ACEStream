# configure C/C++ exceptions
include (SetCppExceptions)

include (Threads)

if (VCPKG_USE)
 include (ImageMagick)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/gl/scripts")
 include (glew)
endif (VCPKG_USE)
include (ffmpeg)

include (ACE)

option (BUILD_TEST_I "build test_i" OFF)

# user interface
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/scripts")
include (PreUIFramework)
include (SetUIFramework)
if (GTK_SUPPORT)
 include (SetGTKVersion)
endif (GTK_SUPPORT)

if (UNIX)
 add_link_options ("LINKER:--as-needed")
endif (UNIX)

add_library (${PROJECT_NAME}_Test_I STATIC "")

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)
find_package (Common
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
#add_definitions (-DCOMMON_HAS_DLL)

if (FFMPEG_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>)
endif (FFMPEG_SUPPORT)

if (CURSES_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             "$<BUILD_INTERFACE:${CURSES_INCLUDE_DIRS}>")
endif (CURSES_SUPPORT)
if (GTK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")
 if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${GTKGLAREA_INCLUDES}>)
 endif (GTKGLAREA_SUPPORT)
endif (GTK_SUPPORT)
if (QT_SUPPORT)
# if (WIN32)
#  cmake_policy (SET CMP0020 OLD)
# endif (WIN32)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${QT_INCLUDE_DIRS}>)
endif (QT_SUPPORT)
if (WXWIDGETS_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             "$<BUILD_INTERFACE:${wxWidgets_INCLUDE_DIRS}>")
endif (WXWIDGETS_SUPPORT)

target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${ACE_INCLUDE_DIR}>)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/event>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/input>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui>)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/gtk>)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/qt>)
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../modules/Common/src/ui/wxwidgets>)
 endif (WXWIDGETS_SUPPORT)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/event>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/input>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui>)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/gtk>)
 endif (GTK_SUPPORT)
 if (QT_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/qt>)
 endif (QT_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/ui/wxwidgets>)
 endif (WXWIDGETS_SUPPORT)
endif ()

target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/tree>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/modules/dev>)
target_include_directories (${PROJECT_NAME}_Test_I PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/modules/lib>)

if (GTK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${GTK_LIBRARIES})
 if (GTKGL_SUPPORT)
  if (GTKGLAREA_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${GTKGLAREA_LIBRARY})
  endif (GTKGLAREA_SUPPORT)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)
if (QT_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${Qt_LIBRARIES})
endif (QT_SUPPORT)
if (WXWIDGETS_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC ${wxWidgets_LIBRARIES})
endif (WXWIDGETS_SUPPORT)

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC X11)

 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC GL)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
elseif (WIN32)
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC OpenGL32.lib)

   if (GTKGLAREA_SUPPORT)
    target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC $ENV{LIB_ROOT}/libepoxy/lib/epoxy.lib)
   endif (GTKGLAREA_SUPPORT)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif ()

target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI)

if (GTK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI_GTK)
endif (GTK_SUPPORT)
if (QT_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI_Qt)
endif (QT_SUPPORT)
if (WXWIDGETS_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC CommonUI_wxWidgets)
endif (WXWIDGETS_SUPPORT)

target_link_libraries (${PROJECT_NAME}_Test_I PUBLIC Common)

##########################################

add_dependencies (${PROJECT_NAME}_Test_I Common)
add_dependencies (${PROJECT_NAME}_Test_I CommonUI)
if (GTK_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_I CommonUI_GTK)
#  if (GTKGL_SUPPORT)
#   add_dependencies (${PROJECT_NAME}_Test_I CommonOpenGL)
#  endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)
if (QT_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_I CommonUI_Qt)
endif (QT_SUPPORT)
if (WXWIDGETS_SUPPORT)
 add_dependencies (${PROJECT_NAME}_Test_I CommonUI_wxWidgets)
endif (WXWIDGETS_SUPPORT)

##########################################

set (source_files
     test_i_configuration.h
     test_i_connection_common.h
     test_i_common.h
     test_i_defines.h
     test_i_tools.cpp
     test_i_tools.h
     test_i_tools.inl)
if (GTK_SUPPORT)
 set (source_files
      ${source_files}
      test_i_gtk_common.h
      test_i_gtk_eventhandler.cpp
      test_i_gtk_eventhandler.h
      test_i_gtk_eventhandler.inl)
endif (GTK_SUPPORT)
if (QT_SUPPORT)
 set (source_files
      ${source_files}
      test_i_qt_common.h)
endif (QT_SUPPORT)
if (WXWIDGETS_SUPPORT)
 set (source_files
      ${source_files}
      test_i_wxwidgets_common.h)
endif (WXWIDGETS_SUPPORT)
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
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Test_I
                           PUBLIC stdafx.h)

##########################################

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Test_I
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################

install (TARGETS ${PROJECT_NAME}_Test_I
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)
#         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
#         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

# ----------------------------------------

add_custom_target (test_i ALL
                   COMMENT "building test_i...")
if (BUILD_TEST_I)
 add_subdirectory (avsave)
 add_subdirectory (camera_ar)
 add_subdirectory (camera_ml)
 add_subdirectory (camera_msa)
 add_subdirectory (commandspeech)
 add_subdirectory (extract_stream)
 add_subdirectory (imagesave)
 add_subdirectory (mp3player)
 add_subdirectory (speechcommand)
# *IMPORTANT NOTE*: comment temporarily to disambiguate ACEStream <-> ACENetwork
#                   interdependencies
 add_subdirectory (camstream)
 add_subdirectory (filestream)
 add_subdirectory (http_get)
 add_subdirectory (http_get_2)
 add_subdirectory (pop_receive)
 add_subdirectory (smtp_send)
# *IMPORTANT NOTE* /END

 add_dependencies (test_i avsave camera_ar camera_ml camera_msa commandspeech extract_stream imagesave mp3player speechcommand)
 if (MEDIAPIPE_SUPPORT)
  add_dependencies (test_i camera_ml_2)
  if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
   add_dependencies (test_i camera_ml_3)
  endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
 endif (MEDIAPIPE_SUPPORT)

# *IMPORTANT NOTE*: comment temporarily to disambiguate ACEStream <-> ACENetwork
#                   interdependencies
 add_dependencies (test_i camsource camtarget filesource filetarget HTTP_get_1 HTTP_get_2 POP_receive SMTP_send)
 if (WIN32)
  add_dependencies (test_i camtarget_source camtarget_source_mf)
 endif (WIN32)
# *IMPORTANT NOTE* /END
endif (BUILD_TEST_I)

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
