if (WIN32)
 include (directxsdk)
endif (WIN32)

include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/gl/scripts")
include (glut)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/scripts")
include (PreUIFramework)
include (SetUIFramework)
if (GTK_SUPPORT) # *NOTE*: _SUPPORT, not _USE !
 include (SetGTKVersion)
endif (GTK_SUPPORT)

if (WIN32)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../lib/scripts")
 include (DirectXBaseClasses)
endif (WIN32)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../dec/scripts")
include (opencv)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (wayland)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)

if (WIN32)
 add_definitions (-D__STDC_CONSTANT_MACROS)

 if (CMAKE_BUILD_TYPE STREQUAL Debug)
 # *NOTE*: DirectShow uses 'DEBUG'
  add_definitions (-DDEBUG) # activate DirectShow debug logging
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif (WIN32)

add_library (${PROJECT_NAME}_Visualization STATIC "")

if (WIN32 AND DIRECTXSDK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${DIRECTXSDK_INCLUDE_DIR}>)
endif (WIN32 AND DIRECTXSDK_SUPPORT)

if (FFMPEG_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>)
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             "$<BUILD_INTERFACE:${ImageMagick_INCLUDE_DIRS}>")
 target_compile_options (${PROJECT_NAME}_Visualization PUBLIC
                         ${MAGICK_CXX_FLAGS})
 target_link_options (${PROJECT_NAME}_Visualization PUBLIC
                      ${MAGICK_LD_FLAGS})
endif (IMAGEMAGICK_SUPPORT)

if (GLUT_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             "$<BUILD_INTERFACE:${GLUT_INCLUDE_DIRS}>")
endif (GLUT_SUPPORT)

if (CURSES_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             "$<BUILD_INTERFACE:${CURSES_INCLUDE_DIRS}>")
endif (CURSES_SUPPORT)
if (GTK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")
 if (GTKGL_SUPPORT)
  if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
   target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                               $<BUILD_INTERFACE:$ENV{LIB_ROOT}/gtkglarea>)
  endif (GTKGLAREA_SUPPORT)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)

if (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${DS_BASECLASSES_INCLUDE_DIRS}>)
endif (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)

if (OPENCV_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             "$<BUILD_INTERFACE:${OPENCV_INCLUDE_DIRS}>")
endif (OPENCV_SUPPORT)
if (WAYLAND_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${WAYLAND_INCLUDE_DIRS}>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
endif (WAYLAND_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/gl>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/math>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui>)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui/gtk>)
 endif (GTK_SUPPORT)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/gl>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/math>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui>)
 if (CURSES_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/curses>)
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/gtk>)
 endif (GTK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../file>)
endif ()
target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
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
     stream_vis_base.cpp
     stream_vis_base.h
     stream_vis_common.h
     stream_vis_defines.h
     stream_vis_iresize.h
     stream_vis_tools.cpp
     stream_vis_tools.h)
if (CURSES_SUPPORT)
 set (source_files
      ${source_files}
      stream_vis_curses_window.cpp
      stream_vis_curses_window.h
      stream_vis_curses_window.inl)
endif (CURSES_SUPPORT)
if (OPENGL_SUPPORT)
 if (GLUT_SUPPORT)
  set (source_files
       ${source_files}
       stream_vis_opengl_glut.cpp
       stream_vis_opengl_glut.h
       stream_vis_opengl_glut.inl)
 endif (GLUT_SUPPORT)
endif (OPENGL_SUPPORT)
if (FFMPEG_SUPPORT)
 set (source_files
      ${source_files}
     stream_vis_libav_resize.cpp
     stream_vis_libav_resize.h
     stream_vis_libav_resize.inl)
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 set (source_files
      ${source_files}
      stream_vis_imagemagick_resize.cpp
      stream_vis_imagemagick_resize.h
      stream_vis_imagemagick_resize.inl)
endif (IMAGEMAGICK_SUPPORT)
if (GTK_SUPPORT)
 set (source_files
      ${source_files}
      stream_vis_gtk_cairo.cpp
      stream_vis_gtk_cairo.h
      stream_vis_gtk_cairo.inl
      stream_vis_gtk_cairo_spectrum_analyzer.cpp
      stream_vis_gtk_cairo_spectrum_analyzer.h
      stream_vis_gtk_cairo_spectrum_analyzer.inl
      stream_vis_gtk_common.h
      stream_vis_gtk_pixbuf.cpp
      stream_vis_gtk_pixbuf.h
      stream_vis_gtk_pixbuf.inl
      stream_vis_gtk_window.cpp
      stream_vis_gtk_window.h
      stream_vis_gtk_window.inl)
 if (GTKGL_SUPPORT)
  set (source_files
       ${source_files}
       stream_vis_gtk_cairo_gl.cpp
       stream_vis_gtk_cairo_gl.h)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)
if (OPENCV_SUPPORT)
 set (source_files
      ${source_files}
      stream_vis_opencv.cpp
      stream_vis_opencv.h
      stream_vis_opencv.inl
      stream_vis_opencv_classifier.cpp
      stream_vis_opencv_classifier.h
      stream_vis_opencv_classifier.inl)
endif (OPENCV_SUPPORT)
if (WAYLAND_SUPPORT)
 set (source_files
      ${source_files}
      include/xdg-shell.cpp
      stream_vis_wayland_window.cpp
      stream_vis_wayland_window.h
      stream_vis_wayland_window.inl)
endif (WAYLAND_SUPPORT)
if (UNIX)
 set (source_files
      ${source_files}
      stream_vis_x11_window.cpp
      stream_vis_x11_window.h
      stream_vis_x11_window.inl)
