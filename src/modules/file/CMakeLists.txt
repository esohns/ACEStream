set (IMAGEMAGICK_DEFAULT ON)
option (IMAGEMAGICK_SUPPORT "support ImageMagick" ${IMAGEMAGICK_DEFAULT})
if (IMAGEMAGICK_SUPPORT)
 if (UNIX)
  find_package (ImageMagick COMPONENTS MagickCore MagickWand)
 endif (UNIX)

 add_definitions (-DIMAGEMAGICK_SUPPORT)
endif (IMAGEMAGICK_SUPPORT)

##########################################

add_library (${PROJECT_NAME}_File STATIC "")

if (UNIX)
 if (IMAGEMAGICK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_File PUBLIC
                              $<BUILD_INTERFACE:${ImageMagick_INCLUDE_DIRS}>)
 endif (IMAGEMAGICK_SUPPORT)

 target_include_directories (${PROJECT_NAME}_File PUBLIC
#                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACE_TAO/ACE>)

 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/cmake>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
elseif (WIN32)
# if (DEFINED ENV{DXSDK_DIR})
#  target_include_directories (${PROJECT_NAME}_File PUBLIC
#                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
# else ()
#  set (DXSDK_DIR
#       "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)/Include")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
#  target_include_directories (${PROJECT_NAME}_File PUBLIC
#                              $<BUILD_INTERFACE:${DXSDK_DIR}>)
# endif (DEFINED ENV{DXSDK_DIR})

 if (IMAGEMAGICK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_File PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ImageMagick/include>)
 endif (IMAGEMAGICK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ffmpeg/include>)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_File PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_File PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACE_TAO/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/cmake>)
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
#     stream_file_exports.h
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

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
      )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_File
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

set_property (TARGET ${PROJECT_NAME}_File PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_File Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_File CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_File ${PROJECT_NAME})

if (IMAGEMAGICK_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_File ${ImageMagick_LIBRARIES})
endif (IMAGEMAGICK_SUPPORT)

target_link_libraries (${PROJECT_NAME}_File ${ACE_LIBRARY})

target_link_libraries (${PROJECT_NAME}_File Common)

target_link_libraries (${PROJECT_NAME}_File ${PROJECT_NAME})

set_target_properties (${PROJECT_NAME}_File PROPERTIES DEFINE_SYMBOL STREAM_FILE_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_File PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_File PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_File)

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

