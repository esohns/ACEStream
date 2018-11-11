include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)
 pkg_check_modules (PKG_SOX REQUIRED sox)

 pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil libavformat libswscale)
 pkg_check_modules (PKG_OPENCV REQUIRED opencv)
endif (UNIX)

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
elseif (WIN32)
 add_definitions (-D_USE_MATH_DEFINES)
 add_definitions (-DWIN32)
endif ()

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11)
#add_compile_options (-fPIC)
endif ()

add_library (${PROJECT_NAME}_Decoder STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${PKG_ALSA_INCLUDE_DIRS}>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${PKG_SOX_INCLUDE_DIRS}>)
# include_directories (${PKG_LIBSNDFILE_INCLUDE_DIRS})

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${PKG_OPENCV_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/cmake>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui>)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
 else ()
  set (DXSDK_DIR
       "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)/Include")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
##      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
  target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                              $<BUILD_INTERFACE:${DXSDK_DIR}>)
 endif (DEFINED ENV{DXSDK_DIR})
# set(_PF86 "ProgramFiles(x86)")
# cmake_policy (SET CMP0053 OLD) # Simplify variable reference and escape sequence evaluation
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "D:/projects/DShowBaseClasses")
#      "$ENV{PROGRAMFILES}/Microsoft Platform SDK/Samples/Multimedia/DirectShow/BaseClasses")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses>)

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/build>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/calib3d/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/core/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/features2d/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/flann/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/highgui/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/imgcodecs/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/imgproc/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/ml/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/objdetect/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/photo/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/shape/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/stitching/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/superres/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/video/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/videoio/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../opencv/modules/videostab/include>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../zlib-1.2.8>)

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc>)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui>)

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party>)
endif ()
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)

target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dev>)
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vis>)

#message (STATUS "INTERFACE_INCLUDE_DIRECTORIES=${INTERFACE_INCLUDE_DIRECTORIES}")

############## next target ################
set (source_files
     stream_dec_avi_decoder.cpp
     stream_dec_avi_decoder.h
     stream_dec_avi_decoder.inl
     stream_dec_avi_encoder.cpp
     stream_dec_avi_encoder.h
     stream_dec_avi_encoder.inl
     stream_dec_avi_parser.cpp
     stream_dec_avi_parser.h
     stream_dec_avi_parser_driver.cpp
     stream_dec_avi_parser_driver.h
     stream_dec_common.h
     stream_dec_defines.h
#     stream_dec_exports.h
     stream_dec_h264_nal_bisector.cpp
     stream_dec_h264_nal_bisector.h
     stream_dec_h264_nal_decoder.cpp
     stream_dec_h264_nal_decoder.h
     stream_dec_h264_nal_decoder.inl
     stream_dec_libav_converter.cpp
     stream_dec_libav_converter.h
     stream_dec_libav_converter.inl
     stream_dec_libav_decoder.cpp
     stream_dec_libav_decoder.h
     stream_dec_libav_decoder.inl
     stream_dec_mpeg_ts_decoder.cpp
     stream_dec_mpeg_ts_decoder.h
     stream_dec_mpeg_ts_decoder.inl
     stream_dec_opencv_decoder.cpp
     stream_dec_opencv_decoder.h
     stream_dec_opencv_decoder.inl
     stream_dec_riff_scanner.cpp
     stream_dec_riff_scanner.h
     stream_dec_tools.cpp
     stream_dec_tools.h
     stream_dec_zip_decoder.cpp
     stream_dec_zip_decoder.h
     stream_dec_zip_decoder.inl)
if (UNIX)
 set (source_files
      ${source_files}
      stream_dec_sox_effect.cpp
      stream_dec_sox_effect.h
      stream_dec_sox_effect.inl)
endif (UNIX)
set (script_files
     scripts/avi_scanner.l
     scripts/avi_parser.y
     scripts/h264_nal_bisector.l)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       ${abs_source_file})
  #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
unset (absolute_script_files)
foreach (script_file ${script_files})
 get_filename_component (abs_script_file ${script_file} ABSOLUTE)
 list (APPEND absolute_script_files
       ${absolute_script_file})
endforeach ()
target_sources (${PROJECT_NAME}_Decoder
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE
                ${absolute_source_files}
                INTERFACE
                ${absolute_script_files})

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Decoder_LIB_SCRIPTS)
endif (WIN32)

set_property (TARGET ${PROJECT_NAME}_Decoder PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Decoder Common)
add_dependencies (${PROJECT_NAME}_Decoder ${PROJECT_NAME})

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_ALSA_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_SOX_LIBRARIES})
# target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_AVCODEC_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_FFMPEG_LIBRARIES})

# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavcodec/libavcodec.a)
# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavformat/libavformat.a)
# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavutil/libavutil.a)
# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libswscale/libswscale.a)
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
 set (DIRECTX_SDK_DIRECTORY
      "C:/Program Files/Microsoft DirectX SDK (June 2010)")
# message (STATUS "DirectX SDK directory: \"${DIRECTX_SDK_DIRECTORY}\"")
 link_directories ("${DIRECTX_SDK_DIRECTORY}/Lib/x86")
 target_link_libraries (${PROJECT_NAME}_Decoder "${DIRECTX_SDK_DIRECTORY}/Lib/x86/dxguid.lib")
# target_link_libraries (${PROJECT_NAME}_Decoder dxva2.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder strmiids.lib)
 if (MSVC)
#  message (STATUS "MSVC_VERSION: ${MSVC_VERSION}")
  if (MSVC_VERSION GREATER 1600)
#  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_link_libraries (${PROJECT_NAME}_Decoder vcruntime${LIB_FILE_SUFFIX}.lib ucrt${LIB_FILE_SUFFIX}.lib)
  endif ()
 endif (MSVC)
 target_link_libraries (${PROJECT_NAME}_Decoder ${ACE_LIBRARY})

# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavcodec/avcodec.lib)
# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavformat/avformat.lib)
# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavutil/avutil.lib)
# target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libswscale/swscale.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavcodec/libavcodec.a)
 target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavformat/libavformat.a)
 target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavutil/libavutil.a)
 target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libswscale/libswscale.a)

 target_link_libraries (${PROJECT_NAME}_Decoder Common)
endif ()

set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES DEFINE_SYMBOL STREAM_DEC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Decoder)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Decoder
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (${PROJECT_NAME}_Decoder_LIB_HDRS
     stream_dec_avi_decoder.h
     stream_dec_avi_decoder.inl
     stream_dec_avi_encoder.h
     stream_dec_avi_encoder.inl
     stream_dec_avi_parser.h
     stream_dec_avi_parser_driver.h
     stream_dec_common.h
     stream_dec_defines.h
#     stream_dec_exports.h
     stream_dec_h264_nal_bisector.h
     stream_dec_h264_nal_decoder.h
     stream_dec_h264_nal_decoder.inl
     stream_dec_libav_converter.h
     stream_dec_libav_converter.inl
     stream_dec_libav_decoder.h
     stream_dec_libav_decoder.inl
     stream_dec_mpeg_ts_decoder.h
     stream_dec_mpeg_ts_decoder.inl
     stream_dec_opencv_decoder.h
     stream_dec_opencv_decoder.inl
     stream_dec_riff_scanner.h
     stream_dec_tools.h
     stream_dec_zip_decoder.h
     stream_dec_zip_decoder.inl)
if (UNIX)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS ${ACEStream_Decoder_LIB_HDRS}
      stream_dec_sox_effect.h
      stream_dec_sox_effect.inl)
endif (UNIX)

install (FILES
         ${PROJECT_NAME}_Decoder_LIB_HDRS
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Decoder
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
