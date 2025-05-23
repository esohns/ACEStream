if (UNIX)
 include (OS) # *TODO*: why is this necessary ?
elseif (WIN32)
 include (directxsdk)
endif ()
include (ffmpeg)
include (ImageMagick)
include (libnoise)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../lib/scripts")
if (WIN32)
 include (DirectXBaseClasses)
endif (WIN32)
include (sox)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (deepspeech)
include (faad)
include (festival)
include (mpg123)
include (opencv)
include (whisper_cpp)
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

if (WIN32 AND DIRECTXSDK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${DIRECTXSDK_INCLUDE_DIR}>)
endif (WIN32 AND DIRECTXSDK_SUPPORT)
if (FFMPEG_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>)
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             "$<BUILD_INTERFACE:${ImageMagick_INCLUDE_DIRS}>")
 target_compile_options (${PROJECT_NAME}_Decoder PUBLIC
                         ${MAGICK_CXX_FLAGS})
 target_link_options (${PROJECT_NAME}_Decoder PUBLIC
                      ${MAGICK_LD_FLAGS})
endif (IMAGEMAGICK_SUPPORT)

if (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${DS_BASECLASSES_INCLUDE_DIRS}>)
endif (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)
if (SOX_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${SOX_INCLUDE_DIRS}>)
endif (SOX_SUPPORT)

if (FAAD_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${faad_INCLUDE_DIRS}>)
endif (FAAD_SUPPORT)
if (DEEPSPEECH_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${DEEPSPEECH_INCLUDE_DIRS}>)
endif (DEEPSPEECH_SUPPORT)
if (FESTIVAL_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             "$<BUILD_INTERFACE:${FESTIVAL_INCLUDE_DIRS}>")
endif (FESTIVAL_SUPPORT)
if (FLITE_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             "$<BUILD_INTERFACE:${FLITE_INCLUDE_DIRS}>")
endif (FLITE_SUPPORT)
if (LIBNOISE_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${LIBNOISE_INCLUDE_DIRS}>)
endif (LIBNOISE_SUPPORT)
if (MPG123_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             $<BUILD_INTERFACE:${MPG123_INCLUDE_DIRS}>)
endif (MPG123_SUPPORT)
if (OPENCV_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             "$<BUILD_INTERFACE:${OPENCV_INCLUDE_DIRS}>")
endif (OPENCV_SUPPORT)
if (WHISPERCPP_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             "$<BUILD_INTERFACE:${WHISPERCPP_INCLUDE_DIRS}>")
endif (WHISPERCPP_SUPPORT)
if (ZLIB_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Decoder PUBLIC
                             "$<BUILD_INTERFACE:${ZLIB_INCLUDE_DIRS}>")
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
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../${BUILD_PATH_SUFFIX}>)
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
     stream_dec_noise_tools.cpp
     stream_dec_noise_tools.h
     stream_dec_noise_tools.inl
     stream_dec_rgb24_hflip.cpp
     stream_dec_rgb24_hflip.h
     stream_dec_rgb24_hflip.inl
     stream_dec_riff_common.h
     stream_dec_riff_scanner.cpp
     stream_dec_riff_scanner.h
     stream_dec_tools.cpp
     stream_dec_tools.h
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
if (FAAD_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_faad_decoder.cpp
      stream_dec_faad_decoder.h
      stream_dec_faad_decoder.inl)
endif (FAAD_SUPPORT)
if (FFMPEG_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_libav_audio_decoder.cpp
      stream_dec_libav_audio_decoder.h
      stream_dec_libav_audio_decoder.inl
      stream_dec_libav_converter.cpp
      stream_dec_libav_converter.h
      stream_dec_libav_converter.inl
      stream_dec_libav_decoder.cpp
      stream_dec_libav_decoder.h
      stream_dec_libav_decoder.inl
      stream_dec_libav_encoder.cpp
      stream_dec_libav_encoder.h
      stream_dec_libav_encoder.inl
      stream_dec_libav_filter.cpp
      stream_dec_libav_filter.h
      stream_dec_libav_filter.inl
      stream_dec_libav_hw_decoder.cpp
      stream_dec_libav_hw_decoder.h
      stream_dec_libav_hw_decoder.inl
      stream_dec_libav_img_decoder.cpp
      stream_dec_libav_img_decoder.h
      stream_dec_libav_img_decoder.inl
      stream_dec_libav_source.cpp
      stream_dec_libav_source.h
      stream_dec_libav_source.inl)
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
      stream_dec_sox_effect.inl
      stream_dec_sox_resampler.cpp
      stream_dec_sox_resampler.h
      stream_dec_sox_resampler.inl)
endif (SOX_SUPPORT)
if (DEEPSPEECH_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_deepspeech_decoder.cpp
      stream_dec_deepspeech_decoder.h
      stream_dec_deepspeech_decoder.inl)
