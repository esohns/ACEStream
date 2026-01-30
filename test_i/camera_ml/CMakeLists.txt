include (ffmpeg)
include (ImageMagick)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)

 include (FindPkgConfig)
 pkg_check_modules (PKG_V4L REQUIRED libv4l2)
elseif (WIN32)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
 include (DirectXBaseClasses)
endif ()

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec/scripts")
include (opencv)

include (tensorflow)
include (libtorch)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis/scripts")
include (wayland)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../scripts")
include (pge)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (mediapipe)
include (box2d)

if (WIN32)
# set (CMAKE_CXX_STANDARD 17)
 if (MSVC)
  if (TENSORFLOW_CC_SUPPORT OR LIBTORCH_SUPPORT)
   add_compile_options ("/std:c++17")
  endif (TENSORFLOW_CC_SUPPORT OR LIBTORCH_SUPPORT)
 endif (MSVC)
endif (WIN32)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (OPENCV_SUPPORT)
 include_directories (${OPENCV_INCLUDE_DIRS})
endif (OPENCV_SUPPORT)
#if (TENSORFLOW_SUPPORT)
# include_directories (${tensorflow_INCLUDE_DIRS})
#endif (TENSORFLOW_SUPPORT)
#if (TENSORFLOW_CC_SUPPORT)
# include_directories (${tensorflow_cc_INCLUDE_DIRS})
#endif (TENSORFLOW_CC_SUPPORT)
#if (LIBTORCH_SUPPORT)
# include_directories (${LIBTORCH_INCLUDE_DIRS})
#endif (LIBTORCH_SUPPORT)
if (UNIX)
 include_directories (${PKG_V4L_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/ml)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE camera_ml)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.h
     test_i.cpp
     test_i_camera_ml_common.h
     test_i_camera_ml_configuration.h
     test_i_camera_ml_defines.h
     test_i_camera_ml_stream_common.h
     test_i_common_modules.h
     test_i_eventhandler.cpp
     test_i_eventhandler.h
     test_i_eventhandler.inl
     test_i_message.cpp
     test_i_message.h
     test_i_message.inl
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_session_message.inl
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i_stream.cpp
     test_i_stream.h)
if (TENSORFLOW_SUPPORT OR TENSORFLOW_CC_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_camera_ml_module_tensorflow.cpp
      test_i_camera_ml_module_tensorflow.h
      test_i_camera_ml_module_tensorflow.inl)
endif (TENSORFLOW_SUPPORT OR TENSORFLOW_CC_SUPPORT)
if (LIBTORCH_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_camera_ml_module_libtorch.cpp
      test_i_camera_ml_module_libtorch.h
      test_i_camera_ml_module_libtorch.inl)
endif (LIBTORCH_SUPPORT)
if (OLC_PGE_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_olc_pge.cpp)
endif (OLC_PGE_SUPPORT)

