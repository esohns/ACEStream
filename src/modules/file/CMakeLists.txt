include (ffmpeg)
include (ImageMagick)

##########################################

add_library (${PROJECT_NAME}_File STATIC "")

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (FFMPEG_SUPPORT)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             "$<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>")
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             "$<BUILD_INTERFACE:${ImageMagick_INCLUDE_DIRS}>")
 target_compile_options (${PROJECT_NAME}_File PUBLIC
                         ${MAGICK_CXX_FLAGS})
 target_link_options (${PROJECT_NAME}_File PUBLIC
                      ${MAGICK_LD_FLAGS})
endif (IMAGEMAGICK_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/${BUILD_PATH_SUFFIX}>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
endif ()
target_include_directories (${PROJECT_NAME}_File PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_File PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)

############## next target ################
set (source_files
     stream_file_defines.h
     stream_file_source.cpp
     stream_file_source.h
     stream_file_source.inl
     stream_file_sink.cpp
     stream_file_sink.h
     stream_file_sink.inl)
if (IMAGEMAGICK_SUPPORT)
 set (source_files
      ${source_files}
      stream_file_imagemagick_source.cpp
      stream_file_imagemagick_source.h
      stream_file_imagemagick_source.inl)
endif (IMAGEMAGICK_SUPPORT)

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
target_sources (${PROJECT_NAME}_File
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_File
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_File PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_File Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_File CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_File ${PROJECT_NAME})

if (IMAGEMAGICK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_File PUBLIC ${ImageMagick_LIBRARIES})
endif (IMAGEMAGICK_SUPPORT)

target_link_libraries (${PROJECT_NAME}_File PUBLIC ${ACE_LIBRARY})

target_link_libraries (${PROJECT_NAME}_File PUBLIC Common)

target_link_libraries (${PROJECT_NAME}_File PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_File PROPERTIES DEFINE_SYMBOL STREAM_FILE_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_File PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_File
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_file_defines.h
#         stream_file_exports.h
         stream_file_source.h
         stream_file_source.inl
         stream_file_sink.h
         stream_file_sink.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
if (IMAGEMAGICK_SUPPORT)
 install (FILES
          stream_file_imagemagick_source.h
          stream_file_imagemagick_source.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif (IMAGEMAGICK_SUPPORT)
install (TARGETS ${PROJECT_NAME}_File
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
