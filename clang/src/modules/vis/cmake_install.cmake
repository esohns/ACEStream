# Install script for directory: /mnt/win_d/projects/ACEStream/src/modules/vis

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xmod_includex" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/ACEStream/stream_vis_common.h;/usr/local/include/ACEStream/stream_vis_defines.h;/usr/local/include/ACEStream/stream_vis_tools.h;/usr/local/include/ACEStream/stream_vis_gtk_cairo.h;/usr/local/include/ACEStream/stream_vis_gtk_cairo.inl;/usr/local/include/ACEStream/stream_vis_gtk_cairo_spectrum_analyzer.h;/usr/local/include/ACEStream/stream_vis_gtk_cairo_spectrum_analyzer.inl;/usr/local/include/ACEStream/stream_vis_gtk_common.h;/usr/local/include/ACEStream/stream_vis_gtk_pixbuf.h;/usr/local/include/ACEStream/stream_vis_gtk_pixbuf.inl;/usr/local/include/ACEStream/stream_vis_gtk_window.h;/usr/local/include/ACEStream/stream_vis_gtk_window.inl;/usr/local/include/ACEStream/stream_vis_libav_resize.h;/usr/local/include/ACEStream/stream_vis_libav_resize.inl;/usr/local/include/ACEStream/stream_vis_imagemagick_resize.h;/usr/local/include/ACEStream/stream_vis_imagemagick_resize.inl;/usr/local/include/ACEStream/stream_vis_opengl_glut.h;/usr/local/include/ACEStream/stream_vis_opengl_glut.inl;/usr/local/include/ACEStream/stream_vis_x11_window.h;/usr/local/include/ACEStream/stream_vis_x11_window.inl")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/ACEStream" TYPE FILE FILES
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_common.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_defines.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_tools.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_cairo.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_cairo.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_cairo_spectrum_analyzer.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_cairo_spectrum_analyzer.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_common.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_pixbuf.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_pixbuf.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_window.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_gtk_window.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_libav_resize.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_libav_resize.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_imagemagick_resize.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_imagemagick_resize.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_opengl_glut.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_opengl_glut.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_x11_window.h"
    "/mnt/win_d/projects/ACEStream/src/modules/vis/stream_vis_x11_window.inl"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xmod_libx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libACEStream_Visualization.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/mnt/win_d/projects/ACEStream/clang/src/modules/vis/libACEStream_Visualization.a")
endif()

