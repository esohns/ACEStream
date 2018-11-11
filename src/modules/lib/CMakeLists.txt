if (WIN32)
 if (${CMAKE_SYSTEM_VERSION} LESS 5.1)      # < Windows XP (see: <sdkddkver.h>)
 elseif (${CMAKE_SYSTEM_VERSION} LESS 6.0)  # < Windows Vista
  option (VMR9_SUPPORT "prefer DirectShow Video Mixing Renderer 9 (VMR-9) over VMR-7"
          OFF)
 else ()                                    # Some other Windows
 endif ()
endif (WIN32)

if (UNIX)
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DD3D_DEBUG_INFO)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif ()

add_library (${PROJECT_NAME}_MediaFramework STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui>)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
 else ()
  set (DXSDK_DIR
       "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
##      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
  target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                              $<BUILD_INTERFACE:${DXSDK_DIR}/Include>)
 endif (DEFINED ENV{DXSDK_DIR})
# set(_PF86 "ProgramFiles(x86)")
# cmake_policy (SET CMP0053 OLD) # Simplify variable reference and escape sequence evaluation
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "D:/projects/DShowBaseClasses")
#      "$ENV{PROGRAMFILES}/Microsoft Platform SDK/Samples/Multimedia/DirectShow/BaseClasses")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
##      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses>)

 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg>)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/math>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui>)

 target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc>)
endif ()
target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dec>)
target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dev>)
target_include_directories (${PROJECT_NAME}_MediaFramework PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

############## next target ################
set (source_files
     stream_lib_common.h
     stream_lib_defines.h
#     stream_lib_exports.h
     stream_lib_tools.cpp
     stream_lib_tools.h)
if (UNIX)
 set (source_files
      ${source_files}
      stream_lib_alsa_common.h
      stream_lib_v4l_common.h)
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
      stream_lib_directshow_source_filter.cpp
      stream_lib_directshow_source_filter.h
      stream_lib_directshow_source_filter.inl
      stream_lib_directshow_target.cpp
      stream_lib_directshow_target.h
      stream_lib_directshow_target.inl
      stream_lib_directshow_tools.cpp
      stream_lib_directshow_tools.h
      stream_lib_guids.h
      stream_lib_mediafoundation_callback.cpp
      stream_lib_mediafoundation_callback.h
      stream_lib_mediafoundation_callback.inl
      stream_lib_mediafoundation_mediasource.cpp
      stream_lib_mediafoundation_mediasource.h
      stream_lib_mediafoundation_mediasource.inl
      stream_lib_mediafoundation_message_base.cpp
      stream_lib_mediafoundation_message_base.h
      stream_lib_mediafoundation_message_base.inl
      stream_lib_mediafoundation_source.cpp
      stream_lib_mediafoundation_source.h
      stream_lib_mediafoundation_source.inl
#      stream_lib_mediafoundation_source_stream.cpp
#      stream_lib_mediafoundation_source_stream.h
#      stream_lib_mediafoundation_source_stream.inl
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

if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
 # *NOTE*: DirectShow uses 'DEBUG'
 add_definitions (-DDEBUG) # activate DirectShow debug logging
endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))

set_property (TARGET ${PROJECT_NAME}_MediaFramework PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_MediaFramework Common)
add_dependencies (${PROJECT_NAME}_MediaFramework ${PROJECT_NAME})

if (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_link_libraries (${PROJECT_NAME}_MediaFramework $ENV{DXSDK_DIR}/Lib/x86/D3dx9.lib)
 else ()
  target_link_libraries (${PROJECT_NAME}_MediaFramework ${DXSDK_DIR}/Lib/x86/D3dx9.lib)
 endif (DEFINED ENV{DXSDK_DIR})

 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  target_link_libraries (${PROJECT_NAME}_MediaFramework ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Debug/strmbasd.lib)
 else ()
  target_link_libraries (${PROJECT_NAME}_MediaFramework ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Release/strmbase.lib)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
# target_link_libraries (${PROJECT_NAME}_MediaFramework winmm.lib)

 target_link_libraries (${PROJECT_NAME}_MediaFramework ${ACE_LIBRARY})
endif (WIN32)

set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES DEFINE_SYMBOL STREAM_LIB_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_MediaFramework)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_MediaFramework
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
unset (source_files)
set (source_files
     stream_lib_common.h
     stream_lib_defines.h
#     stream_lib_exports.h
     stream_lib_tools.h)
if (UNIX)
 set (source_files
      ${source_files}
      stream_lib_alsa_common.h
      stream_lib_v4l_common.h)
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
      stream_lib_directshow_source_filter.h
      stream_lib_directshow_source_filter.inl
      stream_lib_directshow_target.h
      stream_lib_directshow_target.inl
      stream_lib_directshow_tools.h
      stream_lib_guids.h
      stream_lib_mediafoundation_callback.h
      stream_lib_mediafoundation_callback.inl
      stream_lib_mediafoundation_mediasource.h
      stream_lib_mediafoundation_mediasource.inl
      stream_lib_mediafoundation_message_base.h
      stream_lib_mediafoundation_message_base.inl
      stream_lib_mediafoundation_source.h
      stream_lib_mediafoundation_source.inl
      #      stream_lib_mediafoundation_source_stream.h
      #      stream_lib_mediafoundation_source_stream.inl
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
