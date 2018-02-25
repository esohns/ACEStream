include (CMakeDependentOption)
CMAKE_DEPENDENT_OPTION (GTK3_SUPPORT "enable GTK 3 support" ON
                        "GTK_SUPPORT" OFF)
CMAKE_DEPENDENT_OPTION (GTK2_SUPPORT "enable GTK 2 support" ON
                        "GTK_SUPPORT; NOT GTK3_SUPPORT" OFF)
#option (GTKGL_SUPPORT "enable GTK OpenGL support" ON)
 # *NOTE*: implies GTK_SUPPORT on
CMAKE_DEPENDENT_OPTION (GTKGL_SUPPORT "enable GTK OpenGL support" ON
                        "GTK_SUPPORT" OFF)
# *IMPORTANT NOTE*: to use gtkglarea on gtk2, check out the 'gtkglarea-2' branch
#                   of the project
#option (GTKGLAREA_SUPPORT "enable GtkGLArea support" OFF)
CMAKE_DEPENDENT_OPTION (GTKGLAREA_SUPPORT "enable GtkGLArea support" OFF
                        "GTK_SUPPORT; GTKGL_SUPPORT" OFF)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil)
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
  if (GTKGL_SUPPORT AND GTKGLAREA_SUPPORT)
   pkg_check_modules (PKG_GTKGLAREA3 REQUIRED gtkgl-3.0)
  endif ()
 else ()
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
  if (GTKGL_SUPPORT)
   if (GTKGLAREA_SUPPORT)
    pkg_check_modules (PKG_GTKGLAREA2 REQUIRED gtkgl-2.0)
   else ()
    pkg_check_modules (PKG_GTKGL2 REQUIRED gdkglext-1.0 gtkglext-1.0)
   endif ()
  endif ()
 endif ()
elseif (WIN32)
 if (GTK3_SUPPORT)
# *TODO*: repair pkg-config support
#  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
#  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
endif ()

if (GTKGL_SUPPORT)
 add_definitions (-DGTKGL_SUPPORT)
endif ()
if (GTKGLAREA_SUPPORT)
 add_definitions (-DGTKGLAREA_SUPPORT)
endif ()

add_library (${PROJECT_NAME}_Visualization STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)

 if (GTK3_SUPPORT)
#  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
#                              $<BUILD_INTERFACE:${PKG_GTK3_INCLUDE_DIRS}>)
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              ${PKG_GTK3_INCLUDE_DIRS})
  if (GTKGL_SUPPORT AND GTKGLAREA_SUPPORT)
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:${PKG_GTKGLAREA3_INCLUDE_DIRS}>)
  endif ()
 else ()
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:${PKG_GTK2_INCLUDE_DIRS}>)
  if (GTKGL_SUPPORT)
   if (GTKGLAREA_SUPPORT)
    target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                                $<BUILD_INTERFACE:${PKG_GTKGLAREA2_INCLUDE_DIRS}>)
   else ()
    target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                                $<BUILD_INTERFACE:${PKG_GTKGL2_INCLUDE_DIRS}>)
   endif ()
  endif ()
 endif ()

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/math>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/timer>)
elseif (WIN32)
 # *NOTE*: workaround
 set(_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE)
# *TODO*: repair pkgconfig support
# gtk
 if (GTK3_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/atk-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/gdk-pixbuf-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/cairo)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/pango-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/lib/glib-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/glib-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/gtk-3.0)
 else ()
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/atk-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/gdk-pixbuf-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/cairo)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/pango-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/lib/glib-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/glib-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/lib/gtk-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk2/include/gtk-2.0)
 endif ()
 if (GTKGL_SUPPORT)
  if (GTKGLAREA_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtkglarea)
  endif ()
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglext)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglext/gdk)
 endif ()

# libavformat
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)
endif ()
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dec>)
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dev>)
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../stat>)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../lib)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../stat)

############## next target ################
set (source_files
     stream_vis_common.h
     stream_vis_defines.h
     stream_vis_exports.h
     stream_vis_gtk_cairo.cpp
     stream_vis_gtk_cairo.h
     stream_vis_gtk_cairo.inl
     stream_vis_gtk_cairo_spectrum_analyzer.cpp
     stream_vis_gtk_cairo_spectrum_analyzer.h
     stream_vis_gtk_cairo_spectrum_analyzer.inl
     stream_vis_gtk_pixbuf.cpp
     stream_vis_gtk_pixbuf.h
     stream_vis_gtk_pixbuf.inl
     stream_vis_tools.cpp
     stream_vis_tools.h)
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

#get_target_property (debug ${PROJECT_NAME}_Visualization INTERFACE_INCLUDE_DIRECTORIES)
#message (STATUS "INTERFACE_INCLUDE_DIRECTORIES: \"${debug}\"")

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
endif ()

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11 -fPIC)
endif ()

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
 target_link_libraries (${PROJECT_NAME}_Visualization ucrt${LIB_FILE_SUFFIX}.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization User32.lib)
# target_link_libraries (${PROJECT_NAME}_Visualization uuids.lib)

# gtk
 if (GTK3_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Visualization ${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/lib/cairo.lib)
 endif ()

 target_link_libraries (${PROJECT_NAME}_Visualization Common)
endif ()

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
