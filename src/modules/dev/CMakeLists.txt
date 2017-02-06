include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_LIBALSA REQUIRED alsa)
 pkg_check_modules (PKG_LIBV4L2 REQUIRED libv4l2)
endif ()

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
elseif (WIN32)
 # *NOTE*: workaround
 set(_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dec)

############## next target ################
if (UNIX)
 set (${PROJECT_NAME}_Device_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_dev_exports.h
      stream_dev_common.h
      stream_dev_defines.h
      stream_dev_tools.cpp
      stream_dev_tools.h
      stream_dev_tools.inl
      stream_dev_cam_source_v4l.cpp
      stream_dev_cam_source_v4l.h
      stream_dev_cam_source_v4l.inl
      stream_dev_mic_source_alsa.cpp
      stream_dev_mic_source_alsa.h
      stream_dev_mic_source_alsa.inl
      stream_dev_target_alsa.cpp
      stream_dev_target_alsa.h
      stream_dev_target_alsa.inl)
elseif (WIN32)
 set (${PROJECT_NAME}_Device_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_dev_exports.h
      stream_dev_common.h
      stream_dev_defines.h
      stream_dev_directshow_tools.cpp
      stream_dev_directshow_tools.h
      stream_dev_mediafoundation_tools.cpp
      stream_dev_mediafoundation_tools.h
      stream_dev_tools.cpp
      stream_dev_tools.h
      stream_dev_tools.inl
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
      stream_directshow_allocator_base.cpp
      stream_directshow_allocator_base.h
      stream_directshow_allocator_base.inl
      stream_directshow_message_base.cpp
      stream_directshow_message_base.h
      stream_directshow_message_base.inl
      stream_mediafoundation_message_base.cpp
      stream_mediafoundation_message_base.h
      stream_mediafoundation_message_base.inl)
endif ()

add_library (${PROJECT_NAME}_Device SHARED ${ACEStream_Device_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Device Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_Device CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_Device ${PROJECT_NAME})

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Device ${PKG_LIBALSA_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Device ${PKG_LIBV4L2_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME}_Device ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Device d3d9.lib)
 target_link_libraries (${PROJECT_NAME}_Device dmoguids.lib)
 target_link_libraries (${PROJECT_NAME}_Device dxguid.lib)
 target_link_libraries (${PROJECT_NAME}_Device dxva2.lib)
 target_link_libraries (${PROJECT_NAME}_Device mf.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfreadwrite.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Device shlwapi.lib)
 target_link_libraries (${PROJECT_NAME}_Device strmiids.lib)
 target_link_libraries (${PROJECT_NAME}_Device Wmcodecdspuuid.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${PROJECT_NAME}_Device ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${PROJECT_NAME}_Device strmbase.lib)
 endif ()
 target_link_libraries (${PROJECT_NAME}_Device winmm.lib)
 target_link_libraries (${PROJECT_NAME}_Device Common)
endif ()
target_link_libraries (${PROJECT_NAME}_Device ${PROJECT_NAME}_Decoder)

set_target_properties (${PROJECT_NAME}_Device PROPERTIES DEFINE_SYMBOL STREAM_DEV_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Device PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Device PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Device)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Device
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (UNIX)
 install (FILES
          stream_dev_exports.h
          stream_dev_common.h
          stream_dev_defines.h
          stream_dev_tools.h
          stream_dev_tools.inl
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
          stream_dev_exports.h
          stream_dev_common.h
          stream_dev_defines.h
          stream_dev_directshow_tools.h
          stream_dev_mediafoundation_tools.h
          stream_dev_tools.h
          stream_dev_tools.inl
          stream_dev_cam_source_directshow.h
          stream_dev_cam_source_directshow.inl
          stream_dev_cam_source_mediafoundation.h
          stream_dev_cam_source_mediafoundation.inl
          stream_dev_mic_source_directshow.h
          stream_dev_mic_source_directshow.inl
          stream_dev_mic_source_mediafoundation.h
          stream_dev_mic_source_mediafoundation.inl
          stream_directshow_allocator_base.h
          stream_directshow_allocator_base.inl
          stream_directshow_message_base.h
          stream_directshow_message_base.inl
          stream_mediafoundation_message_base.h
          stream_mediafoundation_message_base.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()
install (TARGETS ${PROJECT_NAME}_Device
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
