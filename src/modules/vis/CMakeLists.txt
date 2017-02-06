option (GTK3_SUPPORT "enable Gtk3 support"
        ON)
option (GTKGL_SUPPORT "enable Gtk OpenGL support"
        ON)
option (GTKGLAREA_SUPPORT "enable GtkGLArea support"
        ON)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_LIBAVCODEC REQUIRED libavcodec)
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

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE)
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
  if (GTKGL_SUPPORT AND GTKGLAREA_SUPPORT)
   include_directories (${PKG_GTKGLAREA3_INCLUDE_DIRS})
  endif ()
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
  if (GTKGLAREA_SUPPORT)
   include_directories (${PKG_GTKGLAREA2_INCLUDE_DIRS})
  else ()
   include_directories (${PKG_GTKGL2_INCLUDE_DIRS})
  endif ()
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/math)
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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/include/gtk-3.0)

# libavformat
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../misc)

############## next target ################
if (WIN32)
 set (${PROJECT_NAME}_Visualization_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_vis_common.h
      stream_vis_defines.h
      stream_vis_exports.h
      stream_vis_gtk_cairo.cpp
      stream_vis_gtk_cairo.h
      stream_vis_gtk_cairo.inl
      stream_vis_gtk_cairo_spectrum_analyzer.cpp
      stream_vis_gtk_cairo_spectrum_analyzer.h
      stream_vis_gtk_cairo_spectrum_analyzer.inl
      stream_vis_target_direct3d.cpp
      stream_vis_target_direct3d.h
      stream_vis_target_direct3d.inl
      stream_vis_target_directshow.cpp
      stream_vis_target_directshow.h
      stream_vis_target_directshow.inl
      stream_vis_target_mediafoundation.cpp
      stream_vis_target_mediafoundation.h
      stream_vis_target_mediafoundation.inl
      stream_vis_gtk_pixbuf.cpp
      stream_vis_gtk_pixbuf.h
      stream_vis_gtk_pixbuf.inl
      stream_vis_tools.cpp
      stream_vis_tools.h)
else ()
 set (${PROJECT_NAME}_Visualization_LIB_SRCS
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
      stream_vis_tools.h
      stdafx.cpp
      stdafx.h)
endif ()

add_library (${PROJECT_NAME}_Visualization SHARED ${ACEStream_Visualization_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Visualization Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_Visualization CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_Visualization CommonMath)

add_dependencies (${PROJECT_NAME}_Visualization ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Visualization ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Visualization strmiids.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization mfuuid.lib)

# gtk
 target_link_libraries (${PROJECT_NAME}_Visualization ${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/gtk/Win32/debug/lib/cairo.lib)

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
