include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_LIBALSA REQUIRED alsa)
 pkg_check_modules (PKG_LIBFFMPEG REQUIRED libavcodec libavformat libswscale)
# pkg_check_modules (PKG_LIBSOX REQUIRED sox)
endif ()

if (UNIX)
 include_directories (${PKG_LIBFFMPEG_INCLUDE_DIRS})
# include_directories (${PKG_LIBSNDFILE_INCLUDE_DIRS})
# include_directories (${PKG_LIBSOX_INCLUDE_DIRS})
 include_directories (/usr/local/src/sox-code/src)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/image)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/ui)
elseif (WIN32)
 include_directories ("$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../zlib-1.2.8)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party)
endif ()
#message (STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../..)

############## next target ################
if (UNIX)
 set (${PROJECT_NAME}_Decoder_LIB_SRCS
      stdafx.cpp
      stdafx.h
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
      stream_dec_exports.h
      stream_dec_h264_nal_bisector.cpp
      stream_dec_h264_nal_bisector.h
      stream_dec_h264_nal_decoder.cpp
      stream_dec_h264_nal_decoder.h
      stream_dec_h264_nal_decoder.inl
      stream_dec_libav_decoder.cpp
      stream_dec_libav_decoder.h
      stream_dec_libav_decoder.inl
      stream_dec_mpeg_ts_decoder.cpp
      stream_dec_mpeg_ts_decoder.h
      stream_dec_mpeg_ts_decoder.inl
      stream_dec_riff_scanner.cpp
      stream_dec_riff_scanner.h
      stream_dec_sox_effect.cpp
      stream_dec_sox_effect.h
      stream_dec_sox_effect.inl
      stream_dec_tools.cpp
      stream_dec_tools.h
      stream_dec_zip_decoder.cpp
      stream_dec_zip_decoder.h
      stream_dec_zip_decoder.inl)
elseif (WIN32)
 set (${PROJECT_NAME}_Decoder_LIB_SRCS
      stdafx.cpp
      stdafx.h
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
      stream_dec_exports.h
      stream_dec_h264_nal_bisector.cpp
      stream_dec_h264_nal_bisector.h
      stream_dec_h264_nal_decoder.cpp
      stream_dec_h264_nal_decoder.h
      stream_dec_h264_nal_decoder.inl
      stream_dec_libav_decoder.cpp
      stream_dec_libav_decoder.h
      stream_dec_libav_decoder.inl
      stream_dec_mpeg_ts_decoder.cpp
      stream_dec_mpeg_ts_decoder.h
      stream_dec_mpeg_ts_decoder.inl
      stream_dec_riff_scanner.cpp
      stream_dec_riff_scanner.h
      stream_dec_tools.cpp
      stream_dec_tools.h
      stream_dec_zip_decoder.cpp
      stream_dec_zip_decoder.h
      stream_dec_zip_decoder.inl)
endif ()
set (${PROJECT_NAME}_Decoder_LIB_SCRIPTS
     scripts/avi_scanner.l
     scripts/avi_parser.y
     scripts/h264_nal_bisector.l)

if (WIN32)
 add_definitions (-D_USE_MATH_DEFINES)
endif ()

# *NOTE*: generate .def file and export all symbols on Windows
if (WIN32)
 set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
 source_group (scripts FILES ${PROJECT_NAME}_Decoder_LIB_SCRIPTS)
endif ()

add_library (${PROJECT_NAME}_Decoder SHARED
             ${ACEStream_Decoder_LIB_SRCS}
             ${ACEStream_Decoder_LIB_SCRIPTS})

add_dependencies (${PROJECT_NAME}_Decoder Common)
add_dependencies (${PROJECT_NAME}_Decoder ${PROJECT_NAME})

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_LIBALSA_LIBRARIES})
# target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_AVCODEC_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_LIBFFMPEG_LIBRARIES})
# target_link_libraries (${PROJECT_NAME}_Decoder ${PKG_LIBSOX_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Decoder /usr/local/lib/libsox.so)
elseif (WIN32)
# target_link_libraries (${PROJECT_NAME}_Decoder dxguid.lib)
# target_link_libraries (${PROJECT_NAME}_Decoder dxva2.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder strmiids.lib)
# if (MSVC)
#  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_link_libraries (${PROJECT_NAME}_Decoder vcruntime${LIB_FILE_SUFFIX}.lib ucrt${LIB_FILE_SUFFIX}.lib)
#  endif ()
# endif ()
 target_link_libraries (${PROJECT_NAME}_Decoder ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libavutil/avutil.lib)
 target_link_libraries (${PROJECT_NAME}_Decoder ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg/libswscale/swscale.lib)
endif ()
target_link_libraries (${PROJECT_NAME}_Decoder Common)

set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES DEFINE_SYMBOL STREAM_DEC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Decoder)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Decoder
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (UNIX)
 install (FILES
          stream_dec_avi_decoder.h
          stream_dec_avi_decoder.inl
          stream_dec_avi_encoder.h
          stream_dec_avi_encoder.inl
          stream_dec_avi_parser.h
          stream_dec_avi_parser_driver.h
          stream_dec_common.h
          stream_dec_defines.h
          stream_dec_exports.h
          stream_dec_h264_nal_bisector.h
          stream_dec_h264_nal_decoder.h
          stream_dec_h264_nal_decoder.inl
          stream_dec_libav_decoder.h
          stream_dec_libav_decoder.inl
          stream_dec_mpeg_ts_decoder.h
          stream_dec_mpeg_ts_decoder.inl
          stream_dec_riff_scanner.h
          stream_dec_sox_effect.h
          stream_dec_sox_effect.inl
          stream_dec_tools.h
          stream_dec_zip_decoder.h
          stream_dec_zip_decoder.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
elseif (WIN32)
 install (FILES
          stream_dec_avi_decoder.h
          stream_dec_avi_decoder.inl
          stream_dec_avi_encoder.h
          stream_dec_avi_encoder.inl
          stream_dec_avi_parser.h
          stream_dec_avi_parser_driver.h
          stream_dec_common.h
          stream_dec_defines.h
          stream_dec_exports.h
          stream_dec_h264_nal_decoder.h
          stream_dec_h264_nal_decoder.inl
          stream_dec_libav_decoder.h
          stream_dec_libav_decoder.inl
          stream_dec_mpeg_ts_decoder.h
          stream_dec_mpeg_ts_decoder.inl
          stream_dec_riff_scanner.h
          stream_dec_tools.h
          stream_dec_zip_decoder.h
          stream_dec_zip_decoder.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()
install (TARGETS ${PROJECT_NAME}_Decoder
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