endif (DEEPSPEECH_SUPPORT)
if (FESTIVAL_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_festival_decoder.cpp
      stream_dec_festival_decoder.h
      stream_dec_festival_decoder.inl)
endif (FESTIVAL_SUPPORT)
if (FLITE_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_flite_decoder.cpp
      stream_dec_flite_decoder.h
      stream_dec_flite_decoder.inl)
endif (FLITE_SUPPORT)
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
      stream_dec_opencv_qr_decoder.cpp
      stream_dec_opencv_qr_decoder.h
      stream_dec_opencv_qr_decoder.inl)
endif (OPENCV_SUPPORT)
if (WHISPERCPP_SUPPORT)
 set (source_files
      ${source_files}
      stream_dec_whisper_decoder.cpp
      stream_dec_whisper_decoder.h
      stream_dec_whisper_decoder.inl)
endif (WHISPERCPP_SUPPORT)
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
#                PRIVATE stdafx.cpp
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

if (FAAD_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${faad_LIBRARIES})
endif (FAAD_SUPPORT)
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
# target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${DEEPSPEECH_LIBRARIES})
endif (DEEPSPEECH_SUPPORT)
if (FESTIVAL_SUPPORT)
# target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${FESTIVAL_LIBRARIES})
endif (FESTIVAL_SUPPORT)
if (FLITE_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${FLITE_LIBRARIES})
endif (FLITE_SUPPORT)
if (LIBNOISE_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${LIBNOISE_LIB})
endif (LIBNOISE_SUPPORT)
if (MPG123_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${MPG123_LIBRARIES})
endif (MPG123_SUPPORT)
if (WHISPERCPP_SUPPORT)
# target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${WHISPERCPP_LIBRARIES})
endif (WHISPERCPP_SUPPORT)
if (ZLIB_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${ZLIB_LIBRARIES})
endif (ZLIB_SUPPORT)
if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PKG_ALSA_LIBRARIES})
elseif (WIN32)
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
#target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PROJECT_NAME}_HTML)
target_link_libraries (${PROJECT_NAME}_Decoder PUBLIC ${PROJECT_NAME}_MediaFramework)

if (WIN32)
 if (FESTIVAL_SUPPORT)
  # *WARNING*: linking libFestival.lib requires "/FORCE:MULTIPLE"
#  target_link_options (${PROJECT_NAME}_Decoder PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
 endif (FESTIVAL_SUPPORT)
endif (WIN32)

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
     stream_dec_noise_tools.h
     stream_dec_noise_tools.inl
     stream_dec_rgb24_hflip.h
     stream_dec_rgb24_hflip.inl
     stream_dec_riff_common.h
     stream_dec_riff_scanner.h
     stream_dec_tools.h
     stream_dec_wav_encoder.h
     stream_dec_wav_encoder.inl)
if (WIN32)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_avi_encoder.h
      stream_dec_avi_encoder.inl)
endif (WIN32)
if (FAAD_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_faad_decoder.h
      stream_dec_faad_decoder.inl)
endif (FAAD_SUPPORT)
if (FFMPEG_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_libav_audio_decoder.h
      stream_dec_libav_audio_decoder.inl
      stream_dec_libav_converter.h
      stream_dec_libav_converter.inl
      stream_dec_libav_decoder.h
      stream_dec_libav_decoder.inl
      stream_dec_libav_encoder.h
      stream_dec_libav_encoder.inl
      stream_dec_libav_filter.h
      stream_dec_libav_filter.inl
      stream_dec_libav_hw_decoder.h
      stream_dec_libav_hw_decoder.inl
      stream_dec_libav_img_decoder.h
      stream_dec_libav_img_decoder.inl
      stream_dec_libav_source.h
      stream_dec_libav_source.inl)
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
if (FESTIVAL_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_festival_decoder.h
      stream_dec_festival_decoder.inl)
endif (FESTIVAL_SUPPORT)
if (FLITE_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_flite_decoder.h
      stream_dec_flite_decoder.inl)
endif (FLITE_SUPPORT)
if (MPG123_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_mp3_decoder.h
      stream_dec_mp3_decoder.inl)
endif (MPG123_SUPPORT)
if (OPENCV_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_opencv_qr_decoder.h
      stream_dec_opencv_qr_decoder.inl)
endif (OPENCV_SUPPORT)
if (SOX_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_sox_effect.h
      stream_dec_sox_effect.inl
      stream_dec_sox_resampler.h
      stream_dec_sox_resampler.inl)
endif (SOX_SUPPORT)
if (WHISPERCPP_SUPPORT)
 set (${PROJECT_NAME}_Decoder_LIB_HDRS
      ${${PROJECT_NAME}_Decoder_LIB_HDRS}
      stream_dec_whisper_decoder.h
      stream_dec_whisper_decoder.inl)
endif (WHISPERCPP_SUPPORT)
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
