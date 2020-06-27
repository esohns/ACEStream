option (FFMPEG_SUPPORT "support ffmpeg" ON)
if (FFMPEG_SUPPORT)
 add_definitions (-DFFMPEG_SUPPORT)
endif (FFMPEG_SUPPORT)

set (IMAGEMAGICK_DEFAULT ON)
option (IMAGEMAGICK_SUPPORT "support ImageMagick" ${IMAGEMAGICK_DEFAULT})
if (IMAGEMAGICK_SUPPORT)
 add_definitions (-DIMAGEMAGICK_SUPPORT)
endif (IMAGEMAGICK_SUPPORT)

set (OPENCV_DEFAULT OFF)
option (OPENCV_SUPPORT "support OpenCV" ${OPENCV_DEFAULT})
if (OPENCV_SUPPORT)
 add_definitions (-DOPENCV_SUPPORT)
endif (OPENCV_SUPPORT)

include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/scripts/PreUIFramework.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/scripts/SetUIFramework.cmake)
if (GTK_SUPPORT)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/scripts/SetGTKVersion.cmake)
endif (GTK_SUPPORT)

if (UNIX)
 pkg_check_modules (PKG_SOX REQUIRED sox)
 if (FFMPEG_SUPPORT)
  pkg_check_modules (PKG_FFMPEG REQUIRED libswscale libavcodec libavutil)
 endif (FFMPEG_SUPPORT)
 if (OPENGL_SUPPORT)
#  pkg_check_modules (PKG_OPENGL_GLUT REQUIRED freeglut)
  pkg_check_modules (PKG_OPENGL_GLUT REQUIRED glut)
 endif (OPENGL_SUPPORT)
endif ()
if (IMAGEMAGICK_SUPPORT)
 find_package (ImageMagick COMPONENTS MagickCore MagickWand)
endif (IMAGEMAGICK_SUPPORT)

if (UNIX)
# add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
elseif (WIN32)
 add_definitions (-D__STDC_CONSTANT_MACROS)

 if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
 # *NOTE*: DirectShow uses 'DEBUG'
  add_definitions (-DDEBUG) # activate DirectShow debug logging
 endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
endif ()

#set (LLVM_COMPILER_ID_STRING "Clang")
#string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
#string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
#if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
# add_compile_options (-std=c++11)
# add_compile_options (-fPIC)
#endif ()

add_library (${PROJECT_NAME}_Visualization STATIC "")

if (GTK_SUPPORT)
 message (STATUS "VIS:: GTK_INCLUDE_DIRS: ${GTK_INCLUDE_DIRS}")
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

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${PKG_SOX_INCLUDE_DIRS}>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)
# target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
#                             $<BUILD_INTERFACE:${PKG_OPENGL_GLUT_INCLUDE_DIRS}>)
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             "$<BUILD_INTERFACE:${ImageMagick_INCLUDE_DIRS}>")

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
#                             $<BUILD_INTERFACE:/usr/local/src/ACE_wrappers>)

 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
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
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/gtk>)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
 else ()
  set (DXSDK_DIR
       "$ENV{LIB_ROOT}/DXSDK/Include")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:${DXSDK_DIR}>)
 endif (DEFINED ENV{DXSDK_DIR})
# set(_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      $ENV{LIB_ROOT}/DShowBaseClasses)
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses>)

 if (IMAGEMAGICK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ImageMagick/include>)
 endif (IMAGEMAGICK_SUPPORT)

# libavformat
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ffmpeg/include>)

 if (OPENGL_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/glm>)
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/freeglut/include>)
 endif (OPENGL_SUPPORT)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

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
 target_include_directories (${PROJECT_NAME}_Visualization PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/gtk>)

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

#get_target_property (debug ${PROJECT_NAME}_Visualization INTERFACE_INCLUDE_DIRECTORIES)
#message (STATUS "INTERFACE_INCLUDE_DIRECTORIES: \"${debug}\"")

############## next target ################
set (source_files
     stream_vis_common.h
     stream_vis_defines.h
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
      stream_vis_gtk_common.h
      stream_vis_gtk_pixbuf.cpp
      stream_vis_gtk_pixbuf.h
      stream_vis_gtk_pixbuf.inl
      stream_vis_gtk_window.cpp
      stream_vis_gtk_window.h
      stream_vis_gtk_window.inl)
