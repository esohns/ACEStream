cmake_minimum_required (VERSION 2.8)

include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_LIBAVFORMAT REQUIRED libavformat)
endif ()

if (UNIX)
 include_directories (${PKG_LIBAVFORMAT_INCLUDE_DIRS})
elseif (WIN32)
 include_directories ("$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
set (${PROJECT_NAME}_Decoder_LIB_SRCS
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
     stream_dec_riff_scanner.cpp
     stream_dec_riff_scanner.h
     stream_dec_tools.cpp
     stream_dec_tools.h
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_Decoder SHARED ${ACEStream_Decoder_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Decoder Common)
add_dependencies (${PROJECT_NAME}_Decoder ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Decoder ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES DEFINE_SYMBOL STREAM_DEC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Decoder PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Decoder
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
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
         stream_dec_riff_scanner.h
         stream_dec_tools.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Decoder
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)