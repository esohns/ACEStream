include (ffmpeg)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../dev/scripts")
include (libcamera)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (sox)

if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)
 pkg_check_modules (PKG_V4L REQUIRED libv4l2)
endif (UNIX)

if (WIN32)
 if (${CMAKE_SYSTEM_VERSION} LESS 5.1)      # < Windows XP (see: <sdkddkver.h>)
 elseif (${CMAKE_SYSTEM_VERSION} LESS 6.0)  # < Windows Vista
  option (VMR9_SUPPORT "prefer DirectShow Video Mixing Renderer 9 (VMR-9) over VMR-7"
          OFF)
 else ()                                    # Some other Windows
 endif ()

 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DD3D_DEBUG_INFO)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif (WIN32)

add_library (${PROJECT_NAME}_MediaFramework STATIC "")

if (FFMPEG_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>)
endif (FFMPEG_SUPPORT)
if (LIBCAMERA_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${LIBCAMERA_INCLUDE_DIRS}>)
endif (LIBCAMERA_SUPPORT)
if (SOX_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${SOX_INCLUDE_DIRS}>)
endif (SOX_SUPPORT)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${PKG_ALSA_INCLUDE_DIRS}>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${PKG_V4L_INCLUDE_DIRS}>)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
 else ()
#  set (DXSDK_DIR
#       "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
##      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
  target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/DXSDK/Include>)
 endif (DEFINED ENV{DXSDK_DIR})
# set(_PF86 "ProgramFiles(x86)")
# cmake_policy (SET CMP0053 OLD) # Simplify variable reference and escape sequence evaluation
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "$ENV{LIB_ROOT}/DShowBaseClasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses>)
endif ()

target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${ACE_INCLUDE_DIR}>)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             "$<BUILD_INTERFACE:${Common_INCLUDE_DIRS}>")
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/input>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/image>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/task>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             "$<BUILD_INTERFACE:${Common_INCLUDE_DIRS}>")
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/input>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/image>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/task>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui>)
endif ()
include_directories (${CMAKE_BINARY_DIR})
target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/tree>)
target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../include>)
target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
if (UNIX)
 set (CMAKE_INCLUDE_CURRENT_DIR ON)
endif (UNIX)

############## next target ################
set (source_files
     stream_lib_common.h
     stream_lib_defines.h
     stream_lib_macros.h
     stream_lib_mediatype_converter.cpp
     stream_lib_mediatype_converter.h
     stream_lib_mediatype_converter.inl
     stream_lib_tagger.cpp
     stream_lib_tagger.h
     stream_lib_tagger.inl
     stream_lib_tools.cpp
     stream_lib_tools.h)
if (FFMPEG_SUPPORT)
 set (source_files
      ${source_files}
      stream_lib_ffmpeg_common.h)
endif (FFMPEG_SUPPORT)
if (LIBCAMERA_SUPPORT)
 set (source_files
      ${source_files}
      stream_lib_libcamera_common.h)
endif (LIBCAMERA_SUPPORT)
if (UNIX)
 set (source_files
      ${source_files}
      stream_lib_alsa_common.h
      stream_lib_alsa_defines.h
      stream_lib_alsa_tools.cpp
      stream_lib_alsa_tools.h
      stream_lib_v4l_common.h
      stream_lib_v4l_defines.h)
elseif (WIN32)
 set (source_files
      ${source_files}
      stream_lib_directdraw_common.h
      stream_lib_directdraw_tools.cpp
      stream_lib_directdraw_tools.h
      stream_lib_directshow_allocator.cpp
      stream_lib_directshow_allocator.h
      stream_lib_directshow_allocator.inl
      stream_lib_directshow_asynch_source_filter.cpp
      stream_lib_directshow_asynch_source_filter.h
      stream_lib_directshow_asynch_source_filter.inl
      stream_lib_directshow_common.h
      stream_lib_directshow_message_base.cpp
      stream_lib_directshow_message_base.h
      stream_lib_directshow_message_base.inl
      stream_lib_directshow_source.cpp
      stream_lib_directshow_source.h
      stream_lib_directshow_source.inl
      stream_lib_directshow_source_filter.cpp
      stream_lib_directshow_source_filter.h
      stream_lib_directshow_source_filter.inl
      stream_lib_directshow_target.cpp
      stream_lib_directshow_target.h
      stream_lib_directshow_target.inl
      stream_lib_directshow_tools.cpp
      stream_lib_directshow_tools.h
      stream_lib_directsound_common.h
      stream_lib_directsound_tools.cpp
      stream_lib_directsound_tools.h
      stream_lib_guids.h
      stream_lib_mediafoundation_callback.cpp
      stream_lib_mediafoundation_callback.h
      stream_lib_mediafoundation_callback.inl
      stream_lib_mediafoundation_common.h
      stream_lib_mediafoundation_mediasource.cpp
      stream_lib_mediafoundation_mediasource.h
      stream_lib_mediafoundation_mediasource.inl
      stream_lib_mediafoundation_message_base.cpp
      stream_lib_mediafoundation_message_base.h
      stream_lib_mediafoundation_message_base.inl
      stream_lib_mediafoundation_null.cpp
      stream_lib_mediafoundation_null.h
      stream_lib_mediafoundation_source.cpp
      stream_lib_mediafoundation_source.h
      stream_lib_mediafoundation_source.inl
      stream_lib_mediafoundation_target.cpp
      stream_lib_mediafoundation_target.h
      stream_lib_mediafoundation_target.inl
      stream_lib_mediafoundation_tools.cpp
      stream_lib_mediafoundation_tools.h)
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
target_sources (${PROJECT_NAME}_MediaFramework
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_MediaFramework
                           PUBLIC stdafx.h)