endif (GTK_SUPPORT)
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
if (OPENGL_SUPPORT)
 set (source_files
      ${source_files}
      stream_vis_opengl_glut.cpp
      stream_vis_opengl_glut.h
      stream_vis_opengl_glut.inl)
endif (OPENGL_SUPPORT)

if (UNIX)
 set (source_files
      ${source_files}
      stream_vis_wayland_window.cpp
      stream_vis_wayland_window.h
      stream_vis_wayland_window.inl
      stream_vis_x11_window.cpp
      stream_vis_x11_window.h
      stream_vis_x11_window.inl)
elseif (WIN32)
 set (source_files
      ${source_files}
      stream_vis_target_direct3d.cpp
      stream_vis_target_direct3d.h
      stream_vis_target_direct3d.inl
      stream_vis_target_directshow.cpp
      stream_vis_target_directshow.h
      stream_vis_target_directshow.inl
      stream_vis_target_gdi.cpp
      stream_vis_target_gdi.h
      stream_vis_target_gdi.inl
      stream_vis_target_mediafoundation.cpp
      stream_vis_target_mediafoundation.h
      stream_vis_target_mediafoundation.inl)
endif ()

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
 target_link_libraries (${PROJECT_NAME}_Visualization Advapi32.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization Evr.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization Ole32.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization strmiids.lib)
# target_link_libraries (${PROJECT_NAME}_Visualization ucrt${LIB_FILE_SUFFIX}.lib)
# target_link_libraries (${PROJECT_NAME}_Visualization uuids.lib)
 target_link_libraries (${PROJECT_NAME}_Visualization User32.lib)
endif (WIN32)
if (GTK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Visualization ${GTK_LIBRARIES})
endif (GTK_SUPPORT)
target_link_libraries (${PROJECT_NAME}_Visualization ${PKG_SOX_LIBRARIES})
target_link_libraries (${PROJECT_NAME}_Visualization ${PKG_FFMPEG_LIBRARIES})
target_link_libraries (${PROJECT_NAME}_Visualization ${ImageMagick_LIBRARIES})
if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Visualization ${PKG_OPENGL_GLUT_LIBRARIES})
# target_link_libraries (${PROJECT_NAME}_Visualization /usr/lib/x86_64-linux-gnu/libglut.so.3)
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME}_Visualization $ENV{LIB_ROOT}/freeglut/lib/freeglut.lib)
endif ()

target_link_libraries (${PROJECT_NAME}_Visualization ${ACE_LIBRARY})

target_link_libraries (${PROJECT_NAME}_Visualization Common)
target_link_libraries (${PROJECT_NAME}_Visualization CommonOpenGL)

target_link_libraries (${PROJECT_NAME}_Visualization ${PROJECT_NAME})

set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES DEFINE_SYMBOL STREAM_VIS_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Visualization PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Visualization)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Visualization
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (header_files
     stream_vis_common.h
     stream_vis_defines.h
     stream_vis_tools.h)
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
endif (GTK_SUPPORT)
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
if (OPENCV_SUPPORT)
 set (header_files
      ${header_files}
      stream_vis_opencv.h
      stream_vis_opencv.inl
      stream_vis_opencv_classifier.h
      stream_vis_opencv_classifier.inl)
endif (OPENCV_SUPPORT)
if (OPENGL_SUPPORT)
 set (header_files
      ${header_files}
      stream_vis_opengl_glut.h
      stream_vis_opengl_glut.inl)
endif (OPENGL_SUPPORT)
if (UNIX)
 set (header_files
      ${header_files}
      stream_vis_x11_window.h
      stream_vis_x11_window.inl)
elseif (WIN32)
 set (header_files
      ${header_files}
      stream_vis_target_direct3d.h
      stream_vis_target_direct3d.inl
      stream_vis_target_directshow.h
      stream_vis_target_directshow.inl
      stream_vis_target_gdi.h
      stream_vis_target_gdi.inl
      stream_vis_target_mediafoundation.h
      stream_vis_target_mediafoundation.inl)
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