if (MEDIAPIPE_SUPPORT)
 if (UNIX)
  set (protobuf_source_dir "${MEDIAPIPE_ROOT}/bazel-mediapipe/external/com_google_protobuf")
 elseif (WIN32)
  set (protobuf_source_dir "C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/external/com_google_protobuf")
 endif ()
 include (libprotobuf)
 set_property (TARGET libprotobuf PROPERTY FOLDER "test_i")

 if (UNIX)
  file (GLOB_RECURSE MP_FORMATS_SRC "${MEDIAPIPE_ROOT}/bazel-bin/mediapipe/framework/formats/*.cc")
 elseif (WIN32)
  file (GLOB_RECURSE MP_FORMATS_SRC "C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/bazel-out/x64_windows-opt/bin/mediapipe/framework/formats/*.cc")
 endif ()
 if (MSVC)
  add_definitions (${MSVC_WARNINGS})
 endif (MSVC)
 add_library (MP_FORMATS STATIC ${MP_FORMATS_SRC})
 set_property (TARGET MP_FORMATS PROPERTY FOLDER "test_i")
 if (UNIX)
  target_include_directories (MP_FORMATS PUBLIC "${MEDIAPIPE_ROOT}/bazel-bin")
 elseif (WIN32)
  target_include_directories (MP_FORMATS PUBLIC "C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/bazel-out/x64_windows-opt/bin")
 endif ()
 target_link_libraries (MP_FORMATS libprotobuf)

 set (EXECUTABLE_2 camera_ml_2)
 set (${EXECUTABLE_2}_EXE_SRCS
      stdafx.h
      test_i_2.cpp
      test_i_camera_ml_common.h
      test_i_camera_ml_configuration.h
      test_i_camera_ml_defines.h
      test_i_camera_ml_stream_common.h
      test_i_camera_ml_module_mediapipe.cpp
      test_i_camera_ml_module_mediapipe.h
      test_i_camera_ml_module_mediapipe.inl
      test_i_common_modules_2.h
      test_i_eventhandler.cpp
      test_i_eventhandler.h
      test_i_eventhandler.inl
      test_i_message.cpp
      test_i_message.h
      test_i_message.inl
      test_i_session_message.cpp
      test_i_session_message.h
      test_i_session_message.inl
      test_i_signalhandler.cpp
      test_i_signalhandler.h
      test_i_stream_2.cpp
      test_i_stream_2.h)
 if (OLC_PGE_SUPPORT)
  set (${EXECUTABLE_2}_EXE_SRCS
       ${${EXECUTABLE_2}_EXE_SRCS}
       test_i_olc_pge.cpp)
 endif (OLC_PGE_SUPPORT)

 if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
  include_directories (${BOX2D_INCLUDE_DIRS})
  include_directories (${PGE_INCLUDE_DIR})

  set (EXECUTABLE_3 camera_ml_3)
  set (${EXECUTABLE_3}_EXE_SRCS
       stdafx.h
       test_i_3.cpp
       test_i_camera_ml_common.h
       test_i_camera_ml_configuration.h
       test_i_camera_ml_defines.h
       test_i_camera_ml_stream_common.h
       test_i_camera_ml_module_mediapipe_3.cpp
       test_i_camera_ml_module_mediapipe_3.h
       test_i_camera_ml_module_mediapipe_3.inl
       test_i_common_modules_2.h
       test_i_eventhandler.cpp
       test_i_eventhandler.h
       test_i_eventhandler.inl
       test_i_message.cpp
       test_i_message.h
       test_i_message.inl
       test_i_olc_pge.cpp
       test_i_session_message.cpp
       test_i_session_message.h
       test_i_session_message.inl
       test_i_stream_3.cpp
       test_i_stream_3.h)
 endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
endif (MEDIAPIPE_SUPPORT)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis/include)
# set (${EXECUTABLE}_EXE_SRCS
#      ${${EXECUTABLE}_EXE_SRCS}
#      ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis/include/xdg-shell.cpp)
elseif (WIN32)
 # *NOTE*: DirectShow uses 'DEBUG'
# add_definitions (-DDEBUG) # activate DirectShow debug logging
 add_compile_options ($<$<CONFIG:Debug>:-DDEBUG>)
 add_compile_options ($<$<CONFIG:Debug>:-DD3D_DEBUG_INFO>)
endif ()

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_I)

if (TENSORFLOW_SUPPORT)
 target_include_directories (${EXECUTABLE} PUBLIC ${tensorflow_INCLUDE_DIRS})
endif (TENSORFLOW_SUPPORT)
if (TENSORFLOW_CC_SUPPORT)
 target_include_directories (${EXECUTABLE} PUBLIC ${tensorflow_cc_INCLUDE_DIRS})
endif (TENSORFLOW_CC_SUPPORT)
if (LIBTORCH_SUPPORT)
 target_include_directories (${EXECUTABLE} PUBLIC ${LIBTORCH_INCLUDE_DIRS})
endif (LIBTORCH_SUPPORT)

if (MEDIAPIPE_SUPPORT)
 add_executable (${EXECUTABLE_2}
                 EXCLUDE_FROM_ALL
                 ${${EXECUTABLE_2}_EXE_SRCS})
 set_property (TARGET ${EXECUTABLE_2} PROPERTY FOLDER "test_i")

 target_include_directories (${EXECUTABLE_2} PUBLIC ${MEDIAPIPE_INCLUDE_DIRS})

 if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
  add_executable (${EXECUTABLE_3}
                  EXCLUDE_FROM_ALL
                  ${${EXECUTABLE_3}_EXE_SRCS})
  set_property (TARGET ${EXECUTABLE_3} PROPERTY FOLDER "test_i")

  target_include_directories (${EXECUTABLE_3} PUBLIC ${MEDIAPIPE_INCLUDE_DIRS})
 endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
endif (MEDIAPIPE_SUPPORT)

#add_dependencies (${EXECUTABLE} ACE)
add_dependencies (${EXECUTABLE} Common)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_MediaFramework)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_I)

