set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (libcamera)

if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)
 pkg_check_modules (PKG_V4L REQUIRED libv4l2)
endif (UNIX)

if (UNIX)
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DD3D_DEBUG_INFO)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif ()

add_library (${PROJECT_NAME}_Device STATIC "")

if (LIBCAMERA_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${LIBCAMERA_INCLUDE_DIRS}>)
endif (LIBCAMERA_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${PKG_ALSA_INCLUDE_DIRS}>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${PKG_V4L_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vis>)
elseif (WIN32)
 if (DEFINED ENV{DXSDK_DIR})
  target_include_directories (${PROJECT_NAME}_Device PUBLIC
                              $<BUILD_INTERFACE:$ENV{DXSDK_DIR}/Include>)
 else ()
#  set (DXSDK_DIR
#       "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)/Include")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
##      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectX SDK directory: \"${DXSDK_DIR}\"")
  target_include_directories (${PROJECT_NAME}_Device PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/DXSDK/Include>)
 endif (DEFINED ENV{DXSDK_DIR})
# set(_PF86 "ProgramFiles(x86)")
 cmake_policy (SET CMP0053 OLD) # Simplify variable reference and escape sequence evaluation
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "$ENV{LIB_ROOT}/DShowBaseClasses")
#      "$ENV{PROGRAMFILES}/Microsoft Platform SDK/Samples/Multimedia/DirectShow/BaseClasses")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vis>)
endif ()
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dec>)

############## next target ################
unset (source_files)
set (source_files
#     stream_dev_exports.h
     stream_dev_common.h
     stream_dev_defines.h
     stream_dev_tools.cpp
     stream_dev_tools.h
     stream_dev_tools.inl)
if (LIBCAMERA_SUPPORT)
 set (source_files
      ${source_files}
      stream_dev_cam_source_libcamera.cpp
      stream_dev_cam_source_libcamera.h
      stream_dev_cam_source_libcamera.inl)
endif (LIBCAMERA_SUPPORT)
if (WIN32)
 set (source_files
      ${source_files}
      stream_dev_directshow_tools.cpp
      stream_dev_directshow_tools.h
      stream_dev_mediafoundation_tools.cpp
      stream_dev_mediafoundation_tools.h
      stream_dev_cam_source_directshow.cpp
      stream_dev_cam_source_directshow.h
      stream_dev_cam_source_directshow.inl
      stream_dev_cam_source_mediafoundation.cpp
      stream_dev_cam_source_mediafoundation.h
      stream_dev_cam_source_mediafoundation.inl
      stream_dev_mic_source_directshow.cpp
      stream_dev_mic_source_directshow.h
      stream_dev_mic_source_directshow.inl
      stream_dev_mic_source_mediafoundation.cpp
      stream_dev_mic_source_mediafoundation.h
      stream_dev_mic_source_mediafoundation.inl
      stream_dev_mic_source_wasapi.cpp
      stream_dev_mic_source_wasapi.h
      stream_dev_mic_source_wasapi.inl
      stream_dev_mic_source_wavein.cpp
      stream_dev_mic_source_wavein.h
      stream_dev_mic_source_wavein.inl
      stream_dev_target_wavout.cpp
      stream_dev_target_wavout.h
      stream_dev_target_wavout.inl)
elseif (UNIX)
 set (source_files
      ${source_files}
      stream_dev_cam_source_v4l.cpp
      stream_dev_cam_source_v4l.h
      stream_dev_cam_source_v4l.inl
      stream_dev_mic_source_alsa.cpp
      stream_dev_mic_source_alsa.h
      stream_dev_mic_source_alsa.inl
      stream_dev_target_alsa.cpp
      stream_dev_target_alsa.h
      stream_dev_target_alsa.inl)
endif ()

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
message (STATUS "absolute_source_files: ${absolute_source_files}")
target_sources (${PROJECT_NAME}_Device
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Device
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Device PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Device Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_Device CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_Device ${PROJECT_NAME})

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${PKG_LIBALSA_LIBRARIES})

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${PKG_LIBV4L2_LIBRARIES})
 if (LIBCAMERA_SUPPORT)
  link_directories (/usr/local/lib64)
  target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${LIBCAMERA_LIBRARIES})
 endif (LIBCAMERA_SUPPORT)
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC Avrt.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC d3d9.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC dmoguids.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC dxva2.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC mf.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC mfreadwrite.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC shlwapi.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC strmiids.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC winmm.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC Wmcodecdspuuid.lib)

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/${CMAKE_BUILD_TYPE}/BaseClasses${LIB_FILE_SUFFIX}.lib)

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${ACE_LIBRARY})

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC Common)
endif ()
target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${PROJECT_NAME}_Decoder)

#set_target_properties (${PROJECT_NAME}_Device PROPERTIES DEFINE_SYMBOL STREAM_DEV_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Device PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
#set_target_properties (${PROJECT_NAME}_Device PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${PROJECT_NAME}_Device)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Device
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
#          stream_dev_exports.h
         stream_dev_common.h
         stream_dev_defines.h
         stream_dev_tools.h
         stream_dev_tools.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
if (LIBCAMERA_SUPPORT)
 install (FILES
          stream_dev_cam_source_libcamera.h
          stream_dev_cam_source_libcamera.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif (LIBCAMERA_SUPPORT)
if (UNIX)
 install (FILES
          stream_dev_cam_source_v4l.h
          stream_dev_cam_source_v4l.inl
          stream_dev_mic_source_alsa.h
          stream_dev_mic_source_alsa.inl
          stream_dev_target_alsa.h
          stream_dev_target_alsa.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
elseif (WIN32)
 install (FILES
          stream_dev_directshow_tools.h
          stream_dev_mediafoundation_tools.h
          stream_dev_cam_source_directshow.h
          stream_dev_cam_source_directshow.inl
          stream_dev_cam_source_mediafoundation.h
          stream_dev_cam_source_mediafoundation.inl
          stream_dev_mic_source_directshow.h
          stream_dev_mic_source_directshow.inl
          stream_dev_mic_source_wasapi.h
          stream_dev_mic_source_wasapi.inl
          stream_dev_mic_source_wavein.h
          stream_dev_mic_source_wavein.inl
          stream_dev_mic_source_mediafoundation.h
          stream_dev_mic_source_mediafoundation.inl
          stream_dev_target_wavout.h
          stream_dev_target_wavout.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()
install (TARGETS ${PROJECT_NAME}_Device
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
