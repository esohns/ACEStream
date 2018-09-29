include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui/scripts/PreUIFramework.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui/scripts/SetUIFramework.cmake)

if (WXWIDGETS_SUPPORT)
 message (STATUS "wxWidgets_DEFINITIONS: ${wxWidgets_DEFINITIONS}")
 message (STATUS "wxWidgets_DEFINITIONS_DEBUG: ${wxWidgets_DEFINITIONS_DEBUG}")
 message (STATUS "wxWidgets_CXX_FLAGS: ${wxWidgets_CXX_FLAGS}")
 message (STATUS "wxWidgets_USE_FILE: ${wxWidgets_USE_FILE}")
 # *NOTE*: re-enable ANSI mode
# add_definitions (-DwxUSE_UNICODE=0)
 if (WIN32)
  add_definitions (-D__WXMSW__)
  add_definitions (-DWXUSINGDLL)
  if (MSVC)
   add_definitions (-DwxMSVC_VERSION_AUTO)
  endif (MSVC)
 endif (WIN32)
 add_definitions (-DwxUSE_GUI=1)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-D__WXDEBUG__)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif (WXWIDGETS_SUPPORT)

include (CMakeDependentOption)
if (GTK_USE)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui/scripts/SetGTKVersion.cmake)
 CMAKE_DEPENDENT_OPTION (GTKGL_SUPPORT "enable GTK OpenGL support" ON
                         "GUI_SUPPORT" OFF)
# *IMPORTANT NOTE*: to use gtkglarea on gtk2, check out the 'gtkglarea-2' branch
#                   of the project
 CMAKE_DEPENDENT_OPTION (GTKGLAREA_SUPPORT "enable GtkGLArea support" OFF
                         "GUI_SUPPORT; GTKGL_SUPPORT" OFF)
endif (GTK_USE)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil)
 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
   pkg_check_modules (PKG_GTK REQUIRED gtk+-3.0)
   if (GTKGL_SUPPORT)
    if (GTKGLAREA_SUPPORT)
     pkg_check_modules (PKG_GTKGL REQUIRED gtkgl-3.0)
    endif (GTKGLAREA_SUPPORT)
   endif (GTKGL_SUPPORT)
  else ()
   pkg_check_modules (PKG_GTK REQUIRED gmodule-2.0 gthread-2.0 gtk+-2.0)
   if (GTKGL_SUPPORT)
    if (GTKGLAREA_SUPPORT)
     pkg_check_modules (PKG_GTKGL REQUIRED gtkgl-2.0)
    else ()
     pkg_check_modules (PKG_GTKGL REQUIRED gdkglext-1.0 gtkglext-1.0)
    endif (GTKGLAREA_SUPPORT)
   endif (GTKGL_SUPPORT)
  endif ()
 endif (GTK_SUPPORT)
elseif (WIN32)
 if (GTK3_SUPPORT)
# *TODO*: repair pkg-config support
#  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
#  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
endif ()

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
elseif (WIN32)
 add_definitions (-D__STDC_CONSTANT_MACROS)

 if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
 # *NOTE*: DirectShow uses 'DEBUG'
  add_definitions (-DDEBUG) # activate DirectShow debug logging
 endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
endif ()

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11)
 #add_compile_options (-fPIC)
endif ()

add_library (${PROJECT_NAME}_Visualization STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)

 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
#  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
#                              $<BUILD_INTERFACE:${PKG_GTK3_INCLUDE_DIRS}>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               ${PKG_GTK_INCLUDE_DIRS})
   if (GTKGL_SUPPORT)
    if (GTKGLAREA_SUPPORT)
     target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                                 $<BUILD_INTERFACE:${PKG_GTKGL_INCLUDE_DIRS}>)
    endif (GTKGLAREA_SUPPORT)
   endif (GTKGL_SUPPORT)
  else ()
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${PKG_GTK_INCLUDE_DIRS}>)
   if (GTKGL_SUPPORT)
    if (GTKGLAREA_SUPPORT)
     target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                                 $<BUILD_INTERFACE:${PKG_GTKGL_INCLUDE_DIRS}>)
    else ()
     target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                                 $<BUILD_INTERFACE:${PKG_GTKGL_INCLUDE_DIRS}>)
    endif (GTKGLAREA_SUPPORT)
   endif (GTKGL_SUPPORT)
  endif ()
 endif (GTK_SUPPORT)

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/math>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/timer>)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
 else ()
  set (DXSDK_DIR
       "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)/Include")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:${DXSDK_DIR}>)
 endif (DEFINED ENV{DXSDK_DIR})
# set(_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "D:/projects/DShowBaseClasses")
#      "$ENV{PROGRAMFILES}/Microsoft Platform SDK/Samples/Multimedia/DirectShow/BaseClasses")
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses>)

