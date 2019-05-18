include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/scripts/PreUIFramework.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui//scripts/SetUIFramework.cmake)
if (GTK_SUPPORT)
 include (${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/scripts/SetGTKVersion.cmake)
endif (GTK_SUPPORT)

include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_LIBALSA REQUIRED alsa)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavutil)
 pkg_check_modules (PKG_LIBV4L2 REQUIRED libv4l2)
endif (UNIX)

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DD3D_DEBUG_INFO)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif ()

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
# message (STATUS "using ${CMAKE_CXX_COMPILER_ID} compiler options")
 add_compile_options (-std=c++11)
# add_compile_options (-fPIC)
# add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

add_library (${PROJECT_NAME}_Device STATIC "")

target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
#                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACE_TAO/ACE>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)

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
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ffmpeg/include>)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Device PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Device PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/msvc>)

 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
 target_include_directories (${PROJECT_NAME}_Device PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vis>)
endif ()
#target_include_directories (${PROJECT_NAME}_Device PUBLIC
#                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Device PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dec>)

############## next target ################
set (source_files
     stream_dev_exports.h
     stream_dev_common.h
     stream_dev_defines.h
     stream_dev_tools.cpp
     stream_dev_tools.h
     stream_dev_tools.inl)
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
      stream_dev_mic_source_mediafoundation.inl)
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

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
   PRIVATE ${abs_source_file}
   )
 #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Device
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

set_property (TARGET ${PROJECT_NAME}_Device PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Device Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_Device CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_Device ${PROJECT_NAME})

if (UNIX)
 target_link_libraries (${PROJECT_NAME}_Device ${PKG_LIBALSA_LIBRARIES})
 target_link_libraries (${PROJECT_NAME}_Device ${PKG_LIBV4L2_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${PROJECT_NAME}_Device d3d9.lib)
 target_link_libraries (${PROJECT_NAME}_Device dmoguids.lib)
 target_link_libraries (${PROJECT_NAME}_Device dxva2.lib)
 target_link_libraries (${PROJECT_NAME}_Device mf.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfplat.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfreadwrite.lib)
 target_link_libraries (${PROJECT_NAME}_Device mfuuid.lib)
 target_link_libraries (${PROJECT_NAME}_Device shlwapi.lib)
 target_link_libraries (${PROJECT_NAME}_Device strmiids.lib)
 target_link_libraries (${PROJECT_NAME}_Device Wmcodecdspuuid.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${PROJECT_NAME}_Device ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Debug/BaseClassesd.lib)
 else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${PROJECT_NAME}_Device ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Release/BaseClasses.lib)
 endif ()
 target_link_libraries (${PROJECT_NAME}_Device winmm.lib)

 target_link_libraries (${PROJECT_NAME}_Device ${ACE_LIBRARY})

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
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()
install (TARGETS ${PROJECT_NAME}_Device
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