elseif (WIN32)
 set (source_files
      ${source_files}
      stream_vis_target_direct2d.cpp
      stream_vis_target_direct2d.h
      stream_vis_target_direct2d.inl
      stream_vis_target_direct3d.cpp
      stream_vis_target_direct3d.h
      stream_vis_target_direct3d.inl
      stream_vis_target_direct3d_11.cpp
      stream_vis_target_direct3d_11.h
      stream_vis_target_direct3d_11.inl
      stream_vis_target_directshow.cpp
      stream_vis_target_directshow.h
      stream_vis_target_directshow.inl
      stream_vis_target_gdi.cpp
      stream_vis_target_gdi.h
      stream_vis_target_gdi.inl
      stream_vis_target_mediafoundation.cpp
      stream_vis_target_mediafoundation.h
      stream_vis_target_mediafoundation.inl
      stream_vis_target_win32_base.cpp
      stream_vis_target_win32_base.h
      stream_vis_target_win32_base.inl)
endif ()

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
   PRIVATE ${abs_source_file}
   )
 #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Visualization
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Visualization
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Visualization PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Visualization Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_Visualization CommonError)
endif (WIN32)
add_dependencies (${PROJECT_NAME}_Visualization CommonMath)

add_dependencies (${PROJECT_NAME}_Visualization ${PROJECT_NAME})

if (OPENGL_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${OPENGL_LIBRARIES})
endif (OPENGL_SUPPORT)
if (FFMPEG_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${ImageMagick_LIBRARIES})
endif (IMAGEMAGICK_SUPPORT)
if (CURSES_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${CURSES_LIBRARIES})
endif (CURSES_SUPPORT)
if (GTK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${GTK_LIBRARIES})
endif (GTK_SUPPORT)
if (OPENCV_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${OPENCV_LIBRARIES})
endif (OPENCV_SUPPORT)
if (WAYLAND_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${WAYLAND_LIBRARIES})
endif (WAYLAND_SUPPORT)
if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC X11)
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC Advapi32.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC Evr.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC Ole32.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC strmiids.lib)
# target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ucrt${LIB_FILE_SUFFIX}.lib)
# target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC uuids.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC User32.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC D2d1.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC d3d11.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC d3dcompiler.lib)
endif ()

target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${ACE_LIBRARY})

target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC Common)
if (OPENGL_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC CommonOpenGL)
endif (OPENGL_SUPPORT)

target_link_libraries (${PROJECT_NAME}_Visualization PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES DEFINE_SYMBOL STREAM_VIS_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Visualization
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (header_files
     stream_vis_base.h
     stream_vis_common.h
     stream_vis_defines.h
     stream_vis_iresize.h
     stream_vis_tools.h)
if (CURSES_SUPPORT)
 set (source_files
      ${header_files}
      stream_vis_curses_window.h
      stream_vis_curses_window.inl)
endif (CURSES_SUPPORT)
if (OPENGL_SUPPORT)
 if (GLUT_SUPPORT)
  set (header_files
       ${header_files}
       stream_vis_opengl_glut.h
       stream_vis_opengl_glut.inl)
 endif (GLUT_SUPPORT)
endif (OPENGL_SUPPORT)
if (FFMPEG_SUPPORT)
 set (header_files
      ${header_files}
     stream_vis_libav_resize.h
     stream_vis_libav_resize.inl)
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 set (header_files
      ${header_files}
      stream_vis_imagemagick_resize.h
      stream_vis_imagemagick_resize.inl)
endif (IMAGEMAGICK_SUPPORT)
if (GTK_SUPPORT)
 set (header_files
      ${header_files}
      stream_vis_gtk_cairo.h
      stream_vis_gtk_cairo.inl
      stream_vis_gtk_cairo_spectrum_analyzer.h
      stream_vis_gtk_cairo_spectrum_analyzer.inl
      stream_vis_gtk_common.h
      stream_vis_gtk_pixbuf.h
      stream_vis_gtk_pixbuf.inl
      stream_vis_gtk_window.h
      stream_vis_gtk_window.inl)
 if (GTKGL_SUPPORT)
  set (header_files
       ${header_files}
       stream_vis_gtk_cairo_gl.h)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)
if (OPENCV_SUPPORT)
 set (header_files
      ${header_files}
      stream_vis_opencv.h
      stream_vis_opencv.inl
      stream_vis_opencv_classifier.h
      stream_vis_opencv_classifier.inl)
endif (OPENCV_SUPPORT)
if (WAYLAND_SUPPORT)
 set (header_files
      ${header_files}
      stream_vis_wayland_window.h
      stream_vis_wayland_window.inl)
endif (WAYLAND_SUPPORT)
if (UNIX)
 set (header_files
      ${header_files}
      stream_vis_x11_window.h
      stream_vis_x11_window.inl)
elseif (WIN32)
 set (header_files
      ${header_files}
      stream_vis_target_direct2d.h
      stream_vis_target_direct2d.inl
      stream_vis_target_direct3d.h
      stream_vis_target_direct3d.inl
      stream_vis_target_direct3d_11.h
      stream_vis_target_direct3d_11.inl
      stream_vis_target_directshow.h
      stream_vis_target_directshow.inl
      stream_vis_target_gdi.h
      stream_vis_target_gdi.inl
      stream_vis_target_mediafoundation.h
      stream_vis_target_mediafoundation.inl
      stream_vis_target_win32_base.h
      stream_vis_target_win32_base.inl)
endif ()
install (FILES
         ${header_files}
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Visualization
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
