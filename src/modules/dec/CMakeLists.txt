include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../lib/scripts")
include (sox)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (deepspeech)
include (mpg123)
include (opencv)
include (zlib)

##########################################

if (UNIX)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)
endif (UNIX)

add_library (${PROJECT_NAME}_Decoder STATIC "")

if (WIN32)
 add_definitions (-D_USE_MATH_DEFINES)

# set_property (TARGET ${PROJECT_NAME}_Decoder
#               PROPERTY VS_PACKAGE_REFERENCES "DeepSpeech_0.9.3")
# add_definitions (-DDEEPSPEECH_SUPPORT)
endif (WIN32)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (FFMPEG_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>)
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             "$<BUILD_INTERFACE:${ImageMagick_INCLUDE_DIRS}>")
endif (IMAGEMAGICK_SUPPORT)
if (SOX_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${SOX_INCLUDE_DIRS}>)
endif (SOX_SUPPORT)
if (DEEPSPEECH_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${DEEPSPEECH_INCLUDE_DIRS}>)
endif (DEEPSPEECH_SUPPORT)
if (MPG123_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${MPG123_INCLUDE_DIRS}>)
endif (MPG123_SUPPORT)
if (OPENCV_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${OPENCV_INCLUDE_DIRS}>)
endif (OPENCV_SUPPORT)
if (ZLIB_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${ZLIB_INCLUDE_DIRS}>)
endif (ZLIB_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${PKG_ALSA_INCLUDE_DIRS}>)
# include_directories (${PKG_LIBSNDFILE_INCLUDE_DIRS})

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
 else ()
  set (DXSDK_DIR
       "$ENV{LIB_ROOT}/DXSDK")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
  target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                              $<BUILD_INTERFACE:${DXSDK_DIR}/Include>)
 endif (DEFINED ENV{DXSDK_DIR})
# set(_PF86 "ProgramFiles(x86)")
# cmake_policy (SET CMP0053 OLD) # Simplify variable reference and escape sequence evaluation
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "$ENV{LIB_ROOT}/DShowBaseClasses")
#      "$ENV{PROGRAMFILES}/Microsoft Platform SDK/Samples/Multimedia/DirectShow/BaseClasses")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses>)

 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/3rd_party/msvc>)

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
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc>)
endif ()
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../include>)
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)

target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dev>)
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vis>)

############## next target ################
unset (source_files)
set (source_files
     stream_dec_avi_decoder.cpp
     stream_dec_avi_decoder.h
     stream_dec_avi_decoder.inl
     stream_dec_avi_parser.cpp
     stream_dec_avi_parser.h
     stream_dec_avi_parser_driver.cpp
     stream_dec_avi_parser_driver.h
     stream_dec_common.h
     stream_dec_defines.h
     stream_dec_h264_nal_bisector.cpp
     stream_dec_h264_nal_bisector.h
     stream_dec_h264_nal_decoder.cpp
     stream_dec_h264_nal_decoder.h
     stream_dec_h264_nal_decoder.inl
     stream_dec_mpeg_4_common.h
     stream_dec_mpeg_4_decoder.cpp
     stream_dec_mpeg_4_decoder.h
     stream_dec_mpeg_4_decoder.inl
     stream_dec_mpeg_ts_common.h
     stream_dec_mpeg_ts_decoder.cpp
     stream_dec_mpeg_ts_decoder.h
     stream_dec_mpeg_ts_decoder.inl
     stream_dec_noise_source.cpp
     stream_dec_noise_source.h
     stream_dec_noise_source.inl
     stream_dec_riff_common.h
     stream_dec_riff_scanner.cpp
     stream_dec_riff_scanner.h
     stream_dec_tools.cpp
     stream_dec_tools.h
     stream_dec_tools.inl
     stream_dec_wav_encoder.cpp
     stream_dec_wav_encoder.h
     stream_dec_wav_encoder.inl)
if (WIN32)
 set (source_files
      ${source_files}
      stream_dec_avi_encoder.cpp
      stream_dec_avi_encoder.h
      stream_dec_avi_encoder.inl)
endif (WIN32)
if (FFMPEG_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_libav_converter.cpp
      stream_dec_libav_converter.h
      stream_dec_libav_converter.inl
      stream_dec_libav_decoder.cpp
      stream_dec_libav_decoder.h
      stream_dec_libav_decoder.inl
      stream_dec_libav_encoder.cpp
      stream_dec_libav_encoder.h
      stream_dec_libav_encoder.inl
      stream_dec_libav_img_decoder.cpp
      stream_dec_libav_img_decoder.h
      stream_dec_libav_img_decoder.inl)
 if (UNIX)
  set (source_files
       ${source_files}
       stream_dec_avi_encoder.cpp
       stream_dec_avi_encoder.h
       stream_dec_avi_encoder.inl)
 endif (UNIX)
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_imagemagick_decoder.cpp
      stream_dec_imagemagick_decoder.h
      stream_dec_imagemagick_decoder.inl)
endif (IMAGEMAGICK_SUPPORT)
if (SOX_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_sox_effect.cpp
      stream_dec_sox_effect.h
      stream_dec_sox_effect.inl)
endif (SOX_SUPPORT)
if (DEEPSPEECH_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_deepspeech_decoder.cpp
      stream_dec_deepspeech_decoder.h
      stream_dec_deepspeech_decoder.inl)
endif (DEEPSPEECH_SUPPORT)
if (MPG123_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_mp3_decoder.cpp
      stream_dec_mp3_decoder.h
      stream_dec_mp3_decoder.inl)
