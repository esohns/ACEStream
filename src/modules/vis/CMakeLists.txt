cmake_minimum_required (VERSION 2.8)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
 pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
endif ()
# *TODO*: repair WIN32 pkgconfig support

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
elseif (WIN32)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE)
 include_directories ($ENV{ACE_ROOT})
# *TODO*: repair pkgconfig support
# gtk
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/atk-1.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/pango-1.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/cairo)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/lib/glib-2.0/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/glib-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/gdk-pixbuf-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/lib/gtk-2.0/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/lib/gtk-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gtk3/include/pango-1.0)
# libglade
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libglade/include/libglade-2.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../misc)

############## next target ################
if (WIN32)
 set (${PROJECT_NAME}_Visualization_LIB_SRCS
      stream_vis_common.h
      stream_vis_defines.h
      stream_vis_exports.h
      stream_vis_gtk_drawingarea.cpp
      stream_vis_gtk_drawingarea.h
      stream_vis_gtk_drawingarea.inl
      stream_vis_target_direct3d.cpp
      stream_vis_target_direct3d.h
      stream_vis_target_direct3d.inl
      stream_vis_target_directshow.cpp
      stream_vis_target_directshow.h
      stream_vis_target_directshow.inl
      stream_vis_target_mediafoundation.cpp
      stream_vis_target_mediafoundation.h
      stream_vis_target_mediafoundation.inl
      stream_vis_tools.cpp
      stream_vis_tools.h
      stdafx.cpp
      stdafx.h)
else ()
 set (${PROJECT_NAME}_Visualization_LIB_SRCS
      stream_vis_common.h
      stream_vis_defines.h
      stream_vis_exports.h
      stream_vis_gtk_drawingarea.cpp
      stream_vis_gtk_drawingarea.h
      stream_vis_gtk_drawingarea.inl
      stream_vis_tools.cpp
      stream_vis_tools.h
      stdafx.cpp
      stdafx.h)
endif ()

add_library (${PROJECT_NAME}_Visualization SHARED ${ACEStream_Visualization_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Visualization Common)
add_dependencies (${PROJECT_NAME}_Visualization CommonERR)
add_dependencies (${PROJECT_NAME}_Visualization ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Visualization ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Visualization mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization strmiids.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization Common)
endif ()

set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES DEFINE_SYMBOL STREAM_VIS_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Visualization
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_vis_common.h
         stream_vis_defines.h
         stream_vis_exports.h
         stream_vis_gtk_drawingarea.h
         stream_vis_gtk_drawingarea.inl
         stream_vis_target_directshow.h
         stream_vis_target_directshow.inl
         stream_vis_tools.h
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Visualization
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