if (MEDIAPIPE_SUPPORT)
#add_dependencies (${EXECUTABLE_2} ACE)
 add_dependencies (${EXECUTABLE_2} Common)

 add_dependencies (${EXECUTABLE_2} ${PROJECT_NAME})
 add_dependencies (${EXECUTABLE_2} ${PROJECT_NAME}_Decoder)
 add_dependencies (${EXECUTABLE_2} ${PROJECT_NAME}_Device)
 add_dependencies (${EXECUTABLE_2} ${PROJECT_NAME}_MediaFramework)
 add_dependencies (${EXECUTABLE_2} ${PROJECT_NAME}_Miscellaneous)
 add_dependencies (${EXECUTABLE_2} ${PROJECT_NAME}_Visualization)

 add_dependencies (${EXECUTABLE_2} ${PROJECT_NAME}_Test_I)

 if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
#add_dependencies (${EXECUTABLE_3} ACE)
  add_dependencies (${EXECUTABLE_3} Common)

  add_dependencies (${EXECUTABLE_3} ${PROJECT_NAME})
  add_dependencies (${EXECUTABLE_3} ${PROJECT_NAME}_Decoder)
  add_dependencies (${EXECUTABLE_3} ${PROJECT_NAME}_Device)
  add_dependencies (${EXECUTABLE_3} ${PROJECT_NAME}_MediaFramework)
  add_dependencies (${EXECUTABLE_3} ${PROJECT_NAME}_Miscellaneous)
  add_dependencies (${EXECUTABLE_3} ${PROJECT_NAME}_Visualization)

  add_dependencies (${EXECUTABLE_3} ${PROJECT_NAME}_Test_I)
 endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
endif (MEDIAPIPE_SUPPORT)

if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (OPENCV_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${OPENCV_LIBRARIES})
endif (OPENCV_SUPPORT)
if (UNIX)
 if (TENSORFLOW_CC_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC ${tensorflow_cc_LIBRARIES})
 elseif (TENSORFLOW_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC ${tensorflow_LIBRARIES})
 endif ()
elseif (WIN32)
 if (TENSORFLOW_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC ${tensorflow_LIBRARIES})
 endif (TENSORFLOW_SUPPORT)
 if (TENSORFLOW_CC_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC ${tensorflow_cc_LIBRARIES})
 endif (TENSORFLOW_CC_SUPPORT)
endif ()
if (LIBTORCH_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBTORCH_LIBRARIES})
endif (LIBTORCH_SUPPORT)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries (${EXECUTABLE} PUBLIC stdc++fs)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_V4L_LIBRARIES})
elseif (WIN32)
# target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MachineLearning)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

if (MEDIAPIPE_SUPPORT)
 if (FFMPEG_SUPPORT)
  target_link_libraries (${EXECUTABLE_2} PUBLIC ${ffmpeg_LIBRARIES})
 endif (FFMPEG_SUPPORT)
 if (OPENCV_SUPPORT)
  target_link_libraries (${EXECUTABLE_2} PUBLIC ${OPENCV_LIBRARIES})
 endif (OPENCV_SUPPORT)
 target_link_libraries (${EXECUTABLE_2} PUBLIC MP_FORMATS)
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${MEDIAPIPE_LIBRARIES})
 if (UNIX)
  target_link_libraries (${EXECUTABLE_2} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
 # target_link_libraries (${EXECUTABLE_2} PUBLIC stdc++fs)
  target_link_libraries (${EXECUTABLE_2} PUBLIC ${PKG_V4L_LIBRARIES})
 elseif (WIN32)
 # target_link_options (${EXECUTABLE_2} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
 endif ()
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${ACE_LIBRARY})

 target_link_libraries (${EXECUTABLE_2} PUBLIC Common)
 target_link_libraries (${EXECUTABLE_2} PUBLIC CommonImage)
 target_link_libraries (${EXECUTABLE_2} PUBLIC CommonLog)
 target_link_libraries (${EXECUTABLE_2} PUBLIC CommonSignal)
 target_link_libraries (${EXECUTABLE_2} PUBLIC CommonTimer)

 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME})
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_Decoder)
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_Device)
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_MachineLearning)
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_MediaFramework)
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_Miscellaneous)
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_Statistic)
 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_Visualization)

 target_link_libraries (${EXECUTABLE_2} PUBLIC ${PROJECT_NAME}_Test_I)

 if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
  if (FFMPEG_SUPPORT)
   target_link_libraries (${EXECUTABLE_3} PUBLIC ${ffmpeg_LIBRARIES})
  endif (FFMPEG_SUPPORT)
  if (OPENCV_SUPPORT)
   target_link_libraries (${EXECUTABLE_3} PUBLIC ${OPENCV_LIBRARIES})
  endif (OPENCV_SUPPORT)
  target_link_libraries (${EXECUTABLE_3} PUBLIC MP_FORMATS)
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${MEDIAPIPE_LIBRARIES})
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${BOX2D_LIBRARIES})
  if (UNIX)
   target_link_libraries (${EXECUTABLE_3} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
  # target_link_libraries (${EXECUTABLE_3} PUBLIC stdc++fs)
   target_link_libraries (${EXECUTABLE_3} PUBLIC ${PKG_V4L_LIBRARIES})
  elseif (WIN32)
 # target_link_options (${EXECUTABLE_3} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
  endif ()
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${ACE_LIBRARY})

  target_link_libraries (${EXECUTABLE_3} PUBLIC Common)
  target_link_libraries (${EXECUTABLE_3} PUBLIC CommonImage)
  target_link_libraries (${EXECUTABLE_3} PUBLIC CommonLog)
  target_link_libraries (${EXECUTABLE_3} PUBLIC CommonSignal)
  target_link_libraries (${EXECUTABLE_3} PUBLIC CommonTimer)

  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME})
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_Decoder)
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_Device)
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_MachineLearning)
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_MediaFramework)
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_Miscellaneous)
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_Statistic)
  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_Visualization)

  target_link_libraries (${EXECUTABLE_3} PUBLIC ${PROJECT_NAME}_Test_I)
 endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