endif (MPG123_SUPPORT)
if (OPENCV_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_opencv_decoder.cpp
      stream_dec_opencv_decoder.h
      stream_dec_opencv_decoder.inl)
endif (OPENCV_SUPPORT)
if (ZLIB_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_zip_decoder.cpp
      stream_dec_zip_decoder.h
      stream_dec_zip_decoder.inl)
endif (ZLIB_SUPPORT)

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
                PRIVATE ${absolute_source_files}
                INTERFACE ${absolute_script_files})
target_precompile_headers (${PROJECT_NAME}_Decoder
                           PUBLIC stdafx.h)

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
# set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Decoder_LIB_SCRIPTS)
endif (WIN32)

#set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES DEFINE_SYMBOL STREAM_DEC_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

set_property (TARGET ${PROJECT_NAME}_Decoder PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Decoder Common)
add_dependencies (${PROJECT_NAME}_Decoder ${PROJECT_NAME})

if (FFMPEG_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${ImageMagick_LIBRARIES})
endif (IMAGEMAGICK_SUPPORT)
if (SOX_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${SOX_LIBRARIES})
endif (SOX_SUPPORT)
if (OPENCV_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${OPENCV_LIBRARIES})
endif (OPENCV_SUPPORT)
if (DEEPSPEECH_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${DEEPSPEECH_LIBRARIES})
endif (DEEPSPEECH_SUPPORT)
if (MPG123_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${MPG123_LIBRARIES})
endif (MPG123_SUPPORT)
if (ZLIB_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${ZLIB_LIBRARIES})
endif (ZLIB_SUPPORT)
if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PKG_ALSA_LIBRARIES})
elseif (WIN32)
# set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
 set (DIRECTX_SDK_DIRECTORY
      $ENV{DXSDK_DIR})
# message (STATUS "DirectX SDK directory: \"${DIRECTX_SDK_DIRECTORY}\"")
# link_directories ("${DIRECTX_SDK_DIRECTORY}/Lib/x86")
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC "$ENV{LIB_ROOT}/DXSDK/Lib/x86/dxguid.lib")
# target_link_libraries (${PROJECT_NAME}_Decoder dxva2.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC strmiids.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC wmcodecdspuuid.lib)
 if (MSVC)
#  message (STATUS "MSVC_VERSION: ${MSVC_VERSION}")
  if (MSVC_VERSION GREATER 1600)
#  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC vcruntime${LIB_FILE_SUFFIX}.lib ucrt${LIB_FILE_SUFFIX}.lib)
  endif ()
 endif (MSVC)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC CommonImage)

target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PROJECT_NAME})

target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PROJECT_NAME}_HTML)
target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PROJECT_NAME}_MediaFramework)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Decoder
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (${PROJECT_NAME}_Decoder_LIB_HDRS
     stream_dec_avi_decoder.h
     stream_dec_avi_decoder.inl
     stream_dec_avi_parser.h
     stream_dec_avi_parser_driver.h
     stream_dec_common.h
     stream_dec_defines.h
     stream_dec_h264_nal_bisector.h
     stream_dec_h264_nal_decoder.h
     stream_dec_h264_nal_decoder.inl
     stream_dec_mpeg_4_common.h
     stream_dec_mpeg_4_decoder.h
     stream_dec_mpeg_4_decoder.inl
     stream_dec_mpeg_ts_common.h
     stream_dec_mpeg_ts_decoder.h
     stream_dec_mpeg_ts_decoder.inl
     stream_dec_noise_source.h
     stream_dec_noise_source.inl
     stream_dec_riff_common.h
     stream_dec_riff_scanner.h
     stream_dec_tools.h
     stream_dec_tools.inl
     stream_dec_wav_encoder.h
     stream_dec_wav_encoder.inl)
if (WIN32)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_avi_encoder.h
      stream_dec_avi_encoder.inl)
endif (WIN32)
if (FFMPEG_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
     stream_dec_libav_converter.h
     stream_dec_libav_converter.inl
     stream_dec_libav_decoder.h
     stream_dec_libav_decoder.inl
     stream_dec_libav_encoder.h
     stream_dec_libav_encoder.inl
     stream_dec_libav_img_decoder.h
     stream_dec_libav_img_decoder.inl)
 if (UNIX)
  set (${PROJECT_NAME}_Decoder_LIB_HDRS
       ${${PROJECT_NAME}_Decoder_LIB_HDRS}
       stream_dec_avi_encoder.h
       stream_dec_avi_encoder.inl)
 endif (UNIX)
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_imagemagick_decoder.h
      stream_dec_imagemagick_decoder.inl)
endif (IMAGEMAGICK_SUPPORT)
if (DEEPSPEECH_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_deepspeech_decoder.h
      stream_dec_deepspeech_decoder.inl)
endif (DEEPSPEECH_SUPPORT)
if (MPG123_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_mp3_decoder.h
      stream_dec_mp3_decoder.inl)
endif (MPG123_SUPPORT)
if (OPENCV_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_opencv_decoder.h
      stream_dec_opencv_decoder.inl)
endif (OPENCV_SUPPORT)
if (SOX_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_sox_effect.h
      stream_dec_sox_effect.inl)
endif (SOX_SUPPORT)
if (ZLIB_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_zip_decoder.h
      stream_dec_zip_decoder.inl)
endif (ZLIB_SUPPORT)

install (FILES
         ${${PROJECT_NAME}_Decoder_LIB_HDRS}
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Decoder
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