# *TODO*: repair pkgconfig support
# gtk
 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/atk-1.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/gdk-pixbuf-2.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/cairo>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/pango-1.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/lib/glib-2.0/include>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/glib-2.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/gtk-3.0>)
  else ()
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/atk-1.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/gdk-pixbuf-2.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/cairo>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/pango-1.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/lib/glib-2.0/include>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/glib-2.0>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/lib/gtk-2.0/include>)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/gtk-2.0>)
  endif ()
  if (GTKGL_SUPPORT)
   if (GTK3_SUPPORT)
    if (GTKGLAREA_SUPPORT)
     target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                                 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtkglarea>)
    endif (GTKGLAREA_SUPPORT)
   else ()
    if (GTKGLAREA_SUPPORT)
     target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                                 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtkglarea>)
    else ()
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglext)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglext/gdk)
    endif (GTKGLAREA_SUPPORT)
   endif ()
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)

# libavformat
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg>)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/error>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/image>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/log>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/math>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/timer>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui>)

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../file>)
endif ()
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dec>)
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dev>)
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../stat>)

############## next target ################
set (source_files
     stream_vis_common.h
     stream_vis_defines.h
     stream_vis_exports.h
     stream_vis_tools.cpp
     stream_vis_tools.h)
if (GTK_SUPPORT)
 set (source_files
      ${source_files}
      stream_vis_gtk_cairo.cpp
      stream_vis_gtk_cairo.h
      stream_vis_gtk_cairo.inl
      stream_vis_gtk_cairo_spectrum_analyzer.cpp
      stream_vis_gtk_cairo_spectrum_analyzer.h
      stream_vis_gtk_cairo_spectrum_analyzer.inl
      stream_vis_gtk_pixbuf.cpp
      stream_vis_gtk_pixbuf.h
      stream_vis_gtk_pixbuf.inl)
endif (GTK_SUPPORT)
if (WIN32)
 set (source_files
      ${source_files}
      stream_vis_target_direct3d.cpp
      stream_vis_target_direct3d.h
      stream_vis_target_direct3d.inl
      stream_vis_target_directshow.cpp
      stream_vis_target_directshow.h
      stream_vis_target_directshow.inl
      stream_vis_target_mediafoundation.cpp
      stream_vis_target_mediafoundation.h
      stream_vis_target_mediafoundation.inl)
endif (WIN32)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
   PRIVATE ${abs_source_file}
   )
 #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Visualization
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

get_target_property (debug ${PROJECT_NAME}_Visualization INTERFACE_INCLUDE_DIRECTORIES)
#message (STATUS "INTERFACE_INCLUDE_DIRECTORIES: \"${debug}\"")

set_property (TARGET ${PROJECT_NAME}_Visualization PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Visualization Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_Visualization CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_Visualization CommonMath)

add_dependencies (${PROJECT_NAME}_Visualization ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Visualization ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Visualization Advapi32.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization Evr.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization Ole32.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization strmiids.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization mfuuid.lib)
# target_link_libraries (${PROJECT_NAME}_Visualization ucrt${LIB_FILE_SUFFIX}.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization User32.lib)
# target_link_libraries (${PROJECT_NAME}_Visualization uuids.lib)

# gtk
 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
   target_link_libraries (${PROJECT_NAME}_Visualization ${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/lib/gdk_pixbuf-2.0.lib)
   target_link_libraries (${PROJECT_NAME}_Visualization ${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/lib/cairo.lib)
  endif (GTK3_SUPPORT)
 endif (GTK_SUPPORT)

 target_link_libraries (${PROJECT_NAME}_Visualization Common)
endif (WIN32)

set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES DEFINE_SYMBOL STREAM_VIS_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Visualization)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Visualization
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (WIN32)
 install (FILES
          stream_vis_common.h
          stream_vis_defines.h
          stream_vis_exports.h
          stream_vis_gtk_cairo.h
          stream_vis_gtk_cairo.inl
          stream_vis_gtk_cairo_spectrum_analyzer.h
          stream_vis_gtk_cairo_spectrum_analyzer.inl
          stream_vis_target_direct3d.h
          stream_vis_target_direct3d.inl
          stream_vis_target_directshow.h
          stream_vis_target_directshow.inl
          stream_vis_target_mediafoundation.h
          stream_vis_target_mediafoundation.inl
          stream_vis_gtk_pixbuf.h
          stream_vis_gtk_pixbuf.inl
          stream_vis_tools.h
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
else ()
 install (FILES
          stream_vis_common.h
          stream_vis_defines.h
          stream_vis_exports.h
          stream_vis_gtk_cairo.h
          stream_vis_gtk_cairo.inl
          stream_vis_gtk_cairo_spectrum_analyzer.h
          stream_vis_gtk_cairo_spectrum_analyzer.inl
          stream_vis_gtk_pixbuf.h
          stream_vis_gtk_pixbuf.inl
          stream_vis_tools.h
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()
install (TARGETS ${PROJECT_NAME}_Visualization
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
