cmake_minimum_required (VERSION 2.8)

include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_LIBV4L2 REQUIRED libv4l2)
endif ()

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
elseif (WIN32)
 # *NOTE*: workaround
 set(_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dec)

############## next target ################
if (WIN32)
set (${PROJECT_NAME}_Device_LIB_SRCS
     stdafx.cpp
     stdafx.h
     stream_dev_exports.h
     stream_dev_common.h
     stream_dev_defines.h
     stream_dev_tools.cpp
     stream_dev_tools.h
     stream_dev_tools.inl
     stream_dev_cam_source_directshow.cpp
     stream_dev_cam_source_directshow.h
     stream_dev_cam_source_directshow.inl
     stream_dev_cam_source_mediafoundation.cpp
     stream_dev_cam_source_mediafoundation.h
     stream_dev_cam_source_mediafoundation.inl)
elseif (UNIX)
set (${PROJECT_NAME}_Device_LIB_SRCS
     stream_dev_exports.h
     stream_dev_common.h
     stream_dev_defines.h
     stream_dev_tools.cpp
     stream_dev_tools.h
     stream_dev_tools.inl
     stream_dev_cam_source_v4l.cpp
     stream_dev_cam_source_v4l.h
     stream_dev_cam_source_v4l.inl
     stdafx.cpp
     stdafx.h)
endif ()

add_library (${PROJECT_NAME}_Device SHARED ${ACEStream_Device_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Device Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_Device CommonERR)
endif ()
add_dependencies (${PROJECT_NAME}_Device ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Device ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Device shlwapi.lib)
 target_link_libraries (${PROJECT_NAME}_Device d3d9.lib)
 target_link_libraries (${PROJECT_NAME}_Device dxva2.lib)
 target_link_libraries (${PROJECT_NAME}_Device mf.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfreadwrite.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Device strmiids.lib)
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

set_target_properties (${PROJECT_NAME}_Device PROPERTIES DEFINE_SYMBOL STREAM_DEV_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Device PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Device PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Device)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Device
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (WIN32)
 install (FILES
          stream_dev_exports.h
          stream_dev_common.h
          stream_dev_defines.h
          stream_dev_tools.h
          stream_dev_tools.inl
          stream_dev_cam_source_directshow.h
          stream_dev_cam_source_directshow.inl
          stream_dev_cam_source_mediafoundation.h
          stream_dev_cam_source_mediafoundation.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
elseif (UNIX)
 install (FILES
          stream_dev_exports.h
          stream_dev_common.h
          stream_dev_defines.h
          stream_dev_tools.h
          stream_dev_tools.inl
          stream_dev_cam_source_v4l.h
          stream_dev_cam_source_v4l.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()
install (TARGETS ${PROJECT_NAME}_Device
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
