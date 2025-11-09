if (WIN32)
 include (directxsdk)
endif (WIN32)

include (ffmpeg)

if (WIN32)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../lib/scripts")
 include (DirectXBaseClasses)
endif (WIN32)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (pipewire)
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

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (WIN32 AND DIRECTXSDK_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${DIRECTXSDK_INCLUDE_DIR}>)
endif (WIN32 AND DIRECTXSDK_SUPPORT)
if (FFMPEG_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>)
endif (FFMPEG_SUPPORT)
if (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${DS_BASECLASSES_INCLUDE_DIRS}>)
endif (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)

if (LIBPIPEWIRE_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             "$<BUILD_INTERFACE:${LIBPIPEWIRE_INCLUDE_DIRS}>")
endif (LIBPIPEWIRE_SUPPORT)
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
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/signal>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc>)
endif ()
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../${BUILD_PATH_SUFFIX}>)
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dec>)
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vis>)

############## next target ################
unset (source_files)
set (source_files
     stream_dev_common.h
     stream_dev_defines.h
     stream_dev_tools.cpp
     stream_dev_tools.h
     stream_dev_tools.inl)
if (LIBPIPEWIRE_SUPPORT)
 set (source_files
      ${source_files}
      stream_dev_cam_source_pipewire.cpp
      stream_dev_cam_source_pipewire.h
      stream_dev_cam_source_pipewire.inl
      stream_dev_mic_source_pipewire.cpp
      stream_dev_mic_source_pipewire.h
      stream_dev_mic_source_pipewire.inl
      stream_dev_target_pipewire.cpp
      stream_dev_target_pipewire.h
      stream_dev_target_pipewire.inl)
endif (LIBPIPEWIRE_SUPPORT)
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
      stream_dev_cam_source_directshow.cpp
      stream_dev_cam_source_directshow.h
      stream_dev_cam_source_directshow.inl
      stream_dev_cam_source_mediafoundation.cpp
      stream_dev_cam_source_mediafoundation.h
      stream_dev_cam_source_mediafoundation.inl
      stream_dev_cam_source_vfw.cpp
      stream_dev_cam_source_vfw.h
      stream_dev_cam_source_vfw.inl
      stream_dev_directshow_tools.cpp
      stream_dev_directshow_tools.h
      stream_dev_mediafoundation_tools.cpp
      stream_dev_mediafoundation_tools.h
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
      stream_dev_target_wasapi.cpp
      stream_dev_target_wasapi.h
      stream_dev_target_wasapi.inl
      stream_dev_target_wavout.cpp
      stream_dev_target_wavout.h
      stream_dev_target_wavout.inl
      stream_dev_target_xaudio2.cpp
      stream_dev_target_xaudio2.h
      stream_dev_target_xaudio2.inl
      stream_dev_vfw_tools.cpp
      stream_dev_vfw_tools.h)
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
#message (STATUS "absolute_source_files: ${absolute_source_files}")
target_sources (${PROJECT_NAME}_Device
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Device
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Device PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Device Common)
if (UNIX)
 add_dependencies (${PROJECT_NAME}_Device CommonTask)
elseif (WIN32)
 add_dependencies (${PROJECT_NAME}_Device CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_Device ${PROJECT_NAME})

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${PKG_LIBALSA_LIBRARIES})

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${PKG_LIBV4L2_LIBRARIES})
 if (LIBPIPEWIRE_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${LIBPIPEWIRE_LIBRARIES})
 endif (LIBPIPEWIRE_SUPPORT)
 if (LIBCAMERA_SUPPORT)
  link_directories (/usr/local/lib64)
  target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${LIBCAMERA_LIBRARIES})
 endif (LIBCAMERA_SUPPORT)

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC CommonTask)
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
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC Vfw32.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC winmm.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC Wmcodecdspuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Device PUBLIC Xaudio2.lib)

 if (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${DS_BASECLASSES_LIBRARIES})
 endif (WIN32 AND DIRECTSHOW_BASECLASSES_SUPPORT)

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${ACE_LIBRARY})

 target_link_libraries (${PROJECT_NAME}_Device PUBLIC Common)
endif ()
#target_link_libraries (${PROJECT_NAME}_Device PUBLIC ${PROJECT_NAME}_Decoder)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Device
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_dev_common.h
         stream_dev_defines.h
         stream_dev_tools.h
         stream_dev_tools.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
if (LIBPIPEWIRE_SUPPORT)
 install (FILES
          stream_dev_cam_source_pipewire.h
          stream_dev_cam_source_pipewire.inl
          stream_dev_mic_source_pipewire.h
          stream_dev_mic_source_pipewire.inl
          stream_dev_target_pipewire.h
          stream_dev_target_pipewire.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif (LIBPIPEWIRE_SUPPORT)
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
          stream_dev_cam_source_directshow.h
          stream_dev_cam_source_directshow.inl
          stream_dev_cam_source_mediafoundation.h
          stream_dev_cam_source_mediafoundation.inl
          stream_dev_cam_source_vfw.h
          stream_dev_cam_source_vfw.inl
          stream_dev_directshow_tools.h
          stream_dev_mediafoundation_tools.h
          stream_dev_mic_source_directshow.h
          stream_dev_mic_source_directshow.inl
          stream_dev_mic_source_wasapi.h
          stream_dev_mic_source_wasapi.inl
          stream_dev_mic_source_wavein.h
          stream_dev_mic_source_wavein.inl
          stream_dev_mic_source_mediafoundation.h
          stream_dev_mic_source_mediafoundation.inl
          stream_dev_target_wasapi.h
          stream_dev_target_wasapi.inl
          stream_dev_target_wavout.h
          stream_dev_target_wavout.inl
          stream_dev_target_xaudio2.h
          stream_dev_target_xaudio2.inl
          stream_dev_vfw_tools.h
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()
install (TARGETS ${PROJECT_NAME}_Device
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