endif (MEDIAPIPE_SUPPORT)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)
if (MEDIAPIPE_SUPPORT)
 target_precompile_headers (${EXECUTABLE_2}
                            REUSE_FROM ${EXECUTABLE})
 if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
  target_precompile_headers (${EXECUTABLE_3}
                             REUSE_FROM ${EXECUTABLE})
 endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
endif (MEDIAPIPE_SUPPORT)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (MEDIAPIPE_SUPPORT)
 install (TARGETS ${EXECUTABLE_2}
          EXPORT ${PROJECT_NAME}Targets
          RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
  install (TARGETS ${EXECUTABLE_3}
           EXPORT ${PROJECT_NAME}Targets
           RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
endif (MEDIAPIPE_SUPPORT)
if (UNIX)
# install (FILES etc/${ui_definition_file}
#          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 file (TO_CMAKE_PATH "$ENV{LIB_ROOT}" LIB_ROOT)
 install (FILES ${ACE_DLL_FQPATH}
                ${LIB_ROOT}/opencv/build/bin/Release/opencv_core453.dll
                ${LIB_ROOT}/opencv/build/bin/Release/opencv_imgproc453.dll
#                ${LIB_ROOT}/tensorflow_c/lib/tensorflow.dll *NOTE*: do not package this; it's huge
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 if (MEDIAPIPE_SUPPORT)
  install (FILES C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/bazel-out/x64_windows-opt/bin/mediapipe/examples/desktop/libmp/libmp.dll
           DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
# *NOTE*: do not install these files by default; they just bloat the installer
#  install (DIRECTORY C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/bazel-out/x64_windows-opt/bin/mediapipe/models
#           DESTINATION ${INSTALL_BIN_DIR}/../mediapipe COMPONENT test_i)
#  install (DIRECTORY C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/bazel-out/x64_windows-opt/bin/mediapipe/modules
#           DESTINATION ${INSTALL_BIN_DIR}/../mediapipe COMPONENT test_i)
  if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
   install (FILES ${ACE_DLL_FQPATH}
            DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
  endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
 endif (MEDIAPIPE_SUPPORT)
endif ()

##########################################

set (ARGS "-l")
set (PATH_SUFFIX "Release")
if (DEFINED CMAKE_BUILD_TYPE)
 set (PATH_SUFFIX ${CMAKE_BUILD_TYPE})
endif (DEFINED CMAKE_BUILD_TYPE)
if (PATH_SUFFIX STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (PATH_SUFFIX STREQUAL Debug)
# *NOTE*: (for tensorflow,) model file needs to be relative to cwd
file (TO_NATIVE_PATH "share/model.pb" MODEL_PATH)
if (UNIX)
 set (WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE})
 set (ARGS "${ARGS} -f ${MODEL_PATH}")
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${EXECUTABLE}" COMMAND)
 set (ENVIRONMENT "HSA_OVERRIDE_GFX_VERSION=9.0.a;GFX_ARCH=gfx90c;PYTORCH_ROCM_ARCH=gfx90c")
elseif (WIN32)
 file (TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE} WORKING_DIRECTORY)
 string (REPLACE "\\" "\\\\" WORKING_DIRECTORY ${WORKING_DIRECTORY})
 string (REPLACE "\\" "\\\\" MODEL_PATH ${MODEL_PATH})
 set (ARGS "${ARGS} -f ${MODEL_PATH}")
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 # *NOTE*: ffmpeg was built with (vcpkg-)"libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 if (OPENCV_SUPPORT)
  file (TO_CMAKE_PATH "${OPENCV_LIB_DIR}" OPENCV_PATH)
 endif (OPENCV_SUPPORT)
 if (TENSORFLOW_SUPPORT)
  file (TO_CMAKE_PATH "${tensorflow_LIB_DIR}" TENSORFLOW_PATH)
 endif (TENSORFLOW_SUPPORT)
 if (TENSORFLOW_CC_SUPPORT)
  file (TO_CMAKE_PATH "${tensorflow_cc_LIB_DIR}" TENSORFLOW_CC_PATH)
 endif (TENSORFLOW_CC_SUPPORT)
 if (LIBTORCH_SUPPORT)
  file (TO_CMAKE_PATH "${LIBTORCH_LIB_DIR}" LIBTORCH_PATH)
 endif (LIBTORCH_SUPPORT)
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${VCPKG_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${OPENCV_PATH}
                           ${TENSORFLOW_PATH}
                           ${TENSORFLOW_CC_PATH}
                           ${LIBTORCH_PATH}
                           ${ACE_LIBRARY_PATH})
 if (GTK_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${GTK_PATH})
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${WXWIDGETS_PATH})
 endif (WXWIDGETS_SUPPORT)
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${WORKING_DIRECTORY}
                        ENVIRONMENT ${ENVIRONMENT})
if (MEDIAPIPE_SUPPORT)
 set (ARGS_2 "-l")
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (ARGS_2 "${ARGS_2} -t")
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}" WORKING_DIRECTORY_2)
 if (UNIX)
  file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${EXECUTABLE_2}" COMMAND_2)
 elseif (WIN32)
  file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${EXECUTABLE_2}.exe" COMMAND_2)
  string (REPLACE "\\" "\\\\" COMMAND_2 ${COMMAND_2})
  file (TO_CMAKE_PATH "${MEDIAPIPE_LIB_DIR}" MEDIAPIPE_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${MEDIAPIPE_PATH})
  string (REPLACE "\\" "\\\\" WORKING_DIRECTORY_2 ${WORKING_DIRECTORY_2})
 endif ()
 create_target_launcher (${EXECUTABLE_2}
                         COMMAND ${COMMAND_2}
                         ARGS ${ARGS_2}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${WORKING_DIRECTORY_2}
                         ENVIRONMENT ${ENVIRONMENT})

 if (UNIX)
  # Mediapipe Modules
  add_custom_command (TARGET ${EXECUTABLE_2} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_directory "${MEDIAPIPE_ROOT}/bazel-bin/mediapipe/modules" "$<TARGET_FILE_DIR:${EXECUTABLE_2}>/mediapipe/modules"
                      COMMENT "Copying mediapipe/modules")
  # additional file(s)
  add_custom_command (TARGET ${EXECUTABLE_2} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy "${MEDIAPIPE_ROOT}/mediapipe/modules/hand_landmark/handedness.txt" "$<TARGET_FILE_DIR:${EXECUTABLE_2}>/mediapipe/modules/hand_landmark"
                      COMMENT "Copying mediapipe/modules/hand_landmark/handedness.txt")
  # Mediapipe Models
  add_custom_command (TARGET ${EXECUTABLE_2} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_directory "${MEDIAPIPE_ROOT}/bazel-bin/mediapipe/models" "$<TARGET_FILE_DIR:${EXECUTABLE_2}>/mediapipe/models"
                      COMMENT "Copying mediapipe/models")
 elseif (WIN32)
  # Mediapipe Modules
  add_custom_command (TARGET ${EXECUTABLE_2} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_directory "C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/bazel-out/x64_windows-opt/bin/mediapipe/modules" "$<TARGET_FILE_DIR:${EXECUTABLE_2}>/mediapipe/modules"
                      COMMENT "Copying mediapipe/modules")
  # additional file(s)
  add_custom_command (TARGET ${EXECUTABLE_2} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy "${MEDIAPIPE_ROOT}/mediapipe/modules/hand_landmark/handedness.txt" "$<TARGET_FILE_DIR:${EXECUTABLE_2}>/mediapipe/modules/hand_landmark"
                      COMMENT "Copying mediapipe/modules/hand_landmark/handedness.txt")
  # Mediapipe Models
  add_custom_command (TARGET ${EXECUTABLE_2} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_directory "C:/Users/eriks/_bazel_eriks/dma4fr6d/execroot/mediapipe/bazel-out/x64_windows-opt/bin/mediapipe/models" "$<TARGET_FILE_DIR:${EXECUTABLE_2}>/mediapipe/models"
                      COMMENT "Copying mediapipe/models")
 endif ()

 if (WIN32)
  file (TO_CMAKE_PATH "$ENV{LIB_ROOT}/opencv/build" OPENCV_DIR)
  add_custom_command (TARGET ${EXECUTABLE_2} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy "${OPENCV_DIR}/bin/Release/opencv_imgproc453.dll" "$<TARGET_FILE_DIR:${EXECUTABLE_2}>"
                      COMMENT "Copying OpenCV DLL (Release)")
 endif (WIN32)

 if (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
  set (ARGS_3 "-l")
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
   set (ARGS_3 "${ARGS_3} -t")
  endif (CMAKE_BUILD_TYPE STREQUAL Debug)
  file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}" WORKING_DIRECTORY_3)
  if (UNIX)
   file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${EXECUTABLE_3}" COMMAND_3)
  elseif (WIN32)
   file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${EXECUTABLE_3}.exe" COMMAND_3)
   string (REPLACE "\\" "\\\\" COMMAND_3 ${COMMAND_3})
   file (TO_CMAKE_PATH "${BOX2D_LIB_DIR}" BOX2D_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             ${BOX2D_PATH})
   string (REPLACE "\\" "\\\\" WORKING_DIRECTORY_3 ${WORKING_DIRECTORY_3})
  endif ()
  create_target_launcher (${EXECUTABLE_3}
                          COMMAND ${COMMAND_3}
                          ARGS ${ARGS_3}
#                          FORWARD_ARGS
                          RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                          WORKING_DIRECTORY ${WORKING_DIRECTORY_3}
                          ENVIRONMENT ${ENVIRONMENT})

  # Mediapipe Modules
#  add_custom_command (TARGET ${EXECUTABLE_3} POST_BUILD
#                      COMMAND ${CMAKE_COMMAND} -E copy_directory "${MEDIAPIPE_ROOT}/bazel-bin/mediapipe/modules" "$<TARGET_FILE_DIR:${EXECUTABLE_3}>/mediapipe/modules"
#                      COMMENT "Copying mediapipe/modules"
#  )
  # additional file(s)
#  add_custom_command (TARGET ${EXECUTABLE_3} POST_BUILD
#                      COMMAND ${CMAKE_COMMAND} -E copy "${MEDIAPIPE_ROOT}/mediapipe/modules/hand_landmark/handedness.txt" "$<TARGET_FILE_DIR:${EXECUTABLE_3}>/mediapipe/modules/hand_landmark"
#                      COMMENT "Copying mediapipe/modules/hand_landmark/handedness.txt"
#  )
  # Mediapipe Models
#  add_custom_command (TARGET ${EXECUTABLE_3} POST_BUILD
#                      COMMAND ${CMAKE_COMMAND} -E copy_directory "${MEDIAPIPE_ROOT}/bazel-bin/mediapipe/models" "$<TARGET_FILE_DIR:${EXECUTABLE_3}>/mediapipe/models"
#                      COMMENT "Copying mediapipe/models"
#  )

#  if (WIN32)
#   add_custom_command (TARGET ${EXECUTABLE_3} POST_BUILD
#                       COMMAND ${CMAKE_COMMAND} -E copy "${OPENCV_DIR}/bin/Release/opencv_imgproc453.dll" "$<TARGET_FILE_DIR:${EXECUTABLE_3}>"
#                       COMMENT "Copying OpenCV DLL (Release)"
#   )
#  endif (WIN32)
 endif (BOX2D_SUPPORT AND OLC_PGE_SUPPORT)
endif (MEDIAPIPE_SUPPORT)

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${WORKING_DIRECTORY})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