if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
 # *NOTE*: DirectShow uses 'DEBUG'
 add_definitions (-DDEBUG) # activate DirectShow debug logging
endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))

set_property (TARGET ${PROJECT_NAME}_MediaFramework PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_MediaFramework Common)
add_dependencies (${PROJECT_NAME}_MediaFramework ${PROJECT_NAME})

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_MediaFramework PUBLIC X11)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_link_libraries (${PROJECT_NAME}_MediaFramework PUBLIC $ENV{DXSDK_DIR}/Lib/${CMAKE_VS_PLATFORM_NAME}/D3dx9.lib)
 else ()
  target_link_libraries (${PROJECT_NAME}_MediaFramework PUBLIC $ENV{LIB_ROOT}/DXSDK/Lib/${CMAKE_VS_PLATFORM_NAME}/D3dx9.lib)
 endif (DEFINED ENV{DXSDK_DIR})
 target_link_libraries (${PROJECT_NAME}_MediaFramework PUBLIC dsound.lib)
 target_link_libraries (${PROJECT_NAME}_MediaFramework PUBLIC mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_MediaFramework PUBLIC ${DIRECTSHOW_BASECLASSES_DIRECTORY}/build/msvc/baseclasses/${CMAKE_BUILD_TYPE}/BaseClasses${LIB_FILE_SUFFIX}.lib)

 target_link_libraries (${PROJECT_NAME}_MediaFramework PUBLIC ${ACE_LIBRARY})
endif ()

#set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES DEFINE_SYMBOL STREAM_LIB_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_MediaFramework
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
unset (source_files)
set (source_files
     stream_lib_common.h
     stream_lib_defines.h
     stream_lib_macros.h
     stream_lib_mediatype_converter.h
     stream_lib_mediatype_converter.inl
     stream_lib_tagger.h
     stream_lib_tagger.inl
     stream_lib_tools.h)
if (FFMPEG_SUPPORT)
 set (source_files
      ${source_files}
      stream_lib_ffmpeg_common.h)
endif (FFMPEG_SUPPORT)
if (LIBCAMERA_SUPPORT)
 set (source_files
      ${source_files}
      stream_lib_libcamera_common.h)
endif (LIBCAMERA_SUPPORT)
if (UNIX)
 set (source_files
      ${source_files}
      stream_lib_alsa_common.h
      stream_lib_alsa_defines.h
      stream_lib_alsa_tools.h
      stream_lib_v4l_common.h
      stream_lib_v4l_defines.h)
elseif (WIN32)
 set (source_files
      ${source_files}
      stream_lib_directdraw_common.h
      stream_lib_directdraw_tools.h
      stream_lib_directshow_allocator.h
      stream_lib_directshow_allocator.inl
      stream_lib_directshow_asynch_source_filter.h
      stream_lib_directshow_asynch_source_filter.inl
      stream_lib_directshow_common.h
      stream_lib_directshow_message_base.h
      stream_lib_directshow_message_base.inl
      stream_lib_directshow_source.h
      stream_lib_directshow_source.inl
      stream_lib_directshow_source_filter.h
      stream_lib_directshow_source_filter.inl
      stream_lib_directshow_target.h
      stream_lib_directshow_target.inl
      stream_lib_directshow_tools.h
      stream_lib_directsound_common.h
      stream_lib_directsound_tools.h
      stream_lib_guids.h
      stream_lib_mediafoundation_callback.h
      stream_lib_mediafoundation_callback.inl
      stream_lib_mediafoundation_common.h
      stream_lib_mediafoundation_mediasource.h
      stream_lib_mediafoundation_mediasource.inl
      stream_lib_mediafoundation_message_base.h
      stream_lib_mediafoundation_message_base.inl
      stream_lib_mediafoundation_null.h
      stream_lib_mediafoundation_source.h
      stream_lib_mediafoundation_source.inl
      stream_lib_mediafoundation_target.h
      stream_lib_mediafoundation_target.inl
      stream_lib_mediafoundation_tools.h)
endif ()
install (FILES
         ${source_files}
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)

install (TARGETS ${PROJECT_NAME}_MediaFramework
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
